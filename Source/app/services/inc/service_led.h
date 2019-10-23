/*
 * service_led.h
 *
 *  Created on: Oct 10, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#ifndef SERVICE_LED_H_
#define SERVICE_LED_H_

#include "totem_sys.h"

#define LEDS_SERVICE_NAME	"leds"

void service_led_setup(const char * service_name, UBaseType_t service_priority);
void service_led(void *args);

#endif /* SERVICE_LED_H_ */
