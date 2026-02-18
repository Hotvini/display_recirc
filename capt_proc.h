/*
 * capt_proc.h
 *
 *  Created on: 31 de dez. de 2025
 *      Author: vinicius.andrade
 */

#ifndef CAPT_PROC_H_
#define CAPT_PROC_H_


#include "capt_config.h"

typedef enum {
    CAPT_BTN_S1, // X0
    CAPT_BTN_S2, // X1
    CAPT_BTN_S3, // X2
    CAPT_BTN_S4, // X3
    CAPT_BTN_COUNT
} capt_button_t;

typedef struct //separar em duas structs diferentes?
{
    /* Baseline (lento) */
	int16_t baseline_window[CAPT_BTN_COUNT][TOUCH_BASELINE_WINDOW];
	int32_t baseline_sum[CAPT_BTN_COUNT];
	uint8_t baseline_idx;

	/* Detecção (rápido) */
	int16_t detect_window[CAPT_BTN_COUNT][TOUCH_DETECT_WINDOW];
	int32_t detect_sum[CAPT_BTN_COUNT];
	uint8_t detect_idx;

    int32_t baseline[CAPT_BTN_COUNT];
    int32_t detect[CAPT_BTN_COUNT];
    int32_t delta[CAPT_BTN_COUNT];
} touch_proc_t;

typedef struct {
	capt_button_t current_ch;
	capt_button_t pending_ch;
    //capt_mode_t mode;
    uint8_t     debounce_cnt;
    bool    busy;
} capt_state_t;


typedef struct {
    int8_t acc;
    int8_t acc_max;
    int8_t stable;
} key_debounce_t;


void CMP_CAPT_DriverIRQHandler(void);

bool capt_get_sample(int16_t *raw);
bool touch_proc_is_stable(const touch_proc_t *ctx);
void capt_proc_init(touch_proc_t *ctx);
void touch_proc_push_sample(touch_proc_t *ctx, const int16_t *raw);
void touch_proc_update_baseline(touch_proc_t *ctx);
void touch_proc_compute_fast_delta(touch_proc_t *ctx, const int16_t *raw);
int touch_detect_key(touch_proc_t *ctx);
void key_debounce_init(key_debounce_t *d);
int key_debounce_step(key_debounce_t *d, int key_raw);

#endif /* CAPT_PROC_H_ */
