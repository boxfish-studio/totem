/*
 * 	service_usb_data_handler.h
 *
 * 	Created on: Oct 21, 2019
 * 		Author: Miguel Villalba <mvillalba@boxfish.studio>, Agustin Tena <atena@boxfish.studio>
 */

#ifndef SERVICE_USB_DATA_HANDLER_H_
#define SERVICE_USB_DATA_HANDLER_H_

#include "totem_sys.h"

#define USB_DATA_HANDLER_SERVICE_NAME	"usb_data_handler"

void service_usb_data_handler_setup(const char * service_name,
		UBaseType_t service_priority);
void service_usb_data_handler(void *args);

typedef enum {
	XMODEM_HOST_SUBMSG_STATUS = 1, XMODEM_SLAVE_SUBMSG_STATUS = 1
} xModemMasterSubMsgTypes_t;

#define XMODEM_SLAVE_MSG_STATUS_DATA_SIZE 	20


#endif /* SERVICE_USB_DATA_HANDLER_H_ */
