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

/* ===================== FSM ===================== */
typedef enum
{
    kAPP_TouchStateInit,
    kAPP_TouchStateCalib,
    kAPP_TouchStateDetect
} app_touch_state_t;

/* ===================== Globals ===================== */

static app_touch_state_t appTouchState = kAPP_TouchStateInit;

/* Touch processing */
static touch_proc_t touchProc;

/* Debounce (index-level) */
static key_debounce_t keyDebounce;

/* Raw CAPT snapshot */
static int16_t captRaw[CAPT_BTN_COUNT];

/* FSM helpers */
static uint32_t initCounter = 0;
static int32_t  lastKey     = CAPT_BTN_COUNT;


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

/* ===================== LED helpers ===================== */

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

/* ===================== Main ===================== */

int main(void)
{
    BOARD_InitBootPins();
    BOARD_InitBootClocks();

    systick_init();
    buzzer_init();
    capt_init();
    key_debounce_init(&keyDebounce);
    display_hal_init();

    touch_proc_init(&touchProc);
    appTouchState = kAPP_TouchStateInit;


    while (1)
    {
    	/* ===================== CAPT task ===================== */
    	if (capt_get_sample(captRaw))
    	{
    		touch_proc_push_sample(&touchProc, captRaw);

			switch (appTouchState)
			{
				/* ---------- INIT ---------- */
				case kAPP_TouchStateInit:
					leds_all_on();
					if (initCounter <= TOUCH_BASELINE_WINDOW)
					{
						initCounter++;
					}
					else
					{
						initCounter = 0u;
						grace_digit_set(thermo_digits[0], seven_seg_symbols[SEG_C]);
						grace_digit_set(thermo_digits[1], seven_seg_symbols[SEG_A]);
						grace_digit_set(thermo_digits[2], seven_seg_symbols[SEG_L]);
						tm1629a_display_refresh(); // display state
						appTouchState = kAPP_TouchStateCalib;
						leds_all_on();
					}
					break;

				/* ---------- CALIB ---------- */
				case kAPP_TouchStateCalib:
					if (!touch_proc_is_stable(&touchProc))
					{
						appTouchState = kAPP_TouchStateInit;
						break;
					}
					touch_proc_update_baseline(&touchProc);

					grace_digit_set(thermo_digits[0], seven_seg_symbols[SEG_B]);
					grace_digit_set(thermo_digits[1], seven_seg_symbols[SEG_BLANK]);
					grace_digit_set(thermo_digits[2], seven_seg_symbols[SEG_D]);
					tm1629a_display_refresh();
					appTouchState = kAPP_TouchStateDetect;
					break;

				/* ---------- DETECT ---------- */
				case kAPP_TouchStateDetect:
//					if (!touch_proc_is_stable(&touchProc))
//					{
//						appTouchState = kAPP_TouchStateCalib;
//						break;
//					}
					touch_proc_compute_fast_delta(&touchProc, captRaw);
					int key_raw = touch_detect_key(&touchProc);
					int key     = key_debounce_step(&keyDebounce, key_raw);
					//int key = key_raw; // no key debounce

					if (key >= CAPT_BTN_COUNT)
					{
						leds_all_off();
					}
					else
					{
						if (key != lastKey && lastKey < CAPT_BTN_COUNT)
						{
							led_ctrl(lastKey, LED_OFF);
						}
						else
						{
						led_ctrl(key, LED_ON);
						grace_digit_set(thermo_digits[0], seven_seg_symbols[SEG_BLANK]);
						grace_digit_set(thermo_digits[1], seven_seg_symbols[SEG_5]);
						grace_digit_set(thermo_digits[2], seven_seg_symbols[key + 1]);
						uint16_t v = captRaw[key];
						grace_digit_set(clock_digits[0], seven_seg_symbols[(v / 1000) % 10]);
						grace_digit_set(clock_digits[1], seven_seg_symbols[(v / 100) % 10]);
						grace_digit_set(clock_digits[2], seven_seg_symbols[(v / 10) % 10]);
						grace_digit_set(clock_digits[3], seven_seg_symbols[v % 10]);
						tm1629a_display_refresh();
						}
					}
					lastKey = key;

				break;
				default:
					appTouchState = kAPP_TouchStateInit;
					break;
			}
		}
    }
}
