/*
 * totem_sys.h
 *
 *  Created on: Oct 10, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#ifndef TOTEM_SYS_H_
#define TOTEM_SYS_H_

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "pinmap.h"
#include "_stdio.h"

#include "totem_nvic.h"
#include "totem_sleep.h"
#include "totem_watchdog.h"

#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_rmu.h"

void totem_init();
void totem_start();

#define STACK_SIZE_FOR_TASK    (configMINIMAL_STACK_SIZE + 10)

/* Defines for task priorities */
#define TASK_PRIORITY_LOW           (tskIDLE_PRIORITY + 1)
#define TASK_PRIORITY_MEDIUM        (tskIDLE_PRIORITY + 2)
#define TASK_PRIORITY_HIGH          (tskIDLE_PRIORITY + 3)
#define TASK_PRIORITY_ULTRA_HIGH    (tskIDLE_PRIORITY + 4)

#endif /* TOTEM_SYS_H_ */
