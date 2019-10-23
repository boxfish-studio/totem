/*
 * service_can_data_handler.c
 *
 *  Created on: Oct 21, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#include "service_can_data_handler.h"

#include "mcp2515.h"

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

	CAN_Frame_t frame = { .s = {
			.sidh = (uint8_t) (0x666 >> 3),
			.sidl = (uint8_t) (0x666 << 5),
			.dlc = 8,
			.D0 = 'T',
			.D1 = 'o',
			.D2 = 't',
			.D3 = 'e',
			.D4 = 's',
			.D5 = 't',
			.D6 = 'e',
			.D7 = 'm',
	}};

    portTickType xLastWakeTime = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTime, 1000 / portTICK_RATE_MS);

	for (;;) {

		mcp2515_send(&frame);
		mcp2515_send(&frame);
		mcp2515_send(&frame);

        vTaskDelay(1000 / portTICK_RATE_MS);

        mcp2515_sleepMode(1);

        vTaskDelay(10000 / portTICK_RATE_MS);

        mcp2515_sleepMode(0);

		PRINT_STACKTRACE(stackTrace);
	}
}
