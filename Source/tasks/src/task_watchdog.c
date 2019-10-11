/*
 * task_watchdog.c
 *
 *  Created on: Oct 11, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#include "task_watchdog.h"

#include "totem_watchdog.h"
#include "FreeRTOS.h"
#include "task.h"

#include "em_rmu.h"

#define INITIAL_TIMEOUT	wdg_8_s

#define NEXT_CLEAR		(1 << (3+INITIAL_TIMEOUT))/4	// Wait a fourth part of timeout watchdog

/**
 * @brief	Watchdog used to force a reset if there is a hard fault
 * @param	*args	Arguments used to receive variables
 * @return	None
 */
void task_watchdog(void *args)
{

    portTickType xFirstTime, xLastWakeTime;

    RMU_ResetCauseClear();

	wdg_init(INITIAL_TIMEOUT);

	for(;;) {

		xFirstTime = xTaskGetTickCount();
		wdg_clear();
		xLastWakeTime = xTaskGetTickCount();

		vTaskDelay((NEXT_CLEAR / portTICK_RATE_MS) - (xLastWakeTime - xFirstTime));

	}
}
