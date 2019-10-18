/*
 * task_xmodem_dispatcher.h
 *
 *      Created on: Feb 27, 2015
 *      Author: Simon Fischinger
 *      Mail:   sfischinger@synapticon.com
 */

#ifndef TASK_XMODEM_DISPATCHER_H_
#define TASK_XMODEM_DISPATCHER_H_

#include "totem_sys.h"

static const char DISPATCHER_SERVICE_NAME[] = "dispatcher";

void service_xmodem_dispatcher_setup(const char * service_name, UBaseType_t service_priority);
void service_xmodem_dispatcher(void *args);


#endif /* TASK_XMODEM_DISPATCHER_H_ */
