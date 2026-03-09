#include "touch_di.h"

#define TOUCH_DI_RELEASE_RATIO_NUM 3
#define TOUCH_DI_RELEASE_RATIO_DEN 4
#define TOUCH_DI_INTEGRAL_DEADBAND 2

static inline int32_t clamp(int32_t v, int32_t min, int32_t max)
{
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

static inline int32_t abs_i32(int32_t v)
{
    // todo: tratar INT32_MIN explicitamente para evitar overflow em negacao.
    return (v < 0) ? -v : v;
}

void touch_di_init(touch_di_channel_t *ch)
{
    ch->prev_sample = 0;
    ch->integral = 0;
    ch->filtered = 0;
    ch->initialized = false;
    ch->detected = false;
}

void touch_di_process(touch_di_channel_t *ch,
                      int32_t raw,
                      const touch_di_cfg_t *cfg)
{
    // todo: validar ponteiros ch/cfg e leak_den != 0 para robustez.
    int32_t sample = raw;

    /* ---- IIR FILTER (opcional) ---- */
    if (cfg->iir_shift > 0)
    {
        // todo: limitar iir_shift a faixa segura (0..30) para evitar comportamento indefinido de shift.
        ch->filtered += (raw - ch->filtered) >> cfg->iir_shift;
        sample = ch->filtered;
    }

    if (!ch->initialized)
    {
        ch->filtered = raw;
        ch->prev_sample = (cfg->iir_shift > 0) ? ch->filtered : raw;
        ch->integral = 0;
        ch->detected = false;
        ch->initialized = true;
        return;
    }

    /* ---- DERIVATIVE ---- */
    int32_t d = sample - ch->prev_sample;
    ch->prev_sample = sample;

    /* ---- INTEGRATION ---- */
    if ((d > cfg->dt) || (d < -(int32_t)cfg->dt))
    {
        ch->integral += d;
    }

    /* ---- CLAMP ---- */
    ch->integral = clamp(ch->integral,
                         -cfg->integral_max,
                          cfg->integral_max);

    /* ---- DETECTION WITH HYSTERESIS ---- */
    int32_t abs_integral = abs_i32(ch->integral);
    int32_t it_on = cfg->it;
    int32_t it_off = (cfg->it * TOUCH_DI_RELEASE_RATIO_NUM) / TOUCH_DI_RELEASE_RATIO_DEN;

    if (ch->detected)
    {
        if (abs_integral <= it_off)
        {
            ch->detected = false;
        }
    }
    else
    {
        if (abs_integral >= it_on)
        {
            ch->detected = true;
        }
    }

    /* ---- LEAK (continuous) ---- */
    ch->integral = (ch->integral * cfg->leak_num) / cfg->leak_den;

    /* ---- DEAD-BAND AROUND ZERO ---- */
    if (abs_i32(ch->integral) <= TOUCH_DI_INTEGRAL_DEADBAND)
    {
        ch->integral = 0;
    }
}

bool touch_di_is_detected(const touch_di_channel_t *ch)
{
    return ch->detected;
}
