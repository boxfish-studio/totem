/*
 * xmodem_dispatch_handlers.c
 *
 *  Created on: Mar 9, 2015
 *      Author: Simon Fischinger
 *      Mail:   sfischinger@synapticon.com
 */

#include "xmodem_dispatcher.h"
#include "xmodem.h"
#include "_stdio.h"

enum PCBtype {
	NoPCB   = 0,
    Remote 	= 1,
    Control = 2,
    Drive 	= 3
};

enum AppState {
	NoState = 0,
	Idle   	= 1,
    Busy   	= 2,
    Error  	= 3
};

enum Command {
	NoCommand 	= 0,
    Program 	= 1,
    Test		= 2
};

enum CommandOut {
	NONE   		= 0,
    OK			= 1,
	PROGRESS 	= 2,
    FAIL		= 3
};

typedef struct
{
	enum AppState			state;
	enum PCBtype			pcb_type;
	enum Command			command;
	enum CommandOut			command_out;
} test;

// RTOS Task handles

bool xmodem_dispatch_master_msg(uint8_t data[], uint8_t len)
{
    test	rcv_master;

    if (len < 2)
    {
        PRINT("xmodem_dispatch_master_msg(): ERROR, not enough bytes received!\n");
        return false;
    }

    switch (data[1])
    {
        case XMODEM_MASTER_SUBMSG_STATUS:
            // Set Master status msg
            PRINT("xmodem_dispatch_production(): Received \"Master status msg\"\n");

            rcv_master.pcb_type 	= data[2];
            //rcv_master.pcb_number 	= data[3];
            rcv_master.command 		= data[4];

            break;

        default:
            break;
    }

    return true;
}

bool xmodem_send_slave_status_msg()
{
	uint8_t send_data[XMODEM_SLAVE_MSG_STATUS_DATA_SIZE];

	send_data[0] = Idle;
	send_data[1] = Remote;
	send_data[3] = NoCommand;
	send_data[4] = NONE;

	return xmodem_send_data(XMODEM_SLAVE_MSG,
							XMODEM_SLAVE_SUBMSG_STATUS,
							send_data,
							XMODEM_SLAVE_MSG_STATUS_DATA_SIZE);
}
