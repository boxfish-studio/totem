/*
 * 	service_usb_data_handler.c
 *
 * 	Created on: Oct 21, 2019
 * 		Author: Miguel Villalba <mvillalba@boxfish.studio>, Agustin Tena <atena@boxfish.studio>
 */

#include <service_usb_data_handler.h>

// RTOS Queues
extern xQueueHandle queue_xmodem_communicator_out;

static xTaskHandle service_handler;

/**
 *
 */
void service_usb_data_handler_setup(const char * service_name,
		UBaseType_t service_priority) {
	xTaskCreate(service_usb_data_handler, service_name, 650, NULL,
			service_priority, &service_handler);
}

void service_usb_data_handler(void *args) {
	traceString stack_trace = INIT_STACKTRACE(USB_DATA_HANDLER_SERVICE_NAME);

	xModemCommunicator_t in_data;
	uint8_t rx_counter = 0;

	while (1) {
		// Wait for data to come in from xmodem_communicator
		xQueueReceive(queue_xmodem_communicator_out, &in_data, portMAX_DELAY);
		PRINT("-------------------- Task task_xmodem_dispatcher running -----------------------------\n");

		// Check whether data is meant for this task
		if (in_data.type != XMODEM_RCV)
			continue;

		// Check type of data and take appropriate action
		switch (in_data.data[0]) {
		case XMODEM_MASTER_MSG:
			PRINT ("task_xmodem_dispatcher(): XMODEM_MASTER_MSG received\n");
			xmodem_dispatch_master_msg(in_data.data, in_data.dat_len);

			// Slave msgs half frequency that master msgs
			rx_counter++;
			if (rx_counter >= 2) {
				rx_counter = 0;
				xmodem_send_slave_status_msg();
			}
			break;
		default:
			break;
		}

		PRINT_STACKTRACE(stack_trace);
	}
}
