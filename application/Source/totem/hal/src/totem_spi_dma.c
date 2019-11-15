/*
 * totem_spi_dma.c
 *
 *  Created on: Oct 21, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#include "totem_spi_dma.h"

#include <string.h>

// Device
#include "em_cmu.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

// DMA
#include "dmactrl.h"

/*** Local prototypes ***/
static void SPI_SerialPortInit(SPI_DMA_Handler_t *spi_dma);
static void DMA_SPI_SerialPortInit(SPI_DMA_Handler_t *spi_dma);

/**************************************************************************//**
 * @brief	Setup SPI with USART as Master using DMA to transfer data to TX/RX
 *
 * @param	spi_dma	Pointer to the handler where is specified the peripheral
 * 					characteristics to communications
 *****************************************************************************/
void spi_dma_setup(SPI_DMA_Handler_t *spi_dma)
{
	// SPI initialization
	SPI_SerialPortInit(spi_dma);
	// DMA initialization
    DMA_SPI_SerialPortInit(spi_dma);
}

/**************************************************************************//**
 * @brief	SPI transfer using DMA. When the transfer is reached, the callback
 * 			assign in the SPI_DMA_Handler is called
 *
 * @param	spi_dma		Pointer to the handler where is specified the peripheral
 * @param	txBuffer	Data to transmit over SPI. It can be NULL to transmit
 * 						dummy data
 * @param	bytes		Number of bytes to transmit and receive (the total one,
 * 						due to the CS)
 *****************************************************************************/
void spi_dma_transfer(SPI_DMA_Handler_t *spi_dma, uint8_t *txBuffer, uint8_t bytes)
{
	/* Global copy to transmit */
	memmove(spi_dma->txBuffer, txBuffer, bytes);

    /* Clear TX registers */
    spi_dma->spi_wires.usart->CMD = USART_CMD_CLEARTX;

    /* Activate TX channel */
    DMA_ActivateBasic(spi_dma->dma_tx_channel, true, false, (void *)&(spi_dma->spi_wires.usart->TXDATA), spi_dma->txBuffer, bytes - 1);
}

/**************************************************************************//**
 * @brief	SPI receive using DMA. When the transfer is reached, the callback
 * 			assign in the SPI_DMA_Handler is called
 *
 * @param	spi_dma		Pointer to the handler where is specified the peripheral
 * @param	bytes		Number of bytes to transmit and receive (the total one,
 * 						due to the CS)
 *****************************************************************************/
void spi_dma_receive(SPI_DMA_Handler_t *spi_dma, uint8_t bytes)
{
    /* Clear RX registers */
    spi_dma->spi_wires.usart->CMD = USART_CMD_CLEARRX;

    /* Activate RX channel */
    DMA_ActivateBasic(spi_dma->dma_rx_channel, true, false, spi_dma->rxBuffer, (void *)&(spi_dma->spi_wires.usart->RXDATA),
                      bytes - 1);
}

/**************************************************************************//**
 * @brief	Initialize the UART peripheral
 *
 * @param	spi_dma		Pointer to the handler where is specified the
 * 						peripheral
 *****************************************************************************/
static void SPI_SerialPortInit(SPI_DMA_Handler_t *spi_dma)
{
	USART_InitSync_TypeDef usart_init = USART_INITSYNC_DEFAULT;

	/* Configure GPIO clock */
	CMU_ClockEnable(cmuClock_GPIO, true);

	/* Configure GPIO pins for SPI */
	GPIO_PinModeSet(spi_dma->spi_wires.MOSI_Port, spi_dma->spi_wires.MOSI_Pin, gpioModePushPull, 0); /* MOSI */
	GPIO_PinModeSet(spi_dma->spi_wires.MISO_Port, spi_dma->spi_wires.MISO_Pin, gpioModeInput, 0);    /* MISO */
	GPIO_PinModeSet(spi_dma->spi_wires.CLK_Port, spi_dma->spi_wires.CLK_Pin, gpioModePushPull, 0);   /* Clock */
	GPIO_PinModeSet(spi_dma->spi_wires.CS_Port, spi_dma->spi_wires.CS_Pin, gpioModePushPull, 1);     /* CS */

	/* Enable peripheral clocks */
	if (spi_dma->spi_wires.usart == USART0) {
		CMU_ClockEnable(cmuClock_USART0, true);

	} else if (spi_dma->spi_wires.usart == USART1) {
		CMU_ClockEnable(cmuClock_USART1, true);

	} else if (spi_dma->spi_wires.usart == USART2) {
		CMU_ClockEnable(cmuClock_USART2, true);
	}

	/* Configure USART for SPI operation */
	usart_init.enable = usartDisable;
	usart_init.baudrate = spi_dma->baudrate;
	usart_init.databits = usartDatabits8;
	usart_init.master = true;
	usart_init.msbf = true;
	USART_InitSync(spi_dma->spi_wires.usart, &usart_init);

	/* Turn on automatic Chip Select control */
	spi_dma->spi_wires.usart->CTRL |= USART_CTRL_AUTOCS;

	/* Enable routing for SPI pins from USART to location 0 */
	spi_dma->spi_wires.usart->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CSPEN | USART_ROUTE_CLKPEN |
					USART_ROUTE_LOCATION_LOC0;	// FIXME: If the GPIO location is not Loc0, then the USART is not well configured

	/* Enable SPI */
	USART_Enable(spi_dma->spi_wires.usart, usartEnable);
}

