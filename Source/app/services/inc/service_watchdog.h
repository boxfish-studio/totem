/*
 * service_watchdog.h
 *
 *  Created on: Oct 11, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#ifndef SERVICE_WATCHDOG_H_
#define SERVICE_WATCHDOG_H_

#include "totem_sys.h"

#define WATCHDOG_ENABLED 1

void service_watchdog_start(const char * const, UBaseType_t uxPriority);
void service_watchdog(void *args);

#endif /* SERVICE_WATCHDOG_H_ */
