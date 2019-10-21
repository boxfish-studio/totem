/*
 * 	service_usb_xmodem.h
 *
 * 	Created on: Oct 21, 2019
 * 		Author: Miguel Villalba <mvillalba@boxfish.studio>, Agustin Tena <atena@boxfish.studio>
 */

#ifndef SERVICE_USB_XMODEM_H_
#define SERVICE_USB_XMODEM_H_

#include "totem_sys.h"

typedef struct {
	uint8_t data[USB_PACKET_SIZE]; // Received USB data
} usb_data_packet_t;

static const char USB_XMODEM_SERVICE_NAME[] = "usb_xmodem";

void service_usb_xmodem_setup(const char * service_name,
		UBaseType_t service_priority);
void service_usb_xmodem(void *args);

#endif /* SERVICE_USB_XMODEM_H_ */
