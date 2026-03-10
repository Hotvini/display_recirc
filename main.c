/*
 * main.c
 *
 *  Created on: 6 de jan. de 2026
 *      Author: vinicius.andrade
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "pin_mux.h"
#include "clock_config.h"
#include "systick.h"
#include "gpio_app.h"
#include "buzzer.h"
#include "capt_proc.h"
#include "display_hal.h"
#include "freemaster.h"


/* ===================== Display maps ===================== */

static const grace_digit_id_t thermo_digits[] = {
    GRACE_DIGIT0_THERMOMETER,
    GRACE_DIGIT1_THERMOMETER,
    GRACE_DIGIT2_THERMOMETER
};

static const grace_digit_id_t clock_digits[] = {
    GRACE_DIGIT0_CLOCK,
    GRACE_DIGIT1_CLOCK,
    GRACE_DIGIT2_CLOCK,
    GRACE_DIGIT3_CLOCK
};

static void leds_all_on(void)
{
	led_ctrl(S1, LED_ON);
	led_ctrl(S2, LED_ON);
	led_ctrl(S3, LED_ON);
	led_ctrl(S4, LED_ON);
}


static void leds_all_off(void)
{
    led_ctrl(S1, LED_OFF);
    led_ctrl(S2, LED_OFF);
    led_ctrl(S3, LED_OFF);
    led_ctrl(S4, LED_OFF);
}

// todo: remover funcao comentada se nao houver plano de reutilizacao.

/* ===================== Touch context ===================== */


static touch_proc_t touchDetect;
static volatile uint8_t fm_key = CAPT_BTN_COUNT;
static volatile uint8_t fm_channel = 0U;
static volatile uint8_t fm_frame_ready_map = 0U;
static volatile uint8_t fm_detection_map = 0U;
static volatile capt_touch_data_t fm_capt_touch_data;
static volatile uint16_t fm_raw0, fm_raw1, fm_raw2, fm_raw3;
static volatile uint16_t fm_avg0, fm_avg1, fm_avg2, fm_avg3;
static volatile uint16_t fm_baseline0, fm_baseline1, fm_baseline2, fm_baseline3;
static volatile int32_t fm_delta0, fm_delta1, fm_delta2, fm_delta3;
static volatile int32_t fm_di_integral0, fm_di_integral1, fm_di_integral2, fm_di_integral3;
static volatile int32_t fm_di_filtered0, fm_di_filtered1, fm_di_filtered2, fm_di_filtered3;
static volatile uint8_t fm_di_detected_map = 0U;
static volatile uint32_t fm_loop_start_ms = 0U;
static volatile uint32_t fm_loop_end_ms = 0U;
static volatile uint32_t fm_loop_duration_ms = 0U;
// todo: agrupar variaveis FMSTR em structs/arrays para reduzir repeticao e facilitar expansao de canais.
// todo: define para habilitar/desabilitar telemetria FMSTR para reduzir overhead em build de producao.

#define TOUCH_CALIB_SETTLE_MS 5000U

FMSTR_TSA_TABLE_BEGIN(touch_watch_table)
    FMSTR_TSA_RO_VAR(fm_key, FMSTR_TSA_UINT8)
    FMSTR_TSA_RO_VAR(fm_channel, FMSTR_TSA_UINT8)
    FMSTR_TSA_RO_VAR(fm_frame_ready_map, FMSTR_TSA_UINT8)
    FMSTR_TSA_RO_VAR(fm_detection_map, FMSTR_TSA_UINT8)
    FMSTR_TSA_RO_VAR(fm_capt_touch_data, FMSTR_TSA_USERTYPE(capt_touch_data_t))
    FMSTR_TSA_RO_VAR(fm_raw0, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_raw1, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_raw2, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_raw3, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_avg0, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_avg1, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_avg2, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_avg3, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_baseline0, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_baseline1, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_baseline2, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_baseline3, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_delta0, FMSTR_TSA_SINT32)
    FMSTR_TSA_RO_VAR(fm_delta1, FMSTR_TSA_SINT32)
    FMSTR_TSA_RO_VAR(fm_delta2, FMSTR_TSA_SINT32)
    FMSTR_TSA_RO_VAR(fm_delta3, FMSTR_TSA_SINT32)
    FMSTR_TSA_RO_VAR(fm_di_integral0, FMSTR_TSA_SINT32)
    FMSTR_TSA_RO_VAR(fm_di_integral1, FMSTR_TSA_SINT32)
    FMSTR_TSA_RO_VAR(fm_di_integral2, FMSTR_TSA_SINT32)
    FMSTR_TSA_RO_VAR(fm_di_integral3, FMSTR_TSA_SINT32)
    FMSTR_TSA_RO_VAR(fm_di_filtered0, FMSTR_TSA_SINT32)
    FMSTR_TSA_RO_VAR(fm_di_filtered1, FMSTR_TSA_SINT32)
    FMSTR_TSA_RO_VAR(fm_di_filtered2, FMSTR_TSA_SINT32)
    FMSTR_TSA_RO_VAR(fm_di_filtered3, FMSTR_TSA_SINT32)
    FMSTR_TSA_RO_VAR(fm_di_detected_map, FMSTR_TSA_UINT8)
    FMSTR_TSA_RO_VAR(fm_loop_start_ms, FMSTR_TSA_UINT32) // todo nao precisa 
    FMSTR_TSA_RO_VAR(fm_loop_end_ms, FMSTR_TSA_UINT32) // nao precisar
    FMSTR_TSA_RO_VAR(fm_loop_duration_ms, FMSTR_TSA_UINT32)
    // todo: cercar telemetria de loop com #if DEBUG para retirar overhead em build de producao.
    FMSTR_TSA_STRUCT(capt_touch_data_t)
    FMSTR_TSA_MEMBER(capt_touch_data_t, yesTimeOut, FMSTR_TSA_UINT8)
    FMSTR_TSA_MEMBER(capt_touch_data_t, yesTouch, FMSTR_TSA_UINT8)
    FMSTR_TSA_MEMBER(capt_touch_data_t, XpinsIndex, FMSTR_TSA_UINT8)
    FMSTR_TSA_MEMBER(capt_touch_data_t, sequenceNumber, FMSTR_TSA_UINT8)
    FMSTR_TSA_MEMBER(capt_touch_data_t, count, FMSTR_TSA_UINT16)
