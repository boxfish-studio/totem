/*
 * xmodem_dispatch_handlers.h
 *
 *  Created on: Jul 19, 2016
 *      Author: Agustin Tena
 *      Mail:   tena@fazua.com
 */

#ifndef XMODEM_DISPATCHER_H_
#define XMODEM_DISPATCHER_H_

#include <stdint.h>
#include <stdbool.h>

// Message types
typedef enum
{
    XMODEM_MASTER_MSG = 1,
	XMODEM_SLAVE_MSG = 2
	//,
	//XMODEM_SLAVE_MSG = 1
} xModemMessageTypes_t;

typedef enum
{
    XMODEM_MASTER_SUBMSG_STATUS = 1,
	XMODEM_SLAVE_SUBMSG_STATUS = 1
} xModemMasterSubMsgTypes_t;

#define XMODEM_SLAVE_MSG_STATUS_DATA_SIZE 	20

bool xmodem_dispatch_master_msg(uint8_t data[], uint8_t len);
bool xmodem_send_slave_status_msg();

#endif // XMODEM_DISPATCHER_H_
