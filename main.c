/*
 * main.c
 *
 *  Created on: Oct 9, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>, Agustin Tena <atena@boxfish.studio>
 */

// Totem SDK
#include "totem_sys.h"

// Services
#include "service_can_controller.h"
#include "service_can_data_handler.h"
#include "service_led.h"
#include "service_watchdog.h"

// Semaphores
xSemaphoreHandle sem_can;

// Queues
xQueueHandle q_can_handle;

int main(void) {
	// System initialization
	totem_init();

	// Semaphores initialization
	vSemaphoreCreateBinary(sem_can);
	vTraceSetSemaphoreName(sem_can, "sem_can");

	// Queues initialization
	q_can_handle = xQueueCreate(5, sizeof(CAN_Queue_t));
	vTraceSetQueueName(q_can_handle, "q_can_handle");

	// LEDs service
	service_led_setup(LEDS_SERVICE_NAME, TASK_PRIORITY_MEDIUM);

	// CAN services
	service_can_data_handler_setup(CAN_DATA_HANDLER_SERVICE_NAME, TASK_PRIORITY_MEDIUM);
	service_can_controller_setup(CAN_CONTROLLER_SERVICE_NAME, TASK_PRIORITY_MEDIUM);

	// Watchdog service
	service_watchdog_setup(WATCHDOG_SERVICE_NAME, TASK_PRIORITY_HIGH);

	totem_start();

	return 0;
}
