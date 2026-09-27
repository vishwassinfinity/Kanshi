/**
 * @file app_main.c
 * @brief Kanshi – Real-Time Anomaly Detection on µT-Kernel 3.0
 *
 * Task architecture (priorities from kanshi_config.h):
 *   ML Inference  (P1) – event-driven
 *   Vibration     (P2) – 10 ms cyclic
 *   Temperature   (P3) – 100 ms cyclic
 *   System Monitor(P5) – 1 s cyclic
 */

#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include <string.h>

#include "kanshi_config.h"
#include "circular_buffer.h"
#include "features.h"
#include "inference.h"
#include "alert.h"
#include "hw_hal.h"

/* -------------------------------------------------------------------------- */
/* Global objects                                                             */
/* -------------------------------------------------------------------------- */
static ID               id_sem_buffer;
static ID               id_flg_system;
static circular_buffer_t g_cb;
static alert_ctx_t      g_alert;
static float            g_last_temp = 25.0f;

/* Approximate milliseconds from system time (timer period = 10 ms) */
static UW get_ms(void)
{
    SYSTIM t;
    tk_get_otm(&t);
    return t.lo;
}

/* -------------------------------------------------------------------------- */
/* Vibration Task – highest frequency sampling                                */
/* -------------------------------------------------------------------------- */
static void task_vibration(INT stacd, void *exinf)
{
    sensor_sample_t s;
    UW now;
    (void)stacd; (void)exinf;

    tm_printf((UB*)"Vibration task started\n");

    for (;;) {
        now = get_ms();
        s.timestamp_ms = now;

        if (hw_read_accel(&s.x, &s.y, &s.z) == E_OK) {
            s.temp = g_last_temp;   /* latest temp snapshot */
            if (cb_push(&g_cb, &s) == E_OK) {
                /* signal when a full window is ready */
                if (g_cb.window_ready)
                    tk_set_flg(id_flg_system, EVT_WINDOW_READY);
            }
        }

        tk_dly_tsk(SAMPLE_PERIOD_MS);
    }
}

/* -------------------------------------------------------------------------- */
/* Temperature Task                                                           */
/* -------------------------------------------------------------------------- */
static void task_temperature(INT stacd, void *exinf)
{
    float t;
    (void)stacd; (void)exinf;

    tm_printf((UB*)"Temperature task started\n");

    for (;;) {
        if (hw_read_temp(&t) == E_OK) {
            g_last_temp = t;
            if (t >= TEMP_CRIT_C)
                tk_set_flg(id_flg_system, EVT_TEMP_CRITICAL);
            else if (t >= TEMP_WARN_C)
                tk_set_flg(id_flg_system, EVT_TEMP_WARNING);
        }
        tk_dly_tsk(TEMP_PERIOD_MS);
    }
}

/* -------------------------------------------------------------------------- */
/* ML Inference Task – event driven                                           */
/* -------------------------------------------------------------------------- */
static void task_ml_inference(INT stacd, void *exinf)
{
    UINT flgptn;
    sensor_sample_t window[WINDOW_SAMPLES];
    float features[NUM_FEATURES];
    inference_result_t result;
    UW n, now;
    (void)stacd; (void)exinf;

    tm_printf((UB*)"ML Inference task started\n");

    for (;;) {
        /* wait for any of the interesting events */
        tk_wai_flg(id_flg_system, EVT_ALL, TWF_ORW | TWF_CLR, &flgptn, TMO_FEVR);

        now = get_ms();

        if (flgptn & EVT_WINDOW_READY) {
            if (cb_get_window(&g_cb, window, &n) == E_OK) {
                extract_features(window, n, features);
                if (inference_run(features, &result) == E_OK) {
                    alert_update(&g_alert, &result, g_last_temp, now);
                    hw_led_set(g_alert.state);

                    if (result.class_id != CLASS_NORMAL)
                        tk_set_flg(id_flg_system, EVT_ANOMALY);

                    /* optional latency log */
                    if (result.inference_us > 100000)  /* >100 ms warning */
                        tm_printf((UB*)"WARN: inference %u us\n",
                                  result.inference_us);
                }
                cb_mark_consumed(&g_cb);
            }
        }

        /* temperature events also feed the alert machine */
        if (flgptn & (EVT_TEMP_WARNING | EVT_TEMP_CRITICAL)) {
            inference_result_t dummy = {0};
            dummy.probability = (flgptn & EVT_TEMP_CRITICAL) ? 0.9f : 0.6f;
            dummy.class_id    = (flgptn & EVT_TEMP_CRITICAL) ?
                                CLASS_CRITICAL : CLASS_WARNING;
            alert_update(&g_alert, &dummy, g_last_temp, now);
            hw_led_set(g_alert.state);
        }
    }
}

