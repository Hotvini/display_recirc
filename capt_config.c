/*
 * capt_config.c
 *
 *  Created on: 18 de jan. de 2026
 *      Author: vinicius.andrade
 */

#include "capt_config.h"


static void acomp_init(void)
{
	static acomp_config_t ACOMP_config;
	static acomp_ladder_config_t ACOMP_ladder_config;
	// todo: trocar buffers static por locais se nao houver necessidade de persistencia; reduz estado global oculto.

	ACOMP_config.enableSyncToBusClk = ACOMP_SYNC_TO_BUS_CLK;
	ACOMP_config.hysteresisSelection = ACOMP_HYSTERESIS;

	ACOMP_Init(ACOMP_PERIPHERAL, &ACOMP_config);

	ACOMP_EnableInterrupts(ACOMP_PERIPHERAL, kACOMP_InterruptsDisable);
	ACOMP_SetInputChannel(ACOMP_PERIPHERAL, ACOMP_POSITIVE_INPUT, ACOMP_NEGATIVE_INPUT);

	ACOMP_ladder_config.ladderValue = ACOMP_LADDER_VALUE;
	ACOMP_ladder_config.referenceVoltage = kACOMP_LadderRefVoltagePinVDD;
	ACOMP_SetLadderConfig(ACOMP_PERIPHERAL, &ACOMP_ladder_config);
}


void capt_init(void)
{
	static capt_config_t CAPT_config;
	// todo: remover static se init for chamado uma unica vez; permite stack allocation e facilita teste unitario.
	CAPT_GetDefaultConfig(&CAPT_config);
#if (CAPT_TRIGGER_PROFILE == CAPT_TRIGGER_PROFILE_ACMP)
{
		acomp_init();
		CAPT_config.triggerMode = kCAPT_ComparatorTriggerMode;
		CAPT_config.XpinsMode   = kCAPT_InactiveXpinsHighZMode;
}
#elif (CAPT_TRIGGER_PROFILE == CAPT_TRIGGER_PROFILE_YH)
{
	CAPT_config.triggerMode = kCAPT_YHPortTriggerMode;
	CAPT_config.XpinsMode   = kCAPT_InactiveXpinsDrivenLowMode;
}
#endif
	// todo: remover blocos extras "{" "}" de preprocessor e unificar atribuicoes comuns para reduzir ruido.

#if(CONTINUOS_POLL)
{
	CAPT_config.pollCount = CAPT_DELAY_BETWEEN_POLL;
	CAPT_config.enableXpins = CAPT_ENABLE_PINS;
	CAPT_config.mDelay = CAPT_MEASURE_DELAY;
	CAPT_config.rDelay = CAPT_RESET_DELAY;

}
#else
{
	CAPT_config.enableXpins = 0;
	CAPT_config.mDelay = CAPT_MEASURE_DELAY;
	CAPT_config.rDelay = CAPT_RESET_DELAY;
}
#endif
	CAPT_config.clockDivider = CAPT_CLK_DIVIDER;
	CAPT_Init(CAPT_PERIPHERAL, &CAPT_config);

	/* Enable the interrupts. */
	ENABLE_CAPT_INTERRUPTS;
	EnableIRQ(CAPT_IRQN);
	// todo: encapsular habilitacao de IRQ em helper unico com assert de configuracao para facilitar manutencao.

#if(CONTINUOS_POLL)
	/* Start automatic round-robin polling on enabled X pins. */
	CAPT_SetPollMode(CAPT_PERIPHERAL, kCAPT_PollContinuousMode);
#endif
}
