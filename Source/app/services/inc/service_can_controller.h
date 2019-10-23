/*
 * service_can_controller.h
 *
 *  Created on: Oct 21, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#ifndef SERVICE_CAN_CONTROLLER_H_
#define SERVICE_CAN_CONTROLLER_H_

#include "totem_sys.h"

#include "mcp2515.h"

#define CAN_CONTROLLER_SERVICE_NAME		"can_controller"

void service_can_controller_setup(const char * service_name, UBaseType_t service_priority);
void service_can_controller(void *args);

#endif /* SERVICE_CAN_CONTROLLER_H_ */