FMSTR_TSA_TABLE_END()

FMSTR_TSA_TABLE_LIST_BEGIN()
    FMSTR_TSA_TABLE(touch_watch_table)
FMSTR_TSA_TABLE_LIST_END()

static void fmstr_touch_update(const touch_proc_t *touch, uint8_t key)
{
    uint8_t frame_ready_map = 0U;
    uint8_t detection_map = 0U;
    uint8_t di_detected_map = 0U;
    capt_touch_data_t capt_snapshot;
    touch_di_channel_t di_snapshot[CAPT_BTN_COUNT];

    fm_key = key;
    fm_channel = (uint8_t)touch->current_channel;
    capt_proc_get_last_touch_data(&capt_snapshot);
    fm_capt_touch_data = capt_snapshot;
    capt_proc_get_di_snapshot(di_snapshot);

    for (uint8_t ch = 0U; ch < CAPT_BTN_COUNT; ch++)
    {
        if (touch->frame_ready[ch])
        {
            frame_ready_map |= (uint8_t)(1U << ch);
        }
        if (touch->detection_map[ch])
        {
            detection_map |= (uint8_t)(1U << ch);
        }

        switch (ch)
        {
            // todo: substituir switch fixo (0..3) por arrays indexados para reduzir codigo duplicado.
            case 0U:
                fm_raw0 = touch->raw_count[ch];
                fm_avg0 = touch->frame_avg[ch];
                fm_baseline0 = touch->frame_baseline[ch];
                fm_delta0 = touch->frame_delta[ch];
                fm_di_integral0 = di_snapshot[ch].integral;
                fm_di_filtered0 = di_snapshot[ch].filtered;
                break;

            case 1U:
                fm_raw1 = touch->raw_count[ch];
                fm_avg1 = touch->frame_avg[ch];
                fm_baseline1 = touch->frame_baseline[ch];
                fm_delta1 = touch->frame_delta[ch];
                fm_di_integral1 = di_snapshot[ch].integral;
                fm_di_filtered1 = di_snapshot[ch].filtered;
                break;

            case 2U:
                fm_raw2 = touch->raw_count[ch];
                fm_avg2 = touch->frame_avg[ch];
                fm_baseline2 = touch->frame_baseline[ch];
                fm_delta2 = touch->frame_delta[ch];
                fm_di_integral2 = di_snapshot[ch].integral;
                fm_di_filtered2 = di_snapshot[ch].filtered;
                break;

            default:
                fm_raw3 = touch->raw_count[ch];
                fm_avg3 = touch->frame_avg[ch];
                fm_baseline3 = touch->frame_baseline[ch];
                fm_delta3 = touch->frame_delta[ch];
                fm_di_integral3 = di_snapshot[ch].integral;
                fm_di_filtered3 = di_snapshot[ch].filtered;
                break;
        }

        if (di_snapshot[ch].detected)
        {
            di_detected_map |= (uint8_t)(1U << ch);
        }
    }

    fm_frame_ready_map = frame_ready_map;
    fm_detection_map = detection_map;
    fm_di_detected_map = di_detected_map;
}

