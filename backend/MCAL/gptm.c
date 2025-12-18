
#include "gptm.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include <stdbool.h>

// Timer0A (buzzer)
static void (*timer0a_isr)(void) = 0;
static bool timer0a_running = false;

void Timer0A_Init_OneShot(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)) {}
    TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);
    TimerIntRegister(TIMER0_BASE, TIMER_A, timer0a_isr ? timer0a_isr : Timer0A_DefaultHandler);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntEnable(INT_TIMER0A);
    timer0a_running = false;
}

void Timer0A_Start_OneShot(uint32_t ticks)
{
    TimerDisable(TIMER0_BASE, TIMER_A);
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerLoadSet(TIMER0_BASE, TIMER_A, ticks - 1);
    TimerEnable(TIMER0_BASE, TIMER_A);
    timer0a_running = true;
}

void Timer0A_Stop(void)
{
    TimerDisable(TIMER0_BASE, TIMER_A);
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    timer0a_running = false;
}

void Timer0A_SetISR(void (*handler)(void))
{
    timer0a_isr = handler;
    TimerIntRegister(TIMER0_BASE, TIMER_A, timer0a_isr ? timer0a_isr : Timer0A_DefaultHandler);
}

bool Timer0A_IsRunning(void)
{
    return timer0a_running;
}

void Timer0A_DefaultHandler(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    timer0a_running = false;
}

// Timer1A (motor/door)
static void (*timer1a_isr)(void) = 0;
static bool timer1a_running = false;

void Timer1A_Init_OneShot(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1)) {}
    TimerConfigure(TIMER1_BASE, TIMER_CFG_ONE_SHOT);
    TimerIntRegister(TIMER1_BASE, TIMER_A, timer1a_isr ? timer1a_isr : Timer1A_DefaultHandler);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    IntEnable(INT_TIMER1A);
    timer1a_running = false;
}

void Timer1A_Start_OneShot(uint32_t ticks)
{
    TimerDisable(TIMER1_BASE, TIMER_A);
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    TimerLoadSet(TIMER1_BASE, TIMER_A, ticks - 1);
    TimerEnable(TIMER1_BASE, TIMER_A);
    timer1a_running = true;
}

void Timer1A_Stop(void)
{
    TimerDisable(TIMER1_BASE, TIMER_A);
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    timer1a_running = false;
}

void Timer1A_SetISR(void (*handler)(void))
{
    timer1a_isr = handler;
    TimerIntRegister(TIMER1_BASE, TIMER_A, timer1a_isr ? timer1a_isr : Timer1A_DefaultHandler);
}

bool Timer1A_IsRunning(void)
{
    return timer1a_running;
}

void Timer1A_DefaultHandler(void)
{
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    timer1a_running = false;
}
