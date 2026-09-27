/**
 * @file alert.c
 */

#include "alert.h"
#include <tm/tmonitor.h>
#include <string.h>

void alert_init(alert_ctx_t *ctx)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->state = STATE_NORMAL;
}

static void push_history(alert_ctx_t *ctx, anomaly_class_t c,
                         float p, float temp, UW ts)
{
    event_record_t *r = &ctx->history[ctx->hist_head];
    r->timestamp_ms = ts;
    r->class_id     = c;
    r->probability  = p;
    r->temp_c       = temp;
    ctx->hist_head  = (ctx->hist_head + 1) % EVENT_HISTORY_SIZE;
    if (ctx->hist_count < EVENT_HISTORY_SIZE)
        ctx->hist_count++;
}

void alert_update(alert_ctx_t *ctx, const inference_result_t *inf,
                  float temp_c, UW now_ms)
{
    BOOL is_anomaly = (inf->class_id != CLASS_NORMAL);

    if (is_anomaly)
        ctx->consec_anomaly++;
    else
        ctx->consec_anomaly = 0;

    switch (ctx->state) {
    case STATE_NORMAL:
        if (ctx->consec_anomaly >= CONSEC_ANOMALY_FOR_WARN ||
            temp_c >= TEMP_WARN_C) {
            ctx->state = STATE_WARNING;
            ctx->warn_start_ms = now_ms;
            push_history(ctx, inf->class_id, inf->probability, temp_c, now_ms);
            tm_printf((UB*)"[%u] WARNING  p=%.2f temp=%.1f\n",
                      now_ms, (double)inf->probability, (double)temp_c);
        }
        break;

    case STATE_WARNING:
        if (inf->class_id == CLASS_CRITICAL || temp_c >= TEMP_CRIT_C) {
            ctx->state = STATE_CRITICAL;
            push_history(ctx, CLASS_CRITICAL, inf->probability, temp_c, now_ms);
            tm_printf((UB*)"[%u] CRITICAL p=%.2f temp=%.1f\n",
                      now_ms, (double)inf->probability, (double)temp_c);
        } else if (!is_anomaly &&
                   (now_ms - ctx->warn_start_ms) > NORMAL_RESET_MS) {
            ctx->state = STATE_NORMAL;
            tm_printf((UB*)"[%u] NORMAL (recovered)\n", now_ms);
        } else if (is_anomaly) {
            /* still in warning – refresh timer */
            ctx->warn_start_ms = now_ms;
        }
        break;

    case STATE_CRITICAL:
        /* requires manual reset (button) – do nothing here */
        break;
    }
}

void alert_manual_reset(alert_ctx_t *ctx)
{
    if (ctx->state == STATE_CRITICAL) {
        ctx->state = STATE_NORMAL;
        ctx->consec_anomaly = 0;
        tm_printf((UB*)"CRITICAL cleared by user\n");
    }
}
