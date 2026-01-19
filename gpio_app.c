/*
 * gpio.c
 *
 *  Created on: 31 de dez. de 2025
 *      Author: vinicius.andrade
 */

#include "pin_mux.h"
#include "fsl_gpio.h"
#include "gpio_app.h"

typedef struct {
    GPIO_Type *gpio;
    uint32_t port;
    uint32_t pin;
} led_hw_t;

static const led_hw_t leds[S_COUNT] = {
    [S1] = { BOARD_INITPINS_LED_S1_GPIO, BOARD_INITPINS_LED_S1_PORT, BOARD_INITPINS_LED_S1_PIN },
    [S2] = { BOARD_INITPINS_LED_S2_GPIO, BOARD_INITPINS_LED_S2_PORT, BOARD_INITPINS_LED_S2_PIN },
    [S3] = { BOARD_INITPINS_LED_S3_GPIO, BOARD_INITPINS_LED_S3_PORT, BOARD_INITPINS_LED_S3_PIN },
    [S4] = { BOARD_INITPINS_LED_S4_GPIO, BOARD_INITPINS_LED_S4_PORT, BOARD_INITPINS_LED_S4_PIN },
};

int led_ctrl(button_led_id_t button, led_state_t state)
{
    if (button >= S_COUNT || state >= LED_STATE_COUNT)
        return (-1);

    GPIO_PinWrite(
        leds[button].gpio,
        leds[button].port,
        leds[button].pin,
        (state == LED_OFF) ? 0U : 1U
		);
    return (0);
}
