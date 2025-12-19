/******************************************************************************
 * File: gptm.c
 * Module: General Purpose Timer Module (MCAL Layer)
 * Description: Low-level timer hardware implementation for TM4C123GH6PM
 * Author: Ahmedhh
 * Date: December 18, 2025
 ******************************************************************************/

#include "gptm.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include <stdbool.h>

/******************************************************************************
 *          Timer0 Implementation (Full 32-bit - Buzzer)                     *
 * Note: Uses both A and B in concatenated mode, but only A generates IRQ    *
 ******************************************************************************/

// Timer0 (buzzer) - uses both A and B subtimers concatenated
static volatile bool timer0_running = false;

void Timer0_Init_OneShot(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)) {}
    
    /* Configure as full-width (32-bit concatenated) one-shot timer */
    TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);
    
    /* Enable interrupt for Timer A only (in full-width mode, only A generates IRQ) */
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntEnable(INT_TIMER0A);
    
    timer0_running = false;
}

void Timer0_Start_OneShot(uint32_t ticks)
{
    TimerDisable(TIMER0_BASE, TIMER_BOTH);
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT | TIMER_TIMB_TIMEOUT);
    TimerLoadSet(TIMER0_BASE, TIMER_A, ticks - 1);
    TimerEnable(TIMER0_BASE, TIMER_BOTH);
    timer0_running = true;
}

void Timer0_Stop(void)
{
    TimerDisable(TIMER0_BASE, TIMER_BOTH);
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT | TIMER_TIMB_TIMEOUT);
    timer0_running = false;
}

bool Timer0_IsRunning(void)
{
    return timer0_running;
}

/******************************************************************************
 *          Timer1 Implementation (Full 32-bit - Motor/Door)                 *
 * Note: Uses both A and B in concatenated mode, but only A generates IRQ    *
 ******************************************************************************/

// Timer1 (motor/door) - uses both A and B subtimers concatenated
static volatile bool timer1_running = false;

void Timer1_Init_OneShot(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1)) {}
    
    /* Configure as full-width (32-bit concatenated) one-shot timer */
    TimerConfigure(TIMER1_BASE, TIMER_CFG_ONE_SHOT);
    
    /* Enable interrupt for Timer A only (in full-width mode, only A generates IRQ) */
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    IntEnable(INT_TIMER1A);
    
    timer1_running = false;
}

void Timer1_Start_OneShot(uint32_t ticks)
{
    TimerDisable(TIMER1_BASE, TIMER_BOTH);
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT | TIMER_TIMB_TIMEOUT);
    TimerLoadSet(TIMER1_BASE, TIMER_A, ticks - 1);
    TimerEnable(TIMER1_BASE, TIMER_BOTH);
    timer1_running = true;
}

void Timer1_Stop(void)
{
    TimerDisable(TIMER1_BASE, TIMER_BOTH);
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT | TIMER_TIMB_TIMEOUT);
    timer1_running = false;
}

bool Timer1_IsRunning(void)
{
    return timer1_running;
}
