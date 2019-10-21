/*
 * main.c
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
xSemaphoreHandle sem_ISR_USB_transfer_done;
xSemaphoreHandle sem_activate_xmodem_communicator;

// Queues
xQueueHandle queue_xmodem_communicator_in;  // XMODEM
xQueueHandle queue_xmodem_communicator_out; // XMODEM
xQueueHandle queue_usb_in;                  // USB

int main(void) {
	// System initialization
	totem_init();

    // Semaphores initialization
    vSemaphoreCreateBinary(sem_ISR_USB_transfer_done);
    vTraceSetSemaphoreName(sem_ISR_USB_transfer_done, "sem_ISR_USB_transfer_done");
    vSemaphoreCreateBinary(sem_activate_xmodem_communicator);
    vTraceSetSemaphoreName(sem_activate_xmodem_communicator, "sem_activate_xmodem_communicator");

    // Queues initialization
    queue_xmodem_communicator_in    = xQueueCreate(3, sizeof(xModemCommunicator_t));
    vTraceSetQueueName(queue_xmodem_communicator_in, "queue_xmodem_communicator_in");
    queue_xmodem_communicator_out   = xQueueCreate(3, sizeof(xModemCommunicator_t));
    vTraceSetQueueName(queue_xmodem_communicator_out, "queue_xmodem_communicator_out");
    queue_usb_in                    = xQueueCreate(10, sizeof(usbInData_t));
    vTraceSetQueueName(queue_usb_in, "queue_usb_in");

	// LEDs service
	service_led_setup(LEDS_SERVICE_NAME, TASK_PRIORITY_MEDIUM);

	// Communications service
	service_usb_xmodem_setup(USB_XMODEM_SERVICE_NAME, TASK_PRIORITY_HIGH);
	service_usb_data_handler_setup(USB_DATA_HANDLER_SERVICE_NAME, TASK_PRIORITY_LOW);

	// Watchdog service
	service_watchdog_setup(WATCHDOG_SERVICE_NAME, TASK_PRIORITY_HIGH);

	totem_start();

	return 0;
}
