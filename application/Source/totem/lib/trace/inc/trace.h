/*
 * trace.h
 *
 *  Created on: Oct 11, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#ifndef TRACE_H_
#define TRACE_H_

#include "trcRecorder.h"

#if TRACE_ENABLED
    #define INIT_STACKTRACE(task_name) xTraceRegisterString("stack_" task_name);
	#define INIT_DRIVERTRACE(driver_name) xTraceRegisterString("driver_" driver_name);
    #define PRINT_STACKTRACE(stack_trace) vTracePrintF(stack_trace, "%d", uxTaskGetStackHighWaterMark(NULL));
    #define PRINT_DRIVERTRACE(driver_trace, mssg, value) vTracePrintF(driver_trace, mssg, value);
#else
    #define INIT_STACKTRACE(task_name) NULL
    #define INIT_DRIVERTRACE(driver_name) NULL
    #define PRINT_STACKTRACE(stack_trace)
    #define PRINT_DRIVERTRACE(driver_trace, mssg, value)
#endif

#endif /* TRACE_H_ */
