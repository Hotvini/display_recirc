/*
 * capt_proc.c
 *
 *  Created on: 31 de dez. de 2025
 *      Author: vinicius.andrade
 */
#include "capt_proc.h"
#include <string.h>

static volatile uint16_t captRawDataBuffer[CAPT_BTN_COUNT];
static volatile bool captTimeoutDataBuffer[CAPT_BTN_COUNT];
#if (CONTINUOS_POLL)
static volatile bool captSampleReadyBuffer[CAPT_BTN_COUNT];
#else
const uint16_t captEnabledPins[CAPT_BTN_COUNT] = CAPT_ENABLE_PINS_ARRAY;
static volatile bool busy_polling = false;
#endif
static volatile capt_button_t pending_channel;
static volatile capt_touch_data_t last_touch_data;
static uint32_t baseline_accum[CAPT_BTN_COUNT];
static uint8_t baseline_count[CAPT_BTN_COUNT];
static touch_di_channel_t di_channels[CAPT_BTN_COUNT];
static int32_t baseline_stable_ref[CAPT_BTN_COUNT];
static int32_t baseline_stable_sum[CAPT_BTN_COUNT];
static uint8_t baseline_stable_count[CAPT_BTN_COUNT];
static int32_t baseline_track_accum[CAPT_BTN_COUNT];

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

static uint32_t touch_abs_i32(int32_t v)
{
    if (v >= 0)
    {
        return (uint32_t)v;
    }

    if (v == INT32_MIN)
    {
        return (uint32_t)INT32_MAX + 1U;
    }

    return (uint32_t)(-v);
}

static int32_t touch_get_signed_delta_avg(const touch_proc_t *data_struct, uint8_t channel)
{
    return (int32_t)data_struct->frame_avg[channel] - (int32_t)data_struct->frame_baseline[channel];
}

static bool touch_has_common_mode_drift(const touch_proc_t *data_struct, uint8_t ref_channel, int32_t ref_err)
{
    uint8_t similar_channels = 0U;

    for (uint8_t ch = 0; ch < CAPT_BTN_COUNT; ch++)
    {
        int32_t err;
        int32_t diff;

        if (ch == ref_channel)
        {
            continue;
        }

        err = touch_get_signed_delta_avg(data_struct, ch);
        diff = err - ref_err;
        if (diff < 0)
        {
            diff = -diff;
        }

        if (diff <= CAPT_BASELINE_COMMON_MODE_TOL)
        {
            similar_channels++;
        }
    }

    return (similar_channels > 0U);
}

static int32_t touch_get_di_input(const touch_proc_t *data_struct, uint8_t channel)
{
#if (CAPT_DI_USE_RAW_INPUT == 0U)
    return (int32_t)data_struct->raw_count[channel] - (int32_t)data_struct->frame_baseline[channel];
#elif (CAPT_DI_USE_RAW_INPUT == 1U)
    return touch_get_signed_delta_avg(data_struct, channel);
#elif (CAPT_DI_USE_RAW_INPUT == 2U)
    return (int32_t)data_struct->raw_count[channel]; //todo iir aqui?
#elif (CAPT_DI_USE_RAW_INPUT == 3U)
    return (int32_t)data_struct->frame_avg[channel];
#else
#error "Invalid value for CAPT_DI_USE_RAW_INPUT"
#endif
}

touch_di_cfg_t di_cfg =
{
    .dt = CAPT_DI_DT,                  // 2–4x ruído
    .it = CAPT_DI_IT,                  // sensibilidade
    .leak_num = CAPT_DI_LEAK_NUM,      // 0.99
    .leak_den = CAPT_DI_LEAK_DEN,
    .iir_shift = CAPT_DI_IIR_SHIFT,    // 1/8 - 0 = sem filtro IIR
    .integral_max = CAPT_DI_INTEGRAL_MAX
};

void CMP_CAPT_DriverIRQHandler(void)
{
	uint32_t intStat = CAPT_GetInterruptStatusFlags(CAPT_PERIPHERAL);
	CAPT_ClearInterruptStatusFlags(CAPT_PERIPHERAL, intStat);

    if (intStat &
        (kCAPT_InterruptOfYesTouchStatusFlag | kCAPT_InterruptOfNoTouchStatusFlag | kCAPT_InterruptOfTimeOutStatusFlag))
    {
        capt_touch_data_t data;
        if (CAPT_GetTouchData(CAPT_PERIPHERAL, &data))
        {
            last_touch_data = data;
#if (CONTINUOS_POLL)
            if (data.XpinsIndex < CAPT_BTN_COUNT)
            {
                captRawDataBuffer[data.XpinsIndex] = data.count;
                captTimeoutDataBuffer[data.XpinsIndex] = data.yesTimeOut;
                captSampleReadyBuffer[data.XpinsIndex] = true;
            }
#endif
        }
    }

#if (!CONTINUOS_POLL)
    if (intStat & kCAPT_InterruptOfPollDoneStatusFlag)
    {
        capt_touch_data_t data;
        if (CAPT_GetTouchData(CAPT_PERIPHERAL, &data))
        {
            last_touch_data = data;
            if (data.XpinsIndex == pending_channel)
            {
                captRawDataBuffer[data.XpinsIndex] = data.count;
                captTimeoutDataBuffer[data.XpinsIndex] = data.yesTimeOut;
                busy_polling = false;
            }
        }
    }
#endif
}
/* --------------------------------------------------------------------------
 * Inicialização - zera as janelas, índices e amostras coletadas; limpa o estado de polling.
 * -------------------------------------------------------------------------- */
