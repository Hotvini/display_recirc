/*
 * display_hal.h
 *
 *  Created on: 2 de jan. de 2026
 *      Author: vinicius.andrade
 */

#ifndef DISPLAY_HAL_H_
#define DISPLAY_HAL_H_

#include "pin_mux.h"
#include "fsl_gpio.h"
#include "tm1629a.h"
#include "display_grace.h"

#define STB_HIGH() GPIO_PortSet(BOARD_INITPINS_STB_GPIO, BOARD_INITPINS_STB_PORT, BOARD_INITPINS_STB_GPIO_PIN_MASK)
#define STB_LOW()  GPIO_PortClear(BOARD_INITPINS_STB_GPIO, BOARD_INITPINS_STB_PORT, BOARD_INITPINS_STB_GPIO_PIN_MASK)

#define CLK_HIGH() GPIO_PortSet(BOARD_INITPINS_CLK_GPIO, BOARD_INITPINS_CLK_PORT, BOARD_INITPINS_CLK_GPIO_PIN_MASK)
#define CLK_LOW()  GPIO_PortClear(BOARD_INITPINS_CLK_GPIO, BOARD_INITPINS_CLK_PORT, BOARD_INITPINS_CLK_GPIO_PIN_MASK)

#define DIO_HIGH() GPIO_PortSet(BOARD_INITPINS_DIO_GPIO, BOARD_INITPINS_DIO_PORT, BOARD_INITPINS_DIO_GPIO_PIN_MASK)
#define DIO_LOW()  GPIO_PortClear(BOARD_INITPINS_DIO_GPIO, BOARD_INITPINS_DIO_PORT, BOARD_INITPINS_DIO_GPIO_PIN_MASK)

typedef enum {
    /* Números */
    SEG_0,
    SEG_1,
    SEG_2,
    SEG_3,
    SEG_4,
    SEG_5,
    SEG_6,
    SEG_7,
    SEG_8,
    SEG_9,
    /* Hex clássicos */
    SEG_A,
    SEG_B,
    SEG_C,
    SEG_D,
    SEG_E,
    SEG_F,
    /* Maiúsculas aceitáveis */
	SEG_H,
	SEG_L,
	SEG_P,
	SEG_U,
	SEG_Y,
	/* Minúsculas (aproximações reais) */
	SEG_b,
	SEG_c,
	SEG_d,
	SEG_h,
	SEG_n,
	SEG_o,
	SEG_r,
	SEG_u,
    /* Símbolos */
    SEG_MINUS,
    SEG_UNDERSCORE,
    SEG_EQUAL,
    SEG_BLANK,
    SEG_DEGREE,
	/* Count */
    SEG_COUNT
} seven_seg_symbol_t;

extern const uint8_t seven_seg_symbols[SEG_COUNT];

void display_hal_init(void);

#endif /* DISPLAY_HAL_H_ */
