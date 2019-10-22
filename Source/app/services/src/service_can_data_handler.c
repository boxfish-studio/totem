/*
 * service_can_data_handler.c
 *
 *  Created on: Oct 21, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#include "service_can_data_handler.h"

static xTaskHandle handle_can_data;

/**
 *
 */
void service_can_data_handler_setup(const char * service_name, UBaseType_t service_priority) {
	xTaskCreate(service_can_data_handler, service_name, 500, NULL, service_priority,
			&handle_can_data);
}

/**
 *
 */

void service_can_data_handler(void *args) {

	traceString stackTrace = INIT_STACKTRACE(CAN_DATA_HANDLER_SERVICE_NAME);

	for (;;) {

		PRINT_STACKTRACE(stackTrace);
	}
}
