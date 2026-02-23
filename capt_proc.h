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
	uint16_t baseline_window[CAPT_BTN_COUNT][TOUCH_BASELINE_WINDOW];
	uint32_t baseline_sum[CAPT_BTN_COUNT];
	uint8_t baseline_idx;
	uint16_t baseline_samples;

	/* Detecção (rápido) */
	uint16_t detect_window[CAPT_BTN_COUNT][TOUCH_DETECT_WINDOW];
	uint32_t detect_sum[CAPT_BTN_COUNT];
	uint8_t detect_idx;
    uint16_t detect_samples;

    uint32_t baseline[CAPT_BTN_COUNT];
    uint32_t detect[CAPT_BTN_COUNT];
    /* Short-window detect variance per channel. Lower means quieter channel. */
    uint32_t variance[CAPT_BTN_COUNT];
} touch_proc_t;

typedef struct {
	capt_button_t current_ch;
	capt_button_t pending_ch;
    //capt_mode_t mode;
    /* Currently unused; reserved for future per-channel debounce in CAPT scheduler. */
    uint8_t     debounce_cnt;
    bool    frame_ready;
    bool    busy;
} capt_state_t;


typedef struct {
    int8_t candidate;
    uint8_t count;
    uint8_t press_needed;
    uint8_t release_needed;
    uint8_t switch_needed;
    uint8_t release_count;
    int8_t stable;
} key_debounce_t;


void CMP_CAPT_DriverIRQHandler(void);

bool capt_get_sample(uint16_t *raw);
bool touch_proc_is_stable(const touch_proc_t *ctx);
void capt_proc_init(touch_proc_t *ctx);
void touch_proc_push_sample(touch_proc_t *ctx, const uint16_t *raw);
void touch_proc_update_baseline(touch_proc_t *ctx);
void touch_proc_compute_fast_delta(touch_proc_t *ctx, const uint16_t *raw);
int touch_detect_key(touch_proc_t *ctx);
void key_debounce_init(key_debounce_t *d);
int key_debounce_step(key_debounce_t *d, int key_raw);

#endif /* CAPT_PROC_H_ */
