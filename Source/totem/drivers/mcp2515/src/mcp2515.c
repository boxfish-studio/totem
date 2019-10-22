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

struct MCP2515_T
{
    USART_TypeDef *usart;
    int port;
};

static MCP2515_T mcp2515;

extern xQueueHandle q_can_handle;

static int mcp_write(uint8_t address, uint8_t data);
static int mcp_write_canrts(uint8_t buffer);
static uint8_t mcp_read(char address);
static int setupClock(enum eCANBaudrate baudrate);
static void setupFilter(enum eCANAddressing address, uint32_t filter);

MCP2515_T *mcp2515_init(enum eCANBaudrate baud)
{
	uint8_t caninte;

    mcp2515.usart = USART2;

    CMU_ClockEnable(cmuClock_USART2, true);
    CMU_ClockEnable(cmuClock_GPIO, true);

    setup_spi_dma();

    if (!mcp2515_reset())
    	return NULL;

    /* set clock configuration */
    if (!setupClock(baud))
    	return NULL;

    /* configure mask and filters for the rx frames */
    setupFilter(STANDARD_ADDRESSING, 0);

    /* set receive configuration */
    if (!mcp_write(MCP2515_RXB0CTRL, ((MCP2515_RXCTRL_BUF_FILTERS_OFF << MCP2515_RXCTRL_BUF_SHIFT) | MCP2515_RX0CTRL_BUKT)))
    	return NULL;

    /* config /INT as interrupt notification for all RX buffer */
    caninte = mcp_read(MCP2515_CANINTE);
    mcp_write(MCP2515_CANINTE, (caninte | MCP2515_INT_RX1IE | MCP2515_INT_RX0IE));

    /* reset all interrupts */
    mcp_write(MCP2515_CANINTF, 0);

    /* setup interrupt handling for /INT pin */
    set_gpio_callback(PORT_CAN_INT, PIN_CAN_INT, mcp2515_irq_handler, false, true);

    /* set and check opmode */
    mcp_write(MCP2515_CANCTRL, (MCP2515_CTRL_REQOP_NORMAL << MCP2515_CTRL_REQOP_SHIFT));

#if (DEBUG_PRINT == 1)

    uint8_t rec = mcp_read(MCP_CANCTRL);
    /* Debug output */
    char outstr[50];
    snprintf(outstr, 50, "Mode: 0x%x - ", rec);
    PRINT(outstr);

    if (rec != mcp_opmode)
    {
        PRINT("WROOOONG!\n");
    }
#endif

    return &mcp2515;
}

int mcp2515_reset()
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

void mcp2515_inthandler(CAN_Frame_t *frame)
{
    uint8_t interrupts    = mcp_read(MCP2515_CANINTF);

    if (interrupts & MCP2515_FLAG_RX0IF)
    {
        mcp2515_rxcanbuf(0 /*buffer*/, frame);
        interrupts &= ~MCP2515_FLAG_RX0IF;
    }

    if (interrupts & MCP2515_FLAG_RX1IF)
    {
        mcp2515_rxcanbuf(1 /*buffer*/, frame);
        interrupts &= ~MCP2515_FLAG_RX1IF;
    }

    /* clear the received buffer interrupt bit */
    mcp_write(MCP2515_CANINTF, interrupts);
}

int mcp2515_rxcanbuf(int rxbufno, CAN_Frame_t *candata)
{
	uint8_t txbuf[15];
	uint8_t rxbuf[15];

	char rxbufstart = MCP2515_SPI_READ_RX_BUFFER;
    rxbufstart |= ((((rxbufno == 0) ? MCP2515_RXB0CTRL : MCP2515_RXB1CTRL) >> 2) & 0x04 );

    txbuf[0] = rxbufstart;

    spi_dma_transfer(txbuf, rxbuf, 14);
    memmove(candata->f, rxbuf + 1, 13);

    return 1;
}

int mcp2515_txcanbuf(int txbufno, CAN_Frame_t *candata)
{
    uint8_t txbuf[15];

    if (txbufno > 2)
    	return 0;

    txbuf[0] = MCP2515_SPI_LOAD_TX_BUFFER | (txbufno << 1);
    memmove(txbuf + 1, candata->f, 13);
    spi_dma_transfer(txbuf, NULL /*rxbuf*/, 14);

    return 1;
}

int mcp2515_rxcan(int rxbuf, CAN_Frame_t *candata)
{
	int rxbufstart = MCP2515_DATA_SIDH;
    rxbufstart |= (rxbuf == 0) ? MCP2515_RXB0CTRL : MCP2515_RXB1CTRL;

    for (int i = 0; i < 13; i++)
    {
        candata->f[i] = mcp_read(rxbufstart);
        rxbufstart++;
    }

    return 1;
}

