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
#include <service_can_controller.h>
#include <service_can_data_handler.h>

// Semaphores
xSemaphoreHandle sem_usb;
xSemaphoreHandle sem_xmodem;
xSemaphoreHandle sem_can;

// Queues
xQueueHandle q_usb_in;           // USB
xQueueHandle q_xmodem_stack_in;  // XMODEM
xQueueHandle q_xmodem_stack_out; // XMODEM
xQueueHandle q_can_handle;		 // CAN

int main(void) {
	// System initialization
	totem_init();

	// Semaphores initialization
	vSemaphoreCreateBinary(sem_usb);
	vTraceSetSemaphoreName(sem_usb, "sem_usb");
	vSemaphoreCreateBinary(sem_xmodem);
	vTraceSetSemaphoreName(sem_xmodem, "sem_xmodem");
	vSemaphoreCreateBinary(sem_can);
	vTraceSetSemaphoreName(sem_can, "sem_can");

	// Queues initialization
	q_usb_in = xQueueCreate(10, sizeof(usb_data_packet_t));
	vTraceSetQueueName(q_usb_in, "q_usb_in");
	q_xmodem_stack_in = xQueueCreate(3, sizeof(xmodem_message_t));
	vTraceSetQueueName(q_xmodem_stack_in, "q_xmodem_stack_in");
	q_xmodem_stack_out = xQueueCreate(3, sizeof(xmodem_message_t));
	vTraceSetQueueName(q_xmodem_stack_out, "q_xmodem_stack_out");
	q_can_handle = xQueueCreate(5, sizeof(CAN_Queue_t));
	vTraceSetQueueName(q_can_handle, "q_can_handle");

	// LEDs service
	service_led_setup(LEDS_SERVICE_NAME, TASK_PRIORITY_MEDIUM);

	// CAN services
	service_can_data_handler_setup(CAN_DATA_HANDLER_SERVICE_NAME, TASK_PRIORITY_MEDIUM);
	service_can_controller_setup(CAN_CONTROLLER_SERVICE_NAME, TASK_PRIORITY_MEDIUM);

	// USB services
	service_usb_xmodem_setup(USB_XMODEM_SERVICE_NAME, TASK_PRIORITY_HIGH);
	service_usb_data_handler_setup(USB_DATA_HANDLER_SERVICE_NAME, TASK_PRIORITY_LOW);

	// Watchdog service
	service_watchdog_setup(WATCHDOG_SERVICE_NAME, TASK_PRIORITY_HIGH);

	totem_start();

	return 0;
}
