/*
 * mcp2515.c
 *
 *  Created on: Oct 21, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#include "mcp2515.h"

#include <stdbool.h>
#include <string.h>
#include "totem_nvic.h"
#include "totem_spi_dma.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "pinmap.h"

#include "trace.h"

#define MAX_TIMEOUT	25	// Max 255

/***   Global variables   ***/
extern xQueueHandle q_can_handle;

/***   Local variables   ***/
static const uint8_t full = MCP2515_STATUS_TX2RTS | MCP2515_STATUS_TX1RTS | MCP2515_STATUS_TX0RTS;
static traceString behaviourTrace;

/***   Local functions   ***/
static uint8_t mcp2515_read(char address);
static uint8_t mcp2515_readStatus(uint8_t *status);
static uint8_t mcp2515_bitModify(uint8_t address, uint8_t mask, uint8_t data);
static uint8_t mcp2515_write(uint8_t address, uint8_t data);
static uint8_t mcp2515_sendbuffer(uint8_t buffer, uint8_t priority);
static uint8_t setupClock(enum eCANBaudrate baudrate);
static void setupFilter(enum eCANAddressing address, uint32_t filter);
static void mcp2515_sleepAwaken();

#define BIT_SET(address, bits)		mcp2515_bitModify(address, bits, 0xFF)
#define BIT_CLEAR(address, bits)	mcp2515_bitModify(address, bits, 0x00)

/**
 * @brief 	Initialize MCP2515 CAN device
 * @param 	baud    Baudrate for the CAN transmission
 * @return	1 if successful, 0 otherwise
 */
uint8_t mcp2515_init(enum eCANBaudrate baud)
{

    spi_dma_setup();

    if (!mcp2515_reset())
    	return 0;

    /* set clock configuration */
    if (!setupClock(baud))
    	return 0;

    /* configure mask and filters for the rx frames */
    setupFilter(STANDARD_ADDRESSING, 0);

    /* set receive configuration */
    if (!mcp2515_write(MCP2515_RXB0CTRL, ((MCP2515_RXCTRL_BUF_FILTERS_OFF << MCP2515_RXCTRL_BUF_SHIFT) | MCP2515_RX0CTRL_BUKT)))
    	return 0;

    /* config /INT as interrupt notification for all RX buffer */
    if (!BIT_SET(MCP2515_CANINTE, (MCP2515_INT_WAKIE | MCP2515_INT_RX1IE | MCP2515_INT_RX0IE)))
    	return 0;

    /* reset all interrupts */
    if (!mcp2515_write(MCP2515_CANINTF, 0))
    	return 0;

    /* setup interrupt handling for /INT pin */
    set_gpio_callback(PORT_CAN_INT, PIN_CAN_INT, mcp2515_irq_handler, false, true);

    /* set and check opmode */
    if (!mcp2515_write(MCP2515_CANCTRL, (MCP2515_CTRL_REQOP_NORMAL << MCP2515_CTRL_REQOP_SHIFT)))
    	return 0;

    behaviourTrace = INIT_DRIVERTRACE("MCP2515");

    return 1;
}

/**
 * @brief 	Reset MCP device. After the reset the MCP2515 should
 * 			be in configuration mode.
 * @return 	1 if successful, 0 otherwise
 */
uint8_t mcp2515_reset()
{
    uint8_t txcnf[5];
    uint8_t rxstat[10];

    /* reset device and make sure it is in config mode */
    txcnf[0] = MCP2515_SPI_RESET;

    spi_dma_transfer(txcnf, NULL, 1);

    vTaskDelay(1 / portTICK_RATE_MS); /* Must wait until the device is resetted */

    txcnf[0] = MCP2515_SPI_READ;
    txcnf[1] = MCP2515_CANSTAT;
    txcnf[2] = 0;

    spi_dma_transfer(txcnf, rxstat, 3);
    spi_dma_waitRX();

    if ((rxstat[2] & 0xE0) != (MCP2515_CTRL_REQOP_CONFIG << MCP2515_CTRL_REQOP_SHIFT))
    {
        PRINT("Device reset failed\n");
        return 0;
    }
    else
    {
        PRINT("Config mode entered\n");
    }

    return 1;
}

