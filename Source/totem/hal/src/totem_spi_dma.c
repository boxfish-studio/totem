/*
 * totem_spi_dma.c
 *
 *  Created on: Oct 21, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#include "totem_spi_dma.h"

#include "totem_nvic.h"

#include "em_usart.h"
#include "em_dma.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "pinmap.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#include "dmactrl.h"

#define DMA_CHANNEL_TX  0
#define DMA_CHANNEL_RX  1
#define DMA_CHANNELS    2

#define MAX_SPI_DATA_TRANSFER	15

/* DMA Callback structure */
DMA_CB_TypeDef spiCallback;

/* Transfer Flags */
volatile bool rxActive = false;
volatile bool txActive = false;

/* FreeRTOS Semaphores + Mutexes */
static xSemaphoreHandle sem_spi_dma;
static xSemaphoreHandle sem_rx_wait;

/* Local prototypes */
static void dma_setup();
static void spi_dma_transferCallback(unsigned int channel, bool primary, void *user);

static uint8_t dma_tx_buffer[MAX_SPI_DATA_TRANSFER];

/**
 * @brief	Setup SPI as Master for DMA use
 */
void spi_dma_setup()
{
	USART_InitSync_TypeDef usart_init = USART_INITSYNC_DEFAULT;

    /* Enable clocks */
    CMU_ClockEnable(cmuClock_DMA, true);
    CMU_ClockEnable(cmuClock_USART2, true);

    /* Configure GPIO pins for SPI */
    GPIO_PinModeSet(PORT_CAN_MOSI, PIN_CAN_MOSI, gpioModePushPull, 0); /* MOSI */
    GPIO_PinModeSet(PORT_CAN_MISO, PIN_CAN_MISO, gpioModeInput, 0);    /* MISO */
    GPIO_PinModeSet(PORT_CAN_CLK, PIN_CAN_CLK, gpioModePushPull, 0);   /* Clock */
    GPIO_PinModeSet(PORT_CAN_CS, PIN_CAN_CS, gpioModePushPull, 1);     /* CS */

    /* Initialize SPI */
    usart_init.databits = usartDatabits8;
    usart_init.baudrate = 1000000; /* 1 MBit */
    usart_init.msbf = true;
    USART_InitSync(USART2, &usart_init);

    /* Turn on automatic Chip Select control */
    USART2->CTRL |= USART_CTRL_AUTOCS;

    /* Enable routing for SPI pins from USART to location 0 */
    USART2->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CSPEN | USART_ROUTE_CLKPEN |
                    USART_ROUTE_LOCATION_LOC0;

    /* Enable SPI */
    USART_Enable(USART2, usartEnable);

    /* Enable DMA */
    dma_setup();

    /* Configure semaphores */
    if (sem_spi_dma == NULL)
    	vSemaphoreCreateBinary(sem_spi_dma);
    if (sem_rx_wait == NULL)
    	vSemaphoreCreateBinary(sem_rx_wait);

}

/**
 * @brief	SPI communication using DMA. Combine with spi_dma_waitRX if the receive data is needed
 * 			immediately
 * @param	txBuffer	Data to transmit over SPI. It can be NULL to transmit dummy data
 * @param	rxBuffer	Pointer to memory where save the received data. It can be NULL to skip activation
 * @param	bytes		Number of bytes to transmit and receive (the total one)
 */
void spi_dma_transfer(uint8_t *txBuffer, uint8_t *rxBuffer, int bytes)
{
	/* Wait until DMA is free */
	xSemaphoreTake(sem_spi_dma, portMAX_DELAY);

	memmove(dma_tx_buffer, txBuffer, bytes);

    /* Only activate RX DMA if a receive buffer is specified */
    if (rxBuffer != NULL)
    {
    	/* Free the RX reception semaphore */
    	xSemaphoreGive(sem_rx_wait);

        /* Setting flag to indicate that RX is in progress
         * will be cleared by call-back function */
        rxActive = true;

        /* Clear RX registers */
        USART2->CMD = USART_CMD_CLEARRX;

        /* Activate RX channel */
        DMA_ActivateBasic(DMA_CHANNEL_RX, true, false, rxBuffer, (void *)&(USART2->RXDATA),
                          bytes - 1);


        xSemaphoreTake(sem_rx_wait, portMAX_DELAY);
    }

    /* Setting flag to indicate that TX is in progress
     * will be cleared by call-back function */
    txActive = true;
    /* Clear TX registers */
    USART2->CMD = USART_CMD_CLEARTX;

    /* Activate TX channel */
    DMA_ActivateBasic(DMA_CHANNEL_TX, true, false, (void *)&(USART2->TXDATA), dma_tx_buffer, bytes - 1);
}

