/*
 * test_ack_lights.c - Unit tests for ack lights (status LEDs)
 * 
 * Tests LED GPIO initialization and color control
 * in ack_lights.c
 */

#include "test_common.h"
#include "../ack_lights.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"

/* LED GPIO - Port F as defined in ack_lights.c */
#define LED_PORT_BASE   GPIO_PORTF_BASE
#define LED_RED         GPIO_PIN_1
#define LED_GREEN       GPIO_PIN_3

/*===========================================================================
 * Test: Ack Lights Initialization
 *===========================================================================*/
static TestResult test_ack_lights_init(void)
{
    /* Initialize ack lights */
    ack_lights_init();
    
    /* Verify Port F is enabled */
    TEST_ASSERT(SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
    
    /* Both LEDs should be off after init */
    uint8_t pin_state = GPIOPinRead(LED_PORT_BASE, LED_RED | LED_GREEN);
    TEST_ASSERT_EQUAL(0, pin_state);
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Light Up Green
 *===========================================================================*/
static TestResult test_lightup_green(void)
{
    ack_lights_init();
    
    /* Turn on green LED */
    lightupGreen();
    
    /* PF3 should be high (green), PF1 should be low (red off) */
    uint8_t pin_state = GPIOPinRead(LED_PORT_BASE, LED_RED | LED_GREEN);
    TEST_ASSERT_EQUAL(LED_GREEN, pin_state);
    
    /* Clean up */
    ackLightsOff();
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Light Up Red
 *===========================================================================*/
static TestResult test_lightup_red(void)
{
    ack_lights_init();
    
    /* Turn on red LED */
    lightupRed();
    
    /* PF1 should be high (red), PF3 should be low (green off) */
    uint8_t pin_state = GPIOPinRead(LED_PORT_BASE, LED_RED | LED_GREEN);
    TEST_ASSERT_EQUAL(LED_RED, pin_state);
    
    /* Clean up */
    ackLightsOff();
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Ack Lights Off
 *===========================================================================*/
static TestResult test_ack_lights_off(void)
{
    ack_lights_init();
    
    /* Turn on green first */
    lightupGreen();
    uint8_t pin_state = GPIOPinRead(LED_PORT_BASE, LED_RED | LED_GREEN);
    TEST_ASSERT_EQUAL(LED_GREEN, pin_state);
    
    /* Turn off */
    ackLightsOff();
    
    /* Both should be low */
    pin_state = GPIOPinRead(LED_PORT_BASE, LED_RED | LED_GREEN);
    TEST_ASSERT_EQUAL(0, pin_state);
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Color Switch
 *===========================================================================*/
static TestResult test_color_switch(void)
{
    ack_lights_init();
    
    /* Start with green */
    lightupGreen();
    uint8_t pin_state = GPIOPinRead(LED_PORT_BASE, LED_RED | LED_GREEN);
    TEST_ASSERT_EQUAL(LED_GREEN, pin_state);
    
    /* Switch to red (should turn off green) */
    lightupRed();
    pin_state = GPIOPinRead(LED_PORT_BASE, LED_RED | LED_GREEN);
    TEST_ASSERT_EQUAL(LED_RED, pin_state);
    
    /* Switch back to green */
    lightupGreen();
    pin_state = GPIOPinRead(LED_PORT_BASE, LED_RED | LED_GREEN);
    TEST_ASSERT_EQUAL(LED_GREEN, pin_state);
    
    /* Clean up */
    ackLightsOff();
    
    TEST_PASS();
}

/*===========================================================================
 * Run All Ack Lights Tests
 *===========================================================================*/
void run_ack_lights_tests(void)
{
    printf("\n--- Ack Lights Tests ---\n");
    
    run_test("Ack Lights Init", test_ack_lights_init);
    run_test("Light Up Green", test_lightup_green);
    run_test("Light Up Red", test_lightup_red);
    run_test("Ack Lights Off", test_ack_lights_off);
    run_test("Color Switch", test_color_switch);
}

