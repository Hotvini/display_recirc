/*
 * capt_proc.h
 *
 *  Created on: 31 de dez. de 2025
 *      Author: vinicius.andrade
 */

#ifndef CAPT_PROC_H_
#define CAPT_PROC_H_
#include "capt_config.h"
#include "touch_di.h"

/* Used only when CONTINUOS_POLL == 1 (continuous mode); currently not used. */
//#define CAPT_ENABLE_PINS (kCAPT_X0Pin | kCAPT_X1Pin | kCAPT_X2Pin | kCAPT_X3Pin | kCAPT_X4Pin)
//#define CAPT_ENABLE_PINS (kCAPT_X4Pin) //debug
#define CAPT_ENABLE_PINS_ARRAY {kCAPT_X0Pin, \
                                kCAPT_X1Pin, \
                                kCAPT_X2Pin, \
                                kCAPT_X3Pin}

#define TOUCH_FRAME_WINDOW     8U // todo: 1U para sistema sem média - média maior = menor ruído e maior latência

#define CAPT_POLL_TIMEOUT_MS  5U // timeout em polling mode - dar erro mas não deve rolar
/* Post-calibration baseline tracking: baseline += (avg-baseline)>>shift. */
#define CAPT_BASELINE_TRACK_SHIFT      4U // todo se perde todo
/* Track baseline only when |avg-baseline| is below this limit. */
#define CAPT_BASELINE_TRACK_DELTA_MAX  40U
#define CAPT_BASELINE_STABLE_TOL       2U
#define CAPT_BASELINE_STABLE_FRAMES    24U
#define CAPT_BASELINE_COMMON_MODE_TOL  8U

/* DI input source: 0 = raw_count - baseline, 1 = frame_avg - baseline, 2 = raw_count */
#define CAPT_DI_USE_RAW_INPUT  1U
#define CAPT_DI_DT                 2U
#define CAPT_DI_IT                 24
#define CAPT_DI_LEAK_NUM           97U
#define CAPT_DI_LEAK_DEN           100U
#define CAPT_DI_IIR_SHIFT          1U
#define CAPT_DI_INTEGRAL_MAX       512
/* Inverted DI mode: detect the channel with smallest DI variation while others vary. */
#define CAPT_DI_INVERT_MINVAR_MODE    1U
/* Minimum "other channels activity" to enable inverted decision. */
#define CAPT_DI_INVERT_ACTIVITY_MIN   60U
/* Minimum spread between max and min variation to accept decision. */
#define CAPT_DI_INVERT_SPREAD_MIN     20U

typedef enum {
    CAPT_BTN_S1, // X0
    CAPT_BTN_S2, // X1
    CAPT_BTN_S3, // X2
    CAPT_BTN_S4, // X3
    //CAPT_BTN_S5, // X fake
    CAPT_BTN_COUNT
} capt_button_t;

typedef enum {
    kAPP_TouchStateInit,
    kAPP_TouchStateCalib,
    kAPP_TouchStateDetect
} app_touch_state_t;

typedef struct {
    uint16_t raw_count[CAPT_BTN_COUNT];
	uint16_t frame[CAPT_BTN_COUNT][TOUCH_FRAME_WINDOW];
    uint32_t frame_sum[CAPT_BTN_COUNT];
    //uint64_t frame_sum_sq[CAPT_BTN_COUNT];
	uint16_t frame_avg[CAPT_BTN_COUNT];
    uint16_t frame_baseline[CAPT_BTN_COUNT];
    //uint32_t frame_variance[CAPT_BTN_COUNT];
    //uint16_t frame_stddev[CAPT_BTN_COUNT];
    uint16_t frame_delta[CAPT_BTN_COUNT];
    uint8_t frame_position;
    bool sample_timed_out[CAPT_BTN_COUNT];
    bool frame_ready[CAPT_BTN_COUNT];
    bool detection_map[CAPT_BTN_COUNT];
    app_touch_state_t touch_task_state;
    capt_button_t current_channel;
    bool calibration_done;
} touch_proc_t;

void CMP_CAPT_DriverIRQHandler(void);
void capt_proc_init(touch_proc_t *data_struct);
bool capt_get_sample(touch_proc_t *data_out);
void touch_proc_push_sample(touch_proc_t *data_struct);
void touch_avg_update(touch_proc_t *data_struct);
void touch_baseline_update(touch_proc_t *data_struct);
void touch_proc_delta(touch_proc_t *data_struct);
uint8_t touch_detect_keys_mask(const touch_proc_t *data_struct);
uint8_t touch_detect_key(touch_proc_t *data_struct);
void capt_proc_get_di_snapshot(touch_di_channel_t out[CAPT_BTN_COUNT]);
void capt_proc_get_last_touch_data(capt_touch_data_t *out);

#endif /* CAPT_PROC_H_ */
