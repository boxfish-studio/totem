/*
 * 	main.c
 *
 *  Created on: Oct 9, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>, Agustin Tena <atena@boxfish.studio>
 */

// Totem SDK
#include <totem_sys.h>

// Services
#include <service_watchdog.h>
#include <service_led.h>
#include <service_usb_xmodem.h>
#include <service_usb_data_handler.h>

// Semaphores
xSemaphoreHandle sem_usb_transfer_done;
xSemaphoreHandle sem_xmodem_data_ready;

// Queues
xQueueHandle q_usb_in;           // USB
xQueueHandle q_xmodem_stack_in;  // XMODEM
xQueueHandle q_xmodem_stack_out; // XMODEM

int main(void) {
	// System initialization
	totem_init();

	// Semaphores initialization
	vSemaphoreCreateBinary(sem_usb_transfer_done);
	vTraceSetSemaphoreName(sem_usb_transfer_done, "sem_usb_transfer_done");
	vSemaphoreCreateBinary(sem_xmodem_data_ready);
	vTraceSetSemaphoreName(sem_xmodem_data_ready, "sem_xmodem_ready");

	// Queues initialization
	q_usb_in = xQueueCreate(10, sizeof(usb_data_packet_t));
	vTraceSetQueueName(q_usb_in, "q_usb_in");
	q_xmodem_stack_in = xQueueCreate(3, sizeof(xmodem_message_t));
	vTraceSetQueueName(q_xmodem_stack_in, "q_xmodem_stack_in");
	q_xmodem_stack_out = xQueueCreate(3, sizeof(xmodem_message_t));
	vTraceSetQueueName(q_xmodem_stack_out, "q_xmodem_stack_out");

	// LEDs service
	service_led_setup(LEDS_SERVICE_NAME, TASK_PRIORITY_MEDIUM);

	// Communications service
	service_usb_xmodem_setup(USB_XMODEM_SERVICE_NAME, TASK_PRIORITY_HIGH);
	service_usb_data_handler_setup(USB_DATA_HANDLER_SERVICE_NAME,
	TASK_PRIORITY_LOW);

	// Watchdog service
	service_watchdog_setup(WATCHDOG_SERVICE_NAME, TASK_PRIORITY_HIGH);

	totem_start();

	return 0;
}