/**
 * @brief 	MCP2515 interruption register handler. The function checks which
 * 			flag of the interruption register is set and cleans all.
 * @param 	frame	Pointer to structure where save the frame data from RX buffer
 */
void mcp2515_readBufferFromInterrupt(CAN_Frame_t *frame)
{
    uint8_t interrupts = mcp2515_read(MCP2515_CANINTF);

    // RX buffer 0
    if (interrupts & MCP2515_FLAG_RX0IF)
    {
        mcp2515_readBuffer(0, frame);
        interrupts &= ~MCP2515_FLAG_RX0IF;
    }

    else // Added else because the frame cannot support 2 data

    // RX buffer 1
    if (interrupts & MCP2515_FLAG_RX1IF)
    {
        mcp2515_readBuffer(1, frame);
        interrupts &= ~MCP2515_FLAG_RX1IF;
    }

    /* clear the received buffer interrupt bit */
    mcp2515_write(MCP2515_CANINTF, interrupts);
}

/**
 * @brief 	Read a buffer of 2 available from MCP2515 device
 * @param	rxbufno	Number of the buffer to receive the data from
 * @param 	frame	Pointer to structure where save the frame data from RX buffer
 */
uint8_t mcp2515_readBuffer(uint8_t rxbufno, CAN_Frame_t *frame)
{
	uint8_t txbuf[15], rxbuf[15];

	char rxbufstart = MCP2515_SPI_READ_RX_BUFFER;
	// First buffer 0, second buffer 1
    rxbufstart |= (rxbufno == 0) ? 0b000 : 0b100; // Check Figure 12-3 from MCP2515 datasheet

    txbuf[0] = rxbufstart;

    spi_dma_transfer(txbuf, rxbuf, 14);
    spi_dma_waitRX();

    memmove(frame->f, rxbuf + 1, 13);

    return 1;
}

/**
 * @brief 	Send the CAN frame over the MCP2515
 * @param 	frame	Pointer to structure to send over CAN
 * @return	1 if successful, 0 otherwise
 */
uint8_t mcp2515_send(CAN_Frame_t *frame)
{
	static uint8_t priority = 3;
	uint8_t status, insert, timeout_cnt = 0;

	status = 0xFF;

	while ((status & full) == full) {

		// check timeout
		if (timeout_cnt >= MAX_TIMEOUT_CNT)
		{
			vTracePrint(behaviourTrace, "ERROR! TX buf full -> Throw message away!");
			return 0;
		}

		if (!mcp2515_readStatus(&status))
			return 0;

		if (!(status & full)) {
			insert = 2;
			priority = 3;
			continue;
		}

		if ((status & full) == full){
			timeout_cnt++;
			continue;
		}

		if (status & MCP2515_STATUS_TX0RTS) {
			if (priority) {
				priority--;
			} else {
				status = full;
				continue;
			}
			if (status & MCP2515_STATUS_TX2RTS) {
				insert = 1;
			} else {
				insert = 2;
			}
			continue;
		}

		if (status & MCP2515_STATUS_TX1RTS) {
			insert = 0;
		} else {
			insert = 1;
		}
	}

	PRINT_DRIVERTRACE(behaviourTrace, "Send Chan: %d", insert);
	PRINT_DRIVERTRACE(behaviourTrace, "Prior: %d", priority);

    if (!mcp2515_txcanbuf(insert, frame))
    	return 0;

    if (!mcp2515_sendbuffer(insert, priority))
    	return 0;

    return 1;
}

/**
 * @brief	Enters or exits to/from sleep mode the MCP2515
 * @param	sleep	1 if enters, 0 if exists
 * @return	1 if successful, 0 otherwise
 */
