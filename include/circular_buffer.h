/**
 * @file circular_buffer.h
 * @brief Thread-safe double-buffered circular buffer for sensor samples
 */

#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include "kanshi_config.h"

typedef struct {
    float x, y, z;
    float temp;
    UW    timestamp_ms;
} sensor_sample_t;

typedef struct {
    sensor_sample_t buf[WINDOW_SAMPLES * 2]; /* double buffer */
    UW   head;          /* next write index                  */
    UW   count;         /* samples currently in active window*/
    BOOL window_ready;  /* set when a full window is ready   */
    ID   semid;         /* binary semaphore protecting buf   */
} circular_buffer_t;

ER  cb_init(circular_buffer_t *cb, ID semid);
ER  cb_push(circular_buffer_t *cb, const sensor_sample_t *s);
ER  cb_get_window(circular_buffer_t *cb, sensor_sample_t *out, UW *n);
void cb_mark_consumed(circular_buffer_t *cb);

#endif /* CIRCULAR_BUFFER_H */
