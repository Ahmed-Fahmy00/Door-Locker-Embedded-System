/*
 * test_integration.c - Integration tests for backend system
 * 
 * Tests complete workflows that involve multiple components
 * working together.
 */

#include "test_common.h"
#include "../eeprom_handler.h"
#include "../ack_lights.h"
#include "../components/motor.h"
#include "../components/buzzer.h"
#include "../gptm0_oneshot.h"
#include "../gptm3_oneshot.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"

/* LED GPIO for verification */
#define LED_PORT_BASE   GPIO_PORTF_BASE
#define LED_RED         GPIO_PIN_1
#define LED_GREEN       GPIO_PIN_3

/* Buzzer GPIO for verification */
#define BUZZER_PORT_BASE GPIO_PORTA_BASE
#define BUZZER_PIN       GPIO_PIN_5

/*===========================================================================
 * Test: Authentication Success Flow
 * 
 * Simulates successful authentication and verifies green LED lights up
 *===========================================================================*/
static TestResult test_auth_success_flow(void)
{
    int result;
    uint8_t pin_state;
    
    /* Initialize components */
    ack_lights_init();
    
    /* Set password */
    result = initialize_password(12345);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    
    /* Authenticate with correct password */
    result = authenticate(12345, 0);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    
    /* On success, light up green */
    if (result == STATUS_OK)
    {
        lightupGreen();
    }
    
    /* Verify green LED is on */
    pin_state = GPIOPinRead(LED_PORT_BASE, LED_RED | LED_GREEN);
    TEST_ASSERT_EQUAL(LED_GREEN, pin_state);
    
    /* Clean up */
    ackLightsOff();
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Authentication Failure Flow
 * 
 * Simulates failed authentication and verifies red LED lights up
 *===========================================================================*/
static TestResult test_auth_fail_flow(void)
{
    int result;
    uint8_t pin_state;
    
    /* Initialize components */
    ack_lights_init();
    
    /* Set password */
    result = initialize_password(11111);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    
    /* Authenticate with WRONG password */
    result = authenticate(99999, 0);
    TEST_ASSERT_EQUAL(STATUS_AUTH_FAIL, result);
    
    /* On failure, light up red */
    if (result == STATUS_AUTH_FAIL)
    {
        lightupRed();
    }
    
    /* Verify red LED is on */
    pin_state = GPIOPinRead(LED_PORT_BASE, LED_RED | LED_GREEN);
    TEST_ASSERT_EQUAL(LED_RED, pin_state);
    
    /* Clean up */
    ackLightsOff();
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Door Open Sequence (Motor Forward)
 * 
 * Tests motor forward operation with state verification
 *===========================================================================*/
static TestResult test_door_open_sequence(void)
{
    /* Initialize motor */
    Motor_GPIO_Init();
    ack_lights_init();
    
    /* Simulate door open command */
    lightupGreen();  /* Success indication */
    Motor_Forward();
    
    /* Verify motor is running forward */
    TEST_ASSERT_EQUAL(MOTOR_FORWARD, motorState);
    
    /* Stop motor (in real system, this happens after timeout) */
    Motor_Stop();
    
    /* Verify motor stopped */
    TEST_ASSERT_EQUAL(MOTOR_STOP, motorState);
    
    /* Clean up */
    ackLightsOff();
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Door Close Sequence (Motor Reverse)
 * 
 * Tests motor reverse operation for door closing
 *===========================================================================*/
static TestResult test_door_close_sequence(void)
{
    /* Initialize motor */
    Motor_GPIO_Init();
    
    /* Simulate door close command */
    Motor_Reverse();
    
    /* Verify motor is running reverse */
    TEST_ASSERT_EQUAL(MOTOR_REVERSE, motorState);
    
    /* Stop motor */
    Motor_Stop();
    
    /* Verify motor stopped */
    TEST_ASSERT_EQUAL(MOTOR_STOP, motorState);
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Full Door Cycle
 * 
 * Tests complete door open/close cycle
 *===========================================================================*/
static TestResult test_full_door_cycle(void)
{
    /* Initialize */
    Motor_GPIO_Init();
    ack_lights_init();
    
    /* Phase 1: Open door */
    lightupGreen();
    Motor_Forward();
    TEST_ASSERT_EQUAL(MOTOR_FORWARD, motorState);
    
    /* Simulate wait (in real code this uses timeout) */
    Motor_Stop();
    TEST_ASSERT_EQUAL(MOTOR_STOP, motorState);
    
    /* Phase 2: Close door */
    Motor_Reverse();
    TEST_ASSERT_EQUAL(MOTOR_REVERSE, motorState);
    
    /* Door closed */
    Motor_Stop();
    TEST_ASSERT_EQUAL(MOTOR_STOP, motorState);
    
    /* Clean up */
    ackLightsOff();
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Lockout Sequence
 * 
 * Tests buzzer activation on lockout
 *===========================================================================*/
static TestResult test_lockout_sequence(void)
{
    uint8_t pin_state;
    
    /* Initialize components */
    buzzer_init();
    ack_lights_init();
    gptm0_oneshot_init();
    
    /* Simulate lockout - buzzer should activate */
    lightupRed();
    buzzer_on();
    
    /* Verify red LED is on */
    pin_state = GPIOPinRead(LED_PORT_BASE, LED_RED | LED_GREEN);
    TEST_ASSERT_EQUAL(LED_RED, pin_state);
    
    /* Verify buzzer is on */
    pin_state = GPIOPinRead(BUZZER_PORT_BASE, BUZZER_PIN);
    TEST_ASSERT_EQUAL(BUZZER_PIN, pin_state);
    
    /* Clean up (end lockout) */
    buzzer_off();
    ackLightsOff();
    
    /* Verify buzzer is off */
    pin_state = GPIOPinRead(BUZZER_PORT_BASE, BUZZER_PIN);
    TEST_ASSERT_EQUAL(0, pin_state);
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Password Change and Re-authenticate
 *===========================================================================*/
static TestResult test_password_change_flow(void)
{
    int result;
    
    /* Set initial password */
    result = initialize_password(11111);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    
    /* Verify initial password works */
    result = authenticate(11111, 0);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    
    /* Change password */
    result = change_password(22222);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    
    /* Old password should fail */
    result = authenticate(11111, 0);
    TEST_ASSERT_EQUAL(STATUS_AUTH_FAIL, result);
    
    /* New password should work */
    result = authenticate(22222, 0);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Timeout Configuration Flow
 *===========================================================================*/
static TestResult test_timeout_config_flow(void)
{
    int result;
    uint32_t timeout;
    
    /* Force set default timeout directly */
    result = change_auto_timeout(DEFAULT_TIMEOUT);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    
    /* Verify default */
    result = get_auto_timeout(&timeout);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    TEST_ASSERT_EQUAL(DEFAULT_TIMEOUT, timeout);
    
    /* Change timeout */
    result = change_auto_timeout(20);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    
    /* Verify change */
    result = get_auto_timeout(&timeout);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    TEST_ASSERT_EQUAL(20, timeout);
    
    /* Restore default */
    result = change_auto_timeout(DEFAULT_TIMEOUT);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    
    TEST_PASS();
}

/*===========================================================================
 * Run All Integration Tests
 *===========================================================================*/
void run_integration_tests(void)
{
    printf("\n--- Integration Tests ---\n");
    
    run_test("Auth Success Flow", test_auth_success_flow);
    run_test("Auth Fail Flow", test_auth_fail_flow);
    run_test("Door Open Sequence", test_door_open_sequence);
    run_test("Door Close Sequence", test_door_close_sequence);
    run_test("Full Door Cycle", test_full_door_cycle);
    run_test("Lockout Sequence", test_lockout_sequence);
    run_test("Password Change Flow", test_password_change_flow);
    run_test("Timeout Config Flow", test_timeout_config_flow);
}

