/*****************************************************************************
 * File: timer.c
 * Description: Timer1 for Motor Door Sequence
 * 
 * Used to time the door open/close sequence:
 * - 10 seconds forward (door opens)
 * - 2 seconds reverse (door closes)
 * - Motor stops
 *****************************************************************************/

#include "timer.h"
#include "motor.h"
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"



void Timer1_StartSeconds(uint32_t seconds)
{
    uint32_t clock = SysCtlClockGet();

    TimerLoadSet(TIMER1_BASE, TIMER_A, (clock * seconds) - 1);
    TimerEnable(TIMER1_BASE, TIMER_A);
}

void Timer1A_Handler(void)
{
    /* Clear interrupt FIRST */
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    if (motorState == MOTOR_FORWARD)
    {
        Motor_Reverse();
        motorState = MOTOR_REVERSE;
        
        Timer1_StartSeconds(2);
    }
    else if (motorState == MOTOR_REVERSE)
    {
        Motor_Stop();
        motorState = MOTOR_STOP;
    }
}
