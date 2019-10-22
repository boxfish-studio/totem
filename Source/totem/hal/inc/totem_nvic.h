/*
 * totem_nvic.h
 *
 *  Created on: Oct 10, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#ifndef TOTEM_NVIC_H_
#define TOTEM_NVIC_H_

#include "totem_common.h"

void init_interrupts();
void set_gpio_callback(uint8_t port, uint8_t pin, void (*callback)(void), uint8_t rising, uint8_t falling);

#endif /* TOTEM_NVIC_H_ */
