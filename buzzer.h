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

#define BUZZER_FREQ_HZ      3000   // 3 kHz
#define BUZZER_DUTY         50     // 50%

#define BUZZER_CLOCK_FREQ   CLOCK_GetFreq(kCLOCK_MainClk)

#define NOTE_C4   262
#define NOTE_D4   294
#define NOTE_E4   330
#define NOTE_F4   349
#define NOTE_G4   392
#define NOTE_A4   440
#define NOTE_B4   494
#define NOTE_C5   523
// todo: remover notas nao utilizadas ou mover para modulo de melodias para reduzir poluicao de header.

typedef struct {
    uint16_t freq;
    uint16_t dur_ms;
} buzzer_note_t;

void buzzer_init(void);
void buzzer_on(void);
void buzzer_off(void);
void buzzer_beep_ms(uint32_t ms);
void buzzer_play_melody(const buzzer_note_t *melody, uint32_t len);

#endif /* BUZZER_H_ */
