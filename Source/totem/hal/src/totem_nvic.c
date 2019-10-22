/*
 * totem_nvic.c
 *
 *  Created on: Oct 10, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#include "totem_nvic.h"

#define EXTI_MAX    16

static void (*gpio_callbacks[EXTI_MAX])(void) = {0};

/**
 * @brief	Enable the interruptions used
 * @param	None
 * @return	None
 */
void init_interrupts() {
	NVIC_DisableIRQ(GPIO_EVEN_IRQn);
	NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
	NVIC_SetPriority(GPIO_EVEN_IRQn, 6);
	NVIC_EnableIRQ(GPIO_EVEN_IRQn);

	NVIC_DisableIRQ(GPIO_ODD_IRQn);
	NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
	NVIC_SetPriority(GPIO_ODD_IRQn, 6);
	NVIC_EnableIRQ(GPIO_ODD_IRQn);
}

void set_gpio_callback(uint8_t port, uint8_t pin, void (*callback)(void), uint8_t rising, uint8_t falling)
{
    gpio_callbacks[pin] = callback;
    GPIO_PinModeSet(port, pin, gpioModeInput, 0);
    GPIO_IntConfig(port, pin, rising, falling, 1);
}

/**
 * @brief	IRQ Handler for EVEN GPIO interrupts
 * @param	None
 * @return	None
 */
void GPIO_EVEN_IRQHandler() {
    uint32_t flags = GPIO_IntGet();
    for (int i = 0; i < EXTI_MAX; i += 2)
    {
        if (flags & (1 << i))
        {
            if (gpio_callbacks[i])
            {
                gpio_callbacks[i]();
            }
            GPIO_IntClear(1 << i);
        }
    }
}

/**
 * @brief	IRQ Handler for ODD GPIO interrupts
 * @param	None
 * @return	None
 */
void GPIO_ODD_IRQHandler() {
    uint32_t flags = GPIO_IntGet();
    for (int i = 1; i < EXTI_MAX; i += 2)
    {
        if (flags & (1 << i))
        {
            if (gpio_callbacks[i])
            {
                gpio_callbacks[i]();
            }
            GPIO_IntClear(1 << i);
        }
    }
}