/**************************************************************************//**
 * @brief Initialize the DMA peripheral.
 *
 * @param	spi_dma		Pointer to the handler where is specified the
 * 						peripheral
 *****************************************************************************/
static void DMA_SPI_SerialPortInit(SPI_DMA_Handler_t *spi_dma)
{

    /* Initialization structures */
    DMA_Init_TypeDef dmaInit;
    DMA_CfgChannel_TypeDef rxChnlCfg;
    DMA_CfgDescr_TypeDef rxDescrCfg;
    DMA_CfgChannel_TypeDef txChnlCfg;
    DMA_CfgDescr_TypeDef txDescrCfg;

    /* Enable clock DMA */
	CMU_ClockEnable(cmuClock_DMA, true);

    /* Initializing the DMA */
    dmaInit.hprot = 0;
    dmaInit.controlBlock = dmaControlBlock;
    DMA_Init(&dmaInit);

    /*** RX ***/

    /* Setting up RX channel */
    rxChnlCfg.highPri = false;
    rxChnlCfg.enableInt = true;
	if (spi_dma->spi_wires.usart == USART0) {
	    rxChnlCfg.select = DMAREQ_USART0_RXDATAV;

	} else if (spi_dma->spi_wires.usart == USART1) {
	    rxChnlCfg.select = DMAREQ_USART1_RXDATAV;

	} else if (spi_dma->spi_wires.usart == USART2) {
	    rxChnlCfg.select = DMAREQ_USART2_RXDATAV;
	}
    rxChnlCfg.cb = &(spi_dma->dma_rx_cb);
    DMA_CfgChannel(spi_dma->dma_rx_channel, &rxChnlCfg);

    /* Setting up RX descriptor */
    rxDescrCfg.dstInc = dmaDataInc1;
    rxDescrCfg.srcInc = dmaDataIncNone;
    rxDescrCfg.size = dmaDataSize1;
    rxDescrCfg.arbRate = dmaArbitrate1;
    rxDescrCfg.hprot = 0;
    DMA_CfgDescr(spi_dma->dma_rx_channel, true, &rxDescrCfg);

    /*** TX ***/

    /* Setting up TX channel */
    txChnlCfg.highPri = false;
    txChnlCfg.enableInt = true;
	if (spi_dma->spi_wires.usart == USART0) {
	    txChnlCfg.select = DMAREQ_USART0_TXBL;

	} else if (spi_dma->spi_wires.usart == USART1) {
	    txChnlCfg.select = DMAREQ_USART1_TXBL;

	} else if (spi_dma->spi_wires.usart == USART2) {
	    txChnlCfg.select = DMAREQ_USART2_TXBL;
	}
    txChnlCfg.cb= &(spi_dma->dma_tx_cb);
    DMA_CfgChannel(spi_dma->dma_tx_channel, &txChnlCfg);

    /* Setting up TX channel descriptor */
    txDescrCfg.dstInc = dmaDataIncNone;
    txDescrCfg.srcInc = dmaDataInc1;
    txDescrCfg.size = dmaDataSize1;
    txDescrCfg.arbRate = dmaArbitrate1;
    txDescrCfg.hprot = 0;
    DMA_CfgDescr(spi_dma->dma_tx_channel, true, &txDescrCfg);
}