void capt_proc_init(touch_proc_t *data_struct)
{
    memset(data_struct, 0, sizeof(touch_proc_t));
    memset((void *)captRawDataBuffer, 0, sizeof(captRawDataBuffer));
    memset((void *)captTimeoutDataBuffer, 0, sizeof(captTimeoutDataBuffer));
#if (CONTINUOS_POLL)
    memset((void *)captSampleReadyBuffer, 0, sizeof(captSampleReadyBuffer));
#endif
    memset((void *)baseline_accum, 0, sizeof(baseline_accum));
    memset((void *)baseline_count, 0, sizeof(baseline_count));
    memset((void *)baseline_stable_ref, 0, sizeof(baseline_stable_ref));
    memset((void *)baseline_stable_sum, 0, sizeof(baseline_stable_sum));
    memset((void *)baseline_stable_count, 0, sizeof(baseline_stable_count));
    memset((void *)baseline_track_accum, 0, sizeof(baseline_track_accum));
    memset((void *)&last_touch_data, 0, sizeof(last_touch_data));
    for (uint8_t ch = 0; ch < CAPT_BTN_COUNT; ch++)
    {
        touch_di_init(&di_channels[ch]);
    }
}

bool capt_get_sample(touch_proc_t *data_out)
{
#if (CONTINUOS_POLL)
    uint8_t channel = (uint8_t)data_out->current_channel;

    if (channel >= CAPT_BTN_COUNT)
    {
        return false;
    }

    if (!captSampleReadyBuffer[channel])
    {
        return false;
    }

    pending_channel = (capt_button_t)channel;
    data_out->raw_count[channel] = captRawDataBuffer[channel];
    data_out->sample_timed_out[channel] = captTimeoutDataBuffer[channel];
    captSampleReadyBuffer[channel] = false;
    return true;
#else
    if (busy_polling)
    {
        return false;
    }

    {
        uint8_t channel = (uint8_t)data_out->current_channel;
        uint32_t poll_start_ms;

        if (channel >= CAPT_BTN_COUNT)
        {
            return false;
        }

        pending_channel = (capt_button_t)channel;
        busy_polling = true;
        CAPT_PollNow(CAPT_PERIPHERAL, captEnabledPins[pending_channel]);
        poll_start_ms = systick_get_ms();
        while (busy_polling)
        {
            if ((systick_get_ms() - poll_start_ms) > CAPT_POLL_TIMEOUT_MS)
            {
                busy_polling = false;
                return false;
            }
        }

        data_out->raw_count[pending_channel] = captRawDataBuffer[pending_channel];
        data_out->sample_timed_out[pending_channel] = captTimeoutDataBuffer[pending_channel];
    }

    return true;
#endif
}

/* --------------------------------------------------------------------------
 * Atualização da janela deslizante
 * -------------------------------------------------------------------------- */
