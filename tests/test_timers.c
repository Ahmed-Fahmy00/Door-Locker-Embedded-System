/*
 * test_timers.c - Unit tests for timer modules
 * 
 * Tests Timer0 (buzzer) and Timer3 (ack lights) one-shot timers
 * in gptm0_oneshot.c and gptm3_oneshot.c
 */

#include "test_common.h"
#include "../gptm0_oneshot.h"
#include "../gptm3_oneshot.h"
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"

/*===========================================================================
 * Test: Timer0 Initialization (Buzzer Timer)
 *===========================================================================*/
static TestResult test_timer0_init(void)
{
    /* Initialize Timer0 for buzzer */
    gptm0_oneshot_init();
    
    /* Verify Timer0 peripheral is enabled */
    TEST_ASSERT(SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Timer3 Initialization (Ack Lights Timer)
 *===========================================================================*/
static TestResult test_timer3_init(void)
{
    /* Initialize Timer3 for ack lights */
    gptm3_oneshot_init();
    
    /* Verify Timer3 peripheral is enabled */
    TEST_ASSERT(SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER3));
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Timer0 Start (Buzzer)
 *===========================================================================*/
static TestResult test_timer0_start(void)
{
    bool result;
    
    gptm0_oneshot_init();
    
    /* Start timer for 5 seconds (minimum valid duration) */
    result = gptm0_oneshot_start_seconds(5);
    TEST_ASSERT(result == true);
    
    /* Stop timer immediately (don't wait 5 seconds in test) */
    gptm0_oneshot_stop();
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Timer0 Start with Zero
 *===========================================================================*/
static TestResult test_timer0_start_zero(void)
{
    bool result;
    
    gptm0_oneshot_init();
    
    /* Starting with 0 seconds should return true but not start timer */
    result = gptm0_oneshot_start_seconds(0);
    TEST_ASSERT(result == true);
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Timer0 Stop
 *===========================================================================*/
static TestResult test_timer0_stop(void)
{
    gptm0_oneshot_init();
    
    /* Start timer */
    gptm0_oneshot_start_seconds(10);
    
    /* Stop timer */
    gptm0_oneshot_stop();
    
    /* Timer should be disabled - no assertion needed, just verify no crash */
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Timer3 Start (Ack Lights)
 *===========================================================================*/
static TestResult test_timer3_start(void)
{
    gptm3_oneshot_init();
    
    /* Start timer (1 second duration built into function) */
    gptm3_oneshot_start();
    
    /* Stop timer immediately */
    gptm3_oneshot_stop();
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Timer3 Stop
 *===========================================================================*/
static TestResult test_timer3_stop(void)
{
    gptm3_oneshot_init();
    
    /* Start timer */
    gptm3_oneshot_start();
    
    /* Stop timer */
    gptm3_oneshot_stop();
    
    /* Timer should be disabled - no assertion needed, just verify no crash */
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Timer0 Max Duration
 *===========================================================================*/
static TestResult test_timer0_max_duration(void)
{
    bool result;
    
    gptm0_oneshot_init();
    
    /* Test maximum valid duration (30 seconds) */
    result = gptm0_oneshot_start_seconds(30);
    TEST_ASSERT(result == true);
    
    gptm0_oneshot_stop();
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Multiple Timer Start/Stop
 *===========================================================================*/
static TestResult test_timer_multiple_start_stop(void)
{
    gptm0_oneshot_init();
    gptm3_oneshot_init();
    
    /* Start both timers */
    gptm0_oneshot_start_seconds(10);
    gptm3_oneshot_start();
    
    /* Stop both */
    gptm0_oneshot_stop();
    gptm3_oneshot_stop();
    
    /* Start again */
    gptm0_oneshot_start_seconds(5);
    gptm3_oneshot_start();
    
    /* Stop again */
    gptm0_oneshot_stop();
    gptm3_oneshot_stop();
    
    TEST_PASS();
}

/*===========================================================================
 * Run All Timer Tests
 *===========================================================================*/
void run_timer_tests(void)
{
    printf("\n--- Timer Tests ---\n");
    
    run_test("Timer0 Init (Buzzer)", test_timer0_init);
    run_test("Timer3 Init (Ack Lights)", test_timer3_init);
    run_test("Timer0 Start", test_timer0_start);
    run_test("Timer0 Start Zero", test_timer0_start_zero);
    run_test("Timer0 Stop", test_timer0_stop);
    run_test("Timer3 Start", test_timer3_start);
    run_test("Timer3 Stop", test_timer3_stop);
    run_test("Timer0 Max Duration", test_timer0_max_duration);
    run_test("Multiple Timer Start/Stop", test_timer_multiple_start_stop);
}

