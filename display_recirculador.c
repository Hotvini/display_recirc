///*
// * Copyright 2016-2025 NXP
// * All rights reserved.
// *
// * SPDX-License-Identifier: BSD-3-Clause
// */
//
///**
// * @file    display_recirculador.c
// * @brief   Application entry point.
// */
//#include <stdio.h>
//#include <stdint.h>
//#include "pin_mux.h"
//#include "systick.h"
//#include "gpio_app.h"
//#include "clock_config.h"
//#include "buzzer.h"
//#include "capt_app.h"
//#include "glitch_filter.h"
//#include "display_hal.h"
//
//// button test
//typedef enum
//{
//    kAPP_TouchStateInit   = 0, /* Initialization. */
//    kAPP_TouchStateCalib  = 1, /* Calibration, learn the baseline. */
//    kAPP_TouchStateDetect = 2, /* Detection. */
//} app_touch_state_t;
//
//int32_t appTouchStateInitCounter = 0;
//bool appTouchWindowsIsStable     = true;
//int32_t pressedKeyIndex;
//int32_t oldPressedKeyIndex;
//app_touch_state_t appTouchState = kAPP_TouchStateInit;
//glitch_filter_handle_t appGlitchFilterHandle;
//
//int16_t appTouchValRaw[CAPT_BTN_COUNT] = {0};
//int32_t appTouchWindowsSum[CAPT_BTN_COUNT];
//int32_t appTouchWindowsAverage[CAPT_BTN_COUNT];
//int32_t appTouchWindowsVariance[CAPT_BTN_COUNT];
//
//uint8_t dig = 0;
//
//void led_all_on(void){
//	led_ctrl(S1, LED_ON);
//	led_ctrl(S2, LED_ON);
//	led_ctrl(S3, LED_ON);
//	led_ctrl(S4, LED_ON);
//}
//
//void led_all_off(void){
//	led_ctrl(S1, LED_OFF);
//	led_ctrl(S2, LED_OFF);
//	led_ctrl(S3, LED_OFF);
//	led_ctrl(S4, LED_OFF);
//}
//
//static const grace_digit_id_t thermo_digits[] = {
//    GRACE_DIGIT0_THERMOMETER,
//    GRACE_DIGIT1_THERMOMETER,
//    GRACE_DIGIT2_THERMOMETER
//};
//
//static const grace_digit_id_t clock_digits[] = {
//	GRACE_DIGIT0_CLOCK,
//	GRACE_DIGIT1_CLOCK,
//	GRACE_DIGIT2_CLOCK,
//	GRACE_DIGIT3_CLOCK
//};
//
//int main(void) {
//
//    /* Init board hardware. */
//    BOARD_InitBootPins();
//    BOARD_InitBootClocks();
//    /* Init board peripherals. */
//    systick_init();
//    buzzer_init();
//
//    TOUCH_WINDOWS_Init();
//    CAPT_init();
//    display_hal_init();
//
//    appTouchState = kAPP_TouchStateInit;
//
//
//    while (1)
//    {
//    	CAPT_WaitDataReady(appTouchValRaw);
//
//        /* Push the raw data into window array. */
//        TOUCH_WINDOWS_Puth(appTouchValRaw);
//
//        /* process the state machine. */
//        switch (appTouchState)
//        {
//            /*
//             * Init stage.
//             * In this stage, touch scan is started, the captured raw data is pushed
//             * into the window array, but not processed.
//             * This drops the unstable data after reset.
//             */
//            case kAPP_TouchStateInit:
//            	grace_digit_set(thermo_digits[0], seven_seg_symbols[SEG_0]);
//				grace_digit_set(thermo_digits[1], seven_seg_symbols[SEG_n]);
//				grace_digit_set(thermo_digits[2], seven_seg_symbols[SEG_BLANK]);
//				tm1629a_display_refresh();
//
//                if (appTouchStateInitCounter < 4u * TOUCH_WINDOW_LENGTH)
//                {
//                    appTouchStateInitCounter++;
//                }
//                else
//                {
//                    appTouchStateInitCounter = 0u;
//                    appTouchState            = kAPP_TouchStateCalib;
//
//                    led_all_on();
//                }
//                break;
//
//            /* Calibration stage. */
//            case kAPP_TouchStateCalib:
//            	grace_digit_set(thermo_digits[0], seven_seg_symbols[SEG_C]);
//            	grace_digit_set(thermo_digits[1], seven_seg_symbols[SEG_A]);
//            	grace_digit_set(thermo_digits[2], seven_seg_symbols[SEG_L]);
//            	tm1629a_display_refresh();
//                /* Check variance to make sure the touch sample data is stable. */
//                TOUCH_WINDOWS_CalcAverage(appTouchWindowsAverage);
//                TOUCH_WINDOWS_CalcVariance(appTouchWindowsAverage, appTouchWindowsVariance);
//
//                appTouchWindowsIsStable = true;
//                for (uint8_t i = 0u; i < CAPT_BTN_COUNT; i++)
//                {
//                    if (appTouchWindowsVariance[i] > APP_CHANNEL_STABLE_VARIANCE)
//                    {
//                        appTouchWindowsIsStable = false;
//                        break;
//                    }
//                }
//
//                /* When sample data stable, use the stable data in window to calculate the baseline. */
//                if (appTouchWindowsIsStable)
//                {
//                    TOUCH_WINDOWS_CalcSum(appTouchWindowsSum);
//                    TOUCH_WINDOWS_SetBaseline(appTouchWindowsSum);
//
//                    /* calib done. */
//                    led_all_off();
//                    grace_digit_set(thermo_digits[0], seven_seg_symbols[SEG_BLANK]);
//					grace_digit_set(thermo_digits[1], seven_seg_symbols[SEG_BLANK]);
//					grace_digit_set(thermo_digits[2], seven_seg_symbols[SEG_BLANK]);
//					tm1629a_display_refresh();
//
//                    FILTER_Init(&appGlitchFilterHandle, CAPT_BTN_COUNT, APP_GLITCH_FILTER_LEVEL);
//                    appTouchState = kAPP_TouchStateDetect;
//                }
//
//                break;
//
//            case kAPP_TouchStateDetect:
//
//                TOUCH_WINDOWS_CalcSum(appTouchWindowsSum);
//
//                pressedKeyIndex = TOUCH_GetPressedKeyIndex(appTouchWindowsSum);
//
//                pressedKeyIndex = FILTER_Output(&appGlitchFilterHandle, pressedKeyIndex);
//
//                if (CAPT_BTN_COUNT == pressedKeyIndex)
//                {
//                	led_all_off();
//                }
//                else
//                {
//                    if (oldPressedKeyIndex != pressedKeyIndex)
//                    {
//                    	led_ctrl(oldPressedKeyIndex,LED_OFF);
//                    }
//                    led_ctrl(pressedKeyIndex,LED_ON);
//                    grace_digit_set(thermo_digits[0], seven_seg_symbols[SEG_BLANK]);
//					grace_digit_set(thermo_digits[1], seven_seg_symbols[SEG_5]);
//                    grace_digit_set(thermo_digits[2], seven_seg_symbols[pressedKeyIndex+1U]);
//                    uint8_t mils = (uint8_t)(appTouchValRaw[pressedKeyIndex] / 1000);
//                    uint8_t cens = (uint8_t)(appTouchValRaw[pressedKeyIndex] / 100);
//                    uint8_t tens = (uint8_t)((appTouchValRaw[pressedKeyIndex] / 10) % 10);
//                    uint8_t unit = (uint8_t)(appTouchValRaw[pressedKeyIndex] % 10);
//                    grace_digit_set(clock_digits[0], seven_seg_symbols[mils]);
//                    grace_digit_set(clock_digits[1], seven_seg_symbols[cens]);
//                    grace_digit_set(clock_digits[2], seven_seg_symbols[tens]);
//                    grace_digit_set(clock_digits[3], seven_seg_symbols[unit]);
//                    tm1629a_display_refresh();
//                }
//
//                oldPressedKeyIndex = pressedKeyIndex;
//
//                break;
//
//            default:
//                appTouchState = kAPP_TouchStateInit;
//                break;
//        } /* end switch. */
//    }
//}
