/*
 * gpio.h
 *
 *  Created on: 31 de dez. de 2025
 *      Author: vinicius.andrade
 */

#ifndef GPIO_APP_H_
#define GPIO_APP_H_

typedef enum {
    S1,
    S2,
    S3,
    S4,
    S_COUNT
} button_led_id_t;

typedef enum {
    LED_OFF,
    LED_ON,
	LED_STATE_COUNT
}led_state_t;

int led_ctrl(button_led_id_t button, led_state_t state);
//int buzzer

#endif /* GPIO_APP_H_ */
