/*
 * buzzer.h
 *
 *  Created on: 31 de dez. de 2025
 *      Author: vinicius.andrade
 */

#ifndef BUZZER_H_
#define BUZZER_H_

#include <stdint.h>


#define BUZZER_CTIMER CTIMER0


#define BUZZER_MATCH  kCTIMER_Match_0

#define BUZZER_PERIOD kCTIMER_Match_1

#define BUZZER_FREQ_HZ      2000   // 3 kHz
#define BUZZER_DUTY         50     // 50%

#define BUZZER_CLOCK_FREQ   CLOCK_GetFreq(kCLOCK_MainClk)

void buzzer_init(void);
void buzzer_on(void);
void buzzer_off(void);
void buzzer_beep_ms(uint32_t ms);

#endif /* BUZZER_H_ */
