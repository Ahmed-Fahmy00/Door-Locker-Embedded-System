/*****************************************************************************
 * File: motor.c
 * Description: Motor Control for Door Lock System
 * 
 * Motor uses Port C pins (RIGHT SIDE J4):
 *   - PC4 = Motor IN1 (Forward)
 *   - PC5 = Motor IN2 (Reverse)
 * 
 * RGB LED for status (separate from motor):
 *   - PF3 = Green LED (door opening)
 *   - PF1 = Red LED (door closing)
 * 
 * Door sequence:
 *   1. Motor Forward for 10 seconds (Green LED)
 *   2. Motor Reverse for 2 seconds (Red LED)
 *   3. Motor Stop (LEDs off)
 *****************************************************************************/

#include "motor.h"
#include "timer.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include <stdbool.h>

/* Motor GPIO - Port C (PC4, PC5 on J4) */
#define MOTOR_PERIPH    SYSCTL_PERIPH_GPIOC
#define MOTOR_BASE      GPIO_PORTC_BASE
#define MOTOR_IN1       GPIO_PIN_4  /* Forward - PC4 */
#define MOTOR_IN2       GPIO_PIN_5  /* Reverse - PC5 */

/* LED GPIO - Port F (PF1, PF3) */
#define LED_RED         GPIO_PIN_1
#define LED_GREEN       GPIO_PIN_3

/* Motor state */
volatile MotorState motorState = MOTOR_STOP;

/* Initialization flag */
static bool motor_initialized = false;

/*===========================================================================
 * Motor GPIO Functions
 *===========================================================================*/
void Motor_GPIO_Init(void)
{
    if (motor_initialized) return;
    
    /* Initialize motor pins (PD6, PD7) */
    SysCtlPeripheralEnable(MOTOR_PERIPH);
    while (!SysCtlPeripheralReady(MOTOR_PERIPH)) {}
    GPIOPinTypeGPIOOutput(MOTOR_BASE, MOTOR_IN1 | MOTOR_IN2);
    GPIOPinWrite(MOTOR_BASE, MOTOR_IN1 | MOTOR_IN2, 0);
    
    /* Initialize LED pins (PF1, PF3) */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)) {}
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, LED_RED | LED_GREEN);
    GPIOPinWrite(GPIO_PORTF_BASE, LED_RED | LED_GREEN, 0);
    
    motor_initialized = true;
}

void Motor_Forward(void)
{
    /* Motor: IN1=HIGH, IN2=LOW */
    GPIOPinWrite(MOTOR_BASE, MOTOR_IN1 | MOTOR_IN2, MOTOR_IN1);
    /* LED: Green on */
    GPIOPinWrite(GPIO_PORTF_BASE, LED_RED | LED_GREEN, LED_GREEN);
}

void Motor_Reverse(void)
{
    /* Motor: IN1=LOW, IN2=HIGH */
    GPIOPinWrite(MOTOR_BASE, MOTOR_IN1 | MOTOR_IN2, MOTOR_IN2);
    /* LED: Red on */
    GPIOPinWrite(GPIO_PORTF_BASE, LED_RED | LED_GREEN, LED_RED);
}

void Motor_Stop(void)
{
    /* Motor: IN1=LOW, IN2=LOW */
    GPIOPinWrite(MOTOR_BASE, MOTOR_IN1 | MOTOR_IN2, 0);
    /* LED: Off */
    GPIOPinWrite(GPIO_PORTF_BASE, LED_RED | LED_GREEN, 0);
}

/*===========================================================================
 * Delay Helper (blocking)
 *===========================================================================*/
static void Delay_Seconds(uint32_t seconds)
{
    uint32_t clock = SysCtlClockGet();
    uint32_t loops_per_second = clock / 3;
    
    for (uint32_t i = 0; i < seconds; i++)
    {
        SysCtlDelay(loops_per_second);
    }
}

/*===========================================================================
 * Door Control - Split into phases
 *===========================================================================*/

/* Phase 1: Open door (3 seconds forward) */
void open_door_start(void)
{
    Motor_GPIO_Init();
    
    /* Forward for 3 seconds - Green LED */
    Motor_Forward();
    motorState = MOTOR_FORWARD;
    Delay_Seconds(3);
    
    /* Stop motor, keep green LED on (door is open) */
    GPIOPinWrite(MOTOR_BASE, MOTOR_IN1 | MOTOR_IN2, 0);
    motorState = MOTOR_STOP;
}

/* Phase 2: Close door (3 seconds reverse) */
void close_door(void)
{
    Motor_GPIO_Init();
    
    /* Reverse for 3 seconds - Red LED */
    Motor_Reverse();
    motorState = MOTOR_REVERSE;
    Delay_Seconds(3);
    
    /* Stop */
    Motor_Stop();
    motorState = MOTOR_STOP;
}

/* Legacy function - opens and closes with timeout in between */
void open_door(void)
{
    open_door_start();
    /* Timeout wait would go here - but now handled by frontend */
    close_door();
}
