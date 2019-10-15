/*
    FreeRTOS V8.2.2 - Copyright (C) 2015 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "em_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

/********************** Configuration of FreeRTOS ****************************/

/* Implement FreeRTOS configASSERT as emlib assert */
#define configASSERT( x )       EFM_ASSERT( x )

/* Modes of operations of operation system*/
#define configUSE_PREEMPTION       ( 1 )

/* Energy saving modes */
#define configUSE_TICKLESS_IDLE    ( 0 )
/* Available options when configUSE_TICKLESS_IDLE set to 1
 * or configUSE_SLEEP_MODE_IN_IDLE set to 1 :
 * 1 - EM1, 2 - EM2, 3 - EM3, timer in EM3 is not very accurate*/
#define configSLEEP_MODE           ( 3 )
/* Definition used only if configUSE_TICKLESS_IDLE == 0 */
#define configUSE_SLEEP_MODE_IN_IDLE       ( 0 )


/* EM1 use systick as system clock*/
/* EM2 use crystal 32768Hz and RTC Component as system clock
 * We use 2 times divider of this clock to reduce energy consumtion
 * You can also in this mode choose crystal oscillator to get more preccision in
 * time measurement or RC oscillator for more energy reduction.*/
/* EM3 use 2kHz RC and BURTC Component as system clock*/
#if ( ( configSLEEP_MODE == 2 ) && ( configUSE_TICKLESS_IDLE == 1 || configUSE_SLEEP_MODE_IN_IDLE == 1 ) )
/* Choose source of clock for RTC (system tick)
 * if configCRYSTAL_IN_EM2 set to 1 then Crystal oscillator is used,
 * when 0 RC oscillator */
#define configCRYSTAL_IN_EM2    ( 1 )
#endif
#if (  (configSLEEP_MODE == 2 ) && ( configUSE_TICKLESS_IDLE == 1 || configUSE_SLEEP_MODE_IN_IDLE == 1 ) )
/* When we use EM2 or EM3 System clock has got low frequency,
 * so we reduce Tick rate to 100 Hz and 40 Hz, which give more clock cycles between ticks*/
#define configTICK_RATE_HZ    ( ( TickType_t ) 100 )
#elif (  ( configSLEEP_MODE == 3 ) && ( configUSE_TICKLESS_IDLE == 1 || configUSE_SLEEP_MODE_IN_IDLE == 1 ) )
#define configTICK_RATE_HZ    ( ( TickType_t ) 40 )
#else
#define configTICK_RATE_HZ    ( ( TickType_t ) 1000 )
#endif

/* Definition used by Keil to replace default system clock source when we use EM2 or EM3 mode. */
#if ( ( configSLEEP_MODE == 2 || configSLEEP_MODE == 3 ) && ( configUSE_TICKLESS_IDLE == 1 || configUSE_SLEEP_MODE_IN_IDLE == 1 ) )
#define configOVERRIDE_DEFAULT_TICK_CONFIGURATION ( 1 )
#endif
/* Main functions*/
#define configMAX_PRIORITIES                      (32)
#define configMINIMAL_STACK_SIZE                  (( unsigned short ) 140)

#define configTOTAL_HEAP_SIZE                     (( size_t )(90000))
//Smaller heap size for logging
//#define configTOTAL_HEAP_SIZE                     (( size_t )(30100))

/* Allows pre FreeRTOS 8 naming */
#define configENABLE_BACKWARD_COMPATIBILITY         1

#define configMAX_TASK_NAME_LEN                   ( 10 )
#define configUSE_TRACE_FACILITY                  ( 1 )
#define configUSE_16_BIT_TICKS                    ( 0 )
#define configIDLE_SHOULD_YIELD                   ( 0 )
#define configUSE_MUTEXES                         ( 1 )
#define configUSE_RECURSIVE_MUTEXES               ( 0 )
#define configUSE_COUNTING_SEMAPHORES             ( 0 )
#define configUSE_ALTERNATIVE_API                 ( 0 )/* Deprecated! */
#define configQUEUE_REGISTRY_SIZE                 ( 10 )
#define configUSE_QUEUE_SETS                      ( 0 )

