/**
 * @file inference.c
 *
 * Fallback classical detector that mirrors the intended TinyML behaviour.
 * Replace the body of inference_run() with TFLite Micro calls when ready.
 */

#include "inference.h"
#include <math.h>

#if USE_TFLITE_MICRO
/* ----------------------------------------------------------------------
 *  When you integrate TFLite Micro:
 *  1. Add the TFLite Micro sources / library to the build
 *  2. Place the INT8 model as a C array (model.h)
 *  3. Allocate a static tensor arena (~24 KB)
 *  4. Create interpreter once in inference_init()
 *  5. Copy features → input tensor, Invoke(), read output
 * ---------------------------------------------------------------------- */
#include "tensorflow/lite/micro/micro_interpreter.h"
/* … etc. – left as integration point … */
#endif

/* Simple logistic-style weights learned offline (placeholder).
 * These approximate a model trained on the 12 features.
 * Real weights will come from the Python training script. */
static const float W[NUM_FEATURES] = {
    0.15f, 0.40f, 0.35f, 0.30f, 0.25f,  /* mean,std,rms,peak,p2p */
    0.50f, 0.55f, 0.20f,                /* crest, kurtosis, zcr   */
    0.45f, 0.30f, 0.40f,                /* temp features          */
    0.35f                               /* composite rms          */
};
static const float BIAS = -1.2f;

static float sigmoid(float x)
{
    if (x < -10.0f) return 0.0f;
    if (x >  10.0f) return 1.0f;
    return 1.0f / (1.0f + expf(-x));
}

ER inference_init(void)
{
#if USE_TFLITE_MICRO
    /* create interpreter, allocate tensors … */
#endif
    return E_OK;
}

ER inference_run(const float features[NUM_FEATURES], inference_result_t *res)
{
    UW t0, t1;
    float logit = BIAS;
    UW i;
    SYSTIM st;

    if (!features || !res) return E_PAR;

    /* crude timing – replace with DWT_CYCCNT on Cortex-M4 when available */
    tk_get_otm(&st);
    t0 = st.lo;

#if USE_TFLITE_MICRO
    /* TFLite path:
     *   memcpy input tensor
     *   interpreter->Invoke()
     *   read output[0]
     */
    res->probability = 0.0f; /* placeholder */
#else
    for (i = 0; i < NUM_FEATURES; i++)
        logit += W[i] * features[i];
    res->probability = sigmoid(logit);
#endif

    tk_get_otm(&st);
    t1 = st.lo;
    res->inference_us = (t1 - t0) * 1000u;   /* rough ms→µs */

    if (res->probability > ANOMALY_CRIT_PROB)
        res->class_id = CLASS_CRITICAL;
    else if (res->probability > ANOMALY_WARN_PROB)
        res->class_id = CLASS_WARNING;
    else
        res->class_id = CLASS_NORMAL;

    return E_OK;
}