void touch_proc_push_sample(touch_proc_t *data_struct)
{
    uint32_t old = data_struct->frame[pending_channel][data_struct->frame_position];
    uint32_t new = data_struct->raw_count[pending_channel];

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

void touch_avg_update(touch_proc_t *data_struct)
{
    /* Canal ainda não fechou janela */
    if (!data_struct->frame_ready[pending_channel])
    {
        return;
    }

    data_struct->frame_avg[pending_channel] =
        data_struct->frame_sum[pending_channel] / TOUCH_FRAME_WINDOW;
}

void touch_baseline_update(touch_proc_t *data_struct)
{
    /* Canal ainda não fechou janela */
    if (!data_struct->frame_ready[pending_channel])
    {
        return;
    }

    /* Pós-calibração: rastreio lento de baseline para acompanhar deriva térmica/ambiental. */
    if (data_struct->calibration_done)
    {
        int32_t err = touch_get_signed_delta_avg(data_struct, pending_channel);
        uint32_t abs_err = touch_abs_i32(err);
        int32_t stable_diff = err - baseline_stable_ref[pending_channel];

        if (stable_diff < 0)
        {
            stable_diff = -stable_diff;
        }

        if (stable_diff <= CAPT_BASELINE_STABLE_TOL)
        {
            if (baseline_stable_count[pending_channel] < UINT8_MAX)
            {
                baseline_stable_count[pending_channel]++;
            }
            baseline_stable_sum[pending_channel] += err;
        }
        else
        {
            baseline_stable_ref[pending_channel] = err;
            baseline_stable_sum[pending_channel] = err;
            baseline_stable_count[pending_channel] = 1U;
        }

        if (abs_err <= CAPT_BASELINE_TRACK_DELTA_MAX)
        {
            int32_t accum = baseline_track_accum[pending_channel] + err;
            int32_t adjust = accum / (1 << CAPT_BASELINE_TRACK_SHIFT);

            if (adjust != 0)
            {
                data_struct->frame_baseline[pending_channel] =
                    (uint16_t)((int32_t)data_struct->frame_baseline[pending_channel] + adjust);
                accum -= adjust * (1 << CAPT_BASELINE_TRACK_SHIFT);
            }

            baseline_track_accum[pending_channel] = accum;
        }
        else if (baseline_stable_count[pending_channel] >= CAPT_BASELINE_STABLE_FRAMES &&
                 !touch_di_is_detected(&di_channels[pending_channel]) &&
                 touch_has_common_mode_drift(data_struct, pending_channel, err))
        {
            int32_t stable_mean = baseline_stable_sum[pending_channel] / (int32_t)baseline_stable_count[pending_channel];

            data_struct->frame_baseline[pending_channel] =
                (uint16_t)((int32_t)data_struct->frame_baseline[pending_channel] + stable_mean);
            baseline_stable_ref[pending_channel] = 0;
            baseline_stable_sum[pending_channel] = 0;
            baseline_stable_count[pending_channel] = 0U;
            baseline_track_accum[pending_channel] = 0;
        }
        return;
    }

    /* Durante calibração inicial: baseline = média de 4 frame_avg por canal */
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

void touch_proc_delta(touch_proc_t *data_struct)
{
    if (data_struct->calibration_done && touch_all_frames_ready(data_struct))
    {
        for (uint8_t channel = 0; channel < CAPT_BTN_COUNT; channel++)
        {
            data_struct->frame_delta[channel] = touch_get_signed_delta_avg(data_struct, channel);
        }
    }
}

uint8_t touch_detect_keys_mask(const touch_proc_t *data_struct)
{
    uint8_t mask = 0U;

    for (uint8_t channel = 0; channel < CAPT_BTN_COUNT; channel++)
    {
        if (data_struct->detection_map[channel])
        {
            mask |= (uint8_t)(1U << channel);
        }
    }

    return mask;
}

uint8_t touch_detect_key(touch_proc_t *data_struct)
{
    for (uint8_t channel = 0; channel < CAPT_BTN_COUNT; channel++)
    {
        data_struct->detection_map[channel] = false;
    }

    if (!(data_struct->calibration_done && touch_all_frames_ready(data_struct)))
    {
        return CAPT_BTN_COUNT;
    }

    uint8_t first_key = CAPT_BTN_COUNT;
    uint32_t min_var = UINT32_MAX;
    uint32_t max_var = 0U;
    uint8_t min_var_key = CAPT_BTN_COUNT;

    for (uint8_t channel = 0; channel < CAPT_BTN_COUNT; channel++)
    {
        int32_t signed_delta = touch_get_di_input(data_struct, channel);
        touch_di_process(&di_channels[channel], signed_delta, &di_cfg);

        uint32_t abs_integral = touch_abs_i32(di_channels[channel].integral);
        if (abs_integral < min_var)
        {
            min_var = abs_integral;
            min_var_key = channel;
        }
        if (abs_integral > max_var)
        {
            max_var = abs_integral;
        }

        if (touch_di_is_detected(&di_channels[channel]))
        {
            data_struct->detection_map[channel] = true;

            if (first_key == CAPT_BTN_COUNT)
            {
                first_key = channel;
            }
        }
    }

#if (CAPT_DI_INVERT_MINVAR_MODE == 1U)
    for (uint8_t channel = 0; channel < CAPT_BTN_COUNT; channel++)
    {
        data_struct->detection_map[channel] = false;
    }

    if (min_var_key < CAPT_BTN_COUNT &&
        max_var >= CAPT_DI_INVERT_ACTIVITY_MIN &&
        (max_var - min_var) >= CAPT_DI_INVERT_SPREAD_MIN)
    {
        data_struct->detection_map[min_var_key] = true;
        first_key = min_var_key;
    }
    else
    {
        first_key = CAPT_BTN_COUNT;
    }
#endif

    if (touch_detect_keys_mask(data_struct) == 0U)
    {
        return CAPT_BTN_COUNT;
    }

    return first_key;
}

void capt_proc_get_di_snapshot(touch_di_channel_t out[CAPT_BTN_COUNT])
{
    if (out == NULL)
    {
        return;
    }

    for (uint8_t ch = 0; ch < CAPT_BTN_COUNT; ch++)
    {
        out[ch] = di_channels[ch];
    }
}

void capt_proc_get_last_touch_data(capt_touch_data_t *out)
{
    if (out == NULL)
    {
        return;
    }

    out->yesTimeOut = last_touch_data.yesTimeOut;
    out->yesTouch = last_touch_data.yesTouch;
    out->XpinsIndex = last_touch_data.XpinsIndex;
    out->sequenceNumber = last_touch_data.sequenceNumber;
    out->count = last_touch_data.count;
}
