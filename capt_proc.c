/*
 * capt_proc.c
 *
 *  Created on: 31 de dez. de 2025
 *      Author: vinicius.andrade
 */
#include "capt_proc.h"

static int16_t captRawData[CAPT_BTN_COUNT];
const uint16_t captEnabledPins[CAPT_BTN_COUNT] = CAPT_ENABLE_PINS_ARRAY;
capt_state_t captState;
capt_touch_data_t data;

void CMP_CAPT_DriverIRQHandler(void) // flag de outras interrupções pra debug
{
	uint32_t intStat = CAPT_GetInterruptStatusFlags(CAPT_PERIPHERAL);
	CAPT_ClearInterruptStatusFlags(CAPT_PERIPHERAL, intStat);
	CAPT_GetTouchData(CAPT_PERIPHERAL, &data);
	captRawData[captState.pending_ch] = data.count;
	//DISABLE_CAPT_INTERRUPTS;
	//reset pins?

    if (intStat & kCAPT_InterruptOfPollDoneStatusFlag)
    {
    	captState.busy = false;
    }
}
/* --------------------------------------------------------------------------
 * Inicialização
 * -------------------------------------------------------------------------- */
void touch_proc_init(touch_proc_t *ctx)
{
    /* Zera tudo o contexto: janelas, somatórios, baseline e índice */
    memset(ctx, 0, sizeof(touch_proc_t));
    memset(&captState, 0, sizeof(capt_state_t));
}

static void capt_poll_channel(capt_button_t ch)
{
	CAPT_PollNow(CAPT_PERIPHERAL, captEnabledPins[ch]);
}


bool capt_get_sample(int16_t *raw) //CAPT TASK - reset pins after??
{
	while(captState.busy) // troquei de if return false
	{}
    captState.pending_ch = captState.current_ch;
    captState.busy = true;
    //ENABLE_CAPT_INTERRUPTS;
    //CAPT_PollNow(CAPT_PERIPHERAL, captEnabledPins[captState.current_ch]);
    capt_poll_channel(captState.current_ch);
    //CAPT_SetPollMode(CAPT_PERIPHERAL, kCAPT_PollInactiveMode);
    //CAPT_SetPollMode(CAPT_PERIPHERAL, kCAPT_PollContinuousMode);
    captState.current_ch++;
	if (captState.current_ch == CAPT_BTN_COUNT)
	{
		for (uint8_t i = 0; i < CAPT_BTN_COUNT; i++)
			raw[i] = captRawData[i];

		captState.current_ch = 0;
		return (true);
	}
	return (false);
}

/* --------------------------------------------------------------------------
 * Atualização da janela deslizante (moving average)
 * -------------------------------------------------------------------------- */
void touch_proc_push_sample(touch_proc_t *ctx, const int16_t *raw)
{
	for (uint8_t ch = 0; ch < CAPT_BTN_COUNT; ch++)
	{
		/* ---------- baseline (lento) ---------- */
		ctx->baseline_sum[ch] -= ctx->baseline_window[ch][ctx->baseline_idx];
		ctx->baseline_window[ch][ctx->baseline_idx] = raw[ch];
		ctx->baseline_sum[ch] += raw[ch];

		/* ---------- detecção (rápido) ---------- */
		ctx->detect_sum[ch] -= ctx->detect_window[ch][ctx->detect_idx];
		ctx->detect_window[ch][ctx->detect_idx] = raw[ch];
		ctx->detect_sum[ch] += raw[ch];
	}

	ctx->baseline_idx++;
	if (ctx->baseline_idx >= TOUCH_BASELINE_WINDOW)
		ctx->baseline_idx = 0;

	ctx->detect_idx++;
	if (ctx->detect_idx >= TOUCH_DETECT_WINDOW)
		ctx->detect_idx = 0;
}

/* --------------------------------------------------------------------------
 * Cálculo de média
 * -------------------------------------------------------------------------- */
static inline int32_t touch_proc_get_detect_avg(const touch_proc_t *ctx, uint8_t ch)
{
    return (ctx->detect_sum[ch] / TOUCH_DETECT_WINDOW);
}

static inline int32_t touch_proc_get_baseline_avg(const touch_proc_t *ctx, uint8_t ch)
{
    //return (ctx->sum[ch] / TOUCH_WINDOW_LENGTH);
    return (ctx->baseline_sum[ch] / TOUCH_BASELINE_WINDOW);
}

/* --------------------------------------------------------------------------
 * Baseline (nível sem toque)
 * -------------------------------------------------------------------------- */
