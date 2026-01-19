/*
 * buzzer.c
 *
 *  Created on: 31 de dez. de 2025
 *      Author: vinicius.andrade
 */
#include "buzzer.h"
#include "fsl_ctimer.h"
#include "fsl_clock.h"
#include "fsl_reset.h"
#include "systick.h"


#define BUZZER_CTIMER CTIMER0
#define BUZZER_MATCH  kCTIMER_Match_0

void buzzer_init(void)
{
	ctimer_config_t config;

	/* Clock + reset explícitos */
	CLOCK_EnableClock(kCLOCK_Ctimer0);
	RESET_PeripheralReset(kCTIMER0_RST_N_SHIFT_RSTn);

	CTIMER_GetDefaultConfig(&config);
	CTIMER_Init(BUZZER_CTIMER, &config);

	CTIMER_StopTimer(BUZZER_CTIMER);
}


void buzzer_on(uint32_t freq_hz)
{
    uint32_t timer_clk;
    uint32_t match;

    timer_clk = CLOCK_GetFreq(kCLOCK_CoreSysClk);
    match = timer_clk / (freq_hz * 2U);

    ctimer_match_config_t match_config = {
        .matchValue = match,
        .enableCounterReset = true,
        .enableCounterStop  = false,
        .outControl = kCTIMER_Output_Toggle,
        .outPinInitState = false,
        .enableInterrupt = false,
    };

    CTIMER_StopTimer(BUZZER_CTIMER);
    CTIMER_SetupMatch(BUZZER_CTIMER, BUZZER_MATCH, &match_config);
    CTIMER_StartTimer(BUZZER_CTIMER);
}

void buzzer_off(void)
{
    CTIMER_StopTimer(BUZZER_CTIMER);
}


static uint32_t buzzer_off_time = 0;

void buzzer_beep(uint32_t freq_hz, uint32_t duration_ms)
{
    buzzer_on(freq_hz);
    buzzer_off_time = systick_get_ms() + duration_ms;
}

void buzzer_task(void)
{
    if (buzzer_off_time && systick_get_ms() >= buzzer_off_time)
    {
        buzzer_off();
        buzzer_off_time = 0;
    }
}
