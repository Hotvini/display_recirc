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

/*
static void leds_all_off(void)
{
    led_ctrl(S1, LED_OFF);
    led_ctrl(S2, LED_OFF);
    led_ctrl(S3, LED_OFF);
    led_ctrl(S4, LED_OFF);
}
*/

/* ===================== Touch context ===================== */


static touch_proc_t touchDetect;
static volatile uint8_t fm_key = CAPT_BTN_COUNT;
static volatile uint8_t fm_channel = 0U;
static volatile uint8_t fm_calib_done = 0U;
static volatile uint8_t fm_frame_ready_map = 0U;
static volatile uint8_t fm_detection_map = 0U;
static volatile uint16_t fm_raw[CAPT_BTN_COUNT];
static volatile uint16_t fm_avg[CAPT_BTN_COUNT];
static volatile uint16_t fm_baseline[CAPT_BTN_COUNT];
static volatile uint16_t fm_delta[CAPT_BTN_COUNT];
static volatile uint16_t fm_raw0, fm_raw1, fm_raw2, fm_raw3, fm_raw4;
static volatile uint16_t fm_avg0, fm_avg1, fm_avg2, fm_avg3, fm_avg4;
static volatile uint16_t fm_baseline0, fm_baseline1, fm_baseline2, fm_baseline3, fm_baseline4;
static volatile uint16_t fm_delta0, fm_delta1, fm_delta2, fm_delta3, fm_delta4;

FMSTR_TSA_TABLE_BEGIN(touch_watch_table)
    FMSTR_TSA_RO_VAR(fm_key, FMSTR_TSA_UINT8)
    FMSTR_TSA_RO_VAR(fm_channel, FMSTR_TSA_UINT8)
    FMSTR_TSA_RO_VAR(fm_calib_done, FMSTR_TSA_UINT8)
    FMSTR_TSA_RO_VAR(fm_frame_ready_map, FMSTR_TSA_UINT8)
    FMSTR_TSA_RO_VAR(fm_detection_map, FMSTR_TSA_UINT8)
    FMSTR_TSA_RO_VAR(fm_raw, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_avg, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_baseline, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_delta, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_raw0, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_raw1, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_raw2, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_raw3, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_raw4, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_avg0, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_avg1, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_avg2, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_avg3, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_avg4, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_baseline0, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_baseline1, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_baseline2, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_baseline3, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_baseline4, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_delta0, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_delta1, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_delta2, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_delta3, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(fm_delta4, FMSTR_TSA_UINT16)
FMSTR_TSA_TABLE_END()

FMSTR_TSA_TABLE_LIST_BEGIN()
    FMSTR_TSA_TABLE(touch_watch_table)
FMSTR_TSA_TABLE_LIST_END()

static void fmstr_touch_update(const touch_proc_t *touch, uint8_t key)
{
    uint8_t frame_ready_map = 0U;
    uint8_t detection_map = 0U;

    fm_key = key;
    fm_channel = (uint8_t)touch->current_channel;
    fm_calib_done = touch->calibration_done ? 1U : 0U;

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

        fm_raw[ch] = touch->raw_count[ch];
        fm_avg[ch] = touch->frame_avg[ch];
        fm_baseline[ch] = touch->frame_baseline[ch];
        fm_delta[ch] = touch->frame_delta[ch];
    }

    fm_raw0 = fm_raw[0]; fm_raw1 = fm_raw[1]; fm_raw2 = fm_raw[2]; fm_raw3 = fm_raw[3]; fm_raw4 = fm_raw[4];
    fm_avg0 = fm_avg[0]; fm_avg1 = fm_avg[1]; fm_avg2 = fm_avg[2]; fm_avg3 = fm_avg[3]; fm_avg4 = fm_avg[4];
    fm_baseline0 = fm_baseline[0]; fm_baseline1 = fm_baseline[1]; fm_baseline2 = fm_baseline[2]; fm_baseline3 = fm_baseline[3]; fm_baseline4 = fm_baseline[4];
    fm_delta0 = fm_delta[0]; fm_delta1 = fm_delta[1]; fm_delta2 = fm_delta[2]; fm_delta3 = fm_delta[3]; fm_delta4 = fm_delta[4];

    fm_frame_ready_map = frame_ready_map;
    fm_detection_map = detection_map;
}

//static uint16_t captRawData[CAPT_BTN_COUNT];
//static app_touch_state_t appTouchState = kAPP_TouchStateInit;
//static uint16_t lastKey = CAPT_BTN_COUNT;

//static uint32_t now = 0;
//static uint32_t lastTouchTime = 0;

/* ===================== Main ===================== */

