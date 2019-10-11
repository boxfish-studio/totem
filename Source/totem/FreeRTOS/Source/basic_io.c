///*
//    FreeRTOS V6.0.5 - Copyright (C) 2009 Real Time Engineers Ltd.
//
//    This file is part of the FreeRTOS distribution.
//
//    FreeRTOS is free software; you can redistribute it and/or modify it under
//    the terms of the GNU General Public License (version 2) as published by the
//    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
//    ***NOTE*** The exception to the GPL is included to allow you to distribute
//    a combined work that includes FreeRTOS without being obliged to provide the
//    source code for proprietary components outside of the FreeRTOS kernel.
//    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT
//    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//    more details. You should have received a copy of the GNU General Public
//    License and the FreeRTOS license exception along with FreeRTOS; if not it
//    can be viewed here: http://www.freertos.org/a00114.html and also obtained
//    by writing to Richard Barry, contact details for whom are available on the
//    FreeRTOS WEB site.
//
//    1 tab == 4 spaces!
//
//    http://www.FreeRTOS.org - Documentation, latest information, license and
//    contact details.
//
//    http://www.SafeRTOS.com - A version that is certified for use in safety
//    critical systems.
//
//    http://www.OpenRTOS.com - Commercial support, development, porting,
//    licensing and training services.
//*/
//#include <stdio.h>
//#include <string.h>
//
//#include "FreeRTOS.h"
//#include "task.h"
//#include "em_cmu.h"
//
//#define ioMAX_MSG_LEN	( 50 )
//
//void vPrintString( const char *pcString )
//{
//	/* Print the string, suspending the scheduler as method of mutual
//	exclusion. */
//	vTaskSuspendAll();
//	{
//	    int len = strlen(pcString);
//	    for (int i = 0; i < len; i++)
//	    {
//	       ITM_SendChar((uint32_t)pcString[i]);
//	    }
//	}
//	xTaskResumeAll();
//}
///*-----------------------------------------------------------*/
//
//void vSetupDebugPrint(void)
//{
//  /* Enable GPIO clock. */
//  CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_GPIO;
//
//  /* Enable Serial wire output pin */
//  GPIO->ROUTE |= GPIO_ROUTE_SWOPEN;
//
//#if defined(_EFM32_GIANT_FAMILY) || defined(_EFM32_LEOPARD_FAMILY) || defined(_EFM32_WONDER_FAMILY)
//  /* Set location 0 */
//  GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK)) | GPIO_ROUTE_SWLOCATION_LOC0;
//
//  /* Enable output on pin - GPIO Port F, Pin 2 */
//  GPIO->P[5].MODEL &= ~(_GPIO_P_MODEL_MODE2_MASK);
//  GPIO->P[5].MODEL |= GPIO_P_MODEL_MODE2_PUSHPULL;
//#else
//  /* Set location 1 */
//  GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK)) |GPIO_ROUTE_SWLOCATION_LOC1;
//  /* Enable output on pin */
//  GPIO->P[2].MODEH &= ~(_GPIO_P_MODEH_MODE15_MASK);
//  GPIO->P[2].MODEH |= GPIO_P_MODEH_MODE15_PUSHPULL;
//#endif
//
//  /* Enable debug clock AUXHFRCO */
//#if USE_INTERNAL_DBG_CLK == 1
//  CMU->OSCENCMD = CMU_OSCENCMD_AUXHFRCOEN;
//
//  /* Wait until clock is ready */
//  while (!(CMU->STATUS & CMU_STATUS_AUXHFRCORDY));
//#else
//  CMU->CTRL |= CMU_CTRL_DBGCLK_HFCLK;
//#endif
//
//  /* Enable trace in core debug */
//  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
//  ITM->LAR  = 0xC5ACCE55;
//  ITM->TER  = 0x0;
//  ITM->TCR  = 0x0;
//  TPI->SPPR = 2;
//  TPI->ACPR = 0xf;
//  ITM->TPR  = 0x0;
//  DWT->CTRL = 0x400003FE;
//  ITM->TCR  = 0x0001000D;
//  TPI->FFCR = 0x00000100;
//  ITM->TER  = 0x1;
//}
//
