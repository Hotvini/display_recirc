#include "touch_di.h"

static inline int32_t clamp(int32_t v, int32_t min, int32_t max)
{
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

void touch_di_init(touch_di_channel_t *ch)
{
    ch->prev_sample = 0;
    ch->integral = 0;
    ch->filtered = 0;
    ch->detected = false;
}

void touch_di_process(touch_di_channel_t *ch,
                      int32_t raw,
                      const touch_di_cfg_t *cfg)
{
    int32_t sample = raw;

    /* ---- IIR FILTER (opcional) ---- */
    if (cfg->iir_shift > 0)
    {
        ch->filtered += (raw - ch->filtered) >> cfg->iir_shift;
        sample = ch->filtered;
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

    /* ---- DETECTION ---- */
    if ((ch->integral > cfg->it) ||
        (ch->integral < -cfg->it))
    {
        ch->detected = true;
    }
    else
    {
        ch->detected = false;

        /* ---- LEAK (apenas quando não detectado) ---- */
        ch->integral =
            (ch->integral * cfg->leak_num) / cfg->leak_den;
    }
}

bool touch_di_is_detected(const touch_di_channel_t *ch)
{
    return ch->detected;
}