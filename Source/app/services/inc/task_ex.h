/*
 * task_ex.h
 *
 *  Created on: Oct 10, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#ifndef TASK_EX_H_
#define TASK_EX_H_

// RTOS related includes
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#define STACK_SIZE_FOR_TASK    (configMINIMAL_STACK_SIZE + 10)

/* Defines for task priorities */
#define TASK_PRIORITY_LOW           (tskIDLE_PRIORITY + 1)
#define TASK_PRIORITY_MEDIUM        (tskIDLE_PRIORITY + 2)
#define TASK_PRIORITY_HIGH          (tskIDLE_PRIORITY + 3)
#define TASK_PRIORITY_ULTRA_HIGH    (tskIDLE_PRIORITY + 4)

#endif /* TASKS_EX_H_ */
