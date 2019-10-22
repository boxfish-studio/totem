/*
 * service_can_data_handler.h
 *
 *  Created on: Oct 21, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#ifndef SERVICE_CAN_DATA_HANDLER_H_
#define SERVICE_CAN_DATA_HANDLER_H_

#include "totem_sys.h"

static const char CAN_DATA_HANDLER_SERVICE_NAME[] = "can_data";

void service_can_data_handler_setup(const char * service_name, UBaseType_t service_priority);
void service_can_data_handler(void *args);

#endif /* SERVICE_CAN_DATA_HANDLER_H_ */
