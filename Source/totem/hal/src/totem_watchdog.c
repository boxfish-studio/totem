/*
 * totem_watchdog.c
 *
 *  Created on: Oct 11, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#include "totem_watchdog.h"

static inline void wdg_start(const WDOG_Init_TypeDef *init);

/**
 * @brief	Initialization of watchdog hardware peripheral
 * @param	period	Period used as timeout in the watchdog timer. It can be one
 * 					preloaded in the header file based in the ULFR Clock
 * @return	None
 */
void wdg_init(WDG_Period_t period) {
	WDOG_Init_TypeDef wdg_init = {
	true, /* Start watchdog when init done */
	false, /* WDOG not counting during debug halt */
	false, /* WDOG not counting when in EM2 */
	false, /* WDOG not counting when in EM3 */
	false, /* EM4 can be entered */
	true, /* Block disabling LFRCO/LFXO in CMU */
	false, /* Do not lock WDOG configuration (if locked, reset needed to unlock) */
	wdogClkSelULFRCO, /* Select 1kHZ WDOG oscillator */
	period /* Set longest possible timeout period */
	};

	wdg_start(&wdg_init);
}

/**
 * @brief	Clears the watchdog timer
 * @param	None
 * @return	None
 */
void wdg_clear(void) {
	WDOG_Feed();
}

/**
 * @brief	Changes the period used as timeout by the watchdog timer
 * @param	period	Period used as timeout in the watchdog timer. It can be one
 * 					preloaded in the header file based in the ULFR Clock
 * @return	None
 */
void wdg_changePeriod(WDG_Period_t period) {
	WDOG_Enable(false);

	WDOG_Init_TypeDef wdg_init = {
	true, /* Start watchdog when init done */
	false, /* WDOG not counting during debug halt */
	false, /* WDOG not counting when in EM2 */
	false, /* WDOG not counting when in EM3 */
	false, /* EM4 can be entered */
	false, /* Do not block disabling LFRCO/LFXO in CMU */
	false, /* Do not lock WDOG configuration (if locked, reset needed to unlock) */
	wdogClkSelULFRCO, /* Select 1kHZ WDOG oscillator */
	period /* Set longest possible timeout period */
	};

	wdg_start(&wdg_init);
}

static inline void wdg_start(const WDOG_Init_TypeDef *init) {
	WDOG_Init(init);
	WDOG_Feed();
}
