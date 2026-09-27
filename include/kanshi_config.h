/**
 * @file kanshi_config.h
 * @brief Kanshi configuration constants and feature set
 *
 * Real-Time Anomaly Detection on µT-Kernel 3.0 with On-Device TinyML
 * TRON Programming Contest 2026 — RTOS Application (Student)
 */

#ifndef KANSHI_CONFIG_H
#define KANSHI_CONFIG_H

#include <tk/tkernel.h>

/* -------------------------------------------------------------------------- */
/* Hardware / sampling                                                        */
/* -------------------------------------------------------------------------- */
#define SAMPLE_RATE_HZ          100     /* Vibration sampling rate            */
#define SAMPLE_PERIOD_MS        10      /* 100 Hz → 10 ms                     */
#define TEMP_RATE_HZ            10
#define TEMP_PERIOD_MS          100
#define WINDOW_SAMPLES          100     /* 1-second window at 100 Hz          */
#define NUM_AXES                3       /* X, Y, Z                            */

/* -------------------------------------------------------------------------- */
/* Feature vector (must match training script)                                */
/* -------------------------------------------------------------------------- */
#define NUM_FEATURES            12
/*
 *  0  mean_x          1  std_x           2  rms_x
 *  3  peak_x          4  peak_to_peak_x  5  crest_factor_x
 *  6  kurtosis_x      7  zcr_x
 *  8  temp_ema        9  temp_delta     10  temp_max
 * 11  composite_rms   (sqrt(x²+y²+z²) RMS)
 */

/* -------------------------------------------------------------------------- */
/* Task priorities (lower number = higher priority on µT-Kernel)              */
/* -------------------------------------------------------------------------- */
#define PRI_ML_INFERENCE        1       /* Highest – event driven             */
#define PRI_VIBRATION           2
#define PRI_TEMPERATURE         3
#define PRI_SYSTEM_MONITOR      5       /* Lowest                             */

#define STACK_VIBRATION         1024
#define STACK_TEMPERATURE       512
#define STACK_ML_INFERENCE      2048    /* Larger for TFLite arena later      */
#define STACK_SYSTEM_MONITOR    512

/* -------------------------------------------------------------------------- */
/* Event flags (flg_system)                                                   */
/* -------------------------------------------------------------------------- */
#define EVT_WINDOW_READY        (1u << 0)
#define EVT_TEMP_WARNING        (1u << 1)
#define EVT_TEMP_CRITICAL       (1u << 2)
#define EVT_ANOMALY             (1u << 3)
#define EVT_ALL                 (EVT_WINDOW_READY | EVT_TEMP_WARNING | \
                                 EVT_TEMP_CRITICAL | EVT_ANOMALY)

/* -------------------------------------------------------------------------- */
/* Alert thresholds                                                           */
/* -------------------------------------------------------------------------- */
#define ANOMALY_WARN_PROB       0.50f
#define ANOMALY_CRIT_PROB       0.80f
#define TEMP_WARN_C             45.0f
#define TEMP_CRIT_C             60.0f
#define CONSEC_ANOMALY_FOR_WARN 3
#define PERSIST_MS_FOR_WARN     30000
#define NORMAL_RESET_MS         10000

/* -------------------------------------------------------------------------- */
/* Memory / logging                                                           */
/* -------------------------------------------------------------------------- */
#define EVENT_HISTORY_SIZE      50
#define UART_LOG_LEVEL          2       /* 0=ERROR … 3=DEBUG                  */

/* -------------------------------------------------------------------------- */
/* Compile-time switches                                                      */
/* -------------------------------------------------------------------------- */
#define USE_TFLITE_MICRO        0       /* 1 when TFLite Micro is linked      */
#define USE_SIMULATED_SENSORS   1       /* 1 for host / no hardware testing   */
#define ENABLE_WATCHDOG         0       /* Enable when hardware WDT ready     */

#endif /* KANSHI_CONFIG_H */
