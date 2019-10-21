/*
 * 	service_usb_data_handler.c
 *
 * 	Created on: Oct 21, 2019
 * 		Author: Miguel Villalba <mvillalba@boxfish.studio>, Agustin Tena <atena@boxfish.studio>
 */

#ifndef SERVICE_USB_DATA_HANDLER_H_
#define SERVICE_USB_DATA_HANDLER_H_

#include "totem_sys.h"

static const char USB_DATA_HANDLER_SERVICE_NAME[] = "usb_data_handler";

void service_usb_data_handler_setup(const char * service_name, UBaseType_t service_priority);
void service_usb_data_handler(void *args);

typedef enum
{
    XMODEM_MASTER_SUBMSG_STATUS = 1,
	XMODEM_SLAVE_SUBMSG_STATUS = 1
} xModemMasterSubMsgTypes_t;

#define XMODEM_SLAVE_MSG_STATUS_DATA_SIZE 	20

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

#endif /* SERVICE_USB_DATA_HANDLER_H_ */
