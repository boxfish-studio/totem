/*
 * totem_sys.h
 *
 *  Created on: Oct 10, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#ifndef TOTEM_SYS_H_
#define TOTEM_SYS_H_

#include "em_rmu.h"

#include "FreeRTOS.h"
#include "task.h"

#include "totem_watchdog.h"

void init_system(void);

#endif /* TOTEM_SYS_H_ */
