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
// todo: avaliar tornar static const para limitar escopo do modulo.
typedef enum
{
    kCaptPollStateIdle = 0,
    kCaptPollStateWaiting,
    kCaptPollStateReady
} capt_poll_state_t;
static volatile capt_poll_state_t capt_poll_state = kCaptPollStateIdle;
static uint32_t capt_poll_start_ms = 0U;
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
static uint16_t di_noise_floor[CAPT_BTN_COUNT];
static capt_button_t di_active_key;
static capt_button_t di_candidate_key;
static uint8_t di_candidate_frames;

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
    int32_t input;
#if (CAPT_DI_USE_RAW_INPUT == 0U)
    input = (int32_t)data_struct->raw_count[channel] - (int32_t)data_struct->frame_baseline[channel];
#elif (CAPT_DI_USE_RAW_INPUT == 1U)
    input = (int32_t)data_struct->raw_iir[channel] - (int32_t)data_struct->frame_baseline[channel];
#elif (CAPT_DI_USE_RAW_INPUT == 2U)
    input = (int32_t)data_struct->raw_iir[channel];
#elif (CAPT_DI_USE_RAW_INPUT == 3U)
    input = (int32_t)data_struct->raw_count[channel];
#else
#error "Invalid value for CAPT_DI_USE_RAW_INPUT"
#endif

#if (CAPT_DI_COMMON_MODE_REJECT == 1U)
    int32_t common_mode_sum = 0;

    for (uint8_t ch = 0; ch < CAPT_BTN_COUNT; ch++)
    {
        common_mode_sum += touch_get_signed_delta_avg(data_struct, ch);
    }

    input -= (common_mode_sum / (int32_t)CAPT_BTN_COUNT);
#endif

    return input;
}

touch_di_cfg_t di_cfg =
{
    .dt = CAPT_DI_DT,                  // 2–4x ruído
    .it = CAPT_DI_IT,                  // sensibilidade
    .leak_num = CAPT_DI_LEAK_NUM,      // 0.99
    .leak_den = CAPT_DI_LEAK_DEN,
    .integral_max = CAPT_DI_INTEGRAL_MAX
};
// todo: tornar configuracao DI const (ou atualizavel por API explicita) para evitar escrita acidental global.

