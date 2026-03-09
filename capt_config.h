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

/* CAPT trigger profile selector. Change only this define to switch mode. */
#define CAPT_TRIGGER_PROFILE_ACMP 0U
#define CAPT_TRIGGER_PROFILE_YH   1U
#define CAPT_TRIGGER_PROFILE      CAPT_TRIGGER_PROFILE_YH

#define CONTINUOS_POLL 0 //cont poll vs poll now
// todo: corrigir nome para CONTINUOUS_POLL em todo o projeto para reduzir ambiguidade/erro de busca.

/* Active X channels in continuous mode and in Poll-Now calls. */
#define CAPT_ENABLE_PINS (kCAPT_X0Pin | kCAPT_X1Pin | kCAPT_X2Pin | kCAPT_X3Pin)

/* Kept for optional critical sections/debug. */
#define DISABLE_CAPT_INTERRUPTS                                                                     \
    CAPT_DisableInterrupts(CAPT_PERIPHERAL, kCAPT_InterruptOfYesTouchEnable | kCAPT_InterruptOfNoTouchEnable | \
                                               kCAPT_InterruptOfTimeOutEnable | kCAPT_InterruptOfPollDoneEnable)
// todo: remover se macro continuar sem uso pratico (codigo morto de debug/critical section).

#if (CONTINUOS_POLL)
#define ENABLE_CAPT_INTERRUPTS CAPT_EnableInterrupts(CAPT_PERIPHERAL, kCAPT_InterruptOfTimeOutEnable | kCAPT_InterruptOfPollDoneEnable)

#else
//#define ENABLE_CAPT_INTERRUPTS CAPT_EnableInterrupts(CAPT_PERIPHERAL, kCAPT_InterruptOfPollDoneEnable)
#define ENABLE_CAPT_INTERRUPTS CAPT_EnableInterrupts(CAPT_PERIPHERAL, kCAPT_InterruptOfTimeOutEnable | kCAPT_InterruptOfPollDoneEnable)
#endif

/* Definition of peripheral ID */
#define ACOMP_PERIPHERAL ACOMP
/* Definition of positive input source used in CMP_SetInputChannels() function */
#define ACOMP_POSITIVE_INPUT 2U // inverter P e N?
/* Definition of negative input source used in CMP_SetInputChannels() function */
#define ACOMP_NEGATIVE_INPUT 0U // ACOMP internal ladder DAC output
// todo: validar canal positivo/negativo no hardware final e limpar comentario de duvida.

#define ACMP_TUNE_PROFILE 0U

#if (ACMP_TUNE_PROFILE == 0U)
    #define ACOMP_LADDER_VALUE 10U
    #define CAPT_MEASURE_DELAY kCAPT_MeasureDelayWait9FCLKs
    #define CAPT_RESET_DELAY   kCAPT_ResetDelayWait9FCLKs
    #define ACOMP_HYSTERESIS   kACOMP_HysteresisNoneSelection
    #define ACOMP_SYNC_TO_BUS_CLK false
#elif (ACMP_TUNE_PROFILE == 1U)
    #define ACOMP_LADDER_VALUE 8U
    #define CAPT_MEASURE_DELAY kCAPT_MeasureDelayWait9FCLKs
    #define CAPT_RESET_DELAY   kCAPT_ResetDelayWait5FCLKs
    #define ACOMP_HYSTERESIS   kACOMP_Hysteresis20MVSelection
    #define ACOMP_SYNC_TO_BUS_CLK false
#else
    #define ACOMP_LADDER_VALUE 6U
    #define CAPT_MEASURE_DELAY kCAPT_MeasureDelayWait3FCLKs
    #define CAPT_RESET_DELAY   kCAPT_ResetDelayWait3FCLKs
    #define ACOMP_HYSTERESIS   kACOMP_Hysteresis20MVSelection
    #define ACOMP_SYNC_TO_BUS_CLK false
#endif

/* Definition of peripheral ID */
#define CAPT_PERIPHERAL CAPT
/* CAPT interrupt vector ID (number). */
#define CAPT_IRQN CMP_CAPT_IRQn
/* CAPT interrupt handler identifier. */
#define CAPT_IRQHANDLER CMP_CAPT_DriverIRQHandler

/* Calculate the clock divider to make sure CAPT work in 2Mhz FCLK. */
#define CAPT_CLK_DIVIDER ((CLOCK_GetFroFreq() / 2000000U) - 1U)
// todo: fixar divisor em init apos clock settle para evitar recalculo por macro e facilitar debug.

/* Delay between poll round, the delay time between two poll round
 * is CAPT_DELAY_BETWEEN_POLL * 4096 * FCLK period
 */
/* Used only when CONTINUOS_POLL == 1 (continuous mode). */
#define CAPT_DELAY_BETWEEN_POLL 60

#if ((CAPT_TRIGGER_PROFILE != CAPT_TRIGGER_PROFILE_ACMP) && (CAPT_TRIGGER_PROFILE != CAPT_TRIGGER_PROFILE_YH))
    #error "Invalid CAPT_TRIGGER_PROFILE. Use CAPT_TRIGGER_PROFILE_ACMP or CAPT_TRIGGER_PROFILE_YH."
#endif

void capt_init(void);

#endif /* CAPT_CONFIG_H_ */
