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
			.eid8 = 0,
			.eid0 = 0,
			.dlc = 8,
			.D0 = 0,
			.D1 = 0,
			.D2 = 0,
			.D3 = 0,
			.D4 = 0,
			.D5 = 0,
			.D6 = 0,
			.D7 = 0,
	}};

    portTickType xLastWakeTime = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTime, 1000 / portTICK_RATE_MS);

    unsigned long long timer = 0;
	for (;;) {

		timer++;

		frame.s.D0 = (char) (timer & 0xFF);
		frame.s.D1 = (char) ((timer >> 8) & 0xFF);
		frame.s.D2 = (char) ((timer >> 16) & 0xFF);
		frame.s.D3 = (char) ((timer >> 24) & 0xFF);
		frame.s.D4 = (char) ((timer >> 32) & 0xFF);
		frame.s.D5 = (char) ((timer >> 40) & 0xFF);
		frame.s.D6 = (char) ((timer >> 48) & 0xFF);
		frame.s.D7 = (char) ((timer >> 56) & 0xFF);

		mcp2515_send(&frame);

        vTaskDelay(5000 / portTICK_RATE_MS);

		PRINT_STACKTRACE(stackTrace);
	}
}
