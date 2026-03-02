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
#define CAPT_ENABLE_PINS (kCAPT_X0Pin | kCAPT_X1Pin | kCAPT_X2Pin | kCAPT_X3Pin | kCAPT_X4Pin)
//#define CAPT_ENABLE_PINS (kCAPT_X0Pin) //debug
#define CAPT_ENABLE_PINS_ARRAY {kCAPT_X0Pin, \
                                kCAPT_X1Pin, \
                                kCAPT_X2Pin, \
                                kCAPT_X3Pin, \
                                kCAPT_X4Pin}

#define TOUCH_FRAME_WINDOW     4U // todo: 1U para sistema sem média?

#define CAPT_POLL_TIMEOUT_MS  5U //??
/* Gate a channel after N consecutive timeouts for this many milliseconds. */
#define CAPT_TIMEOUT_GATE_HITS 2U
#define CAPT_TIMEOUT_GATE_MS   80U
/* DI input source: 0 = frame_avg - baseline, 1 = raw_count - baseline. */
#define CAPT_DI_USE_RAW_INPUT  0U
/* Multi-press handling: 0 = return strongest key, 1 = allow multi and return first detected key. */
#define CAPT_MULTI_PRESS_ENABLE 1U

typedef enum {
    CAPT_BTN_S1, // X0
    CAPT_BTN_S2, // X1
    CAPT_BTN_S3, // X2
    CAPT_BTN_S4, // X3
    CAPT_BTN_S5, // todo: usar de baseline?
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
    uint32_t gate_until_ms[CAPT_BTN_COUNT];
    uint8_t timeout_streak[CAPT_BTN_COUNT];
    bool sample_timed_out[CAPT_BTN_COUNT];
    bool frame_ready[CAPT_BTN_COUNT];
    bool detection_map[CAPT_BTN_COUNT];
    app_touch_state_t touch_task_state;
	uint8_t frame_position;
    capt_button_t current_channel;
    capt_button_t max_delta_key;
    capt_button_t min_delta_key;
    bool calibration_done;
} touch_proc_t;

void CMP_CAPT_DriverIRQHandler(void);
void capt_proc_init(touch_proc_t *data_struct);
bool capt_get_sample(touch_proc_t *data_out);
void touch_proc_push_sample(touch_proc_t *data_struct);
uint8_t touch_detect_key(touch_proc_t *data_struct);

#endif /* CAPT_PROC_H_ */
