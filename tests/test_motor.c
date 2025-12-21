/*
 * test_motor.c - Unit tests for motor control
 * 
 * Tests motor GPIO initialization and direction control
 * in components/motor.c
 */

#include "test_common.h"
#include "../components/motor.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"

/* Motor GPIO - Port C (PC4, PC5) as defined in motor.c */
#define MOTOR_PORT_BASE GPIO_PORTC_BASE
#define MOTOR_IN1       GPIO_PIN_4  /* Forward */
#define MOTOR_IN2       GPIO_PIN_5  /* Reverse */

/*===========================================================================
 * Test: Motor GPIO Initialization
 *===========================================================================*/
static TestResult test_motor_init(void)
{
    /* Initialize motor GPIO */
    Motor_GPIO_Init();
    
    /* Verify Port C is enabled */
    TEST_ASSERT(SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC));
    
    /* Motor should be stopped after init */
    TEST_ASSERT_EQUAL(MOTOR_STOP, motorState);
    
    /* GPIO pins should be low (motor stopped) */
    uint8_t pin_state = GPIOPinRead(MOTOR_PORT_BASE, MOTOR_IN1 | MOTOR_IN2);
    TEST_ASSERT_EQUAL(0, pin_state);
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Motor Forward
 *===========================================================================*/
static TestResult test_motor_forward(void)
{
    Motor_GPIO_Init();
    
    /* Set motor forward */
    Motor_Forward();
    
    /* Check state */
    TEST_ASSERT_EQUAL(MOTOR_FORWARD, motorState);
    
    /* IN1 should be high, IN2 should be low */
    uint8_t pin_state = GPIOPinRead(MOTOR_PORT_BASE, MOTOR_IN1 | MOTOR_IN2);
    TEST_ASSERT_EQUAL(MOTOR_IN1, pin_state);
    
    /* Clean up */
    Motor_Stop();
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Motor Reverse
 *===========================================================================*/
static TestResult test_motor_reverse(void)
{
    Motor_GPIO_Init();
    
    /* Set motor reverse */
    Motor_Reverse();
    
    /* Check state */
    TEST_ASSERT_EQUAL(MOTOR_REVERSE, motorState);
    
    /* IN1 should be low, IN2 should be high */
    uint8_t pin_state = GPIOPinRead(MOTOR_PORT_BASE, MOTOR_IN1 | MOTOR_IN2);
    TEST_ASSERT_EQUAL(MOTOR_IN2, pin_state);
    
    /* Clean up */
    Motor_Stop();
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Motor Stop
 *===========================================================================*/
static TestResult test_motor_stop(void)
{
    Motor_GPIO_Init();
    
    /* Start motor forward first */
    Motor_Forward();
    TEST_ASSERT_EQUAL(MOTOR_FORWARD, motorState);
    
    /* Stop motor */
    Motor_Stop();
    
    /* Check state */
    TEST_ASSERT_EQUAL(MOTOR_STOP, motorState);
    
    /* Both pins should be low */
    uint8_t pin_state = GPIOPinRead(MOTOR_PORT_BASE, MOTOR_IN1 | MOTOR_IN2);
    TEST_ASSERT_EQUAL(0, pin_state);
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Motor Direction Change
 *===========================================================================*/
static TestResult test_motor_direction_change(void)
{
    Motor_GPIO_Init();
    
    /* Forward */
    Motor_Forward();
    TEST_ASSERT_EQUAL(MOTOR_FORWARD, motorState);
    
    /* Direct to reverse (no stop in between) */
    Motor_Reverse();
    TEST_ASSERT_EQUAL(MOTOR_REVERSE, motorState);
    
    /* Back to forward */
    Motor_Forward();
    TEST_ASSERT_EQUAL(MOTOR_FORWARD, motorState);
    
    /* Stop */
    Motor_Stop();
    TEST_ASSERT_EQUAL(MOTOR_STOP, motorState);
    
    TEST_PASS();
}

/*===========================================================================
 * Run All Motor Tests
 *===========================================================================*/
void run_motor_tests(void)
{
    printf("\n--- Motor Control Tests ---\n");
    
    run_test("Motor Init", test_motor_init);
    run_test("Motor Forward", test_motor_forward);
    run_test("Motor Reverse", test_motor_reverse);
    run_test("Motor Stop", test_motor_stop);
    run_test("Motor Direction Change", test_motor_direction_change);
}

