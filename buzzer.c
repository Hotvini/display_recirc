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

#define REST      0U
#define NOTE_A3   220U
#define NOTE_AS4  466U
#define NOTE_B4   494U
#define NOTE_B5   988U
#define NOTE_C4   262U
#define NOTE_C5   523U
#define NOTE_C6   1047U
#define NOTE_CS5  554U
#define NOTE_D5   587U
#define NOTE_D6   1175U
#define NOTE_DS5  622U
#define NOTE_E4   330U
#define NOTE_E5   659U
#define NOTE_E6   1319U
#define NOTE_F4   349U
#define NOTE_F5   698U
#define NOTE_FS4  370U
#define NOTE_FS5  740U
#define NOTE_G4   392U
#define NOTE_G5   784U
#define NOTE_GS4  415U
#define NOTE_GS5  831U
#define NOTE_A4   440U
#define NOTE_A5   880U
#define NOTE_AS5  932U

#define BUZZER_STARTUP_TEMPO            140U
#define BUZZER_WHOLE_NOTE_MS            ((60000U * 2U) / BUZZER_STARTUP_TEMPO)
#define BUZZER_NOTE_DURATION_MS(divider) \
    ((uint16_t)(((divider) > 0) ? \
    (BUZZER_WHOLE_NOTE_MS / (uint32_t)(divider)) : \
    ((BUZZER_WHOLE_NOTE_MS / (uint32_t)(-(divider))) * 3U / 2U)))
#define BUZZER_NOTE(note, divider)      { (uint16_t)(note), BUZZER_NOTE_DURATION_MS(divider) }

static const buzzer_note_t buzzer_startup_melody[] = {
    BUZZER_NOTE(NOTE_B4, -4), BUZZER_NOTE(NOTE_E5, -4), BUZZER_NOTE(NOTE_B4, -4), BUZZER_NOTE(NOTE_E5, -4),
    BUZZER_NOTE(NOTE_B4, 8), BUZZER_NOTE(NOTE_E5, -4), BUZZER_NOTE(NOTE_B4, 8), BUZZER_NOTE(REST, 8), BUZZER_NOTE(NOTE_AS4, 8), BUZZER_NOTE(NOTE_B4, 8),
    BUZZER_NOTE(NOTE_B4, 8), BUZZER_NOTE(NOTE_AS4, 8), BUZZER_NOTE(NOTE_B4, 8), BUZZER_NOTE(NOTE_A4, 8), BUZZER_NOTE(REST, 8), BUZZER_NOTE(NOTE_GS4, 8), BUZZER_NOTE(NOTE_A4, 8), BUZZER_NOTE(NOTE_G4, 8),
    BUZZER_NOTE(NOTE_G4, 4), BUZZER_NOTE(NOTE_E4, -2),
    BUZZER_NOTE(NOTE_B4, -4), BUZZER_NOTE(NOTE_E5, -4), BUZZER_NOTE(NOTE_B4, -4), BUZZER_NOTE(NOTE_E5, -4),
    BUZZER_NOTE(NOTE_B4, 8), BUZZER_NOTE(NOTE_E5, -4), BUZZER_NOTE(NOTE_B4, 8), BUZZER_NOTE(REST, 8), BUZZER_NOTE(NOTE_AS4, 8), BUZZER_NOTE(NOTE_B4, 8),
    BUZZER_NOTE(NOTE_A4, -4), BUZZER_NOTE(NOTE_A4, -4), BUZZER_NOTE(NOTE_GS4, 8), BUZZER_NOTE(NOTE_A4, -4),
    BUZZER_NOTE(NOTE_D5, 8), BUZZER_NOTE(NOTE_C5, -4), BUZZER_NOTE(NOTE_B4, -4), BUZZER_NOTE(NOTE_A4, -4),
    BUZZER_NOTE(NOTE_B4, -4), BUZZER_NOTE(NOTE_E5, -4), BUZZER_NOTE(NOTE_B4, -4), BUZZER_NOTE(NOTE_E5, -4),
    BUZZER_NOTE(NOTE_B4, 8), BUZZER_NOTE(NOTE_E5, -4), BUZZER_NOTE(NOTE_B4, 8), BUZZER_NOTE(REST, 8), BUZZER_NOTE(NOTE_AS4, 8), BUZZER_NOTE(NOTE_B4, 8),
    BUZZER_NOTE(NOTE_D5, 4), BUZZER_NOTE(NOTE_D5, -4), BUZZER_NOTE(NOTE_B4, 8), BUZZER_NOTE(NOTE_A4, -4),
    BUZZER_NOTE(NOTE_G4, -4), BUZZER_NOTE(NOTE_E4, -2),
    BUZZER_NOTE(NOTE_E4, 2), BUZZER_NOTE(NOTE_G4, 2),
    BUZZER_NOTE(NOTE_B4, 2), BUZZER_NOTE(NOTE_D5, 2),
    BUZZER_NOTE(NOTE_F5, -4), BUZZER_NOTE(NOTE_E5, -4), BUZZER_NOTE(NOTE_AS4, 8), BUZZER_NOTE(NOTE_AS4, 8), BUZZER_NOTE(NOTE_B4, 4), BUZZER_NOTE(NOTE_G4, 4),
};

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

        delay_ms(melody[i].dur_ms);
    }

    buzzer_off();
}

void buzzer_play_startup_melody(void)
{
    buzzer_play_melody(buzzer_startup_melody, sizeof(buzzer_startup_melody) / sizeof(buzzer_startup_melody[0]));
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
