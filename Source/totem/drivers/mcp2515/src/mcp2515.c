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

/***   Global variables   ***/
extern xQueueHandle q_can_handle;

/***   Local functions   ***/
static uint8_t mcp2515_read(char address);
static uint8_t mcp2515_readStatus(uint8_t *status);
static uint8_t mcp2515_write(uint8_t address, uint8_t data);
static uint8_t mcp2515_sendbuffer(uint8_t buffer, uint8_t priority);
static uint8_t setupClock(enum eCANBaudrate baudrate);
static void setupFilter(enum eCANAddressing address, uint32_t filter);

/**
 * @brief 	Initialize MCP2515 CAN device
 * @param 	baud    Baudrate for the CAN transmission
 * @return	1 if successful, 0 otherwise
 */
uint8_t mcp2515_init(enum eCANBaudrate baud)
{
	uint8_t caninte;

    CMU_ClockEnable(cmuClock_USART2, true);
    CMU_ClockEnable(cmuClock_GPIO, true);

    setup_spi_dma();

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
    caninte = 0; //mcp2515_read(MCP2515_CANINTE);
    if (!mcp2515_write(MCP2515_CANINTE, (caninte | MCP2515_INT_RX1IE | MCP2515_INT_RX0IE)))
    	return 0;

    /* reset all interrupts */
    if (!mcp2515_write(MCP2515_CANINTF, 0))
    	return 0;

    /* setup interrupt handling for /INT pin */
    set_gpio_callback(PORT_CAN_INT, PIN_CAN_INT, mcp2515_irq_handler, false, true);

    /* set and check opmode */
    if (!mcp2515_write(MCP2515_CANCTRL, (MCP2515_CTRL_REQOP_NORMAL << MCP2515_CTRL_REQOP_SHIFT)))
    	return 0;

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

    spi_dma_transfer(txcnf, rxstat, 1);

    vTaskDelay(100 / portTICK_RATE_MS); /* Must wait until the device is resetted */

    txcnf[0] = MCP2515_SPI_READ;
    txcnf[1] = MCP2515_CANSTAT;
    txcnf[2] = 0;

    spi_dma_transfer(txcnf, rxstat, 3);

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
	uint8_t txbuf[15];
	uint8_t rxbuf[15];

	char rxbufstart = MCP2515_SPI_READ_RX_BUFFER;
	// First buffer 0, second buffer 1
    rxbufstart |= (rxbufno == 0) ? 0b000 : 0b100; // Check Figure 12-3 from MCP2515 datasheet

    txbuf[0] = rxbufstart;

    spi_dma_transfer(txbuf, rxbuf, 14);
    memmove(frame->f, rxbuf + 1, 13);

    return 1;
}


uint8_t mcp2515_send(CAN_Frame_t *frame)
{
	static uint8_t priority = 3;
	uint8_t status, insert, timeout_cnt = 0;

	status = 0xFF;

	while ((status & 0x54) == 0x54) {

		// check timeout
		if (timeout_cnt >= MAX_TIMEOUT_CNT)
		{
			vTracePrint(NULL, "[CAN] ERROR! TX buf full -> Throw message away!");
			return 0;
		}

		if (!mcp2515_readStatus(&status))
			return 0;

		if (!(status & 0x54)) {
			insert = 2;
			priority = 3;
			PRINT("Envia por 2\n");
			continue;
		}

		if ((status & 0x54) == 0x54){
			timeout_cnt++;
			continue;
		}

		if (status & 0x04) {
			priority--;
			if (status & 0x40) {
				insert = 2;
				PRINT("Envia por 2\n");
			} else {
				insert = 1;
				PRINT("Envia por 1\n");
			}
			continue;
		}

		if (status & 0x10) {
			insert = 0;
			PRINT("Envia por 0\n");
		} else {
			insert = 1;
			PRINT("Envia por 1\n");
		}
	}

    mcp2515_txcanbuf(insert, frame);

    mcp2515_sendbuffer(insert, priority);

    return 1;
}

uint8_t mcp2515_txcanbuf(uint8_t txbufno, CAN_Frame_t *candata)
{
    uint8_t txbuf[15];

    if (txbufno > 2)
    	return 0;

    txbuf[0] = MCP2515_SPI_LOAD_TX_BUFFER | ((1 << txbufno) & 0x06); // 0x07 mask to protect overflow and buffer 0
    memmove(txbuf + 1, candata->f, 13);
    spi_dma_transfer(txbuf, NULL, 14);

    return 1;
}

void mcp2515_get_errors(uint8_t *errflags, uint8_t *txerr, uint8_t *rxerr)
{
    *errflags = mcp2515_read(MCP2515_EFLG);
    *txerr    = mcp2515_read(MCP2515_TEC);
    *rxerr    = mcp2515_read(MCP2515_REC);
}

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

static uint8_t mcp2515_read(char address)
{
    uint8_t txbuf[3];
    uint8_t rxbuf[5];
    uint8_t data = 0;

    txbuf[0] = MCP2515_SPI_READ;
    txbuf[1] = address;
    txbuf[2] = 0;

    spi_dma_transfer(txbuf, rxbuf, 3);
    data = rxbuf[2];

    return data;
}

static uint8_t mcp2515_readStatus(uint8_t *status)
{
    uint8_t txbuf[3];
    uint8_t rxbuf[3];

    txbuf[0] = MCP2515_SPI_READ_STATUS;
    txbuf[1] = 0;
    txbuf[2] = 0;

    spi_dma_transfer(txbuf, rxbuf, 3);
    *status = rxbuf[2];

    return 1;
}

static uint8_t mcp2515_write(uint8_t address, uint8_t data)
{
    uint8_t txbuf[5];
    uint8_t rxbuf[5];

    txbuf[0] = MCP2515_SPI_WRITE;
    txbuf[1] = address;
    txbuf[2] = data;

    spi_dma_transfer(txbuf, rxbuf, 3);

    return 1;
}

static uint8_t mcp2515_sendbuffer(uint8_t buffer, uint8_t priority)
{
    uint8_t txbuf[5];
    uint8_t rxbuf[5];

    if ((buffer > 2) || (priority > 3))
    	return 0;

    txbuf[0] = MCP2515_SPI_WRITE;
    txbuf[1] = (((MCP2515_TXB0CTRL >> 4) + buffer) << 4);
    txbuf[2] = (MCP2515_TXCTRL_RTS | (priority & 0x3));

    spi_dma_transfer(txbuf, rxbuf, 3);

    return 1;
}

/* Setup clock for CAN transmission
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
            clk_conf3 = 0x00;
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

/* setup of mask and filter registers for the RXBUF in the MCP25x device.
 * Asumptions/limitations:
 * - RX0B and RX1B have the same configuration, so they receive the same CAN frames.
 *   With this configuration it is possible to receive in RX1B while RX0B is still full.
 * - Extended addressing is currently not supported.
 * - PRE: MCP25x must be in CONF mode!
 * - STANDARD_ADDRESSING: mask and filter use bits [11:0]
 *   EXTENDEDADDRESSING: mask and filter use bits [29:0]
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
        case EXTENDED_ADDRESSING:
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
