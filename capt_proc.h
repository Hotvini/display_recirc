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

/* Per-channel pin mask used by Poll-Now mode. */
#define CAPT_ENABLE_PINS_ARRAY {kCAPT_X0Pin, \
                                kCAPT_X1Pin, \
                                kCAPT_X2Pin, \
                                kCAPT_X3Pin}
// todo: manter apenas uma fonte de verdade para pinos habilitados (este array vs CAPT_ENABLE_PINS).

#define TOUCH_FRAME_WINDOW     3U // todo: 1U para sistema sem média - média maior = menor ruído e maior latência
// todo: mover janela para perfil por produto/placa (build flag ou NVM) para evitar reteste via recompilacao.

#define CAPT_POLL_TIMEOUT_MS  20U // timeout em polling mode

/* Post-calibration baseline tracking: baseline += (avg-baseline)>>shift. */
#define CAPT_BASELINE_TRACK_SHIFT      8U // baseline mais lento para não "colar" no avg
/* Track baseline only when |avg-baseline| is below this limit. */
#define CAPT_BASELINE_TRACK_DELTA_MAX  40U
#define CAPT_BASELINE_STABLE_TOL       2U
#define CAPT_BASELINE_STABLE_FRAMES    24U
#define CAPT_BASELINE_COMMON_MODE_TOL  8U

/* DI input source: 0 = raw_iir - baseline, 1 = frame_avg - baseline, 2 = raw_iir */
#define CAPT_DI_USE_RAW_INPUT  2U
/* Pre-DI IIR filter over raw_count (0 disables filtering). */
#define CAPT_DI_INPUT_IIR_SHIFT    2U
// todo: consolidar parametros DI em uma struct de configuracao para reduzir macros espalhadas no cabecalho.
#define CAPT_DI_DT                 8U
#define CAPT_DI_IT                 40U
#define CAPT_DI_LEAK_NUM           95U
#define CAPT_DI_LEAK_DEN           100U
#define CAPT_DI_INTEGRAL_MAX       1024
/* Inverted DI mode: detect the channel with smallest DI variation while others vary. */
#define CAPT_DI_INVERT_MINVAR_MODE    0U
/* Minimum "other channels activity" to enable inverted decision. */
#define CAPT_DI_INVERT_ACTIVITY_MIN   60U
/* Minimum spread between max and min variation to accept decision. */
#define CAPT_DI_INVERT_SPREAD_MIN     20U
/* Reject common-mode movement before DI processing (helps isolate the pressed key). */
#define CAPT_DI_COMMON_MODE_REJECT    0U
/* Final arbitration by strongest DI absolute integral. */
#define CAPT_DI_DOMINANT_MODE         1U
#define CAPT_DI_DOMINANT_ACTIVITY_MIN 120U
#define CAPT_DI_DOMINANT_SPREAD_MIN   50U
#define CAPT_DI_DOMINANT_RELEASE_MIN  80U
#define CAPT_DI_DOMINANT_CONFIRM_FRAMES 2U
/* Per-channel noise normalization (Q8 score = |integral| / noise_floor). */
#define CAPT_DI_NOISE_SHIFT           4U
#define CAPT_DI_NOISE_FLOOR           16U
#define CAPT_DI_NOISE_TRACK_MAX       96U
#define CAPT_DI_SCORE_MIN_Q8          512U
#define CAPT_DI_SCORE_SPREAD_MIN_Q8   96U

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
    int32_t raw_iir[CAPT_BTN_COUNT];
    int32_t raw_iir_error[CAPT_BTN_COUNT];
    bool raw_iir_initialized[CAPT_BTN_COUNT];
	uint16_t frame[CAPT_BTN_COUNT][TOUCH_FRAME_WINDOW];
    uint32_t frame_sum[CAPT_BTN_COUNT];
	uint16_t frame_avg[CAPT_BTN_COUNT];
    uint16_t frame_baseline[CAPT_BTN_COUNT];
    int32_t frame_delta[CAPT_BTN_COUNT];
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
