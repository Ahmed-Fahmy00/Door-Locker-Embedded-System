/*
 * test_common.h - Common test infrastructure for TM4C123 backend tests
 * 
 * Uses ITM (Instrumentation Trace Macrocell) to output test results
 * to the IAR Debugger Console.
 */

#ifndef TEST_COMMON_H_
#define TEST_COMMON_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/*===========================================================================
 * Test Result Types
 *===========================================================================*/
typedef enum {
    TEST_RESULT_PASS = 1,
    TEST_RESULT_FAIL = 0
} TestResult;

typedef TestResult (*TestFunction)(void);

/*===========================================================================
 * Test Macros
 *===========================================================================*/

/* Assert condition is true */
#define TEST_ASSERT(cond) do { \
    if (!(cond)) { \
        printf("  FAIL: %s:%d - Assertion failed: %s\n", __FILE__, __LINE__, #cond); \
        return TEST_RESULT_FAIL; \
    } \
} while(0)

/* Assert two values are equal */
#define TEST_ASSERT_EQUAL(expected, actual) do { \
    if ((expected) != (actual)) { \
        printf("  FAIL: %s:%d - Expected %d, got %d\n", __FILE__, __LINE__, (int)(expected), (int)(actual)); \
        return TEST_RESULT_FAIL; \
    } \
} while(0)

/* Assert two values are not equal */
#define TEST_ASSERT_NOT_EQUAL(not_expected, actual) do { \
    if ((not_expected) == (actual)) { \
        printf("  FAIL: %s:%d - Values should not be equal: %d\n", __FILE__, __LINE__, (int)(actual)); \
        return TEST_RESULT_FAIL; \
    } \
} while(0)

/* Mark test as passed (use at end of test function) */
#define TEST_PASS() return TEST_RESULT_PASS

/* Mark test as failed with message */
#define TEST_FAIL(msg) do { \
    printf("  FAIL: %s:%d - %s\n", __FILE__, __LINE__, msg); \
    return TEST_RESULT_FAIL; \
} while(0)

/*===========================================================================
 * Test Runner Functions
 *===========================================================================*/

/* Initialize test framework (call once at start) */
void test_init(void);

/* Run a single test and track results */
void run_test(const char *test_name, TestFunction test_func);

/* Print final test summary */
void print_test_summary(void);

/* Get test statistics */
uint32_t get_tests_passed(void);
uint32_t get_tests_failed(void);
uint32_t get_tests_total(void);

/*===========================================================================
 * Test Suite Declarations
 *===========================================================================*/

/* Each test file implements a run_xxx_tests() function */
void run_eeprom_tests(void);
void run_motor_tests(void);
void run_buzzer_tests(void);
void run_ack_lights_tests(void);
void run_timer_tests(void);
void run_integration_tests(void);

#endif /* TEST_COMMON_H_ */

