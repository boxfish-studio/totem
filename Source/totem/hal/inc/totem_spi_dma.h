/*
 * totem_spi_dma.h
 *
 *  Created on: Oct 21, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#ifndef TOTEM_SPI_DMA_H_
#define TOTEM_SPI_DMA_H_

#include <stdint.h>

void setup_spi_dma();
void spi_dma_transfer(uint8_t *txBuffer, uint8_t *rxBuffer, int bytes);

#endif /* TOTEM_SPI_DMA_H_ */
