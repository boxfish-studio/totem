/*
 * main.c
 *
 *  Created on: Oct 9, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>, Agustin Tena <atena@boxfish.studio>
 */

// Totem SDK
#include "totem_sys.h"

// Services
#include "service_led.h"
#include "service_watchdog.h"

int main(void) {
	// System initialization
	totem_init();

	// LEDs service
	service_led_setup(LEDS_SERVICE_NAME, TASK_PRIORITY_MEDIUM);

	// Watchdog service
	service_watchdog_setup(WATCHDOG_SERVICE_NAME, TASK_PRIORITY_HIGH);

	totem_start();

	return 0;
}
