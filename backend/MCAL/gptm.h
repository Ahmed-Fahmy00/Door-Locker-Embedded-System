#ifndef TIMER_H_
#define TIMER_H_
#include <stdint.h>
#include <stdbool.h>


// Timer0A (used for buzzer)
void Timer0A_Init_OneShot(void);
void Timer0A_Start_OneShot(uint32_t ticks);
void Timer0A_Stop(void);
void Timer0A_SetISR(void (*handler)(void));
bool Timer0A_IsRunning(void);

// Timer1A (used for motor/door)
void Timer1A_Init_OneShot(void);
void Timer1A_Start_OneShot(uint32_t ticks);
void Timer1A_Stop(void);
void Timer1A_SetISR(void (*handler)(void));
bool Timer1A_IsRunning(void);

#endif /* TIMER_H_ */
