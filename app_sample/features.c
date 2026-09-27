/**
 * @file features.c
 * Fixed-point friendly floating-point feature extraction.
 */

#include "features.h"
#include <math.h>

static float mean_f(const float *v, UW n)
{
    float s = 0.0f;
    UW i;
    for (i = 0; i < n; i++) s += v[i];
    return s / (float)n;
}

static float std_f(const float *v, UW n, float m)
{
    float s = 0.0f;
    UW i;
    for (i = 0; i < n; i++) {
        float d = v[i] - m;
        s += d * d;
    }
    return sqrtf(s / (float)n);
}

static float rms_f(const float *v, UW n)
{
    float s = 0.0f;
    UW i;
    for (i = 0; i < n; i++) s += v[i] * v[i];
    return sqrtf(s / (float)n);
}

static float peak_f(const float *v, UW n)
{
    float p = 0.0f;
    UW i;
    for (i = 0; i < n; i++) {
        float a = fabsf(v[i]);
        if (a > p) p = a;
    }
    return p;
}

static float peak_to_peak_f(const float *v, UW n)
{
    float mn = v[0], mx = v[0];
    UW i;
    for (i = 1; i < n; i++) {
        if (v[i] < mn) mn = v[i];
        if (v[i] > mx) mx = v[i];
    }
    return mx - mn;
}

static float kurtosis_f(const float *v, UW n, float m, float sd)
{
    float s = 0.0f;
    UW i;
    if (sd < 1e-6f) return 0.0f;
    for (i = 0; i < n; i++) {
        float z = (v[i] - m) / sd;
        s += z * z * z * z;
    }
    return s / (float)n - 3.0f;   /* excess kurtosis */
}

static float zcr_f(const float *v, UW n)
{
    UW crossings = 0, i;
    for (i = 1; i < n; i++) {
        if ((v[i-1] >= 0.0f && v[i] < 0.0f) ||
            (v[i-1] < 0.0f && v[i] >= 0.0f))
            crossings++;
    }
    return (float)crossings / (float)(n - 1);
}

void extract_features(const sensor_sample_t *win, UW n, float features[NUM_FEATURES])
{
    float x[WINDOW_SAMPLES];
    float y[WINDOW_SAMPLES];
    float z[WINDOW_SAMPLES];
    float comp[WINDOW_SAMPLES];
    float temps[WINDOW_SAMPLES];
    UW i;
    float mx, sx, rx, px, p2p, cf, ku, zc;
    float tema, tdelta, tmax;
    float crms;

    if (n > WINDOW_SAMPLES) n = WINDOW_SAMPLES;

    for (i = 0; i < n; i++) {
        x[i] = win[i].x;
        y[i] = win[i].y;
        z[i] = win[i].z;
        comp[i] = sqrtf(x[i]*x[i] + y[i]*y[i] + z[i]*z[i]);
        temps[i] = win[i].temp;
    }

    /* Use X-axis as primary vibration channel (plan focuses on it) */
    mx  = mean_f(x, n);
    sx  = std_f(x, n, mx);
    rx  = rms_f(x, n);
    px  = peak_f(x, n);
    p2p = peak_to_peak_f(x, n);
    cf  = (rx > 1e-6f) ? (px / rx) : 0.0f;
    ku  = kurtosis_f(x, n, mx, sx);
    zc  = zcr_f(x, n);

    tema   = mean_f(temps, n);          /* simple EMA proxy for window */
    tdelta = temps[n-1] - temps[0];
    tmax   = temps[0];
    for (i = 1; i < n; i++)
        if (temps[i] > tmax) tmax = temps[i];

    crms = rms_f(comp, n);

    /* Rough normalisation to [-1, +1] range typical for vibration/temp */
    features[0]  = mx  / 2.0f;          /* accel ~ ±2g */
    features[1]  = sx  / 1.0f;
    features[2]  = rx  / 2.0f;
    features[3]  = px  / 2.0f;
    features[4]  = p2p / 4.0f;
    features[5]  = (cf - 1.0f) / 5.0f;  /* crest factor often 1–6 */
    features[6]  = ku  / 10.0f;
    features[7]  = zc;
    features[8]  = (tema - 25.0f) / 40.0f;
    features[9]  = tdelta / 10.0f;
    features[10] = (tmax - 25.0f) / 40.0f;
    features[11] = crms / 2.0f;
}
