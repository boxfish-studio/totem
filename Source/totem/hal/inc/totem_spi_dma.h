/*
 * totem_spi_dma.h
 *
 *  Created on: Oct 21, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#ifndef TOTEM_SPI_DMA_H_
#define TOTEM_SPI_DMA_H_

// Device
#include "em_usart.h"
#include "em_dma.h"
#include "em_gpio.h"

/** SPI wires to transmit the data */
typedef struct {

	/** USART specified to transmit and receive*/
	USART_TypeDef		*usart;

	/** GPIO pin structure to use the SPI */
	GPIO_Port_TypeDef	MOSI_Port;
	unsigned int		MOSI_Pin;
	GPIO_Port_TypeDef	MISO_Port;
	unsigned int		MISO_Pin;
	GPIO_Port_TypeDef	CLK_Port;
	unsigned int		CLK_Pin;
	GPIO_Port_TypeDef	CS_Port;
	unsigned int		CS_Pin;

} SPI_Connections;

/** SPI using DMA structure */
typedef struct {

	/** Physical SPI wires and peripheral to transmit the data */
	SPI_Connections		spi_wires;

	/** Baudrate to transmit the data over SPI */
	uint32_t			baudrate;

	/** DMA configuration */
	/* TX */
	unsigned int		dma_tx_channel;
	DMA_CB_TypeDef		dma_tx_cb;
	/* RX */
	unsigned int		dma_rx_channel;
	DMA_CB_TypeDef		dma_rx_cb;

	/** Buffer used to transmit and receive */
	uint8_t				*txBuffer;	/* < Internal use. For transfer, create a previous local variable */
	uint8_t				*rxBuffer;

} SPI_DMA_Handler_t;

void spi_dma_setup(SPI_DMA_Handler_t *spi_dma);
void spi_dma_transfer(SPI_DMA_Handler_t *spi_dma, uint8_t *txBuffer, uint8_t bytes);
void spi_dma_receive(SPI_DMA_Handler_t *spi_dma, uint8_t bytes);

#endif /* TOTEM_SPI_DMA_H_ */
