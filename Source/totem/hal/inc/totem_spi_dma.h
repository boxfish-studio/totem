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

typedef struct {
	USART_TypeDef		*usart;

	GPIO_Port_TypeDef	MOSI_Port;
	unsigned int		MOSI_Pin;
	GPIO_Port_TypeDef	MISO_Port;
	unsigned int		MISO_Pin;
	GPIO_Port_TypeDef	CLK_Port;
	unsigned int		CLK_Pin;
	GPIO_Port_TypeDef	CS_Port;
	unsigned int		CS_Pin;
} SPI_Connections;

typedef struct {
	SPI_Connections		spi_wires;
	uint32_t			baudrate;

	unsigned int		dma_tx_channel;
	DMA_CB_TypeDef		dma_tx_cb;
	unsigned int		dma_rx_channel;
	DMA_CB_TypeDef		dma_rx_cb;

	uint8_t				*txBuffer;	/* < Internal use. For transfer, create a previous local variable */
	uint8_t				*rxBuffer;
} SPI_DMA_Handler_t;

void spi_dma_setup(SPI_DMA_Handler_t *spi_dma);
void spi_dma_transfer(SPI_DMA_Handler_t *spi_dma, uint8_t *txBuffer, uint8_t bytes);
void spi_dma_receive(SPI_DMA_Handler_t *spi_dma, uint8_t bytes);

#endif /* TOTEM_SPI_DMA_H_ */
