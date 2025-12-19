/******************************************************************************
 * File: buzzer_service.h
 * Module: Buzzer Service (Application Layer)
 * Description: High-level buzzer control with automatic timeout
 * Author: Ahmedhh
 * Date: December 19, 2025
 ******************************************************************************/

#ifndef BUZZER_SERVICE_H_
#define BUZZER_SERVICE_H_

#include <stdint.h>
#include <stdbool.h>

/******************************************************************************
 *                        Function Prototypes                                  *
 ******************************************************************************/

/*
 * BuzzerService_Init
 * Initializes the buzzer service, buzzer GPIO, and Timer0.
 * Must be called before using other buzzer service functions.
 */
void BuzzerService_Init(void);

/*
 * BuzzerService_Activate
 * Activates the buzzer for a specified duration.
 * The buzzer will automatically turn off when the timer expires.
 * 
 * Parameters:
 *   seconds - Duration in seconds for the buzzer to stay on
 */
void BuzzerService_Activate(uint32_t seconds);

/*
 * BuzzerService_Cancel
 * Immediately stops the buzzer and cancels any active timeout.
 */
void BuzzerService_Cancel(void);

/*
 * BuzzerService_IsActive
 * Returns whether the buzzer is currently active.
 * 
 * Return:
 *   true if buzzer is on, false otherwise
 */
bool BuzzerService_IsActive(void);

/******************************************************************************
 *                     Timer Interrupt Handler (Public)                       *
 * Must be registered in the EWARM interrupt vector table                     *
 * Note: Timer0 uses full 32-bit mode (A+B) but only A generates interrupt    *
 ******************************************************************************/

/* Timer0A interrupt handler - register in startup_ewarm.c */
void Timer0A_Handler(void);

#endif /* BUZZER_SERVICE_H_ */
