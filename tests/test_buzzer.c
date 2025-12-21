/*
 * test_buzzer.c - Unit tests for buzzer control
 * 
 * Tests buzzer GPIO initialization and on/off control
 * in components/buzzer.c
 */

#include "test_common.h"
#include "../components/buzzer.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"

/* Buzzer GPIO - Port A, Pin 5 as defined in buzzer.c */
#define BUZZER_PORT_BASE GPIO_PORTA_BASE
#define BUZZER_PIN       GPIO_PIN_5

/*===========================================================================
 * Test: Buzzer Initialization
 *===========================================================================*/
static TestResult test_buzzer_init(void)
{
    /* Initialize buzzer */
    buzzer_init();
    
    /* Verify Port A is enabled */
    TEST_ASSERT(SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
    
    /* Buzzer should be off after init */
    uint8_t pin_state = GPIOPinRead(BUZZER_PORT_BASE, BUZZER_PIN);
    TEST_ASSERT_EQUAL(0, pin_state);
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Buzzer On
 *===========================================================================*/
static TestResult test_buzzer_on(void)
{
    buzzer_init();
    
    /* Turn buzzer on */
    buzzer_on();
    
    /* PA5 should be high */
    uint8_t pin_state = GPIOPinRead(BUZZER_PORT_BASE, BUZZER_PIN);
    TEST_ASSERT_EQUAL(BUZZER_PIN, pin_state);
    
    /* Clean up */
    buzzer_off();
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Buzzer Off
 *===========================================================================*/
static TestResult test_buzzer_off(void)
{
    buzzer_init();
    
    /* Turn buzzer on first */
    buzzer_on();
    uint8_t pin_state = GPIOPinRead(BUZZER_PORT_BASE, BUZZER_PIN);
    TEST_ASSERT_EQUAL(BUZZER_PIN, pin_state);
    
    /* Turn buzzer off */
    buzzer_off();
    
    /* PA5 should be low */
    pin_state = GPIOPinRead(BUZZER_PORT_BASE, BUZZER_PIN);
    TEST_ASSERT_EQUAL(0, pin_state);
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Buzzer Toggle
 *===========================================================================*/
static TestResult test_buzzer_toggle(void)
{
    buzzer_init();
    
    /* Start with off */
    buzzer_off();
    uint8_t pin_state = GPIOPinRead(BUZZER_PORT_BASE, BUZZER_PIN);
    TEST_ASSERT_EQUAL(0, pin_state);
    
    /* On */
    buzzer_on();
    pin_state = GPIOPinRead(BUZZER_PORT_BASE, BUZZER_PIN);
    TEST_ASSERT_EQUAL(BUZZER_PIN, pin_state);
    
    /* Off */
    buzzer_off();
    pin_state = GPIOPinRead(BUZZER_PORT_BASE, BUZZER_PIN);
    TEST_ASSERT_EQUAL(0, pin_state);
    
    /* On again */
    buzzer_on();
    pin_state = GPIOPinRead(BUZZER_PORT_BASE, BUZZER_PIN);
    TEST_ASSERT_EQUAL(BUZZER_PIN, pin_state);
    
    /* Clean up */
    buzzer_off();
    
    TEST_PASS();
}

/*===========================================================================
 * Run All Buzzer Tests
 *===========================================================================*/
void run_buzzer_tests(void)
{
    printf("\n--- Buzzer Tests ---\n");
    
    run_test("Buzzer Init", test_buzzer_init);
    run_test("Buzzer On", test_buzzer_on);
    run_test("Buzzer Off", test_buzzer_off);
    run_test("Buzzer Toggle", test_buzzer_toggle);
}

