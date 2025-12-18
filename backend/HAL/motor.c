/******************************************************************************
 * File: motor.c
 * Module: Motor Driver
 * Description: DC Motor HAL implementation for direction control
 * Author: Ahmedhh
 * Date: December 14, 2025
 ******************************************************************************/

#include "motor.h"
#include "dio.h"

/******************************************************************************
 *                              Pin Configuration                              *
 ******************************************************************************/

/*
 * Motor pins connected to Port F
 * IN1 -> PF0 (Motor input 1)
 * IN4 -> PF4 (Motor input 2)
 */
#define MOTOR_PORT      PORTF
#define MOTOR_IN1       PIN0
#define MOTOR_IN2       PIN4

/******************************************************************************
 *                          Function Definitions                               *
 ******************************************************************************/

/*
 * Motor_Init
 * Initializes PF0 and PF4 as output pins for motor control.
 * Sets both pins to LOW initially (motor stopped).
 */
void Motor_Init(void) {
    /* Initialize IN1 (PF0) as output */
    DIO_Init(MOTOR_PORT, MOTOR_IN1, OUTPUT);
    
    /* Initialize IN2 (PF4) as output */
    DIO_Init(MOTOR_PORT, MOTOR_IN2, OUTPUT);
    
    /* Start with motor stopped (both pins LOW) */
    DIO_WritePin(MOTOR_PORT, MOTOR_IN1, LOW);
    DIO_WritePin(MOTOR_PORT, MOTOR_IN2, LOW);
}

/*
 * Motor_RotateCW
 * Rotates the motor clockwise: IN1=HIGH, IN2=LOW
 */
void Motor_RotateCW(void) {
    DIO_WritePin(MOTOR_PORT, MOTOR_IN1, HIGH);
    DIO_WritePin(MOTOR_PORT, MOTOR_IN2, LOW);
}

/*
 * Motor_RotateCCW
 * Rotates the motor counter-clockwise: IN1=LOW, IN2=HIGH
 */
void Motor_RotateCCW(void) {
    DIO_WritePin(MOTOR_PORT, MOTOR_IN1, LOW);
    DIO_WritePin(MOTOR_PORT, MOTOR_IN2, HIGH);
}

/*
 * Motor_Stop
 * Stops the motor: IN1=LOW, IN2=LOW
 */
void Motor_Stop(void) {
    DIO_WritePin(MOTOR_PORT, MOTOR_IN1, LOW);
    DIO_WritePin(MOTOR_PORT, MOTOR_IN2, LOW);
}
