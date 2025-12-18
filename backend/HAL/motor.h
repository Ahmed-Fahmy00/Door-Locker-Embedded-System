/******************************************************************************
 * File: motor.h
 * Module: Motor Driver
 * Description: Header file for DC Motor HAL with direction control
 * Author: Ahmedhh
 * Date: December 14, 2025
 ******************************************************************************/

#ifndef MOTOR_H_
#define MOTOR_H_

#include <stdint.h>

/******************************************************************************
 * Function Prototypes
 * API for Motor control.
 ******************************************************************************/

/*
 * Motor_Init
 * Initializes the motor control pins (PF0 and PF4) as outputs.
 * Must be called before using other motor functions.
 */
void Motor_Init(void);

/*
 * Motor_RotateCW
 * Rotates the motor clockwise.
 */
void Motor_RotateCW(void);

/*
 * Motor_RotateCCW
 * Rotates the motor counter-clockwise.
 */
void Motor_RotateCCW(void);

/*
 * Motor_Stop
 * Stops the motor completely.
 */
void Motor_Stop(void);

#endif /* MOTOR_H_ */
