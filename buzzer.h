/*
 * buzzer.h
 *
 *  Created on: 31 de dez. de 2025
 *      Author: vinicius.andrade
 */

#ifndef BUZZER_H_
#define BUZZER_H_

#include <stdint.h>

void buzzer_init(void);
void buzzer_on(uint32_t freq_hz);
void buzzer_off(void);
void buzzer_beep(uint32_t freq_hz, uint32_t duration_ms);
void buzzer_task(void);

#endif /* BUZZER_H_ */
