/*
 * task_led.c
 *
 *  Created on: Oct 10, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#include "task_led.h"

#include "_stdio.h"
#include "trace.h"

#include "FreeRTOS.h"
#include "task.h"
#include "em_gpio.h"
#include "pinmap.h"

/**
 *
 */
void task_led_setup(void *args) {
	return;
}

/**
 * @brief	Simple task which blinks a led
 * @param 	*pParameters	Pointer to parameters passed to the function
 * @return	None
 */

void task_led_start(void *args) {
	uint8_t led_on = 0;

	traceString stackTrace = INIT_STACKTRACE("LED")
	;

	for (;;) {
		vTaskDelay(500 / portTICK_RATE_MS); //delay of 500ms

		GPIO_PinOutSet(PORT_LED_RED, PIN_LED_RED);
		GPIO_PinOutSet(PORT_LED_GREEN, PIN_LED_GREEN);
		GPIO_PinOutSet(PORT_LED_BLUE, PIN_LED_BLUE);

		if (!GPIO_PinInGet(PORT_PUSH0, PIN_PUSH0)) {
			GPIO_PinOutClear(PORT_LED_RED, PIN_LED_RED);
			PRINT("Change!");
		}

		if (led_on)
			GPIO_PinOutClear(PORT_LED_BLUE, PIN_LED_BLUE);

		led_on = !led_on;

		PRINT_STACKTRACE(stackTrace);
	}
}
