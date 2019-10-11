/*
 * totem_sys.c
 *
 *  Created on: Oct 10, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#include "totem_sys.h"

#include "em_cmu.h"
#include "em_gpio.h"
#include "em_chip.h"
#include "totem_nvic.h"
#include "totem_sleep.h"

#include "FreeRTOSConfig.h"

#include "_stdio.h"

#include "pinmap.h"

static void set_core_speed(void);
static void enable_clocks(void);

static void setup_gpio(void);
static inline void setup_gpio_leds(void);
static inline void setup_gpio_pushbuttons(void);

/**
 * @brief	Initializes the system during boot
 */
void init_system(void)
{
    // Internal function
    CHIP_Init();

    // Configure SWD print for debug
#if DEBUG_PRINT
	setup_swd_print();
#endif

    // Enable the needed clocks
    enable_clocks();

    // Enable GPIOs
    setup_gpio();

    // Set core speed to 48 MHz
    set_core_speed();

    // Initialize the interrupts (set priorities etc.)
    init_interrupts();

    // Initialize SLEEP driver, no calbacks are used
    SLEEP_Init(NULL, NULL);
#if (configSLEEP_MODE < 3)
    /* do not let to sleep deeper than define */
    SLEEP_SleepBlockBegin((SLEEP_EnergyMode_t)(configSLEEP_MODE+1));
#endif
}

/**
 * @brief	Enables the clocks used in the system
 */
void enable_clocks(void)
{
    // Enable GPIO clock
    CMU_ClockEnable(cmuClock_GPIO, true);
    // Enable ADC Clock for on-Chip Temp Sensor
    CMU_ClockEnable(cmuClock_ADC0, true);
    /* Enabling clock to the interface of the low energy modules (including the Watchdog)*/
    CMU_ClockEnable(cmuClock_CORELE, true);
    /*Enabling High Frequency peripherical clock->for USART*/
    CMU_ClockEnable(cmuClock_HFPER, true);
    /* Enabling clock to USART and GPIO */
    CMU_ClockEnable(cmuClock_USART1,true);
    // Enabled EBI clock
    //CMU_ClockEnable(cmuClock_EBI, true);

    /* External Low Frequency Clock (32,768KHz) does not boot up when PCB coated */
    CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
    /* Enabling internal RC Low Frequency Clock (32,768KHz) */
    //CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);
    /* Enabling internal Ultra Low Frequency RC Clock (1kHz) */
    CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true);
    /* Enabling external High Frequency Clock (48MHz) */
    CMU_OscillatorEnable(cmuOsc_HFXO, true, true);

}

static void set_core_speed(void)
{
    // Set up core to 48MHz
    CMU_ClockDivSet(cmuClock_HF, cmuClkDiv_1);
    CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
}


static void setup_gpio(void)
{
    // Setup LED pins
    setup_gpio_leds();

    // Setup Pushbutton pins
    setup_gpio_pushbuttons();

}

static inline void setup_gpio_leds(void)
{
    //RGB LED
    GPIO_PinModeSet(PORT_LED_RED, PIN_LED_RED ,  gpioModePushPull, 1);		//red
    GPIO_PinModeSet(PORT_LED_BLUE, PIN_LED_BLUE,  gpioModePushPull, 1);    //blue
    GPIO_PinModeSet(PORT_LED_GREEN, PIN_LED_GREEN,  gpioModePushPull, 1);  //green
}

static inline void setup_gpio_pushbuttons(void)
{
    //push buttons
    GPIO_PinModeSet(PORT_PUSH0, PIN_PUSH0, gpioModeInputPullFilter, 1);   //push0
    GPIO_PinModeSet(PORT_PUSH1, PIN_PUSH1,  gpioModeInputPullFilter, 1);   //push1

    GPIO_IntConfig(PORT_PUSH1, PIN_PUSH1, 0, 1, 1);

}
