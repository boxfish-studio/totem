/*
 * service_led.c
 *
 *  Created on: Oct 10, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#include "service_led.h"

static xTaskHandle handle_led;

/**
 *
 */
void service_led_setup(const char * service_name, UBaseType_t service_priority)
{
	xTaskCreate(service_led, service_name, 150, NULL, service_priority, &handle_led);
}

/**
 * @brief	Simple task which blinks a led
 * @param 	*pParameters	Pointer to parameters passed to the function
 * @return	None
 */

void service_led(void *args) {
	uint8_t led_on = 0;

	traceString stackTrace = INIT_STACKTRACE("LED");

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