/**
 * @brief	Wait until the DMA has received all data expected
 */
void spi_dma_waitRX()
{
	xSemaphoreTake(sem_rx_wait, portMAX_DELAY);
}

/**
 * @brief	Configure DMA in basic mode for TX/RX to/from USART
 */
static void dma_setup()
{
    /* Initialization structures */
    DMA_Init_TypeDef dmaInit;
    DMA_CfgChannel_TypeDef rxChnlCfg;
    DMA_CfgDescr_TypeDef rxDescrCfg;
    DMA_CfgChannel_TypeDef txChnlCfg;
    DMA_CfgDescr_TypeDef txDescrCfg;

    /* Initializing the DMA */
    dmaInit.hprot = 0;
    dmaInit.controlBlock = dmaControlBlock;
    DMA_Init(&dmaInit);

    /* Setup SPI callback function */
    spiCallback.cbFunc = spi_dma_transferCallback;
    spiCallback.userPtr = NULL;

    /*** RX ***/

    /* Setting up RX channel */
    rxChnlCfg.highPri = false;
    rxChnlCfg.enableInt = true;
    rxChnlCfg.select = DMAREQ_USART2_RXDATAV;
    rxChnlCfg.cb = &spiCallback;
    DMA_CfgChannel(DMA_CHANNEL_RX, &rxChnlCfg);

    /* Setting up RX descriptor */
    rxDescrCfg.dstInc = dmaDataInc1;
    rxDescrCfg.srcInc = dmaDataIncNone;
    rxDescrCfg.size = dmaDataSize1;
    rxDescrCfg.arbRate = dmaArbitrate1;
    rxDescrCfg.hprot = 0;
    DMA_CfgDescr(DMA_CHANNEL_RX, true, &rxDescrCfg);

    /*** TX ***/

    /* Setting up TX channel */
    txChnlCfg.highPri = false;
    txChnlCfg.enableInt = true;
    txChnlCfg.select = DMAREQ_USART2_TXBL;
    txChnlCfg.cb = &spiCallback;
    DMA_CfgChannel(DMA_CHANNEL_TX, &txChnlCfg);

    /* Setting up TX channel descriptor */
    txDescrCfg.dstInc = dmaDataIncNone;
    txDescrCfg.srcInc = dmaDataInc1;
    txDescrCfg.size = dmaDataSize1;
    txDescrCfg.arbRate = dmaArbitrate1;
    txDescrCfg.hprot = 0;
    DMA_CfgDescr(DMA_CHANNEL_TX, true, &txDescrCfg);
}

/**
 * @brief	Callback for DMA transfer complete event
 * @param	channel		DMA channel the callback function is invoked for.
 * @param	primary		Indicates if callback is invoked for completion of primary
 *     		(true) or alternate (false) descriptor. This is mainly useful for
 *     		ping-pong DMA cycles, in order to know which descriptor to refresh.
 * @param	user		User definable reference that may be used to pass information
 *     		to be used by the callback handler. If used, the referenced data must be
 *     		valid at the point when the interrupt handler invokes the callback.
 *     		If callback changes  any data in the provided user structure, remember
 *     		that those changes are done in interrupt context, and proper protection
 *     		of data may be required.
 */
static void spi_dma_transferCallback(unsigned int channel, bool primary, void *user)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (channel == DMA_CHANNEL_TX)
    {
        txActive = false;
    }

    if (channel == DMA_CHANNEL_RX)
    {
        // RX, if expected, done
        rxActive = false;
    	xSemaphoreGiveFromISR(sem_rx_wait, &xHigherPriorityTaskWoken);
    }

    if (txActive == false && rxActive == false)
    {
    	// End of use of DMA
        xSemaphoreGiveFromISR(sem_spi_dma, &xHigherPriorityTaskWoken);
    }

    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}