int mcp2515_txcan(CAN_Frame_t *data)
{
    int timeout_cnt = 0;

    uint8_t txctrl = mcp_read(MCP2515_TXB0CTRL);

    while ((txctrl & MCP2515_TXCTRL_RTS) != 0)
    {
        timeout_cnt++;

        txctrl = mcp_read(MCP2515_TXB0CTRL);

        if (timeout_cnt >= MAX_TIMEOUT_CNT)
        {
            vTracePrint(NULL, "[CAN] ERROR! TX buf full -> Throw message away!");
            return 0;
        }
    }

    mcp2515_txcanbuf(0, data);

    mcp_write_canrts(MCP2515_SPI_RTS | 0x01); // Send message RTS to TXB0

    return 1;
}

void mcp2515_get_errors(uint8_t *errflags, uint8_t *txerr, uint8_t *rxerr)
{
    *errflags = mcp_read(MCP2515_EFLG);
    *txerr    = mcp_read(MCP2515_TEC);
    *rxerr    = mcp_read(MCP2515_REC);
}

void mcp2515_irq_handler(void)
{
    signed portBASE_TYPE pxHigherPriorityTaskWoken = pdFALSE;
    CAN_Queue_t can_data;
    can_data.dir = CAN_QUEUE_IN;

    int loopCnt = 0;
    while (xQueueSendFromISR(q_can_handle, (void *)&can_data, &pxHigherPriorityTaskWoken) ==
               pdFALSE &&
           loopCnt < 10)
    {
        loopCnt++;
        xQueueReset(q_can_handle);
    }

    portEND_SWITCHING_ISR(pxHigherPriorityTaskWoken);
}

static int mcp_write(uint8_t address, uint8_t data)
{
    uint8_t txbuf[5];
    uint8_t rxbuf[5];

    txbuf[0] = MCP2515_SPI_WRITE;
    txbuf[1] = address;
    txbuf[2] = data;

    spi_dma_transfer(txbuf, rxbuf, 3);

    return 1;
}

static int mcp_write_canrts(uint8_t buffer)
{
    uint8_t txbuf[5];
    uint8_t rxbuf[5];

    txbuf[0] = buffer;

    spi_dma_transfer(txbuf, rxbuf, 1);

    return 1;
}

static uint8_t mcp_read(char address)
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

/* Setup clock for CAN transmission
 */
static int setupClock(enum eCANBaudrate baudrate)
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

    if (!mcp_write(MCP2515_CNF1, clk_conf1))
    	return 0;
    if (!mcp_write(MCP2515_CNF2, clk_conf2))
    	return 0;
    if (!mcp_write(MCP2515_CNF3, clk_conf3))
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
            mask = 0x000007FF; // 11 bit std address

            mcp_write(MCP2515_RXM0 | MCP2515_FILTMASK_SIDH, (uint8_t)((mask >> 3) & 0xff));
            mcp_write(MCP2515_RXM0 | MCP2515_FILTMASK_SIDL, (uint8_t)((mask << 5) & 0xe0));
            mcp_write(MCP2515_RXM0 | MCP2515_FILTMASK_EID8, (uint8_t)0);
            mcp_write(MCP2515_RXM0 | MCP2515_FILTMASK_EID0, (uint8_t)0);

            mcp_write(MCP2515_RXM1 | MCP2515_FILTMASK_SIDH, (uint8_t)((mask >> 3) & 0xff));
            mcp_write(MCP2515_RXM1 | MCP2515_FILTMASK_SIDL, (uint8_t)((mask << 5) & 0xe0));
            mcp_write(MCP2515_RXM1 | MCP2515_FILTMASK_EID8, (uint8_t)0);
            mcp_write(MCP2515_RXM1 | MCP2515_FILTMASK_EID0, (uint8_t)0);

            break;
        case EXTENDED_ADDRESSING:
        case STANDARD_ADDRESSING_DATA_FILTER: /* the first two data bytes are checked too */
            mask = 0x03ffffff;                // 11 bit std address, 17 bit extended address
            break;
        default:
            mask = 0x0;
            break;
    }
    /* STANDARD_ADDRESSING is used */

    /* Add filtering */
    /* If no filter are provided accept everything */
    if (filter == 0)
    {
        for (int i = 0; i < 12; i++)
            mcp_write(MCP2515_RXF0 | (MCP2515_FILTMASK_SIDH + i), 0);
        for (int i = 0; i < 12; i++)
            mcp_write(MCP2515_RXF3 | (MCP2515_FILTMASK_SIDH + i), 0);
    }

    /* DEBUG print masks: */
#if (DEBUG_PRINT == 1)
    char outstr[100];
    memset(outstr, 0, 100);
    uint8_t tmp;

    snprintf(outstr, 100, "%s", "Filtermask: ");

    tmp = mcp_read(MCP_RXM0SIDH);
    snprintf(outstr, 100, "%s 0x%x", outstr, tmp);
    tmp = mcp_read(MCP_RXM0SIDL);
    snprintf(outstr, 100, "%s 0x%x", outstr, tmp);

    tmp = mcp_read(MCP_RXM1SIDH);
    snprintf(outstr, 100, "%s 0x%x", outstr, tmp);
    tmp = mcp_read(MCP_RXM1SIDL);
    snprintf(outstr, 100, "%s 0x%x", outstr, tmp);

    snprintf(outstr, 100, "%s\n", outstr);
    PRINT(outstr);
#endif
}
