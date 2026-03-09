/*
 * display_hal.c
 *
 *  Created on: 31 de dez. de 2025
 *      Author: vinicius.andrade
 */

#include "display_hal.h"

/* --- Implementações HAL --- */
static void hal_clk_rise(void)
{
    CLK_HIGH();
}

static void hal_clk_down(void)
{
    CLK_LOW();
}

static void hal_data_set(void)
{
    DIO_HIGH();
}

static void hal_data_clr(void)
{
    DIO_LOW();
}

static void hal_stb_set(void)
{
    STB_HIGH();
}

static void hal_stb_clr(void)
{
    STB_LOW();
}

/* --- Registro do HAL --- */
tm1629a_hal_driver_t tm_hal = {
    .clk_rise = hal_clk_rise,
    .clk_down = hal_clk_down,
    .data_set = hal_data_set,
    .data_clr = hal_data_clr,
    .stb_set  = hal_stb_set,
    .stb_clr  = hal_stb_clr
};
// todo: tornar este driver static const se a API tm1629a nao precisar de mutacao em runtime.

const uint8_t seven_seg_symbols[SEG_COUNT] = {
	/* 0–9 */
	[SEG_0] = 0x3F, // 0
	[SEG_1] = 0x06, // 1
	[SEG_2] = 0x5B, // 2
	[SEG_3] = 0x4F, // 3
	[SEG_4] = 0x66, // 4
	[SEG_5] = 0x6D, // 5
	[SEG_6] = 0x7D, // 6
	[SEG_7] = 0x07, // 7
	[SEG_8] = 0x7F, // 8
	[SEG_9] = 0x6F, // 9
	/* Hex */
	[SEG_A] = 0x77, // A
	[SEG_B] = 0x7C, // b
	[SEG_C] = 0x39, // C
	[SEG_D] = 0x5E, // d
	[SEG_E] = 0x79, // E
	[SEG_F] = 0x71, // F
	/* Maiúsculas aceitáveis */
	[SEG_H] = 0x76, // H
	[SEG_L] = 0x38, // L
	[SEG_P] = 0x73, // P
	[SEG_U] = 0x3E, // U
	[SEG_Y] = 0x6E, // Y
	/* Minúsculas (aproximações reais) */
	[SEG_b] = 0x7C, // b
	[SEG_c] = 0x58, // c
	[SEG_d] = 0x5E, // d
	[SEG_h] = 0x74, // h
	[SEG_n] = 0x54, // n
	[SEG_o] = 0x5C, // o
	[SEG_r] = 0x50, // r
	[SEG_u] = 0x1C, // u
	/* Símbolos */
	[SEG_MINUS]      = 0x40, // -
	[SEG_UNDERSCORE] = 0x08, // _
	[SEG_EQUAL]      = 0x48, // =
	[SEG_BLANK]      = 0x00, // apagado
	[SEG_DEGREE]     = 0x63  // °
};
// todo: mapear simbolos realmente usados e remover entradas nao utilizadas para reduzir manutencao.

void display_hal_init(void)
{
	tm1629a_register_hal_driver(&tm_hal);
	tm1629a_init();
}