/* Hook function related definitions. */
#define configUSE_TICK_HOOK                       ( 0 )
#define configCHECK_FOR_STACK_OVERFLOW            ( 0 )
#define configUSE_MALLOC_FAILED_HOOK              ( 0 )

/* Run time stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS             ( 0 )

/* Co-routine related definitions. */
#define configUSE_CO_ROUTINES                     ( 0 )
#define configMAX_CO_ROUTINE_PRIORITIES           ( 1 )

/* Software timer related definitions. */
#define configUSE_TIMERS                          ( 1 )
#define configTIMER_TASK_PRIORITY                 ( configMAX_PRIORITIES - 1 ) /* Highest priority */
#define configTIMER_QUEUE_LENGTH                  ( 10 )
#define configTIMER_TASK_STACK_DEPTH              ( configMINIMAL_STACK_SIZE )

/* Interrupt nesting behaviour configuration. */
#define configKERNEL_INTERRUPT_PRIORITY           ( 255 ) /* Priority 7 */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY      ( 191 ) /* equivalent to 0xa0, or priority 5. */

/* Optional functions - most linkers will remove unused functions anyway. */
#define INCLUDE_vTaskPrioritySet                  ( 1 )
#define INCLUDE_uxTaskPriorityGet                 ( 1 )
#define INCLUDE_vTaskDelete                       ( 1 )
#define INCLUDE_vTaskSuspend                      ( 1 )
#define INCLUDE_xResumeFromISR                    ( 1 )
#define INCLUDE_vTaskDelayUntil                   ( 1 )
#define INCLUDE_vTaskDelay                        ( 1 )
#define INCLUDE_xTaskGetSchedulerState            ( 1 )
#define INCLUDE_xTaskGetCurrentTaskHandle         ( 1 )
#define INCLUDE_uxTaskGetStackHighWaterMark       ( 1 )
#define INCLUDE_xTaskGetIdleTaskHandle            ( 0 )
#define INCLUDE_xTimerGetTimerDaemonTaskHandle    ( 0 )
#define INCLUDE_pcTaskGetTaskName                 ( 0 )
#define INCLUDE_eTaskGetState                     ( 0 )

/* Default value of CPU clock (RC)*/
//#define configCPU_CLOCK_HZ                        (( unsigned long ) 14000000)
#define configCPU_CLOCK_HZ                        (( unsigned long ) 48000000) // Running at 48 MHz

/* Defines used in energy modes */
#if ( ( configSLEEP_MODE == 2 )  && ( ( configUSE_SLEEP_MODE_IN_IDLE == 1 ) || ( configUSE_TICKLESS_IDLE == 1 ) ) )
        #define configSYSTICK_CLOCK_HZ    ( 16384 )
#endif

#if ( ( configSLEEP_MODE == 3 )  && ( ( configUSE_SLEEP_MODE_IN_IDLE == 1 ) || ( configUSE_TICKLESS_IDLE == 1 ) ) )
       #define configSYSTICK_CLOCK_HZ    ( 2000 )
#endif

#if ( ( configUSE_TICKLESS_IDLE == 0 ) && ( configUSE_SLEEP_MODE_IN_IDLE == 1 ) )
#define configUSE_IDLE_HOOK  ( 1 )
#else
#define configUSE_IDLE_HOOK  ( 0 )
#endif

/*-----------------------------------------------------------*/


/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
 * standard names. */
#define vPortSVCHandler        SVC_Handler
#define xPortPendSVHandler     PendSV_Handler
#define xPortSysTickHandler    SysTick_Handler

#if configUSE_TRACE_FACILITY
	#include "trcRecorder.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* FREERTOS_CONFIG_H */
