/*
 * capt_proc.c
 *
 *  Created on: 31 de dez. de 2025
 *      Author: vinicius.andrade
 */
#include "capt_proc.h"
#include <limits.h>
#include <string.h>
#include <math.h>

static volatile uint16_t captRawDataBuffer[CAPT_BTN_COUNT];
static volatile bool captTimeoutDataBuffer[CAPT_BTN_COUNT];
const uint16_t captEnabledPins[CAPT_BTN_COUNT] = CAPT_ENABLE_PINS_ARRAY;
static volatile capt_button_t pending_channel;
static volatile bool busy_polling = false;
static uint32_t baseline_accum[CAPT_BTN_COUNT];
static uint8_t baseline_count[CAPT_BTN_COUNT];
static touch_di_channel_t di_channels[CAPT_BTN_COUNT];

static bool touch_channel_is_gated(const touch_proc_t *data_struct, uint8_t channel)
{
    uint32_t now = systick_get_ms();
    return ((int32_t)(now - data_struct->gate_until_ms[channel]) < 0);
}

static bool touch_all_frames_ready(const touch_proc_t *data_struct)
{
    for (uint8_t ch = 0; ch < CAPT_BTN_COUNT; ch++)
    {
        if (!data_struct->frame_ready[ch])
        {
            return false;
        }
    }
    return true;
}

static int32_t touch_get_di_input(const touch_proc_t *data_struct, uint8_t channel)
{
#if (CAPT_DI_USE_RAW_INPUT == 1U)
    return (int32_t)data_struct->raw_count[channel] - (int32_t)data_struct->frame_baseline[channel];
#else
    return (int32_t)data_struct->frame_avg[channel] - (int32_t)data_struct->frame_baseline[channel];
#endif
}

touch_di_cfg_t di_cfg =
{
    .dt = 8,              // 2–4x ruído
    .it = 60,             // sensibilidade
    .leak_num = 99,       // 0.99
    .leak_den = 100,
    .iir_shift = 3,       // 1/8
    .integral_max = 2000
};

void CMP_CAPT_DriverIRQHandler(void)
{
	uint32_t intStat = CAPT_GetInterruptStatusFlags(CAPT_PERIPHERAL);
	CAPT_ClearInterruptStatusFlags(CAPT_PERIPHERAL, intStat);

    if (intStat & kCAPT_InterruptOfPollDoneStatusFlag)
    {
        capt_touch_data_t data;
        if (CAPT_GetTouchData(CAPT_PERIPHERAL, &data))
        {
            if (data.XpinsIndex == pending_channel)
            {
                captRawDataBuffer[data.XpinsIndex] = data.count;
                captTimeoutDataBuffer[data.XpinsIndex] = data.yesTimeOut;
                busy_polling = false;
            }
        }
    }
}
/* --------------------------------------------------------------------------
 * Inicialização - zera as janelas, índices e amostras coletadas; limpa o estado de polling.
 * -------------------------------------------------------------------------- */
void capt_proc_init(touch_proc_t *data_struct)
{
    memset(data_struct, 0, sizeof(touch_proc_t));
    memset((void *)captRawDataBuffer, 0, sizeof(captRawDataBuffer));
    memset((void *)captTimeoutDataBuffer, 0, sizeof(captTimeoutDataBuffer));
    memset((void *)baseline_accum, 0, sizeof(baseline_accum));
    memset((void *)baseline_count, 0, sizeof(baseline_count));
    for (uint8_t ch = 0; ch < CAPT_BTN_COUNT; ch++)
    {
        touch_di_init(&di_channels[ch]);
    }
}

