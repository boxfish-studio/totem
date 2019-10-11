/*
 * _stdio.c
 *
 *  Created on: Oct 11, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#include "_stdio.h"

#include "em_device.h"

#define USE_INTERNAL_DBG_CLK 0

/**
 * @brief	Prints in console the string pass in argument
 * @param	*pcString	string to print
 * @return	None
 */
void swd_print(const char *pcString)
{
	while(*pcString != 0x00) {
	   ITM_SendChar((uint32_t) *pcString);
	   pcString++;
	}
}

/**
 * @brief	Setups the registers to print in the console with SWD interface
 * @param	None
 * @return	None
 */
void setup_swd_print(void)
{
	/* Enable GPIO clock. */
	CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_GPIO;

	/* Enable Serial wire output pin */
	GPIO->ROUTE |= GPIO_ROUTE_SWOPEN;

#if defined(_EFM32_GIANT_FAMILY) || defined(_EFM32_LEOPARD_FAMILY) || defined(_EFM32_WONDER_FAMILY)
	/* Set location 0 */
	GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK)) | GPIO_ROUTE_SWLOCATION_LOC0;

	/* Enable output on pin - GPIO Port F, Pin 2 */
	GPIO->P[5].MODEL &= ~(_GPIO_P_MODEL_MODE2_MASK);
	GPIO->P[5].MODEL |= GPIO_P_MODEL_MODE2_PUSHPULL;

#else
	/* Set location 1 */
	GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK)) |GPIO_ROUTE_SWLOCATION_LOC1;
	/* Enable output on pin */
	GPIO->P[2].MODEH &= ~(_GPIO_P_MODEH_MODE15_MASK);
	GPIO->P[2].MODEH |= GPIO_P_MODEH_MODE15_PUSHPULL;
#endif

	/* Enable debug clock AUXHFRCO */
#if USE_INTERNAL_DBG_CLK == 1
	CMU->OSCENCMD = CMU_OSCENCMD_AUXHFRCOEN;

	/* Wait until clock is ready */
	while (!(CMU->STATUS & CMU_STATUS_AUXHFRCORDY));

#else
	CMU->CTRL |= CMU_CTRL_DBGCLK_HFCLK;
#endif

	/* Enable trace in core debug */
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	ITM->LAR  = 0xC5ACCE55;
	ITM->TER  = 0x0;
	ITM->TCR  = 0x0;
	TPI->SPPR = 2;
	TPI->ACPR = 0xf;
	ITM->TPR  = 0x0;
	DWT->CTRL = 0x400003FE;
	ITM->TCR  = 0x0001000D;
	TPI->FFCR = 0x00000100;
	ITM->TER  = 0x1;
}