void CMP_CAPT_DriverIRQHandler(void)
{
	uint32_t intStat = CAPT_GetInterruptStatusFlags(CAPT_PERIPHERAL);
	CAPT_ClearInterruptStatusFlags(CAPT_PERIPHERAL, intStat);
	// todo: reduzir duplicacao dos blocos CAPT_GetTouchData entre modos de polling com helper interno.

#if (CONTINUOS_POLL)
    if (intStat &
        (kCAPT_InterruptOfYesTouchStatusFlag | kCAPT_InterruptOfNoTouchStatusFlag |
         kCAPT_InterruptOfTimeOutStatusFlag | kCAPT_InterruptOfPollDoneStatusFlag))
    {
        capt_touch_data_t data;
        if (CAPT_GetTouchData(CAPT_PERIPHERAL, &data))
        {
            last_touch_data = data;
            if (data.XpinsIndex < CAPT_BTN_COUNT)
            {
                captRawDataBuffer[data.XpinsIndex] = data.count;
                captTimeoutDataBuffer[data.XpinsIndex] = data.yesTimeOut;
                captSampleReadyBuffer[data.XpinsIndex] = true;
            }
        }
    }
#else
    if (intStat & kCAPT_InterruptOfPollDoneStatusFlag)
    {
        capt_touch_data_t data;
        if (CAPT_GetTouchData(CAPT_PERIPHERAL, &data))
        {
            last_touch_data = data;
            if ((capt_poll_state == kCaptPollStateWaiting) &&
                (data.XpinsIndex == pending_channel))
            {
                captRawDataBuffer[data.XpinsIndex] = data.count;
                captTimeoutDataBuffer[data.XpinsIndex] = data.yesTimeOut;
                capt_poll_state = kCaptPollStateReady;
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
    memset((void *)di_noise_floor, 0, sizeof(di_noise_floor));
    memset((void *)&last_touch_data, 0, sizeof(last_touch_data));
#if (!CONTINUOS_POLL)
    capt_poll_state = kCaptPollStateIdle;
    capt_poll_start_ms = 0U;
#endif
    for (uint8_t ch = 0; ch < CAPT_BTN_COUNT; ch++)
    {
        touch_di_init(&di_channels[ch]);
        di_noise_floor[ch] = CAPT_DI_NOISE_FLOOR;
    }
    di_active_key = CAPT_BTN_COUNT;
    di_candidate_key = CAPT_BTN_COUNT;
    di_candidate_frames = 0U;
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
    uint8_t channel = (uint8_t)data_out->current_channel;

    if (channel >= CAPT_BTN_COUNT)
    {
        return false;
    }

    if (capt_poll_state == kCaptPollStateWaiting)
    {
        if ((systick_get_ms() - capt_poll_start_ms) > CAPT_POLL_TIMEOUT_MS)
        {
            capt_poll_state = kCaptPollStateIdle;
        }
        return false;
    }

    if (capt_poll_state == kCaptPollStateReady)
    {
        data_out->raw_count[pending_channel] = captRawDataBuffer[pending_channel];
        data_out->sample_timed_out[pending_channel] = captTimeoutDataBuffer[pending_channel];
        capt_poll_state = kCaptPollStateIdle;
        return true;
    }

    pending_channel = (capt_button_t)channel;
    capt_poll_start_ms = systick_get_ms();
    capt_poll_state = kCaptPollStateWaiting;
    CAPT_PollNow(CAPT_PERIPHERAL, captEnabledPins[pending_channel]);
    return false;
#endif
}

/* --------------------------------------------------------------------------
 * Atualização da janela deslizante
 * -------------------------------------------------------------------------- */
void touch_proc_push_sample(touch_proc_t *data_struct)
{
    uint32_t old = data_struct->frame[pending_channel][data_struct->frame_position];
    uint32_t new = data_struct->raw_count[pending_channel];
    int32_t raw = (int32_t)new;

    if (!data_struct->raw_iir_initialized[pending_channel] || (CAPT_DI_INPUT_IIR_SHIFT == 0U))
    {
        data_struct->raw_iir[pending_channel] = raw;
        data_struct->raw_iir_error[pending_channel] = 0;
        data_struct->raw_iir_initialized[pending_channel] = true;
    }
    else
    {
        int32_t diff = raw - data_struct->raw_iir[pending_channel];
        int32_t acc = data_struct->raw_iir_error[pending_channel] + diff;
        int32_t step;

        if (acc >= 0)
        {
            step = acc >> CAPT_DI_INPUT_IIR_SHIFT;
        }
        else
        {
            step = -(((-acc) >> CAPT_DI_INPUT_IIR_SHIFT));
        }

        data_struct->raw_iir[pending_channel] += step;
        data_struct->raw_iir_error[pending_channel] =
            acc - (step << CAPT_DI_INPUT_IIR_SHIFT);
    }

    data_struct->frame[pending_channel][data_struct->frame_position] = new;

    /* Atualiza soma */
    data_struct->frame_sum[pending_channel] -= old;
    data_struct->frame_sum[pending_channel] += new;

    /* Atualiza soma dos quadrados */
    // todo: remover bloco legado comentado ou reativar via flag de variancia para evitar codigo morto.
    //data_struct->frame_sum_sq[pending_channel] -= (uint64_t)old * old;
    //data_struct->frame_sum_sq[pending_channel] += (uint64_t)new * new;

    data_struct->frame_position++;

    if (data_struct->frame_position >= TOUCH_FRAME_WINDOW)
    {
        data_struct->frame_position = 0;
        data_struct->frame_ready[pending_channel] = true;
    }
}

// todo: remover funcao inteira comentada abaixo, ou implementar com flag de compilacao especifica.
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

        if ((abs_err <= CAPT_BASELINE_TRACK_DELTA_MAX) &&
            !touch_di_is_detected(&di_channels[pending_channel]))
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

    /* Durante calibração inicial: baseline = média de N frame_avg por canal */
    // todo: corrigir comentario antigo ("4") para refletir TOUCH_FRAME_WINDOW configuravel.
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
    uint32_t abs_integral_per_channel[CAPT_BTN_COUNT];

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
    uint32_t dominant_abs = 0U;
    uint32_t second_abs = 0U;
    uint32_t dominant_score_q8 = 0U;
    uint32_t second_score_q8 = 0U;
    uint8_t dominant_key = CAPT_BTN_COUNT;

    for (uint8_t channel = 0; channel < CAPT_BTN_COUNT; channel++)
    {
        int32_t di_input = touch_get_di_input(data_struct, channel);
        touch_di_process(&di_channels[channel], di_input, &di_cfg);

        uint32_t abs_integral = touch_abs_i32(di_channels[channel].integral);
        uint32_t score_q8;
        uint32_t denom;
        int32_t noise_err;

        abs_integral_per_channel[channel] = abs_integral;

        if ((!touch_di_is_detected(&di_channels[channel])) &&
            (abs_integral <= CAPT_DI_NOISE_TRACK_MAX))
        {
            noise_err = (int32_t)abs_integral - (int32_t)di_noise_floor[channel];
            di_noise_floor[channel] =
                (uint16_t)((int32_t)di_noise_floor[channel] + (noise_err >> CAPT_DI_NOISE_SHIFT));

            if (di_noise_floor[channel] < CAPT_DI_NOISE_FLOOR)
            {
                di_noise_floor[channel] = CAPT_DI_NOISE_FLOOR;
            }
        }

        denom = (uint32_t)di_noise_floor[channel];
        score_q8 = (denom > 0U) ? ((abs_integral << 8) / denom) : 0U;

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

        if (abs_integral >= dominant_abs)
        {
            second_abs = dominant_abs;
            dominant_abs = abs_integral;
            dominant_key = channel;
        }
        else if (abs_integral > second_abs)
        {
            second_abs = abs_integral;
        }

        if (score_q8 >= dominant_score_q8)
        {
            second_score_q8 = dominant_score_q8;
            dominant_score_q8 = score_q8;
        }
        else if (score_q8 > second_score_q8)
        {
            second_score_q8 = score_q8;
        }
    }

#if (CAPT_DI_INVERT_MINVAR_MODE == 1U)
    // todo: evitar limpar detection_map duas vezes; integrar fluxo invertido sem trabalho redundante.
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

#if (CAPT_DI_DOMINANT_MODE == 1U)
    for (uint8_t channel = 0; channel < CAPT_BTN_COUNT; channel++)
    {
        data_struct->detection_map[channel] = false;
    }

    if ((di_active_key < CAPT_BTN_COUNT) &&
        (abs_integral_per_channel[di_active_key] >= CAPT_DI_DOMINANT_RELEASE_MIN))
    {
        data_struct->detection_map[di_active_key] = true;
        first_key = (uint8_t)di_active_key;
    }
    else
    {
        di_active_key = CAPT_BTN_COUNT;

        if ((dominant_key < CAPT_BTN_COUNT) &&
            (dominant_abs >= CAPT_DI_DOMINANT_ACTIVITY_MIN) &&
            ((dominant_abs - second_abs) >= CAPT_DI_DOMINANT_SPREAD_MIN) &&
            (dominant_score_q8 >= CAPT_DI_SCORE_MIN_Q8) &&
            ((dominant_score_q8 - second_score_q8) >= CAPT_DI_SCORE_SPREAD_MIN_Q8))
        {
            if (di_candidate_key == (capt_button_t)dominant_key)
            {
                if (di_candidate_frames < UINT8_MAX)
                {
                    di_candidate_frames++;
                }
            }
            else
            {
                di_candidate_key = (capt_button_t)dominant_key;
                di_candidate_frames = 1U;
            }

            if (di_candidate_frames >= CAPT_DI_DOMINANT_CONFIRM_FRAMES)
            {
                di_active_key = (capt_button_t)dominant_key;
                data_struct->detection_map[dominant_key] = true;
                first_key = dominant_key;
            }
            else
            {
                first_key = CAPT_BTN_COUNT;
            }
        }
        else
        {
            di_candidate_key = CAPT_BTN_COUNT;
            di_candidate_frames = 0U;
            first_key = CAPT_BTN_COUNT;
        }
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
