/**
 * @file circular_buffer.c
 */

#include "circular_buffer.h"
#include <string.h>

ER cb_init(circular_buffer_t *cb, ID semid)
{
    if (!cb) return E_PAR;
    memset(cb, 0, sizeof(*cb));
    cb->semid = semid;
    return E_OK;
}

ER cb_push(circular_buffer_t *cb, const sensor_sample_t *s)
{
    ER er;

    if (!cb || !s) return E_PAR;

    er = tk_wai_sem(cb->semid, 1, TMO_FEVR);
    if (er < E_OK) return er;

    cb->buf[cb->head] = *s;
    cb->head = (cb->head + 1) % (WINDOW_SAMPLES * 2);
    if (cb->count < WINDOW_SAMPLES)
        cb->count++;

    if (cb->count >= WINDOW_SAMPLES)
        cb->window_ready = TRUE;

    tk_sig_sem(cb->semid, 1);
    return E_OK;
}

ER cb_get_window(circular_buffer_t *cb, sensor_sample_t *out, UW *n)
{
    ER er;
    UW i, start;

    if (!cb || !out || !n) return E_PAR;

    er = tk_wai_sem(cb->semid, 1, TMO_FEVR);
    if (er < E_OK) return er;

    if (!cb->window_ready || cb->count < WINDOW_SAMPLES) {
        tk_sig_sem(cb->semid, 1);
        return E_OBJ;   /* not ready */
    }

    /* oldest sample of the current full window */
    start = (cb->head + (WINDOW_SAMPLES * 2) - WINDOW_SAMPLES) % (WINDOW_SAMPLES * 2);
    for (i = 0; i < WINDOW_SAMPLES; i++)
        out[i] = cb->buf[(start + i) % (WINDOW_SAMPLES * 2)];

    *n = WINDOW_SAMPLES;
    tk_sig_sem(cb->semid, 1);
    return E_OK;
}

void cb_mark_consumed(circular_buffer_t *cb)
{
    if (!cb) return;
    /* keep sliding window; just clear the ready flag so next full window
       will be signalled again after another WINDOW_SAMPLES pushes */
    cb->window_ready = FALSE;
}
