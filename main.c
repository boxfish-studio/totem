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
#include "task_ex.h"

xTaskHandle handle_led;

int main(void)
{
    // System initialization
	init_system();

    // LEDs
    xTaskCreate(task_led, (const char *) "ledblink", 150, NULL, TASK_PRIORITY_MEDIUM, &handle_led);

    // Start FreeRTOS Scheduler
    vTaskStartScheduler();

    return 0;
}
