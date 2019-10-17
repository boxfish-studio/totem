/*
 * service_watchdog.c
 *
 *  Created on: Oct 11, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>, Agustin Tena <atena@boxfish.studio>
 */

#include "service_watchdog.h"

#define INITIAL_TIMEOUT	wdg_8_s

#define NEXT_CLEAR		(1 << (3+INITIAL_TIMEOUT))/4	// Wait a fourth part of timeout watchdog

static xTaskHandle handle_service_watchdog;

void service_watchdog_setup(const char * const service_name,
		UBaseType_t service_priority) {

#if WATCHDOG_ENABLED
	xTaskCreate(service_watchdog, service_name, 400, NULL, service_priority, &handle_service_watchdog);
#endif
}

/**
 * @brief	Simple service that initialize and feeds the watchdog peripheral
 * @param	*args	Arguments used to receive variables
 * @return	None
 */
void service_watchdog(void *args) {

	portTickType xFirstTime, xLastWakeTime;

	traceString stackTrace = INIT_STACKTRACE(WATCHDOG_SERVICE_NAME);

	RMU_ResetCauseClear();

	wdg_init(INITIAL_TIMEOUT);

	for (;;) {

		xFirstTime = xTaskGetTickCount();
		wdg_clear();
		xLastWakeTime = xTaskGetTickCount();

		vTaskDelay(
				(NEXT_CLEAR / portTICK_RATE_MS) - (xLastWakeTime - xFirstTime));

		PRINT_STACKTRACE(stackTrace);
	}
}
