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
const uint16_t captEnabledPins[CAPT_BTN_COUNT] = CAPT_ENABLE_PINS_ARRAY;
static volatile capt_button_t pending_channel;
static volatile bool busy_polling = false;
static uint8_t base_window_count = 0U;

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
        SET_FLAG(data_struct->frame_ready, pending_channel);
    }
    else
    {
        CLEAR_FLAG(data_struct->frame_ready, pending_channel);
    }
}

// static void touch_var_std_calc(touch_proc_t *data_struct) // todo fazer para baseline?
// {
//     if (CHECK_FLAG(data_struct->frame_ready, pending_channel))
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
    if (!CHECK_FLAG(data_struct->frame_ready, pending_channel))
        return;

    /* Atualiza média */
    data_struct->frame_avg[pending_channel] =
        data_struct->frame_sum[pending_channel] / TOUCH_FRAME_WINDOW;

    /* Se já calibrado, nada mais a fazer */
    if (data_struct->calibration_done)
        return;

    /* Durante calibração */
    data_struct->frame_baseline[pending_channel] =
        data_struct->frame_avg[pending_channel];

    /* Verifica se todos canais estão prontos */
    if ((data_struct->frame_ready & CH_ALL_MASK) != CH_ALL_MASK)
        return;

    if (base_window_count < TOUCH_FRAME_WINDOW)
    {
        base_window_count++;
        return;
    }

    data_struct->calibration_done = true;
}

static void touch_proc_delta(touch_proc_t *data_struct)
{
    if (data_struct->calibration_done && ((data_struct->frame_ready & CH_ALL_MASK) == CH_ALL_MASK)) // assuming 5 channels
    {
        for (uint8_t channel = 0; channel < CAPT_BTN_COUNT; channel++)
        {
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

// static void touch_proc_dia(touch_proc_t *data_struct)
// {
//     touch_di_channel_t di[5];

//     for (int i = 0; i < 5; i++)
//         touch_di_init(&di[i]);

//     /* No loop principal */
//     touch_di_process(&di[ch], raw_value, &di_cfg);

//     if (touch_di_is_detected(&di[ch]))
//     {
//         SET_FLAG(pressed_bitmap, ch);
//     }
//     else
//     {
//         CLEAR_FLAG(pressed_bitmap, ch);
//     }
// }

uint8_t touch_detect_key(touch_proc_t *data_struct)
{
    if (CHECK_FLAG(data_struct->frame_ready, pending_channel))
    {
        //dia_initialized = false;
        //data_struct->current_channel = CAPT_BTN_COUNT;
        //return (CAPT_BTN_COUNT);
    }

    touch_baseline_update(data_struct);
    //touch_var_std_calc(data_struct);
    touch_proc_delta(data_struct); // delta vale para todos os canais
    //touch_proc_dia(data_struct);

    return data_struct->detection_map;
}