int main(void)
{
    uint32_t calib_start_ms = 0U;

    BOARD_InitBootPins();
    BOARD_InitBootClocks();

    systick_init();
    buzzer_init();
	display_hal_init();

    capt_init();
	capt_proc_init(&touchDetect);
    FMSTR_Init();

	//leds_all_on();
	//grace_all_on();

    while (1)
    {
        uint32_t loop_start_ms = systick_get_ms();

        fm_loop_start_ms = loop_start_ms;

    	/* ===================== CAPT task ===================== */
		// todo: extrair politica de escalonamento de canal para funcao dedicada e simplificar leitura do loop.
		touchDetect.current_channel = (touchDetect.frame_ready[touchDetect.current_channel]) ? (touchDetect.current_channel + 1) % CAPT_BTN_COUNT : touchDetect.current_channel;
    	if (capt_get_sample(&touchDetect))
    	{
			touch_proc_push_sample(&touchDetect);
            touch_avg_update(&touchDetect);
			switch (touchDetect.touch_task_state)
			{
				/* ---------- INIT ---------- */
				case kAPP_TouchStateInit:
					leds_all_on();
					grace_digit_set(thermo_digits[0], seven_seg_symbols[SEG_C]);
					grace_digit_set(thermo_digits[1], seven_seg_symbols[SEG_A]);
					grace_digit_set(thermo_digits[2], seven_seg_symbols[SEG_L]);
					tm1629a_display_refresh(); // display state
                    fmstr_touch_update(&touchDetect, CAPT_BTN_COUNT);
                    calib_start_ms = systick_get_ms();
					touchDetect.touch_task_state = kAPP_TouchStateCalib;
					break;

				/* ---------- CALIB ---------- */
				case kAPP_TouchStateCalib:
					//grace_all_on();
					//grace_all_off();
                    if ((systick_get_ms() - calib_start_ms) >= TOUCH_CALIB_SETTLE_MS)
                    {
                        touch_baseline_update(&touchDetect);
                    }
                    fmstr_touch_update(&touchDetect, CAPT_BTN_COUNT);
                    if (touchDetect.calibration_done)
                    {
					    touchDetect.touch_task_state = kAPP_TouchStateDetect;
                    }
					break;

				/* ---------- DETECT ---------- */
                case kAPP_TouchStateDetect:
                {
                    static uint32_t led_next_toggle_ms = 0U;
                    static bool leds_on = false;
                    uint32_t now_ms = systick_get_ms();

                    if ((int32_t)(now_ms - led_next_toggle_ms) >= 0)
                    {
                        leds_on = !leds_on;
                        led_next_toggle_ms = now_ms + 1000U;
                        if (leds_on)
                        {
                            leds_all_on();
                        }
                        else
                        {
                            leds_all_off();
                        }
                    }

                    touch_baseline_update(&touchDetect); // correção lenta
                    touch_proc_delta(&touchDetect);
					uint8_t key = touch_detect_key(&touchDetect);
                    uint8_t key_map = touch_detect_keys_mask(&touchDetect);
					uint8_t display_channel = (touchDetect.current_channel < CAPT_BTN_COUNT) ? (uint8_t)touchDetect.current_channel : 0U;
					uint16_t display_avg = touchDetect.frame_avg[display_channel];

                    /* Thermometer: show channel average value using 3 digits. */
					grace_digit_set(thermo_digits[0], seven_seg_symbols[(display_avg / 100) % 10]);
					grace_digit_set(thermo_digits[1], seven_seg_symbols[(display_avg / 10) % 10]);
					grace_digit_set(thermo_digits[2], seven_seg_symbols[display_avg % 10]);

                    /* Clock: show key-map bits (ch3..ch0), one bit per digit. */
					grace_digit_set(clock_digits[3], seven_seg_symbols[(key_map >> 3) & 0x1U]);
					grace_digit_set(clock_digits[2], seven_seg_symbols[(key_map >> 2) & 0x1U]);
					grace_digit_set(clock_digits[1], seven_seg_symbols[(key_map >> 1) & 0x1U]);
					grace_digit_set(clock_digits[0], seven_seg_symbols[(key_map >> 0) & 0x1U]);
					// todo: reduzir refresh de display (throttle/change-detect) para baixar consumo e CPU.
					tm1629a_display_refresh();
                    fmstr_touch_update(&touchDetect, key);
				    break;
                }
				default:
                    fmstr_touch_update(&touchDetect, CAPT_BTN_COUNT);
					touchDetect.touch_task_state = kAPP_TouchStateInit;
					break;
			}
		}
        FMSTR_Poll();
        fm_loop_end_ms = systick_get_ms();
        fm_loop_duration_ms = fm_loop_end_ms - loop_start_ms;
    }
}
