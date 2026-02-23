/*
 * capt_proc.c
 *
 *  Created on: 31 de dez. de 2025
 *      Author: vinicius.andrade
 */
#include "capt_proc.h"
#include <limits.h>

static uint16_t captRawData[CAPT_BTN_COUNT];
const uint16_t captEnabledPins[CAPT_BTN_COUNT] = CAPT_ENABLE_PINS_ARRAY;
capt_state_t captState;
capt_touch_data_t data;

void CMP_CAPT_DriverIRQHandler(void) // flag de outras interrupções pra debug
{
	uint32_t intStat = CAPT_GetInterruptStatusFlags(CAPT_PERIPHERAL);
	CAPT_ClearInterruptStatusFlags(CAPT_PERIPHERAL, intStat);

    if (intStat & kCAPT_InterruptOfPollDoneStatusFlag)
    {
        if (CAPT_GetTouchData(CAPT_PERIPHERAL, &data))
        {
            if (data.XpinsIndex < CAPT_BTN_COUNT)
            {
                captRawData[data.XpinsIndex] = data.count;
            }
            else
            {
                captRawData[captState.pending_ch] = data.count;
            }
        }
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

    if (captState.frame_ready)
    {
        for (uint8_t i = 0; i < CAPT_BTN_COUNT; i++)
            raw[i] = captRawData[i];

        captState.frame_ready = false;
        return true;
    }

    captState.pending_ch = captState.current_ch;
    captState.busy = true;
    capt_poll_channel(captState.current_ch);

	if (captState.current_ch == (CAPT_BTN_COUNT - 1U))
	{
		captState.current_ch = 0;
		captState.frame_ready = true;
	}
	else
	{
		captState.current_ch++;
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

static uint32_t touch_proc_get_detect_var(const touch_proc_t *ctx, uint8_t ch)
{
    uint32_t variance = 0U;
    int32_t avg = (int32_t)ctx->detect[ch];

    for (uint8_t i = 0; i < TOUCH_DETECT_WINDOW; i++)
    {
        int32_t diff = (int32_t)ctx->detect_window[ch][i] - avg;
        variance += (uint32_t)(diff * diff);
    }

    return (variance / TOUCH_DETECT_WINDOW);
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
    (void)raw;
	touch_proc_update_detect(ctx);
    static uint32_t deltaA[CAPT_BTN_COUNT];
    static uint32_t deltaB[CAPT_BTN_COUNT];
	for (uint8_t ch = 0; ch < CAPT_BTN_COUNT; ch++)
	{
        ctx->variance[ch] = touch_proc_get_detect_var(ctx, ch);
	}
}

/* --------------------------------------------------------------------------
 * Detecção de tecla (winner-takes-all)
 * -------------------------------------------------------------------------- */
int touch_detect_key(touch_proc_t *ctx)
{
    if (ctx->detect_samples < TOUCH_DETECT_WINDOW)
        return (CAPT_BTN_COUNT);

    uint32_t avg_variance = 0U;
    for (uint8_t ch = 0; ch < CAPT_BTN_COUNT; ch++)
    {
        avg_variance += ctx->variance[ch];
    }
    avg_variance /= CAPT_BTN_COUNT;

    if (avg_variance == 0U)
        return (CAPT_BTN_COUNT);

    uint32_t best_score = UINT32_MAX;
    uint32_t second_score = UINT32_MAX;
    uint8_t best_key = CAPT_BTN_COUNT;

    /* Variance-only rule: detect key after full detect window, choose quietest channel. */
    for (uint8_t ch = 0; ch < CAPT_BTN_COUNT; ch++)
    {
        bool quieter = (avg_variance == 0U) ? false :
                       (ctx->variance[ch] * 100U <= avg_variance * TOUCH_QUIET_VARIANCE_RATIO_PCT);
        bool enough_quiet_gain = (avg_variance == 0U) ? false :
                                 ((avg_variance - ((avg_variance < ctx->variance[ch]) ? avg_variance : ctx->variance[ch])) * 100U >=
                                  avg_variance * TOUCH_MIN_QUIET_GAIN_PCT);

        if (!quieter || !enough_quiet_gain)
            continue;

        uint32_t score = (ctx->variance[ch] / TOUCH_VARIANCE_SCALE);
        if (score < best_score)
        {
            second_score = best_score;
            best_score = score;
            best_key = ch;
        }
        else if (score < second_score)
        {
            second_score = score;
        }
    }

    if (best_key >= CAPT_BTN_COUNT)
        return (CAPT_BTN_COUNT);

    if (second_score != UINT32_MAX)
    {
        if ((second_score - best_score) < TOUCH_NOISY_SCORE_MARGIN)
            return (CAPT_BTN_COUNT);
    }

    return (best_key);
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
    d->candidate = CAPT_BTN_COUNT;
    d->count = 0U;
    d->press_needed = APP_DEBOUNCE_PRESS_LEVEL;
    d->release_needed = APP_DEBOUNCE_RELEASE_LEVEL;
    d->switch_needed = APP_DEBOUNCE_SWITCH_LEVEL;
    d->release_count = 0U;
    d->stable = CAPT_BTN_COUNT;  // NO_KEY
}

int key_debounce_step(key_debounce_t *d, int key_raw)
{
    if (key_raw >= CAPT_BTN_COUNT)
        key_raw = CAPT_BTN_COUNT;

    if (d->stable >= CAPT_BTN_COUNT)
    {
        /* Idle -> pressed transition requires consecutive confirmations. */
        if (key_raw >= CAPT_BTN_COUNT)
        {
            d->candidate = CAPT_BTN_COUNT;
            d->count = 0U;
            return (CAPT_BTN_COUNT);
        }

        if (key_raw != d->candidate)
        {
            d->candidate = (int8_t)key_raw;
            d->count = 1U;
        }
        else if (d->count < 255U)
        {
            d->count++;
        }

        if (d->count >= d->press_needed)
        {
            d->stable = d->candidate;
            d->candidate = CAPT_BTN_COUNT;
            d->count = 0U;
            d->release_count = 0U;
        }
        return (d->stable);
    }

    /* Stable pressed key: keep latched unless release/switch is persistent. */
    if (key_raw == d->stable)
    {
        d->candidate = CAPT_BTN_COUNT;
        d->count = 0U;
        d->release_count = 0U;
        return (d->stable);
    }

    if (key_raw >= CAPT_BTN_COUNT)
    {
        if (d->release_count < 255U)
            d->release_count++;

        if (d->release_count >= d->release_needed)
        {
            d->stable = CAPT_BTN_COUNT;
            d->candidate = CAPT_BTN_COUNT;
            d->count = 0U;
            d->release_count = 0U;
        }
        return (d->stable);
    }

    /* Different pressed key: switch only with stronger persistence. */
    d->release_count = 0U;
    if (key_raw != d->candidate)
    {
        d->candidate = (int8_t)key_raw;
        d->count = 1U;
    }
    else if (d->count < 255U)
    {
        d->count++;
    }

    if (d->count >= d->switch_needed)
    {
        d->stable = d->candidate;
        d->candidate = CAPT_BTN_COUNT;
        d->count = 0U;
    }

    return (d->stable);
}



