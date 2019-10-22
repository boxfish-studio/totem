/*
 * totem_spi_dma.c
 *
 *  Created on: Oct 21, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#include "totem_spi_dma.h"

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

/* DMA Callback structure */
DMA_CB_TypeDef spiCallback;

/* Transfer Flags */
volatile bool rxActive = false;
volatile bool txActive = false;

/* FreeRTOS Semaphores + Mutexes */
static xSemaphoreHandle sem_spi_dma;
static xSemaphoreHandle mut_spi_dma;

/* Local prototypes */
static void setupDma(void);
void spi_dma_transfer_complete_callback(unsigned int channel, bool primary, void *user);

/******************************************************************************
 * @brief  Call-back called when transfer is complete
 *****************************************************************************/
void spi_dma_transfer_complete_callback(unsigned int channel, bool primary, void *user)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    /* Clear flag to indicate complete transfer */
    if (channel == DMA_CHANNEL_TX)
    {
        txActive = false;
    }

    if (channel == DMA_CHANNEL_RX)
    {
        rxActive = false;
    }

    if (txActive == false && rxActive == false)
    {
        xSemaphoreGiveFromISR(sem_spi_dma, &xHigherPriorityTaskWoken);
        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    }
}

/******************************************************************************
 * @brief Configure DMA in basic mode for both TX and RX to/from USART
 *****************************************************************************/
static void setupDma(void)
{
    /* Initialization structs */
    DMA_Init_TypeDef dmaInit;
    DMA_CfgChannel_TypeDef rxChnlCfg;
    DMA_CfgDescr_TypeDef rxDescrCfg;
    DMA_CfgChannel_TypeDef txChnlCfg;
    DMA_CfgDescr_TypeDef txDescrCfg;

    /* Initializing the DMA */
    dmaInit.hprot        = 0;
    dmaInit.controlBlock = dmaControlBlock;
    DMA_Init(&dmaInit);

    /* Setup call-back function */
    spiCallback.cbFunc  = spi_dma_transfer_complete_callback;
    spiCallback.userPtr = NULL;

    /*** Setting up RX DMA ***/

    /* Setting up channel */
    rxChnlCfg.highPri   = false;
    rxChnlCfg.enableInt = true;
    rxChnlCfg.select    = DMAREQ_USART2_RXDATAV;
    rxChnlCfg.cb        = &spiCallback;
    DMA_CfgChannel(DMA_CHANNEL_RX, &rxChnlCfg);

    /* Setting up channel descriptor */
    rxDescrCfg.dstInc  = dmaDataInc1;
    rxDescrCfg.srcInc  = dmaDataIncNone;
    rxDescrCfg.size    = dmaDataSize1;
    rxDescrCfg.arbRate = dmaArbitrate1;
    rxDescrCfg.hprot   = 0;
    DMA_CfgDescr(DMA_CHANNEL_RX, true, &rxDescrCfg);

    /*** Setting up TX DMA ***/

    /* Setting up channel */
    txChnlCfg.highPri   = false;
    txChnlCfg.enableInt = true;
    txChnlCfg.select    = DMAREQ_USART2_TXBL;
    txChnlCfg.cb        = &spiCallback;
    DMA_CfgChannel(DMA_CHANNEL_TX, &txChnlCfg);

    /* Setting up channel descriptor */
    txDescrCfg.dstInc  = dmaDataIncNone;
    txDescrCfg.srcInc  = dmaDataInc1;
    txDescrCfg.size    = dmaDataSize1;
    txDescrCfg.arbRate = dmaArbitrate1;
    txDescrCfg.hprot   = 0;
    DMA_CfgDescr(DMA_CHANNEL_TX, true, &txDescrCfg);
}

/******************************************************************************
 * @brief  Setup SPI as Master
 *****************************************************************************/
void setup_spi_dma() // uint8_t spiNumber, uint8_t location, bool master)
{
    /* Enable needed clocks */
    CMU_ClockEnable(cmuClock_DMA, true);
    CMU_ClockEnable(cmuClock_USART2, true);

    USART_InitSync_TypeDef usart_init = USART_INITSYNC_DEFAULT;
    usart_init.msbf                   = true;

    /* Initialize SPI */
    usart_init.databits = usartDatabits8;
    usart_init.baudrate = 1000000; /* 1 MBit */
    USART_InitSync(USART2, &usart_init);

    /* Turn on automatic Chip Select control */
    USART2->CTRL |= USART_CTRL_AUTOCS;

    /* Enable SPI transmit and receive */
    USART_Enable(USART2, usartEnable);

    /* Configure GPIO pins for SPI */
    GPIO_PinModeSet(PORT_CAN_MOSI, PIN_CAN_MOSI, gpioModePushPull, 0); /* MOSI */
    GPIO_PinModeSet(PORT_CAN_MISO, PIN_CAN_MISO, gpioModeInput, 0);    /* MISO */
    GPIO_PinModeSet(PORT_CAN_CLK, PIN_CAN_CLK, gpioModePushPull, 0);   /* Clock */
    GPIO_PinModeSet(PORT_CAN_CS, PIN_CAN_CS, gpioModePushPull, 1);     /* CS */

    /* Enable routing for SPI pins from USART to location 0 */
    USART2->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CSPEN | USART_ROUTE_CLKPEN |
                    USART_ROUTE_LOCATION_LOC0;

    setupDma();

    vSemaphoreCreateBinary(sem_spi_dma);
    vSemaphoreCreateBinary(mut_spi_dma);

}

/******************************************************************************
 * @brief  SPI DMA Transfer
 * NULL can be input as txBuffer if tx data to transmit dummy data
 * If only sending data, set rxBuffer as NULL to skip DMA activation on RX
 *****************************************************************************/
void spi_dma_transfer(uint8_t *txBuffer, uint8_t *rxBuffer, int bytes)
{
    /* Block SPI transfers until current transfer is done (RTOS MUTEX) */
    xSemaphoreTake(mut_spi_dma, portMAX_DELAY);

    /* Only activate RX DMA if a receive buffer is specified */
    if (rxBuffer != NULL)
    {
        /* Setting flag to indicate that RX is in progress
         * will be cleared by call-back function */
        rxActive = true;

        /* Clear RX regsiters */
        USART2->CMD = USART_CMD_CLEARRX;

        /* Activate RX channel */
        DMA_ActivateBasic(DMA_CHANNEL_RX, true, false, rxBuffer, (void *)&(USART2->RXDATA),
                          bytes - 1);
    }

    /* Setting flag to indicate that TX is in progress
     * will be cleared by call-back function */
    txActive = true;
    /* Clear TX regsiters */
    USART2->CMD = USART_CMD_CLEARTX;

    /* Activate TX channel */
    DMA_ActivateBasic(DMA_CHANNEL_TX, true, false, (void *)&(USART2->TXDATA), txBuffer, bytes - 1);

    /* Put task to sleep until transfer has finished */
    xSemaphoreTake(sem_spi_dma, portMAX_DELAY);

    /* Give MUTEX back for next transfer operation */
    xSemaphoreGive(mut_spi_dma);
}
