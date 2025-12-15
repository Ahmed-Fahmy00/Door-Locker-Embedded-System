#include "gptm0_oneshot.h"
#include "components/buzzer.h"
#include <stdbool.h>
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

/* Forward declaration of ISR */
static void Timer0A_Handler(void);

static bool timer0_initialized = false;

void gptm0_oneshot_init(void)
{
    if (timer0_initialized) return;
    
    /* Enable Timer0 peripheral */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)) {}

    /* Configure Timer0A as one-shot */
    TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);

    /* Register interrupt handler */
    TimerIntRegister(TIMER0_BASE, TIMER_A, Timer0A_Handler);
    
    /* Enable timeout interrupt */
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    /* Enable interrupt in NVIC */
    IntEnable(INT_TIMER0A);
    
    timer0_initialized = true;
}

bool gptm0_oneshot_start_seconds(uint32_t seconds)
{
    if (seconds == 0) {
        buzzer_off();
        return true;
    }

    uint32_t clock = SysCtlClockGet();
    uint64_t ticks = (uint64_t)seconds * clock;

    /* 32-bit timer limit */
    if (ticks > 0xFFFFFFFFULL) {
        return false;
    }

    /* Stop any previous timer */
    TimerDisable(TIMER0_BASE, TIMER_A);
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    
    /* Load timer */
    TimerLoadSet(TIMER0_BASE, TIMER_A, (uint32_t)(ticks - 1));

    /* Turn buzzer ON */
    buzzer_on();

    /* Start timer */
    TimerEnable(TIMER0_BASE, TIMER_A);

    return true;
}

void gptm0_oneshot_stop(void)
{
    TimerDisable(TIMER0_BASE, TIMER_A);
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    buzzer_off();
}

/* Timer0A ISR */
static void Timer0A_Handler(void)
{
    /* Clear interrupt */
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    /* Turn buzzer OFF */
    buzzer_off();
}
