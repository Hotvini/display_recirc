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
#include "capt_task.h"
#include "display_hal.h"


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
//LED DIMMER??


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

static const buzzer_note_t melody_boot[] = {
    { NOTE_C4, 150 },
    { NOTE_E4, 150 },
    { NOTE_G4, 200 },
    { 0,       80  },
    { NOTE_G4, 150 },
    { NOTE_C5, 300 },
};
*/

/* ===================== Touch context ===================== */

static touch_proc_t touchProc;
static uint16_t captRaw[CAPT_BTN_COUNT];
static app_touch_state_t appTouchState = kAPP_TouchStateInit;
static int32_t lastKey = CAPT_BTN_COUNT;
static key_debounce_t keyDebounce;
static uint32_t now = 0;
static uint32_t lastTouchTime = 0;

/* ===================== Main ===================== */

int main(void)
{
    BOARD_InitBootPins();
    BOARD_InitBootClocks();

    systick_init();
    buzzer_init();
	display_hal_init();
    capt_init();
    capt_proc_init(&touchProc);
    key_debounce_init(&keyDebounce);

    //capt_task_init();
	//grace_all_on();
	//buzzer_play_melody(melody_boot,sizeof(melody_boot)/sizeof(melody_boot[0]));

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
					if (!touch_proc_is_stable(&touchProc))
					{
						grace_digit_set(thermo_digits[0], seven_seg_symbols[SEG_C]);
						grace_digit_set(thermo_digits[1], seven_seg_symbols[SEG_A]);
						grace_digit_set(thermo_digits[2], seven_seg_symbols[SEG_L]);
						tm1629a_display_refresh(); // display state
						appTouchState = kAPP_TouchStateInit;
					}
					else
					{
						appTouchState = kAPP_TouchStateCalib;
						//leds_all_off();
					}
					break;

				/* ---------- CALIB ---------- */
				case kAPP_TouchStateCalib:
					touch_proc_update_baseline(&touchProc);
					grace_all_on();
					//grace_digit_set(thermo_digits[0], seven_seg_symbols[SEG_B]);
					//grace_digit_set(thermo_digits[1], seven_seg_symbols[SEG_BLANK]);
					//grace_digit_set(thermo_digits[2], seven_seg_symbols[SEG_D]);
					//tm1629a_display_refresh();
					//lastTouchTime = systick_get_ms();
					//appTouchState = kAPP_TouchStateDetect;
					break;

				/* ---------- DETECT ---------- */
				case kAPP_TouchStateDetect:
					touch_proc_compute_fast_delta(&touchProc, captRaw);
					int key_raw = touch_detect_key(&touchProc);
					int key     = key_debounce_step(&keyDebounce, key_raw);
					//int key = key_raw; // no key debounce

					if (key >= CAPT_BTN_COUNT)
					{
						leds_all_on();
						grace_all_off();
						buzzer_off();
						//now = systick_get_ms();
						//if (now - lastTouchTime > 5000) // 5 segundos
						//{
						//	touch_proc_update_baseline(&touchProc);
						//	lastTouchTime = now;
						//}
					}
					else
					{
						if (key != lastKey && lastKey < CAPT_BTN_COUNT)
						{
							led_ctrl(lastKey, LED_ON);
							//lastTouchTime = systick_get_ms();
						}
						//buzzer_on();
						//grace_all_off();
						led_ctrl(key, LED_OFF);
						grace_digit_set(thermo_digits[0], seven_seg_symbols[SEG_BLANK]);
						grace_digit_set(thermo_digits[1], seven_seg_symbols[SEG_5]);
						grace_digit_set(thermo_digits[2], seven_seg_symbols[key + 1]);
						grace_digit_set(clock_digits[0], seven_seg_symbols[(captRaw[key] / 1000) % 10]);
						grace_digit_set(clock_digits[1], seven_seg_symbols[(captRaw[key] / 100) % 10]);
						grace_digit_set(clock_digits[2], seven_seg_symbols[(captRaw[key] / 10) % 10]);
						grace_digit_set(clock_digits[3], seven_seg_symbols[(captRaw[key]) % 10]);
						tm1629a_display_refresh();
						//now = systick_get_ms();
						//if (now - lastTouchTime > 10000) // 10 segundos
						//{
						//	touch_proc_update_baseline(&touchProc);
						//	lastTouchTime = now;
						//}
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
