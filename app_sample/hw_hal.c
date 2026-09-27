/**
 * @file hw_hal.c
 *
 * When USE_SIMULATED_SENSORS == 1 a synthetic vibration + temperature
 * signal is generated so the whole pipeline can be tested without hardware.
 * Replace the #if blocks with real I2C / GPIO register access for the
 * nRF52833 / LSM303AGR when running on the micro:bit.
 */

#include "hw_hal.h"
#include "alert.h"
#include <math.h>
#include <tm/tmonitor.h>

#if USE_SIMULATED_SENSORS
static float sim_phase = 0.0f;
static float sim_temp  = 28.0f;
static UW    sim_tick  = 0;
static BOOL  sim_fault = FALSE;
#endif

ER hw_init(void)
{
#if USE_SIMULATED_SENSORS
    tm_printf((UB*)"HAL: simulated sensors enabled\n");
    return E_OK;
#else
    /* TODO: initialise I2C, configure LSM303AGR ±2g / 100 Hz,
     *       set up GPIO for LEDs and buttons, enable WDT */
    return E_OK;
#endif
}

ER hw_read_accel(float *x, float *y, float *z)
{
#if USE_SIMULATED_SENSORS
    /* Nominal 50 Hz vibration + occasional fault (high kurtosis / crest) */
    float amp = sim_fault ? 1.8f : 0.15f;
    float noise = ((float)(sim_tick % 17) - 8.0f) * 0.01f;
    *x = amp * sinf(sim_phase) + noise;
    *y = amp * 0.3f * cosf(sim_phase * 1.3f);
    *z = 1.0f + noise * 0.2f;          /* gravity dominant on Z */
    sim_phase += 2.0f * 3.14159265f * 50.0f / SAMPLE_RATE_HZ;
    if (sim_phase > 6.2831853f) sim_phase -= 6.2831853f;
    sim_tick++;
    /* inject a fault every ~20 s for demo */
    if ((sim_tick % 2000) == 1000) sim_fault = TRUE;
    if ((sim_tick % 2000) == 1500) sim_fault = FALSE;
    return E_OK;
#else
    /* TODO: I2C read OUT_X/Y/Z_L/H registers, convert to g */
    *x = *y = *z = 0.0f;
    return E_OK;
#endif
}

ER hw_read_temp(float *temp_c)
{
#if USE_SIMULATED_SENSORS
    /* slow drift + fault heat */
    sim_temp += (sim_fault ? 0.05f : -0.01f);
    if (sim_temp < 25.0f) sim_temp = 25.0f;
    if (sim_temp > 70.0f) sim_temp = 70.0f;
    *temp_c = sim_temp;
    return E_OK;
#else
    /* TODO: on-die temp or LSM303AGR temp sensor */
    *temp_c = 25.0f;
    return E_OK;
#endif
}

void hw_led_set(alert_state_t state)
{
#if USE_SIMULATED_SENSORS
    const char *name[] = {"GREEN", "YELLOW", "RED"};
    static alert_state_t last = (alert_state_t)-1;
    if (state != last) {
        tm_printf((UB*)"LED → %s\n", name[state]);
        last = state;
    }
#else
    /* TODO: micro:bit LED matrix patterns */
    (void)state;
#endif
}

void hw_watchdog_feed(void)
{
#if ENABLE_WATCHDOG
    /* TODO: reload nRF52833 WDT */
#endif
}

BOOL hw_button_pressed(void)
{
#if USE_SIMULATED_SENSORS
    return FALSE;   /* no button in sim */
#else
    /* TODO: read BUTTON_A / BUTTON_B GPIO */
    return FALSE;
#endif
}