/* -------------------------------------------------------------------------- */
/* System Monitor Task                                                        */
/* -------------------------------------------------------------------------- */
static void task_system_monitor(INT stacd, void *exinf)
{
    UW now;
    (void)stacd; (void)exinf;

    tm_printf((UB*)"System monitor started\n");

    for (;;) {
        now = get_ms();
        hw_watchdog_feed();

        if (hw_button_pressed())
            alert_manual_reset(&g_alert);

        /* simple liveness / state report every 5 s */
        static UW last_report;
        if (now - last_report >= 5000) {
            tm_printf((UB*)"[mon] state=%d temp=%.1f hist=%u\n",
                      (int)g_alert.state, (double)g_last_temp,
                      g_alert.hist_count);
            last_report = now;
        }

        tk_dly_tsk(1000);
    }
}

/* -------------------------------------------------------------------------- */
/* Object creation helpers                                                    */
/* -------------------------------------------------------------------------- */
static ID create_task(FP entry, PRI pri, SZ stksz, const char *name)
{
    T_CTSK ctsk;
    ID id;

    memset(&ctsk, 0, sizeof(ctsk));
    ctsk.tskatr  = TA_HLNG | TA_RNG0;
    ctsk.task    = entry;
    ctsk.itskpri = pri;
    ctsk.stksz   = stksz;
#if USE_OBJECT_NAME
    strncpy((char*)ctsk.dsname, name, 8);
#endif

    id = tk_cre_tsk(&ctsk);
    if (id < E_OK) {
        tm_printf((UB*)"FAIL create %s: %d\n", name, id);
        return id;
    }
    return id;
}

/* -------------------------------------------------------------------------- */
/* usermain – entry point required by µT-Kernel                               */
/* -------------------------------------------------------------------------- */
EXPORT INT usermain(void)
{
    T_CSEM csem;
    T_CFLG cflg;
    ID id_vib, id_temp, id_ml, id_mon;
    ER er;

    tm_printf((UB*)"\n=== Kanshi: Real-Time Anomaly Detection ===\n");
    tm_printf((UB*)"µT-Kernel 3.0 + On-Device TinyML (fallback classifier)\n\n");

    /* --- hardware --- */
    er = hw_init();
    if (er < E_OK) {
        tm_printf((UB*)"hw_init failed %d\n", er);
        return er;
    }

    /* --- semaphore for circular buffer --- */
    memset(&csem, 0, sizeof(csem));
    csem.sematr  = TA_TFIFO | TA_FIRST;
    csem.isemcnt = 1;
    csem.maxsem  = 1;
    id_sem_buffer = tk_cre_sem(&csem);
    if (id_sem_buffer < E_OK) return id_sem_buffer;

    /* --- event flag --- */
    memset(&cflg, 0, sizeof(cflg));
    cflg.flgatr  = TA_TFIFO | TA_WMUL;
    cflg.iflgptn = 0;
    id_flg_system = tk_cre_flg(&cflg);
    if (id_flg_system < E_OK) return id_flg_system;

    /* --- application objects --- */
    cb_init(&g_cb, id_sem_buffer);
    alert_init(&g_alert);
    inference_init();

    /* --- create & start tasks --- */
    id_vib  = create_task(task_vibration,     PRI_VIBRATION,      STACK_VIBRATION,      "vib");
    id_temp = create_task(task_temperature,   PRI_TEMPERATURE,    STACK_TEMPERATURE,    "temp");
    id_ml   = create_task(task_ml_inference,  PRI_ML_INFERENCE,   STACK_ML_INFERENCE,   "ml");
    id_mon  = create_task(task_system_monitor,PRI_SYSTEM_MONITOR, STACK_SYSTEM_MONITOR, "mon");

    if (id_vib < E_OK || id_temp < E_OK || id_ml < E_OK || id_mon < E_OK)
        return E_SYS;

    tk_sta_tsk(id_vib,  0);
    tk_sta_tsk(id_temp, 0);
    tk_sta_tsk(id_ml,   0);
    tk_sta_tsk(id_mon,  0);

    tm_printf((UB*)"All tasks running. Monitoring…\n");

    /* usermain may return; kernel continues with the created tasks */
    return E_OK;
}
