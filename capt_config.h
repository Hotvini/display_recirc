/*
 * capt_config.h
 *
 *  Created on: 18 de jan. de 2026
 *      Author: vinicius.andrade
 */

#ifndef CAPT_CONFIG_H_
#define CAPT_CONFIG_H_

#include <stdbool.h>
#include "fsl_common.h"
#include "fsl_acomp.h"
#include "fsl_capt.h"
#include "fsl_reset.h"
#include "systick.h"

#define DISABLE_CAPT_INTERRUPTS                                                                     \
    CAPT_DisableInterrupts(CAPT_PERIPHERAL, kCAPT_InterruptOfYesTouchEnable | kCAPT_InterruptOfNoTouchEnable | \
                                               kCAPT_InterruptOfTimeOutEnable | kCAPT_InterruptOfPollDoneEnable)

#define ENABLE_CAPT_INTERRUPTS CAPT_EnableInterrupts(CAPT_PERIPHERAL, kCAPT_InterruptOfPollDoneEnable); // POll Now mode

#define ACOMP_ON 0 //com ou sem acomp

#define CONTINUOS_POLL 0 //cont poll vs poll now

/* Definition of peripheral ID */
#define ACOMP_PERIPHERAL ACOMP
/* Definition of positive input source used in CMP_SetInputChannels() function */
#define ACOMP_POSITIVE_INPUT 2U
/* Definition of negative input source used in CMP_SetInputChannels() function */
#define ACOMP_NEGATIVE_INPUT 0U //ladder ref
/* Ladder value */
#define ACOMP_LADDER_VALUE 8U

/* Definition of peripheral ID */
#define CAPT_PERIPHERAL CAPT
/* CAPT interrupt vector ID (number). */
#define CAPT_IRQN CMP_CAPT_IRQn
/* CAPT interrupt handler identifier. */
#define CAPT_IRQHANDLER CMP_CAPT_DriverIRQHandler

/* Calculate the clock divider to make sure CAPT work in 2Mhz FCLK. */
//#define CAPT_CLK_DIVIDER ((CLOCK_GetFroFreq() / 2000000U) - 1U)
#define CAPT_CLK_DIVIDER ((CLOCK_GetFroFreq() / 4000000U) - 1U)

/* Delay between poll round, the delay time between two poll round
 * is CAPT_DELAY_BETWEEN_POLL * 4096 * FCLK period
 */
#define CAPT_DELAY_BETWEEN_POLL 60

/* If the channel sample data variance is less than this value, then the channel
 * sample data is stable, used in calibration stage.
 */
#define APP_CHANNEL_STABLE_VARIANCE 20

/* Debounce level used in pressed key detection. (armotecedor final) */
#define APP_GLITCH_FILTER_LEVEL 4

#define CAPT_ENABLE_PINS (kCAPT_X0Pin | kCAPT_X1Pin | kCAPT_X2Pin | kCAPT_X3Pin)
//#define CAPT_ENABLE_PINS (kCAPT_X0Pin) //debug

#define CAPT_ENABLE_PINS_ARRAY {kCAPT_X0Pin, kCAPT_X1Pin, kCAPT_X2Pin, kCAPT_X3Pin}

// How many samples are saved and used to determine touch result.
#define TOUCH_BASELINE_WINDOW   16   // lento, estável

#define TOUCH_DETECT_WINDOW    8   // rápido

#define TOUCH_RELATIVE_THRESHOLD 10 // quanto menor mais sensivel

void capt_init(void);

#endif /* CAPT_CONFIG_H_ */
