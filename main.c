/*
 * main.c
 *
 *  Created on: Oct 9, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

// Totem SDK
#include "totem_sys.h"

// Services
#include "service_led.h"
#include "service_watchdog.h"

int main(void) {
	// System initialization
	totem_init();

#if DBG_STACKTRACE
	vTraceEnable(TRC_INIT);
#endif

	// LEDs service
	service_led_setup((const char *) "blink_led", TASK_PRIORITY_MEDIUM);

	// Watchdog service
	service_watchdog_setup((const char *) "watchdog", TASK_PRIORITY_HIGH);

	totem_start();

	return 0;
}
