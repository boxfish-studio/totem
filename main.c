/*
 * main.c
 *
 *  Created on: Oct 9, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#include "totem_sys.h"
#include "FreeRTOS.h"

// Tasks
#include "task_led.h"
#include "task_watchdog.h"
#include "task_ex.h"

xTaskHandle handle_led;
xTaskHandle handle_watchdog;

int main(void)
{
    // System initialization
	init_system();

    // LEDs
    xTaskCreate(task_led, (const char *) "ledblink", 150, NULL, TASK_PRIORITY_MEDIUM, &handle_led);

    // Watchdog
#if WATCHDOG_ACTIVE
    xTaskCreate(task_watchdog, (const char *) "watchdog", 400, NULL, TASK_PRIORITY_HIGH, &handle_watchdog);
#endif

    // Start FreeRTOS Scheduler
    vTaskStartScheduler();

    return 0;
}