void touch_proc_update_baseline(touch_proc_t *ctx)
{
	//int32_t general_baseline_avg = 0;
    for (uint8_t ch = 0; ch < CAPT_BTN_COUNT; ch++)
    {
    	ctx->baseline[ch] = touch_proc_get_baseline_avg(ctx, ch);
    	//general_baseline_avg += ctx->baseline[ch];
    }
    //CAPT_SetThreshold(CAPT_PERIPHERAL, general_baseline_avg / TOUCH_BASELINE_WINDOW); // vale a pena?
}

static void touch_proc_update_detect(touch_proc_t *ctx)
{
    for (uint8_t ch = 0; ch < CAPT_BTN_COUNT; ch++)
    {
    	ctx->detect[ch] = touch_proc_get_detect_avg(ctx, ch);
    }
}

/* --------------------------------------------------------------------------
 * Delta = média atual - baseline
 * -------------------------------------------------------------------------- */
void touch_proc_compute_fast_delta(touch_proc_t *ctx, const int16_t *raw)
{
	/* Pré-condição: touch_proc_push_sample() já foi chamado */
	touch_proc_update_detect(ctx);
	for (uint8_t ch = 0; ch < CAPT_BTN_COUNT; ch++)
	{
		int32_t delta = ctx->detect[ch] - ctx->baseline[ch];
		//int32_t delta = raw[ch] - ctx->baseline[ch];
		ctx->delta[ch] = (delta >= 0) ? delta : -delta;
		//ctx->fast_delta[ch] += (delta - ctx->fast_delta[ch]) >> TOUCH_FAST_ALPHA_SHIFT;
	}
}

/* --------------------------------------------------------------------------
 * Detecção de tecla (winner-takes-all)
 * -------------------------------------------------------------------------- */
int touch_detect_key(touch_proc_t *ctx)
{
    int32_t max_delta = 0;
    int32_t sum_delta = 0;
    uint8_t max_delta_key   = CAPT_BTN_COUNT;
    //uint8_t min_delta_key   = CAPT_BTN_COUNT;

    /* Procura apenas deltas positivos */
    for (uint8_t ch = 0; ch < CAPT_BTN_COUNT; ch++)
    {
    	sum_delta += ctx->delta[ch];
    	if (ctx->delta[ch] > max_delta)
        {
            max_delta = ctx->delta[ch];
            max_delta_key   = ch;
        }
    }

    int32_t avg_delta = sum_delta / CAPT_BTN_COUNT;

    /* rejeita ruído global */
    if (max_delta < ((ctx->baseline[max_delta_key] * TOUCH_RELATIVE_THRESHOLD)/1000))
    	//captState.current_ch = CAPT_BTN_COUNT;
    	return (CAPT_BTN_COUNT);

    /* rejeita se não se destaca dos outros */
    if (max_delta < (avg_delta * TOUCH_RELATIVE_THRESHOLD)/10)
		return (CAPT_BTN_COUNT);
    //captState.current_ch = delta_key;
    return (max_delta_key);
}

/* --------------------------------------------------------------------------
 * Verificação de estabilidade (variância)
 * -------------------------------------------------------------------------- */
bool touch_proc_is_stable(const touch_proc_t *ctx)
{
    for (uint8_t ch = 0; ch < CAPT_BTN_COUNT; ch++)
    {
        int32_t avg = touch_proc_get_detect_avg(ctx, ch);
        int32_t variance = 0;

        for (uint8_t i = 0; i < TOUCH_DETECT_WINDOW; i++)
        {
            int32_t diff = ctx->detect_window[ch][i] - avg;
            variance += diff * diff;
        }

        variance /= TOUCH_DETECT_WINDOW;

        /* Se qualquer canal estiver instável, rejeita */
        if (variance > ((avg*APP_CHANNEL_STABLE_VARIANCE)/100))
            return (false);
    }
    return (true);
}

void key_debounce_init(key_debounce_t *d)
{
    d->acc = 0;
    d->acc_max = APP_GLITCH_FILTER_LEVEL;              // ajuste conforme ruído
    d->stable = CAPT_BTN_COUNT;  // NO_KEY
}

int key_debounce_step(key_debounce_t *d, int key_raw)
{
    if (key_raw == d->stable)
    {
        d->acc = 0;
        return (d->stable);
    }

    if (key_raw == CAPT_BTN_COUNT)   // soltando
        d->acc--;
    else                             // pressionando
        d->acc++;

    if (d->acc > d->acc_max)
    {
        d->stable = key_raw;
        d->acc = 0;
    }
    else if (d->acc < -d->acc_max)
    {
        d->stable = CAPT_BTN_COUNT;
        d->acc = 0;
    }

    return (d->stable);
}