bool capt_get_sample(touch_proc_t *data_out)
{
    if (!busy_polling)
    {
        if (!busy_polling)
        {
            uint32_t poll_start_ms;
            pending_channel = data_out->current_channel;
            busy_polling = true;
            CAPT_PollNow(CAPT_PERIPHERAL, captEnabledPins[pending_channel]);
            poll_start_ms = systick_get_ms();
            while (busy_polling)
            {
                if ((systick_get_ms() - poll_start_ms) > CAPT_POLL_TIMEOUT_MS)
                {
                    busy_polling = false;
                    return (false);
                }
            }
            data_out->raw_count[pending_channel] = captRawDataBuffer[pending_channel];
            data_out->sample_timed_out[pending_channel] = captTimeoutDataBuffer[pending_channel];
        }
        else
        {
            return (false);
        }
        return (true);
    }
    return (false);
}

/* --------------------------------------------------------------------------
 * Atualização da janela deslizante
 * -------------------------------------------------------------------------- */
void touch_proc_push_sample(touch_proc_t *data_struct)
{
    bool timed_out = data_struct->sample_timed_out[pending_channel];
    bool gated = touch_channel_is_gated(data_struct, pending_channel);
    uint32_t old = data_struct->frame[pending_channel][data_struct->frame_position];

    uint32_t new = data_struct->raw_count[pending_channel];

    if (timed_out)
    {
        if (data_struct->timeout_streak[pending_channel] < UINT8_MAX)
        {
            data_struct->timeout_streak[pending_channel]++;
        }

        if (data_struct->timeout_streak[pending_channel] >= CAPT_TIMEOUT_GATE_HITS)
        {
            data_struct->gate_until_ms[pending_channel] = systick_get_ms() + CAPT_TIMEOUT_GATE_MS;
            gated = true;
        }
    }
    else
    {
        data_struct->timeout_streak[pending_channel] = 0U;
    }

    if (timed_out || gated)
    {
        /* Freeze window content on invalid sample; keep pipeline cadence running. */
        new = old;
    }

    data_struct->frame[pending_channel][data_struct->frame_position] = new;

    /* Atualiza soma */
    data_struct->frame_sum[pending_channel] -= old;
    data_struct->frame_sum[pending_channel] += new;

    /* Atualiza soma dos quadrados */
    //data_struct->frame_sum_sq[pending_channel] -= (uint64_t)old * old;
    //data_struct->frame_sum_sq[pending_channel] += (uint64_t)new * new;

    data_struct->frame_position++;

    if (data_struct->frame_position >= TOUCH_FRAME_WINDOW)
    {
        data_struct->frame_position = 0;
        data_struct->frame_ready[pending_channel] = true;
    }
    else
    {
        data_struct->frame_ready[pending_channel] = false;
    }
}

// static void touch_var_std_calc(touch_proc_t *data_struct) // todo fazer para baseline?
// {
//     if (data_struct->frame_ready[pending_channel])
//     {
//         //uint32_t mean = data_struct->frame_sum[pending_channel] / TOUCH_FRAME_WINDOW;
//         //data_struct->frame_avg[pending_channel] = mean;

//         uint32_t mean = data_struct->frame_avg[pending_channel];

//         uint64_t ex2 = data_struct->frame_sum_sq[pending_channel] / TOUCH_FRAME_WINDOW;

//         uint64_t var = ex2 - ((uint64_t)mean * mean);

//         data_struct->frame_variance[pending_channel] = (uint32_t)var;

//         //data_struct->frame_stddev[pending_channel] = (uint16_t)sqrt((double)var);
//         data_struct->frame_stddev[pending_channel] = 0U; // debug
//     }
// }

