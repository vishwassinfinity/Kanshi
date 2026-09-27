/**
 * @file hw_hal.h
 * @brief Hardware abstraction for micro:bit v2 (LSM303AGR + on-die temp + LEDs)
 */

#ifndef HW_HAL_H
#define HW_HAL_H

#include "kanshi_config.h"

ER   hw_init(void);
ER   hw_read_accel(float *x, float *y, float *z);   /* g units          */
ER   hw_read_temp(float *temp_c);
void hw_led_set(alert_state_t state);               /* green/yellow/red */
void hw_watchdog_feed(void);
BOOL hw_button_pressed(void);                       /* for CRITICAL reset*/

#endif /* HW_HAL_H */
