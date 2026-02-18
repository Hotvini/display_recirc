#include "capt_task.h"
#include "systick.h"
#include "capt_proc.h"

#define SYSTICK_HZ 1000U


static capt_task_t captTask;

/* Touch processing */
static touch_proc_t touchProc;

/* Debounce (index-level) */
static key_debounce_t keyDebounce;

/* Raw CAPT snapshot */
static int16_t captRaw[CAPT_BTN_COUNT];

/* FSM helpers */
static int32_t  lastKey;

static uint8_t prev_bitmap;

static void systick_init(void)
{
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / SYSTICK_HZ);
}

void capt_task_init(void)
{
    systick_init();
    capt_init();
    touch_proc_init(&touchProc);
    key_debounce_init(&keyDebounce);
    memset(&captTask, 0, sizeof(captTask));
    lastKey = CAPT_BTN_COUNT;
    prev_bitmap = 0;
}

bool capt_task(capt_task_t *ctx)
{
    uint32_t now_ms = systick_get_ms();

    uint32_t period = (ctx->mode == CAPT_MODE_FAST) ? CAPT_FAST_PERIOD_MS : CAPT_SLOW_PERIOD_MS;

    if ((now_ms - ctx->last_sample_ms) < period)
        return false;

    ctx->last_sample_ms = now_ms;

    if (!capt_get_sample(captRaw))
        return; false;

    touch_proc_push_sample(&touchProc, captRaw);

    switch (ctx->task_state)
    {
        /* ---------- INIT ---------- */
        case kAPP_TouchStateInit:

            leds_all_off();

            if (touch_proc_is_stable(&touchProc))
            {
                ctx->task_state = kAPP_TouchStateCalib;
                leds_all_on();
            }
            break;

        /* ---------- CALIB ---------- */
        case kAPP_TouchStateCalib:

            touch_proc_update_baseline(&touchProc);
            ctx->last_baseline_ms = now_ms;
            ctx->task_state = kAPP_TouchStateDetect;
            break;

        /* ---------- DETECT ---------- */
        case kAPP_TouchStateDetect:
        {
            touch_proc_compute_fast_delta(&touchProc, captRaw);

            int key_raw = touch_detect_key(&touchProc);
            int key = key_raw;
            //int key     = key_debounce_step(&keyDebounce, key_raw);

            bool touched = (key < CAPT_BTN_COUNT);

            if (touched)
            {
                captTask.last_touch_ms = now_ms;
                captTask.mode = CAPT_MODE_SLOW;
            }

            if (key >= CAPT_BTN_COUNT)
            {
                //ctx->key_bitmap = 0;
            }
            else
            {
                if (key != lastKey && lastKey < CAPT_BTN_COUNT)
                    //ctx->key_bitmap = (1 << key);
            }

            lastKey = key;
            break;
        }

        default:
            ctx->task_state = kAPP_TouchStateInit;
            break;
    }

    /* Volta para FAST após tempo sem toque */
    if (ctx->mode == CAPT_MODE_SLOW &&
        (now_ms - ctx->last_touch_ms) > CAPT_IDLE_TO_FAST_MS)
    {
        ctx->mode = CAPT_MODE_FAST;
    }

    /* Recalibração automática de baseline */
    if ((now_ms - ctx->last_touch_ms) > CAPT_BASELINE_UPDATE_MS &&
        (now_ms - ctx->last_baseline_ms) > CAPT_BASELINE_UPDATE_MS)
    {
        touch_proc_update_baseline(&touchProc);
        ctx->last_baseline_ms = now_ms;
    }

    return true;
}

static volatile uint32_t ms_ticks = 0;


void SysTick_Handler(void)
{
	ms_ticks++;
   if (capt_task(&captTask))
    touch_proc_push_sample(&touchProc, captRaw);
}

uint32_t systick_get_ms(void)
{
    return (ms_ticks);
}

//debug
void delay_ms(uint32_t ms)
{
    uint32_t start = ms_ticks;
    while ((ms_ticks - start) < ms)
    {
        __NOP();
    }
}
