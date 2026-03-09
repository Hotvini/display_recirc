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

void buzzer_init(void)
{
	ctimer_config_t config;

    /* Clock + reset explícitos */
	CLOCK_EnableClock(kCLOCK_Ctimer0);
	RESET_PeripheralReset(kCTIMER0_RST_N_SHIFT_RSTn);

	CTIMER_GetDefaultConfig(&config);
    config.prescale = 0;
	CTIMER_Init(BUZZER_CTIMER, &config);
    CTIMER_SetupPwm(BUZZER_CTIMER,
                     BUZZER_PERIOD,
                     BUZZER_MATCH,
                     BUZZER_DUTY,
                     BUZZER_FREQ_HZ,
                     BUZZER_CLOCK_FREQ,
                     false);

    //CTIMER_EnableResetMatchChannel(BUZZER_CTIMER, BUZZER_PERIOD, false);
    //CTIMER_EnableStopMatchChannel (BUZZER_CTIMER, BUZZER_PERIOD, false);
	CTIMER_StopTimer(BUZZER_CTIMER);
}

void buzzer_on(void)
{
    CTIMER_StartTimer(BUZZER_CTIMER);
    //BUZZER_CTIMER->EMR |= (1 << BUZZER_MATCH);
}

void buzzer_off(void)
{
    CTIMER_StopTimer(BUZZER_CTIMER);
    //BUZZER_CTIMER->EMR &= ~(1 << BUZZER_MATCH);
}

void buzzer_beep_ms(uint32_t ms)
{
    // todo: evitar API bloqueante; migrar para beep nao bloqueante por estado/timer.
    buzzer_on();
    delay_ms(ms);
    buzzer_off();
}

static void buzzer_set_freq(uint32_t freq)
{
    if (freq == 0) {
        buzzer_off();
        return;
    }

    uint32_t period = BUZZER_CLOCK_FREQ / freq;
    // todo: validar faixa de freq para evitar periodos invalidos e limitar duty minimo/maximo.

    BUZZER_CTIMER->MR[BUZZER_PERIOD] = period;
    BUZZER_CTIMER->MR[BUZZER_MATCH]    = period / 2;
}

void buzzer_play_melody(const buzzer_note_t *melody, uint32_t len)
{
    // todo: trocar loop com delay bloqueante por scheduler para nao travar loop principal.
    for (uint32_t i = 0; i < len; i++) {
        if (melody[i].freq) {
            buzzer_set_freq(melody[i].freq);
            buzzer_on();
        } else {
            buzzer_off();
        }

        SDK_DelayAtLeastUs(melody[i].dur_ms * 1000,BUZZER_CLOCK_FREQ);
    }

    buzzer_off();
}

/*
// todo: remover implementacao antiga comentada abaixo para reduzir ruido no arquivo.
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

static uint32_t buzzer_off_time = 0;

void buzzer_beep(uint32_t freq_hz, uint32_t duration_ms)
{
    buzzer_on(freq_hz);
    buzzer_off_time = systick_get_ms() + duration_ms;
    if (buzzer_off_time && systick_get_ms() >= buzzer_off_time)
    {
        buzzer_off();
        buzzer_off_time = 0;
    }
}


void buzzer_task(void)
{
    if (buzzer_off_time && systick_get_ms() >= buzzer_off_time)
    {
        buzzer_off();
        buzzer_off_time = 0;
    }
}
*/
