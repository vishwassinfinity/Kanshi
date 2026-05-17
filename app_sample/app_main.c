/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.06
 *
 *    Copyright (C) 2006-2022 by Ken Sakamura.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 *
 *    Released by TRON Forum(http://www.tron.org) at 2022/10.
 *
 *----------------------------------------------------------------------
 */

#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include <sys/sysdepend/cpu/nrf5/sysdef.h>

/* micro:bit v2 LED matrix GPIO pins (nRF52833) */
/* Rows - P0 pins */
#define LED_ROW1  (1UL << 21)
#define LED_ROW2  (1UL << 22)
#define LED_ROW3  (1UL << 15)
#define LED_ROW4  (1UL << 24)
#define LED_ROW5  (1UL << 19)

/* Columns - P0 pins (active LOW) */
#define LED_COL1  (1UL << 28)
#define LED_COL2  (1UL << 11)
#define LED_COL3  (1UL << 31)
#define LED_COL5  (1UL << 30)

/* P1 pin */
#define LED_COL4  (1UL << 5)

/* Register access macro */
#define REG(addr)  (*((volatile UW*)(addr)))

LOCAL void led_init(void)
{
    /* Set all ROW and COL pins as OUTPUT on P0 */
    REG(GPIO_P0_BASE + GPIO_DIRSET) = 
        LED_ROW1 | LED_ROW2 | LED_ROW3 | LED_ROW4 | LED_ROW5 |
        LED_COL1 | LED_COL2 | LED_COL3 | LED_COL5;

    /* COL4 is on P1 */
    REG(GPIO_P1_BASE + GPIO_DIRSET) = LED_COL4;

    /* Turn all LEDs OFF first */
    /* Rows LOW, Cols HIGH (cols are active low) */
    REG(GPIO_P0_BASE + GPIO_OUTCLR) =
        LED_ROW1 | LED_ROW2 | LED_ROW3 | LED_ROW4 | LED_ROW5;
    REG(GPIO_P0_BASE + GPIO_OUTSET) =
        LED_COL1 | LED_COL2 | LED_COL3 | LED_COL5;
    REG(GPIO_P1_BASE + GPIO_OUTSET) = LED_COL4;
}

LOCAL void led_on(void)
{
    /* Light up top-left LED: ROW1 HIGH, COL1 LOW */
    REG(GPIO_P0_BASE + GPIO_OUTSET) = LED_ROW1;
    REG(GPIO_P0_BASE + GPIO_OUTCLR) = LED_COL1;
}

LOCAL void led_off(void)
{
    REG(GPIO_P0_BASE + GPIO_OUTCLR) = LED_ROW1;
    REG(GPIO_P0_BASE + GPIO_OUTSET) = LED_COL1;
}

LOCAL void main_task(INT stacd, void *exinf)
{
    INT count = 0;

    led_init();

    while(1) {
        count++;
        tm_printf((UB*)"Kanshi tick %d - LED ON\n", count);
        led_on();
        tk_dly_tsk(500);

        tm_printf((UB*)"Kanshi tick %d - LED OFF\n", count);
        led_off();
        tk_dly_tsk(500);
    }
}

EXPORT INT usermain(void)
{
    T_CTSK ctsk;
    tm_putstring((UB*)"Kanshi booting...\n");
    ctsk.itskpri = 10;
    ctsk.stksz   = 1024;
    ctsk.task    = main_task;
    ctsk.tskatr  = TA_HLNG | TA_RNG3;
    ID tskid = tk_cre_tsk(&ctsk);
    tk_sta_tsk(tskid, 0);
    tk_slp_tsk(TMO_FEVR);
    return 0;
}