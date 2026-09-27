/**
 * @file inference.h
 * @brief On-device anomaly classification
 *
 * When USE_TFLITE_MICRO == 1 the real TFLite Micro interpreter is used.
 * Otherwise a lightweight classical detector (logistic-style + rules) is used
 * so the full RTOS pipeline can be demonstrated immediately.
 */

#ifndef INFERENCE_H
#define INFERENCE_H

#include "kanshi_config.h"

typedef enum {
    CLASS_NORMAL   = 0,
    CLASS_WARNING  = 1,
    CLASS_CRITICAL = 2
} anomaly_class_t;

typedef struct {
    float            probability;   /* P(anomaly) ∈ [0,1] */
    anomaly_class_t  class_id;
    UW               inference_us;  /* measured latency   */
} inference_result_t;

ER  inference_init(void);
ER  inference_run(const float features[NUM_FEATURES], inference_result_t *res);

#endif /* INFERENCE_H */
