/*
 * @file task_xmodem_communicator.h
 *
 *      Created on: Feb 26, 2015
 *      Author: Simon Fischinger
 *      Mail:   sfischinger@synapticon.com
 */

#ifndef TASK_USB_H_
#define TASK_USB_H_

#include "totem_sys.h"

typedef struct
{
    uint8_t data[XMODEM_PACKET_SIZE];        /* Received USB data */
} usbInData_t;

static const char COMMUNICATIONS_SERVICE_NAME[] = "communications";

void service_communications_setup(const char * service_name, UBaseType_t service_priority);

/**
 * @brief Task that handles outgoing and incoming xmodem communication via the USB interface
 *
 * @return none
 */
void service_communications(void *args);

#endif /* TASK_USB_H_ */
