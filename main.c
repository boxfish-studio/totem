/*
 * main.c
 *
 *  Created on: Oct 9, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

// Totem SDK
#include "totem_sys.h"

// Services
#include "service_watchdog.h"
#include "task_led.h"

xTaskHandle handle_led;

int main(void) {
	// System initialization
	totem_init();

	// LEDs service
	xTaskCreate(task_led, (const char *) "ledblink", 150, NULL,
			TASK_PRIORITY_MEDIUM, &handle_led);

	// Watchdog service
	service_watchdog_setup((const char *) "watchdog", TASK_PRIORITY_HIGH);

	totem_start();

	return 0;
}
