///*
// * clock_app.c
// *
// *  Created on: 6 de jan. de 2026
// *      Author: vinicius.andrade
// */
//
//
///*
// * Copyright 2016-2025 NXP
// * All rights reserved.
// *
// * SPDX-License-Identifier: BSD-3-Clause
// */
//
//
//#include <stdio.h>
//#include <stdint.h>
//#include "pin_mux.h"
//#include "systick.h"
//#include "gpio_app.h"
//#include "clock_config.h"
//#include "buzzer.h"
//#include "display_hal.h"
//
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
//static const grace_dot_id_t clock_dots[] = {
//	GRACE_DOT0_CLOCK,
//	GRACE_DOT1_CLOCK
//};
//
//typedef struct {
//    uint8_t sec;
//    uint8_t min;
//    uint8_t hour;
//} clock_time_t;
//
//static uint32_t last_ms = 0;
//
//void clock_init(clock_time_t *t, uint8_t h, uint8_t m, uint8_t s)
//{
//    t->hour = h;
//    t->min  = m;
//    t->sec  = s;
//
//    last_ms = systick_get_ms();
//}
//
//void clock_task(clock_time_t *t)
//{
//    uint32_t now = systick_get_ms();
//
//    if ((now - last_ms) >= 1000) {
//        last_ms += 1000;
//
//        t->sec++;
//
//        if (t->sec >= 60) {
//            t->sec = 0;
//            t->min++;
//
//            if (t->min >= 60) {
//                t->min = 0;
//                t->hour++;
//
//                if (t->hour >= 24) {
//                    t->hour = 0;
//                }
//            }
//        }
//    }
//}
//
//clock_time_t clock_time;
//uint8_t last_sec = 0xFF;
//
//int main(void) {
//
//    /* Init board hardware. */
//    BOARD_InitBootPins();
//    BOARD_InitBootClocks();
//    /* Init board peripherals. */
//    systick_init();
//    clock_init(&clock_time, 10, 8, 0);
//    buzzer_init();
//    display_hal_init();
//
//    while (1)
//    {
//    	clock_task(&clock_time);
//
//    	if (clock_time.sec != last_sec)
//		{
//			last_sec = clock_time.sec;
//
//			uint8_t sec0  = clock_time.sec  / 10;
//			uint8_t sec1  = clock_time.sec  % 10;
//			uint8_t min0  = clock_time.min  / 10;
//			uint8_t min1  = clock_time.min  % 10;
//			uint8_t hour0 = clock_time.hour / 10;
//			uint8_t hour1 = clock_time.hour % 10;
//
//			grace_digit_set(thermo_digits[0], seven_seg_symbols[SEG_BLANK]);
//			grace_digit_set(thermo_digits[1], seven_seg_symbols[sec0]);
//			grace_digit_set(thermo_digits[2], seven_seg_symbols[sec1]);
//			grace_dot_set(clock_dots[0],1);
//			grace_dot_set(clock_dots[1],1);
//			grace_digit_set(clock_digits[0], seven_seg_symbols[hour0]);
//			grace_digit_set(clock_digits[1], seven_seg_symbols[hour1]);
//			grace_digit_set(clock_digits[2], seven_seg_symbols[min0]);
//			grace_digit_set(clock_digits[3], seven_seg_symbols[min1]);
//
//			tm1629a_display_refresh();
//		}
//
//    }
//}
