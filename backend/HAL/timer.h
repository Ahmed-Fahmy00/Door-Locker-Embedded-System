#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

/* -------------------- Timer Functions -------------------- */
void Timer1_Init(void);
void Timer1_StartSeconds(uint32_t seconds);
void Timer1A_Handler(void);

#endif /* TIMER_H */
