/**
 * @file alert.h
 * @brief Alert state machine + event history
 */

#ifndef ALERT_H
#define ALERT_H

#include "kanshi_config.h"
#include "inference.h"

typedef enum {
    STATE_NORMAL   = 0,
    STATE_WARNING  = 1,
    STATE_CRITICAL = 2
} alert_state_t;

typedef struct {
    UW               timestamp_ms;
    anomaly_class_t  class_id;
    float            probability;
    float            temp_c;
} event_record_t;

typedef struct {
    alert_state_t    state;
    UW               consec_anomaly;
    UW               warn_start_ms;
    UW               last_normal_ms;
    event_record_t   history[EVENT_HISTORY_SIZE];
    UW               hist_head;
    UW               hist_count;
} alert_ctx_t;

void alert_init(alert_ctx_t *ctx);
void alert_update(alert_ctx_t *ctx, const inference_result_t *inf,
                  float temp_c, UW now_ms);
void alert_manual_reset(alert_ctx_t *ctx);   /* button press for CRITICAL */

#endif /* ALERT_H */
