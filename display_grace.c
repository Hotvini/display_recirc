/*
 * display_grace.c
 *
 *  Created on: 31 de dez. de 2025
 *      Author: vinicius.andrade
 */


#include "display_grace.h"

#ifndef NULL
#define NULL      ((void*)0)
#endif

#ifndef  TRUE
#define  TRUE     (1)
#endif

#ifndef  FALSE
#define  FALSE    (0)
#endif
// todo: remover redefinicoes de NULL/TRUE/FALSE e usar stdbool/stddef padrao.

#ifndef  ASSERT_NULL_PTR
#define  ASSERT_NULL_PTR(x)     \
{                               \
	     if((x)== NULL){        \
		 return -1;             \
	     }                      \
}
#endif

typedef struct {
    uint8_t addr;   // endereço TM1629A
    uint8_t mask;   // bit dentro do byte
} grace_map_t;

static const grace_map_t grace_icon_map[GRACE_ICON_COUNT] = {
    [GRACE_ICON_WIFI]        = { .addr = 0x0E, .mask = (1 << 0) },
    [GRACE_ICON_PUMP]        = { .addr = 0x0E, .mask = (1 << 1) },
    [GRACE_ICON_CELSIUS]     = { .addr = 0x0E, .mask = (1 << 2) },
    [GRACE_ICON_CONFIG_MODE] = { .addr = 0x0E, .mask = (1 << 3) },
    [GRACE_ICON_MANUAL_MODE] = { .addr = 0x0E, .mask = (1 << 4) },
    [GRACE_ICON_AUTO_MODE]   = { .addr = 0x0E, .mask = (1 << 5) },
    [GRACE_ICON_ALERT]       = { .addr = 0x0E, .mask = (1 << 6) },
};

static const grace_map_t grace_week_map[GRACE_WEEK_COUNT] = {
    [GRACE_WEEK_DOM]        = { .addr = 0x00, .mask = (1 << 7) },
    [GRACE_WEEK_SEG]        = { .addr = 0x02, .mask = (1 << 7) },
    [GRACE_WEEK_TER]        = { .addr = 0x04, .mask = (1 << 7) },
    [GRACE_WEEK_QUA]        = { .addr = 0x06, .mask = (1 << 7) },
    [GRACE_WEEK_QUI]        = { .addr = 0x08, .mask = (1 << 7) },
    [GRACE_WEEK_SEX]        = { .addr = 0x0A, .mask = (1 << 7) },
    [GRACE_WEEK_SAB]        = { .addr = 0x0C, .mask = (1 << 7) }
};

static const grace_map_t grace_dot_map[GRACE_DOT_COUNT] = {
    [GRACE_DOT0_THERMOMETER] = { .addr = 0x01, .mask = (1 << 0) },
    [GRACE_DOT1_THERMOMETER] = { .addr = 0x03, .mask = (1 << 0) },
    [GRACE_DOT0_CLOCK]       = { .addr = 0x09, .mask = (1 << 0) },
    [GRACE_DOT1_CLOCK]       = { .addr = 0x0B, .mask = (1 << 0) },
};

static const grace_map_t grace_digit_map[GRACE_DIGIT_COUNT] = {
    [GRACE_DIGIT0_THERMOMETER] = { .addr = 0x00, .mask = 0x7F },
    [GRACE_DIGIT1_THERMOMETER] = { .addr = 0x02, .mask = 0x7F },
    [GRACE_DIGIT2_THERMOMETER] = { .addr = 0x04, .mask = 0x7F },
    [GRACE_DIGIT0_CLOCK]       = { .addr = 0x06, .mask = 0x7F },
    [GRACE_DIGIT1_CLOCK]       = { .addr = 0x08, .mask = 0x7F },
    [GRACE_DIGIT2_CLOCK]       = { .addr = 0x0A, .mask = 0x7F },
    [GRACE_DIGIT3_CLOCK]       = { .addr = 0x0C, .mask = 0x7F },
};

static int grace_map_set(const grace_map_t *map,uint8_t index,bool state){
    ASSERT_NULL_PTR(map);
    // todo: validar faixa de index para cada mapa e evitar acesso fora de limite.
    tm1629a_buffer_update_bit_state(map[index].addr,map[index].mask,state);
    return 0;
}

int grace_icon_set(grace_icon_id_t icon, bool state) {
    return grace_map_set(grace_icon_map, icon, state);
}

int grace_week_set(grace_week_id_t week, bool state) {
    return grace_map_set(grace_week_map, week, state);
}

int grace_dot_set(grace_dot_id_t dot, bool state) {
    return grace_map_set(grace_dot_map, dot, state);
}

int grace_digit_set(grace_digit_id_t digit, uint8_t value) {
    // todo: checar digit < GRACE_DIGIT_COUNT antes de indexar.
    uint8_t addr = grace_digit_map[digit].addr;
    uint8_t mask = grace_digit_map[digit].mask;
    tm1629a_buffer_update_masked(addr, mask, value);
    return 0;
}

int grace_all_on(void)
{
    uint32_t i;

    /* Liga todos os ícones */
    for (i = 0; i < GRACE_ICON_COUNT; i++) {
        grace_icon_set((grace_icon_id_t)i, TRUE);
    }

    /* Liga todos os dias da semana */
    for (i = 0; i < GRACE_WEEK_COUNT; i++) {
        grace_week_set((grace_week_id_t)i, TRUE);
    }

    /* Liga todos os pontos */
    for (i = 0; i < GRACE_DOT_COUNT; i++) {
        grace_dot_set((grace_dot_id_t)i, TRUE);
    }

    /* Liga todos os segmentos dos dígitos */
    for (i = 0; i < GRACE_DIGIT_COUNT; i++) {
        grace_digit_set((grace_digit_id_t)i, 0x7F);  // todos os segmentos
    }
    tm1629a_display_refresh();
    // todo: unificar grace_all_on/off em helper unico para evitar duplicacao de loops.
    return 0;
}


int grace_all_off(void)
{
    uint32_t i;

    /* Desliga todos os ícones */
    for (i = 0; i < GRACE_ICON_COUNT; i++) {
        grace_icon_set((grace_icon_id_t)i, FALSE);
    }

    /* Desliga todos os dias da semana */
    for (i = 0; i < GRACE_WEEK_COUNT; i++) {
        grace_week_set((grace_week_id_t)i, FALSE);
    }

    /* Desliga todos os pontos */
    for (i = 0; i < GRACE_DOT_COUNT; i++) {
        grace_dot_set((grace_dot_id_t)i, FALSE);
    }

    /* Apaga todos os segmentos dos dígitos */
    for (i = 0; i < GRACE_DIGIT_COUNT; i++) {
        grace_digit_set((grace_digit_id_t)i, 0x00);
    }
    tm1629a_display_refresh();
    return 0;
}


