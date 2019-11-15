/*
 * mcp2515.h
 *
 *  Created on: Oct 21, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#ifndef MCP2515_H_
#define MCP2515_H_

#include "mcp2515_ex.h"

#include <stdint.h>

#define MAX_TIMEOUT_CNT             3

enum eDirection {
    CAN_QUEUE_OUT, ///< Data from software to BUS
    CAN_QUEUE_IN  ///<- Data from BUS to software
};

typedef struct {
    uint8_t sidh;
    uint8_t sidl;
    uint8_t eid8;
    uint8_t eid0;
    uint8_t dlc; // Data Lenght Code
    uint8_t D0;
    uint8_t D1;
    uint8_t D2;
    uint8_t D3;
    uint8_t D4;
    uint8_t D5;
    uint8_t D6;
    uint8_t D7;
} s;

typedef union {
	s s;
    uint8_t f[13];
} CAN_Frame_t; // ?

typedef struct {
    enum eDirection dir;
    CAN_Frame_t canframe;
} CAN_Queue_t;

enum eCANAddressing {
    INVALID_ADDRESSING,
    STANDARD_ADDRESSING,
    EXTENDED_ADDRESSING,
    STANDARD_ADDRESSING_DATA_FILTER
};

enum eCANErrorState {
    CAN_NO_ERROR,        /* Error Active State */
    CAN_ERROR_PASSIVE,  /* REC>127 or TEC>127 */
    CAN_BUS_OFF        /* TEC>255 */
};

enum eCANBaudrate {
    CAN_BAUD_50KHZ,
    CAN_BAUD_100KHZ,
    CAN_BAUD_125KHZ,
    CAN_BAUD_250KHZ,
    CAN_BAUD_500KHZ,
    CAN_BAUD_1000KHZ
};

uint8_t mcp2515_init(enum eCANBaudrate baud);

uint8_t mcp2515_reset();

uint8_t mcp2515_sleepMode(uint8_t sleep);

void mcp2515_readBufferFromInterrupt(CAN_Frame_t *frame);

uint8_t mcp2515_readBuffer(uint8_t rxbufno, CAN_Frame_t *frame);

uint8_t mcp2515_send(CAN_Frame_t *frame);

uint8_t mcp2515_txcanbuf(uint8_t txbufno, CAN_Frame_t *candata);

void mcp2515_get_errors(uint8_t *errflags, uint8_t *txerr, uint8_t *rxerr);

void mcp2515_irq_handler(void);

#endif /* MCP2515_H_ */
