/******************************************************************************
 * File: door_controller.h
 * Module: Door Controller (Application Layer)
 * Description: High-level door control logic with automated open/close sequence
 * Author: Ahmedhh
 * Date: December 18, 2025
 ******************************************************************************/

#ifndef DOOR_CONTROLLER_H_
#define DOOR_CONTROLLER_H_

#include <stdint.h>
#include <stdbool.h>

/******************************************************************************
 *                           Type Definitions                                  *
 ******************************************************************************/

/* Door state enumeration */
typedef enum {
    DOOR_IDLE,          /* Door is idle (closed and not moving) */
    DOOR_OPENING,       /* Door is opening (motor forward) */
    DOOR_CLOSING        /* Door is closing (motor reverse) */
} DoorState_t;

/******************************************************************************
 *                        Function Prototypes                                  *
 ******************************************************************************/

/*
 * DoorController_Init
 * Initializes the door controller, motor, and required timers.
 * Must be called before using other door controller functions.
 */
void DoorController_Init(void);

/*
 * DoorController_OpenDoor
 * Starts the automated door sequence:
 * 1. Opens door (motor forward) for specified seconds
 * 2. Closes door (motor reverse) for 2 seconds
 * 3. Stops motor and returns to IDLE state
 * 
 * Parameters:
 *   seconds - Time in seconds for door to open (motor forward)
 */
void DoorController_OpenDoor(uint32_t seconds);

/*
 * DoorController_GetState
 * Returns the current state of the door.
 * 
 * Return:
 *   Current door state (DOOR_IDLE, DOOR_OPENING, etc.)
 */
DoorState_t DoorController_GetState(void);

/*
 * DoorController_Stop
 * Emergency stop - immediately stops the motor and returns to IDLE state.
 * This will abort any ongoing door sequence.
 */
void DoorController_Stop(void);

/******************************************************************************
 *                     Timer Interrupt Handler (Public)                       *
 * Must be registered in the EWARM interrupt vector table                     *
 * Note: Timer1 uses full 32-bit mode (A+B) but only A generates interrupt    *
 ******************************************************************************/

/* Timer1A interrupt handler - register in startup_ewarm.c */
void Timer1A_Handler(void);

#endif /* DOOR_CONTROLLER_H_ */
