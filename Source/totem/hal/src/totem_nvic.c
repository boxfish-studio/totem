/*
 * totem_nvic.c
 *
 *  Created on: Oct 10, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#include "totem_nvic.h"

#include "em_device.h"
#include "em_gpio.h"
#include "pinmap.h"

void init_interrupts(void)
{
    NVIC_DisableIRQ(GPIO_EVEN_IRQn);
    NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
    NVIC_SetPriority (GPIO_EVEN_IRQn, 6);
    NVIC_EnableIRQ(GPIO_EVEN_IRQn);

    NVIC_DisableIRQ(GPIO_ODD_IRQn);
    NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
    NVIC_SetPriority (GPIO_ODD_IRQn, 4);
    NVIC_EnableIRQ(GPIO_ODD_IRQn);
}

/**
 * @brief IRQ Handler for EVEN GPIO interrupts
 * @param None
 * @return None
 */
void GPIO_EVEN_IRQHandler(void)
{
	GPIO_PinOutClear(PORT_LED_GREEN, PIN_LED_GREEN);
    GPIO_IntClear(GPIO_IntGet());
}

/**
 * @brief IRQ Handler for ODD GPIO interrupts
 * @param None
 * @return None
 */
void GPIO_ODD_IRQHandler(void)
{
	GPIO_PinOutClear(PORT_LED_GREEN, PIN_LED_GREEN);
    GPIO_IntClear(GPIO_IntGet());
}
