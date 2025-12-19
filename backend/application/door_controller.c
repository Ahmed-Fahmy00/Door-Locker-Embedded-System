/******************************************************************************
 * File: door_controller.c
 * Module: Door Controller (Application Layer)
 * Description: High-level door control logic with automated open/close sequence
 * Author: Ahmedhh
 * Date: December 18, 2025
 ******************************************************************************/

#include "door_controller.h"
#include "../HAL/motor.h"
#include "../MCAL/gptm.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "inc/hw_memmap.h"

/******************************************************************************
 *                           Timing Constants                                  *
 ******************************************************************************/

#define DOOR_CLOSE_TIME_SEC     2       /* Time for door to close (reverse) */

/******************************************************************************
 *                           Private Variables                                 *
 ******************************************************************************/

static volatile DoorState_t doorState = DOOR_IDLE;

/******************************************************************************
 *                      Private Function Prototypes                            *
 ******************************************************************************/

static void DoorController_StartTimer(uint32_t seconds);

/******************************************************************************
 *                          Function Definitions                               *
 ******************************************************************************/

/*
 * DoorController_Init
 * Initializes motor and Timer1A for door sequence control.
 */
void DoorController_Init(void)
{
    /* Initialize motor GPIO */
    Motor_Init();
    
    /* Initialize Timer1 in full 32-bit one-shot mode */
    Timer1_Init_OneShot();
    
    /* Start in IDLE state */
    doorState = DOOR_IDLE;
    
    /* NOTE: Timer1A_Handler must be registered in the EWARM */
    /* interrupt vector table (startup_ewarm.c) */
}

/*
 * DoorController_OpenDoor
 * Starts the automated door sequence.
 */
void DoorController_OpenDoor(uint32_t seconds)
{
    /* Only start if we're currently idle */
    if (doorState == DOOR_IDLE)
    {
        /* Start opening the door */
        doorState = DOOR_OPENING;
        Motor_RotateCW();  /* CW = door opens */
        
        /* Start timer for opening phase */
        DoorController_StartTimer(seconds);
    }
}

/*
 * DoorController_GetState
 * Returns the current door state.
 */
DoorState_t DoorController_GetState(void)
{
    return doorState;
}

/*
 * DoorController_Stop
 * Emergency stop - stops motor and returns to idle.
 */
void DoorController_Stop(void)
{
    Motor_Stop();
    Timer1_Stop();
    doorState = DOOR_IDLE;
}

/******************************************************************************
 *                         Private Functions                                   *
 ******************************************************************************/

/*
 * DoorController_StartTimer
 * Starts Timer1A for the specified number of seconds.
 */
static void DoorController_StartTimer(uint32_t seconds)
{
    uint32_t systemClock = SysCtlClockGet();
    uint32_t ticks = systemClock * seconds;
    
    Timer1_Start_OneShot(ticks);
}

/*
 * Timer1A_Handler
 * ISR for Timer1 - handles state transitions in door sequence.
 * Note: Timer1 is configured in full 32-bit mode (A+B concatenated).
 * Only Timer A generates the timeout interrupt in this configuration.
 * This handler must be registered in EWARM interrupt vector table.
 */
void Timer1A_Handler(void)
{
    /* Clear the timer interrupt flag */
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    
    switch (doorState)
    {
        case DOOR_OPENING:
            /* Door has finished opening, start closing immediately */
            doorState = DOOR_CLOSING;
            Motor_RotateCCW();  /* CCW = door closes */
            DoorController_StartTimer(DOOR_CLOSE_TIME_SEC);
            break;
            
        case DOOR_CLOSING:
            /* Door has finished closing, stop and go idle */
            Motor_Stop();
            doorState = DOOR_IDLE;
            break;
            
        default:
            /* Should not get here, but stop motor just in case */
            Motor_Stop();
            doorState = DOOR_IDLE;
            break;
    }
}
