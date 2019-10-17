/*
 * trace.h
 *
 *  Created on: Oct 11, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#ifndef TRACE_H_
#define TRACE_H_

#if TRACE_ENABLED
    #define INIT_STACKTRACE(task_name) xTraceRegisterString("stack_" task_name);
    #define PRINT_STACKTRACE(stack_trace) vTracePrintF(stackTrace, "%d", uxTaskGetStackHighWaterMark(NULL));
#else
    #define INIT_STACKTRACE(task_name) NULL
    #define PRINT_STACKTRACE(stack_trace)
#endif

#endif /* TRACE_H_ */
