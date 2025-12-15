#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include <stdbool.h>

/* -------------------- Motor States -------------------- */
typedef enum {
    MOTOR_FORWARD,
    MOTOR_REVERSE,
    MOTOR_STOP
} MotorState;

extern volatile MotorState motorState;

/* -------------------- Motor Functions -------------------- */
void Motor_GPIO_Init(void);
void Motor_Forward(void);
void Motor_Reverse(void);
void Motor_Stop(void);

/* -------------------- Door Control -------------------- */
void open_door(void);        /* Legacy: open + close */
void open_door_start(void);  /* Phase 1: Open door (3 sec) */
void close_door(void);       /* Phase 2: Close door (3 sec) */

#endif /* MOTOR_H */