int main(void)
{
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
    	/* ===================== CAPT task ===================== */
		touchDetect.current_channel = (touchDetect.frame_ready[touchDetect.current_channel]) ? (touchDetect.current_channel + 1) % CAPT_BTN_COUNT : touchDetect.current_channel;
    	if (capt_get_sample(&touchDetect))
    	{
			touch_proc_push_sample(&touchDetect);
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
					touchDetect.touch_task_state = kAPP_TouchStateCalib;
					break;

				/* ---------- CALIB ---------- */
				case kAPP_TouchStateCalib:
					//grace_all_on();
					grace_all_off();
					//TODO: calibrar aqui, talvez com uma contagem regressiva ou algo assim

					//grace_digit_set(thermo_digits[0], seven_seg_symbols[SEG_B]);
					//grace_digit_set(thermo_digits[1], seven_seg_symbols[SEG_BLANK]);
					//grace_digit_set(thermo_digits[2], seven_seg_symbols[SEG_D]);
					//tm1629a_display_refresh();
					//lastTouchTime = systick_get_ms();
                    fmstr_touch_update(&touchDetect, CAPT_BTN_COUNT);
					touchDetect.touch_task_state = kAPP_TouchStateDetect;
					break;

				/* ---------- DETECT ---------- */
				case kAPP_TouchStateDetect:
					//touchDetect.calibration_done = true;
					uint8_t key = touch_detect_key(&touchDetect);
					uint8_t display_channel = (touchDetect.current_channel < CAPT_BTN_COUNT) ? (uint8_t)touchDetect.current_channel : 0U;
					uint16_t display_count = touchDetect.raw_count[display_channel];
                    uint8_t frame_ready_digit = touchDetect.frame_ready[display_channel] ? 1U : 0U;
					//grace_digit_set(thermo_digits[0], seven_seg_symbols[touchDetect.current_channel]);
					grace_digit_set(thermo_digits[1], seven_seg_symbols[frame_ready_digit]);
					grace_digit_set(thermo_digits[2], (key < CAPT_BTN_COUNT) ? seven_seg_symbols[key] : seven_seg_symbols[SEG_BLANK]);
					grace_digit_set(clock_digits[0], seven_seg_symbols[(display_count / 1000) % 10]);
					grace_digit_set(clock_digits[1], seven_seg_symbols[(display_count / 100) % 10]);
					grace_digit_set(clock_digits[2], seven_seg_symbols[(display_count / 10) % 10]);
					grace_digit_set(clock_digits[3], seven_seg_symbols[(display_count) % 10]);
					tm1629a_display_refresh();
                    fmstr_touch_update(&touchDetect, key);

					//touch_proc_compute_delta(&touchDetect);
					//int key_raw = touch_detect_key(&touchDetect);
					//int key     = key_debounce_step(&keyDebounce, key_raw);
					//int key = key_raw; // no key debounce

					//if (key >= CAPT_BTN_COUNT)
					//{
						//leds_all_on();
						//grace_all_off();
						//buzzer_off();
						//now = systick_get_ms();
						//if (now - lastTouchTime > 5000) // 5 segundos
						//{
						//	touch_proc_update_baseline(&touchProc);
						//	lastTouchTime = now;
						//}
					//}
					//else
					//{
						//if (key != lastKey && lastKey < CAPT_BTN_COUNT)
						//{
							//led_ctrl(lastKey, LED_ON);
							//lastTouchTime = systick_get_ms();
						//}
						//buzzer_on();
						//grace_all_off();
						//led_ctrl(key, LED_OFF);
						// grace_digit_set(thermo_digits[0], seven_seg_symbols[SEG_BLANK]);
						// grace_digit_set(thermo_digits[1], seven_seg_symbols[SEG_5]);
						// grace_digit_set(thermo_digits[2], seven_seg_symbols[key + 1]);
						// grace_digit_set(clock_digits[0], seven_seg_symbols[(captRawData[key] / 1000) % 10]);
						// grace_digit_set(clock_digits[1], seven_seg_symbols[(captRawData[key] / 100) % 10]);
						// grace_digit_set(clock_digits[2], seven_seg_symbols[(captRawData[key] / 10) % 10]);
						// grace_digit_set(clock_digits[3], seven_seg_symbols[(captRawData[key]) % 10]);
						//tm1629a_display_refresh();
						//now = systick_get_ms();
						//if (now - lastTouchTime > 10000) // 10 segundos
						//{
						//	touch_proc_update_baseline(&touchProc);
						//	lastTouchTime = now;
						//}
					//}
					//lastKey = key;

				break;
				default:
                    fmstr_touch_update(&touchDetect, CAPT_BTN_COUNT);
					touchDetect.touch_task_state = kAPP_TouchStateInit;
					break;
			}
		}
        FMSTR_Poll();
    }
}
