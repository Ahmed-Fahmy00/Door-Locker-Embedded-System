/******************************************************************************
 * File: gptm.h
 * Module: General Purpose Timer Module (MCAL Layer)
 * Description: Low-level timer hardware abstraction
 * Author: Ahmedhh
 * Date: December 18, 2025
 ******************************************************************************/

#ifndef GPTM_H_
#define GPTM_H_

#include <stdint.h>
#include <stdbool.h>

/******************************************************************************
 *          Timer0 API (Full 32-bit mode - Used for Buzzer)                  *
 * Note: In full-width mode, both A and B are combined but only Timer A      *
 * generates the timeout interrupt.                                           *
 ******************************************************************************/

/* Initialize Timer0 in full 32-bit one-shot mode */
void Timer0_Init_OneShot(void);

/* Start Timer0 with specified tick count */
void Timer0_Start_OneShot(uint32_t ticks);

/* Stop Timer0 */
void Timer0_Stop(void);

/* Check if Timer0 is currently running */
bool Timer0_IsRunning(void);

/******************************************************************************
 *          Timer1 API (Full 32-bit mode - Used for Motor/Door)              *
 * Note: In full-width mode, both A and B are combined but only Timer A      *
 * generates the timeout interrupt.                                           *
 ******************************************************************************/

/* Initialize Timer1 (both A and B) in one-shot mode */
void Timer1_Init_OneShot(void);

/* Start Timer1 (both A and B) with specified tick count */
void Timer1_Start_OneShot(uint32_t ticks);

/* Stop Timer1 (both A and B) */
void Timer1_Stop(void);

/* Check if Timer1 is currently running */
bool Timer1_IsRunning(void);

#endif /* GPTM_H_ */