uint8_t mcp2515_sleepMode(uint8_t sleep)
{
	uint8_t status, timeout = MAX_TIMEOUT;
	if (sleep) {
		mcp2515_write(MCP2515_CANINTF, 0);
		if (!mcp2515_write(MCP2515_CANCTRL, MCP2515_CTRL_REQOP_SLEEP << MCP2515_CTRL_REQOP_SHIFT))
			return 0;
		while (((status = mcp2515_read(MCP2515_CANSTAT)) >> MCP2515_CTRL_REQOP_SHIFT) != MCP2515_CTRL_REQOP_SLEEP){
		    vTaskDelay(1 / portTICK_RATE_MS);
		    timeout--;
		    if (!timeout)
		    	return 0;
		}
	    set_gpio_callback(PORT_CAN_INT, PIN_CAN_INT, mcp2515_sleepAwaken, false, true);
	} else {
		BIT_SET(MCP2515_CANINTF, MCP2515_FLAG_WAKIF);
		if (!BIT_SET(MCP2515_CANINTE, MCP2515_INT_WAKIE))
			return 0;
		if (!mcp2515_write(MCP2515_CANCTRL, MCP2515_CTRL_REQOP_NORMAL << MCP2515_CTRL_REQOP_SHIFT))
			return 0;
		while (((status = mcp2515_read(MCP2515_CANSTAT)) >> MCP2515_CTRL_REQOP_SHIFT) != MCP2515_CTRL_REQOP_NORMAL){
		    vTaskDelay(1 / portTICK_RATE_MS);
		    timeout--;
		    if (!timeout)
		    	return 0;
		}
		BIT_CLEAR(MCP2515_CANINTF, MCP2515_FLAG_WAKIF);
	}
	return 1;

}

/**
 * @brief 	Send a frame to a MCP2515 TX buffer
 * @param	txbufno	Number of the buffer where save the frame
 * @param 	frame	Pointer to structure to send over CAN
 * @return	1 if successful, 0 otherwise
 */
uint8_t mcp2515_txcanbuf(uint8_t txbufno, CAN_Frame_t *frame)
{
    uint8_t txbuf[15];

    if (txbufno > 2)
    	return 0;

    txbuf[0] = MCP2515_SPI_LOAD_TX_BUFFER | ((1 << txbufno) & 0x06); // 0x06 mask to protect overflow and take count of buffer 0
    memmove(txbuf + 1, frame->f, 13);
    spi_dma_transfer(txbuf, NULL, 14);

    return 1;
}

/**
 * @brief 	Get all the errors from the MCP2515
 * @param	errflags	Pointer to byte where save the error flag register
 * @param 	txerr		Pointer to byte where save the transmission error counter register
 * @param	rxerr		Pointer to byte where save the reception error counter register
 */
void mcp2515_get_errors(uint8_t *errflags, uint8_t *txerr, uint8_t *rxerr)
{
    *errflags = mcp2515_read(MCP2515_EFLG);
    *txerr    = mcp2515_read(MCP2515_TEC);
    *rxerr    = mcp2515_read(MCP2515_REC);
}

/**
 * @brief 	Interrupt function handler. It sends a message to the queue to inform other tasks
 */
void mcp2515_irq_handler(void)
{
    signed portBASE_TYPE pxHigherPriorityTaskWoken = pdFALSE;
    uint8_t loopCnt = 0;

    CAN_Queue_t can_data;
    can_data.dir = CAN_QUEUE_IN;

    while (xQueueSendFromISR(q_can_handle, (void *)&can_data, &pxHigherPriorityTaskWoken) ==
               pdFALSE &&
           loopCnt < 10)
    {
        loopCnt++;
        xQueueReset(q_can_handle);
    }

    portEND_SWITCHING_ISR(pxHigherPriorityTaskWoken);
}

/**
 * @brief 	Read a register from MCP2515
 * @param 	address	Register address
 * @return	1 if successful, 0 otherwise
 */
static uint8_t mcp2515_read(char address)
{
    uint8_t txbuf[3], rxbuf[3], data;

    txbuf[0] = MCP2515_SPI_READ;
    txbuf[1] = address;
    txbuf[2] = 0;

    spi_dma_transfer(txbuf, rxbuf, 3);
    spi_dma_waitRX();

    data = rxbuf[2];

    return data;
}

