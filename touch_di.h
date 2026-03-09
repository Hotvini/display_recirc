#ifndef TOUCH_DI_H
#define TOUCH_DI_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    int32_t prev_sample;
    int32_t integral;
    int32_t filtered;      // IIR opcional
    bool    initialized;
    bool    detected;
} touch_di_channel_t;

typedef struct
{
    uint16_t dt;          // derivative threshold
    int32_t  it;          // integration threshold
    uint16_t leak_num;    // ex: 99
    uint16_t leak_den;    // ex: 100
    uint8_t  iir_shift;   // 0 desabilita IIR
    int32_t  integral_max;
} touch_di_cfg_t;
// todo: mover parametros de tuning para perfil externo (NVM/build) em vez de recompilacao fixa.

void touch_di_init(touch_di_channel_t *ch);

void touch_di_process(touch_di_channel_t *ch,
                      int32_t raw,
                      const touch_di_cfg_t *cfg);

bool touch_di_is_detected(const touch_di_channel_t *ch);

#endif
