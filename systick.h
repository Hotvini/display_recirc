/*
 * systick.h
 *
 *  Created on: 31 de dez. de 2025
 *      Author: vinicius.andrade
 */

#ifndef SYSTICK_H_
#define SYSTICK_H_

#include <stdint.h>

uint32_t systick_get_ms(void);
void systick_init(void);
void delay_ms(uint32_t ms);

#endif /* SYSTICK_H_ */
