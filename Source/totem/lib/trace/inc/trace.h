/*
 * trace.h
 *
 *  Created on: Oct 11, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#ifndef TRACE_H_
#define TRACE_H_

#if DBG_STACKTRACE
    #define INIT_STACKTRACE(task_name) xTraceRegisterString("stack_" task_name);
    #define PRINT_STACKTRACE(stackTrace) vTracePrintF(stackTrace, "%d", uxTaskGetStackHighWaterMark(NULL));
#else
    #define INIT_STACKTRACE(task_name) NULL
    #define PRINT_STACKTRACE(task_name)
#endif

#endif /* TRACE_H_ */
