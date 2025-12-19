/******************************************************************************
 * File: buzzer_service.c
 * Module: Buzzer Service (Application Layer)
 * Description: High-level buzzer control with automatic timeout
 * Author: Ahmedhh
 * Date: December 19, 2025
 ******************************************************************************/

#include "buzzer_service.h"
#include "../HAL/buzzer.h"
#include "../MCAL/gptm.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "inc/hw_memmap.h"

/******************************************************************************
 *                           Private Variables                                 *
 ******************************************************************************/

static volatile bool buzzer_active = false;

/******************************************************************************
 *                      Private Function Prototypes                            *
 ******************************************************************************/

static void BuzzerService_StartTimer(uint32_t seconds);

/******************************************************************************
 *                          Function Definitions                               *
 ******************************************************************************/

/*
 * BuzzerService_Init
 * Initializes buzzer GPIO and Timer0 for timeout control.
 */
void BuzzerService_Init(void)
{
    /* Initialize buzzer GPIO */
    buzzer_init();
    
    /* Initialize Timer0 in full 32-bit one-shot mode */
    Timer0_Init_OneShot();
    
    /* Start with buzzer off */
    buzzer_active = false;
    
    /* NOTE: Timer0A_Handler must be registered in the EWARM */
    /* interrupt vector table (startup_ewarm.c) */
}

/*
 * BuzzerService_Activate
 * Activates the buzzer for the specified duration.
 */
void BuzzerService_Activate(uint32_t seconds)
{
    /* Turn on the buzzer */
    buzzer_on();
    buzzer_active = true;
    
    /* Start timer to automatically turn off buzzer */
    BuzzerService_StartTimer(seconds);
}

/*
 * BuzzerService_Cancel
 * Immediately stops the buzzer and timer.
 */
void BuzzerService_Cancel(void)
{
    buzzer_off();
    Timer0_Stop();
    buzzer_active = false;
}

/*
 * BuzzerService_IsActive
 * Returns the current buzzer state.
 */
bool BuzzerService_IsActive(void)
{
    return buzzer_active;
}

/******************************************************************************
 *                         Private Functions                                   *
 ******************************************************************************/

/*
 * BuzzerService_StartTimer
 * Starts Timer0 for the specified number of seconds.
 */
static void BuzzerService_StartTimer(uint32_t seconds)
{
    uint32_t systemClock = SysCtlClockGet();
    uint32_t ticks = systemClock * seconds;
    
    Timer0_Start_OneShot(ticks);
}

/******************************************************************************
 *                         Interrupt Handler                                   *
 ******************************************************************************/

/*
 * Timer0A_Handler
 * ISR for Timer0 - automatically turns off the buzzer when timer expires.
 * Note: Timer0 is configured in full 32-bit mode (A+B concatenated).
 * Only Timer A generates the timeout interrupt in this configuration.
 * This handler must be registered in EWARM interrupt vector table.
 */
void Timer0A_Handler(void)
{
    /* Clear the timer interrupt flag */
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    
    /* Turn off the buzzer */
    buzzer_off();
    buzzer_active = false;
}
