/*
 * totem_watchdog.h
 *
 *  Created on: Oct 11, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#ifndef TOTEM_WATCHDOG_H_
#define TOTEM_WATCHDOG_H_

#include "em_wdog.h"

typedef enum {
	wdg_9_ms = wdogPeriod_9,
	wdg_17_ms = wdogPeriod_17,
	wdg_33_ms = wdogPeriod_33,
	wdg_65_ms = wdogPeriod_65,
	wdg_129_ms = wdogPeriod_129,
	wdg_257_ms = wdogPeriod_257,
	wdg_513_ms = wdogPeriod_513,
	wdg_1_s = wdogPeriod_1k,
	wdg_2_s = wdogPeriod_2k,
	wdg_4_s = wdogPeriod_4k,
	wdg_8_s = wdogPeriod_8k,
	wdg_16_s = wdogPeriod_16k,
	wdg_32_s = wdogPeriod_32k,
	wdg_1_min = wdogPeriod_64k,
	wdg_2_min = wdogPeriod_128k,
	wdg_4_min = wdogPeriod_256k,
} WDG_Period_t;

void wdg_init(WDG_Period_t period);

void wdg_clear(void);
void wdg_changePeriod(WDG_Period_t period);

#endif /* TOTEM_WATCHDOG_H_ */