/**
 * @brief 	Read a register from MCP2515
 * @param 	status	Pointer to byte return by the read status spi command
 * @return	1 if successful, 0 otherwise
 */
static uint8_t mcp2515_readStatus(uint8_t *status)
{
    uint8_t txbuf[3], rxbuf[3];

    txbuf[0] = MCP2515_SPI_READ_STATUS;
    txbuf[1] = 0;
    txbuf[2] = 0;

    spi_dma_transfer(txbuf, rxbuf, 3);
    spi_dma_waitRX();

    *status = rxbuf[2];

    return 1;
}

/**
 * @brief 	Modify bits from a register of MCP2515
 * @param 	address	Register to change of MCP2515
 * @param 	mask	Mask to apply between register and data
 * @param 	data	Data to write in the register
 * @return	1 if successful, 0 otherwise
 */
static uint8_t mcp2515_bitModify(uint8_t address, uint8_t mask, uint8_t data)
{
    uint8_t txbuf[4];

    txbuf[0] = MCP2515_SPI_BIT_MODIFY;
    txbuf[1] = address;
    txbuf[2] = mask;
    txbuf[3] = data;

    spi_dma_transfer(txbuf, NULL, 4);

    return 1;
}

/**
 * @brief 	Write a register of MCP2515
 * @param 	address	Register to write of MCP2515
 * @param 	data	Data to write in the register
 * @return	1 if successful, 0 otherwise
 */
static uint8_t mcp2515_write(uint8_t address, uint8_t data)
{
    uint8_t txbuf[3];

    txbuf[0] = MCP2515_SPI_WRITE;
    txbuf[1] = address;
    txbuf[2] = data;

    spi_dma_transfer(txbuf, NULL, 3);

    return 1;
}

/**
 * @brief 	Command the MCP2515 to send a TX buffer with a certain priority
 * @param 	txbufno		Number of the TX buffer to send over CAN
 * @param 	priority	Number of priority: 3 (highest) to 0 (lowest)
 * @return	1 if successful, 0 otherwise
 */
static uint8_t mcp2515_sendbuffer(uint8_t txbufno, uint8_t priority)
{
    uint8_t txbuf[3];

    if ((txbufno > 2) || (priority > 3))
    	return 0;

    txbuf[0] = MCP2515_SPI_WRITE;
    txbuf[1] = (((MCP2515_TXB0CTRL >> 4) + txbufno) << 4);
    txbuf[2] = (MCP2515_TXCTRL_RTS | (priority & 0x3));

    spi_dma_transfer(txbuf, NULL, 3);

    return 1;
}

/**
 * @brief 	Setup clock from MCP2515 to CAN transmissions
 * @param 	baudrate	Baudrate used in the CAN transmissions
 * @return	1 if successful, 0 otherwise
 */
