/*
 * pinmap.h
 *
 *  Created on: Oct 9, 2019
 *      Author: Agustin Tena <atena@boxfish.studio>
 */

#ifndef PINMAP_H_
#define PINMAP_H_

// RGB LED
#define PORT_LED_RED 	gpioPortA
#define PIN_LED_RED 	8
#define PORT_LED_BLUE 	gpioPortA
#define PIN_LED_BLUE 	9
#define PORT_LED_GREEN	gpioPortA
#define PIN_LED_GREEN	10

// PUSHBUTTONS
#define PORT_PUSH0	 	gpioPortE
#define PIN_PUSH0	 	4
#define PORT_PUSH1	 	gpioPortE
#define PIN_PUSH1	 	5

// CAN PINs
#define PORT_CAN_MOSI	gpioPortC
#define PIN_CAN_MOSI	2
#define PORT_CAN_MISO	gpioPortC
#define PIN_CAN_MISO	3
#define PORT_CAN_CLK	gpioPortC
#define PIN_CAN_CLK		4
#define PORT_CAN_CS		gpioPortC
#define PIN_CAN_CS		5
#define PORT_CAN_INT	gpioPortC
#define PIN_CAN_INT		6

// CAN DMA config
#define CAN_DMA_TX_CHANNEL	0
#define CAN_DMA_RX_CHANNEL	1

#endif /* PINMAP_H_ */
