/*
 * mcp2515.h
 *
 *  Created on: Oct 21, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#ifndef MCP2515_H_
#define MCP2515_H_

#include "mcp2515_ex.h"

#define MAX_TIMEOUT_CNT             3

enum eDirection {
    CAN_QUEUE_OUT = 0, ///< Data from software to BUS
    CAN_QUEUE_IN  ///<- Data from BUS to software
};

typedef struct _mcp25x_if MCP2515_IF_T;

typedef struct {
    uint8_t sidh; // TXBnSIDH
    uint8_t sidl; // TXBnSIDL
    uint8_t eid8; // extended identifier high
    uint8_t eid0; // extended identifier low
    uint8_t dlc; // data lenght code
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

enum eCANaddressing {
    INVALID_ADDRESSING,
    STANDARD_ADDRESSING,
    EXTENDED_ADDRESSING,
    STANDARD_ADDRESSING_DATA_FILTER
};

enum eCANErrorState {
    CAN_NO_ERROR,        /* aka Error Active State */
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

/**
 * @brief 	Initialize MCP2515 CAN device
 * @param 	baud    Baudrate for the CAN transmission
 * @return	Device descriptor
 */
MCP25X_IF_T *mcp2515_init(enum eCANBaudrate baud);

/**
 * @brief 	Reset MCP device. After the reset the MCP25x should
 * 			be in configuration mode.
 * @return 	0 if reset is successful, 1 otherwise
 */
int mcp2515_reset();

int mcp2515_data_ready(MCP25X_IF_T *mcpif);

/**
 * @brief	Handle the incomming interrupt by /INT pin.
 *
 * @param	mcp2515	Device handler
 */
void mcp2515_inthandler(CAN_Frame_t *frame);

int mcp2515_rxcan( int buffer, CAN_Frame_t *data); //MCP25X_IF_T *mcpif,

int mcp2515_txcan( CAN_Frame_t *data); //MCP25X_IF_T *mcpif,

/* transfer functions to load/store a complete RX/TX buffer */
int mcp2515_rxcanbuf( int rxbufno, CAN_Frame_t *candata); //MCP25X_IF_T *mcp,

int mcp2515_txcanbuf(int txbufno, CAN_Frame_t *candata); //MCP25X_IF_T *mcp,

#if (DEBUG_PRINT == 1)
int mcp2515_debug(MCP25X_IF_T *mcpif);
#endif

void mcp2515_irq_handler(void);

void mcp2515_get_errors( uint8_t *errflags, uint8_t *txerr, uint8_t *rxerr); //MCP25X_IF_T *mcpif,

#endif /* MCP2515_H_ */