static void touch_baseline_update(touch_proc_t *data_struct)
{
    /* Canal ainda não fechou janela */
    if (!data_struct->frame_ready[pending_channel])
        return;

    /* Atualiza média */
    data_struct->frame_avg[pending_channel] =
        data_struct->frame_sum[pending_channel] / TOUCH_FRAME_WINDOW;

    if (data_struct->sample_timed_out[pending_channel] || touch_channel_is_gated(data_struct, pending_channel))
        return;

    /* Se já calibrado, nada mais a fazer */
    if (data_struct->calibration_done)
        return;

    /* Durante calibração: baseline = média de 4 frame_avg por canal */
    if (baseline_count[pending_channel] < TOUCH_FRAME_WINDOW)
    {
        baseline_accum[pending_channel] += data_struct->frame_avg[pending_channel];
        baseline_count[pending_channel]++;
    }

    if (baseline_count[pending_channel] == TOUCH_FRAME_WINDOW)
    {
        data_struct->frame_baseline[pending_channel] =
            (uint16_t)(baseline_accum[pending_channel] / TOUCH_FRAME_WINDOW);
    }

    /* Verifica se todos canais estão prontos */
    if (!touch_all_frames_ready(data_struct))
        return;

    for (uint8_t ch = 0; ch < CAPT_BTN_COUNT; ch++)
    {
        if (baseline_count[ch] < TOUCH_FRAME_WINDOW)
        {
            return;
        }
    }

    data_struct->calibration_done = true;
}

static void touch_proc_delta(touch_proc_t *data_struct)
{
    if (data_struct->calibration_done && touch_all_frames_ready(data_struct))
    {
        for (uint8_t channel = 0; channel < CAPT_BTN_COUNT; channel++)
        {
            if (touch_channel_is_gated(data_struct, channel))
            {
                data_struct->frame_delta[channel] = 0U;
                continue;
            }

            //uint16_t raw = data_struct->raw_count[channel];
            uint16_t avg = data_struct->frame_avg[channel];
            uint16_t baseline = data_struct->frame_baseline[channel];

            //data_struct->frame_delta[channel] = (raw > baseline) ? (raw - baseline) : (baseline - raw);
            data_struct->frame_delta[channel] = (avg > baseline) ? (avg - baseline) : (baseline - avg);


            if (channel == 0)
            {
                data_struct->max_delta_key = channel;
                data_struct->min_delta_key = channel;
            }

            if (data_struct->frame_delta[channel] > data_struct->frame_delta[data_struct->max_delta_key])
            {
                data_struct->max_delta_key = channel;
            }
            if (data_struct->frame_delta[channel] < data_struct->frame_delta[data_struct->min_delta_key])
            {
                data_struct->min_delta_key = channel;
            }
        }
    }
}

uint8_t touch_detect_key(touch_proc_t *data_struct)
{
    touch_baseline_update(data_struct);
    touch_proc_delta(data_struct);

    for (uint8_t channel = 0; channel < CAPT_BTN_COUNT; channel++)
    {
        data_struct->detection_map[channel] = false;
    }

    if (!(data_struct->calibration_done && touch_all_frames_ready(data_struct)))
    {
        return CAPT_BTN_COUNT;
    }

    uint8_t first_key = CAPT_BTN_COUNT;
#if (CAPT_MULTI_PRESS_ENABLE == 0U)
    uint8_t best_key = CAPT_BTN_COUNT;
    uint16_t best_delta = 0U;
#endif

    for (uint8_t channel = 0; channel < CAPT_BTN_COUNT; channel++)
    {
        bool invalid = data_struct->sample_timed_out[channel] || touch_channel_is_gated(data_struct, channel);
        if (invalid)
        {
            touch_di_init(&di_channels[channel]);
            continue;
        }

        int32_t signed_delta = touch_get_di_input(data_struct, channel);
        touch_di_process(&di_channels[channel], signed_delta, &di_cfg);

        if (touch_di_is_detected(&di_channels[channel]))
        {
            data_struct->detection_map[channel] = true;

            if (first_key == CAPT_BTN_COUNT)
            {
                first_key = channel;
            }

#if (CAPT_MULTI_PRESS_ENABLE == 0U)
            if (best_key == CAPT_BTN_COUNT || data_struct->frame_delta[channel] > best_delta)
            {
                best_key = channel;
                best_delta = data_struct->frame_delta[channel];
            }
#endif
        }
    }

#if (CAPT_MULTI_PRESS_ENABLE == 1U)
    return first_key;
#else
    return best_key;
#endif
}
