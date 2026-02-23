/*
 * capt_proc.c
 *
 *  Created on: 31 de dez. de 2025
 *      Author: vinicius.andrade
 */
#include "capt_proc.h"

static uint16_t captRawData[CAPT_BTN_COUNT];
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
void capt_proc_init(touch_proc_t *ctx)
{
    /* Zera tudo o contexto: janelas, somatórios, baseline e índice */
    memset(ctx, 0, sizeof(touch_proc_t));
    memset(&captState, 0, sizeof(capt_state_t));
}

static void capt_poll_channel(capt_button_t ch)
{
	CAPT_PollNow(CAPT_PERIPHERAL, captEnabledPins[ch]);
}


bool capt_get_sample(uint16_t *raw)
{
	//while(captState.busy)
	//{}
    if (captState.busy)
        return false;

    captState.pending_ch = captState.current_ch;
    captState.busy = true;
    capt_poll_channel(captState.current_ch);
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
void touch_proc_push_sample(touch_proc_t *ctx, const uint16_t *raw)
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
	if (ctx->baseline_samples < TOUCH_BASELINE_WINDOW)
		ctx->baseline_samples++;

	ctx->detect_idx++;
	if (ctx->detect_idx >= TOUCH_DETECT_WINDOW)
		ctx->detect_idx = 0;
    if (ctx->detect_samples < TOUCH_DETECT_WINDOW)
		ctx->detect_samples++;
}

/* --------------------------------------------------------------------------
 * Cálculo de média
 * -------------------------------------------------------------------------- */
static inline uint32_t touch_proc_get_detect_avg(const touch_proc_t *ctx, uint8_t ch)
{
    return (ctx->detect_sum[ch] / TOUCH_DETECT_WINDOW);
}

static inline uint32_t touch_proc_get_baseline_avg(const touch_proc_t *ctx, uint8_t ch)
{
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
void touch_proc_compute_fast_delta(touch_proc_t *ctx, const uint16_t *raw)
{
	/* Pré-condição: touch_proc_push_sample() já foi chamado */
	touch_proc_update_detect(ctx);
    static uint32_t deltaA[CAPT_BTN_COUNT];
    static uint32_t deltaB[CAPT_BTN_COUNT];
	for (uint8_t ch = 0; ch < CAPT_BTN_COUNT; ch++)
	{
		deltaA[ch] = (raw[ch] >= ctx->baseline[ch]) ? (raw[ch] - ctx->baseline[ch]) : (ctx->baseline[ch] - raw[ch]);
        deltaB[ch] = (ctx->detect[ch] >= ctx->baseline[ch]) ? (ctx->detect[ch] - ctx->baseline[ch]) : (ctx->baseline[ch] - ctx->detect[ch]);
        ctx->delta[ch] = (deltaA[ch] > deltaB[ch]) ? deltaA[ch] : deltaB[ch]; // why though
	}
}

/* --------------------------------------------------------------------------
 * Detecção de tecla (winner-takes-all)
 * -------------------------------------------------------------------------- */
int touch_detect_key(touch_proc_t *ctx)
{
    uint32_t max_delta = 0;
    uint32_t sum_delta = 0;
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

    uint32_t out_delta = sum_delta - max_delta;

    /* rejeita ruído global */
    if (max_delta < ((ctx->baseline[max_delta_key] * TOUCH_RELATIVE_THRESHOLD)/1000))
    	//captState.current_ch = CAPT_BTN_COUNT;
    	return (CAPT_BTN_COUNT);

    /* rejeita se não se destaca dos outros */
    if (max_delta < out_delta)
		return (CAPT_BTN_COUNT);
    //captState.current_ch = delta_key;
    return (max_delta_key);
}

/* --------------------------------------------------------------------------
 * Verificação de estabilidade (variância)
 * -------------------------------------------------------------------------- */
bool touch_proc_is_stable(const touch_proc_t *ctx)
{
    // ainda não encheu a janela (falso negativo após init)
    if (ctx->baseline_samples < TOUCH_BASELINE_WINDOW)
        return (false);

    for (uint8_t ch = 0; ch < CAPT_BTN_COUNT; ch++)
    {
        uint32_t avg = touch_proc_get_baseline_avg(ctx, ch);
        uint32_t variance = 0;

        for (uint8_t i = 0; i < TOUCH_BASELINE_WINDOW; i++)
        {
            int32_t diff = (int32_t)ctx->baseline_window[ch][i] - (int32_t)avg;
            variance += (uint32_t)(diff * diff);
        }

        variance /= TOUCH_BASELINE_WINDOW;

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