static uint8_t setupClock(enum eCANBaudrate baudrate)
{
    uint8_t clk_conf1, clk_conf2, clk_conf3;

    switch (baudrate)
    {
        case CAN_BAUD_50KHZ:
            clk_conf1 = 7 << MCP2515_CNF_BRP_SHIFT;
            clk_conf2 = (MCP2515_CNF_BTL | (4 << MCP2515_CNF_PHSEG1_SHIFT));
            clk_conf3 = 2 << MCP2515_CNF_PHSEG2_SHIFT;
            break;
        case CAN_BAUD_100KHZ:
            clk_conf1 = 3 << MCP2515_CNF_BRP_SHIFT;
            clk_conf2 = (MCP2515_CNF_BTL | (4 << MCP2515_CNF_PHSEG1_SHIFT));
            clk_conf3 = 2 << MCP2515_CNF_PHSEG2_SHIFT;
            break;
        case CAN_BAUD_125KHZ:
            clk_conf1 = 1 << MCP2515_CNF_BRP_SHIFT;
            clk_conf2 = (MCP2515_CNF_BTL | (7 << MCP2515_CNF_PHSEG1_SHIFT));
            clk_conf3 = 5 << MCP2515_CNF_PHSEG2_SHIFT;
            break;
        case CAN_BAUD_250KHZ:
            clk_conf1 = 0x00;
            clk_conf2 = (MCP2515_CNF_BTL | (7 << MCP2515_CNF_PHSEG1_SHIFT));
            clk_conf3 = 5 << MCP2515_CNF_PHSEG2_SHIFT;
            break;
        case CAN_BAUD_500KHZ:
            /* Warning: possible problem with MCP2515 according to "Minimum Bus Idle Time" */
            clk_conf1 = 0x00;
            clk_conf2 = (MCP2515_CNF_BTL | (2 << MCP2515_CNF_PHSEG1_SHIFT));
            clk_conf3 = 2 << MCP2515_CNF_PHSEG2_SHIFT;
            break;
        case CAN_BAUD_1000KHZ:
            /* Impossible with 8MHz clock not enough time quantas available */
            break;
        default:
            clk_conf1 = 0x00; // syncronization jump width is 1 TQ
            clk_conf2 = MCP2515_CNF_BTL; // BTLMOD, PS2 length is determined by PHSEG22:PHSEG20 in CNF3
            clk_conf3 = 0x00; // Wake-up filter enabled
            break;
    }

    if (!mcp2515_write(MCP2515_CNF1, clk_conf1))
    	return 0;
    if (!mcp2515_write(MCP2515_CNF2, clk_conf2))
    	return 0;
    if (!mcp2515_write(MCP2515_CNF3, clk_conf3))
    	return 0;

    return 1;
}

/**
 * @brief 	Setup of mask and filter registers for the RXnBUF in the MCP2515 device.
 * @param 	address	Type of address used in CAN frames
 * @param 	filter	Filter to use in the RX data received
 */
static void setupFilter(enum eCANAddressing address, uint32_t filter)
{
    uint32_t mask = 0;

    if (address == EXTENDED_ADDRESSING || address == INVALID_ADDRESSING)
    {
        PRINT("[CAN setup_filter()] Extended Addressing is not supported");
        return; /* FIXME error condition */
    }

    switch (address)
    {
        case STANDARD_ADDRESSING:
            mask = 0x000007FF; // 11 bit standard address

            mcp2515_write(MCP2515_RXM0 | MCP2515_FILTMASK_SIDH, (uint8_t)((mask >> 3) & 0xff));
            mcp2515_write(MCP2515_RXM0 | MCP2515_FILTMASK_SIDL, (uint8_t)((mask << 5) & 0xe0));
            mcp2515_write(MCP2515_RXM0 | MCP2515_FILTMASK_EID8, (uint8_t)0);
            mcp2515_write(MCP2515_RXM0 | MCP2515_FILTMASK_EID0, (uint8_t)0);

            mcp2515_write(MCP2515_RXM1 | MCP2515_FILTMASK_SIDH, (uint8_t)((mask >> 3) & 0xff));
            mcp2515_write(MCP2515_RXM1 | MCP2515_FILTMASK_SIDL, (uint8_t)((mask << 5) & 0xe0));
            mcp2515_write(MCP2515_RXM1 | MCP2515_FILTMASK_EID8, (uint8_t)0);
            mcp2515_write(MCP2515_RXM1 | MCP2515_FILTMASK_EID0, (uint8_t)0);

            break;
        case STANDARD_ADDRESSING_DATA_FILTER:
            mask = 0x03ffffff;
            break;
        default:
            mask = 0x0;
            break;
    }

    /* If no filter is provided, accept everything */
    if (filter == 0)
    {
        for (int i = 0; i < 12; i++)
            mcp2515_write(MCP2515_RXF0 | (MCP2515_FILTMASK_SIDH + i), 0);
        for (int i = 0; i < 12; i++)
            mcp2515_write(MCP2515_RXF3 | (MCP2515_FILTMASK_SIDH + i), 0);
    }
}

static void mcp2515_sleepAwaken()
{
	PRINT_DRIVERTRACE(behaviourTrace, "Wake up", NULL);
	PRINT("Wake up\n");
	set_gpio_callback(PORT_CAN_INT, PIN_CAN_INT, mcp2515_irq_handler, false, true);
}
