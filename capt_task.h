/*
 * capt_task.h
 *
 *  Created on: 31 de dez. de 2025
 *      Author: vinicius.andrade
 */

#ifndef CAPT_TASK_H_
#define CAPT_TASK_H_

#define CAPT_FAST_PERIOD_MS      10      // varredura rápida
#define CAPT_SLOW_PERIOD_MS      50      // varredura lenta (economia de CPU + estabilidade)
#define CAPT_IDLE_TO_FAST_MS    200      // tempo sem toque → volta para FAST
#define CAPT_BASELINE_UPDATE_MS 2000     // tempo estável → recalibra baseline

//#define CAPT_BTN_MASK(btn)    (1u << (btn))

//#define CAPT_BTN_SET(map,btn)   ((map) |=  CAPT_BTN_MASK(btn))
//#define CAPT_BTN_CLR(map,btn)   ((map) &= ~CAPT_BTN_MASK(btn))
//#define CAPT_BTN_TST(map,btn)   ((map) &   CAPT_BTN_MASK(btn))

typedef enum {
    CAPT_MODE_FAST,
    CAPT_MODE_SLOW,
} capt_mode_t;

typedef enum {
    kAPP_TouchStateInit,
    kAPP_TouchStateCalib,
    kAPP_TouchStateDetect
} app_touch_state_t;

typedef struct {
    capt_mode_t mode;
    app_touch_state_t task_state;
    uint8_t     key_bitmap; // parei aqui
    uint32_t    last_touch_ms;
    uint32_t    last_sample_ms;
    uint32_t    last_baseline_ms;
} capt_task_t;

uint32_t systick_get_ms(void);
void delay_ms(uint32_t ms);

void capt_task_init(void);
void capt_task(capt_task_t *ctx);

#endif /* CAPT_TASK_H_ */