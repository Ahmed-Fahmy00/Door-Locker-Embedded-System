/*
 * test_common.c - Common test infrastructure implementation
 * 
 * Implements the test runner and statistics tracking functions
 * declared in test_common.h
 */

#include "test_common.h"
#include <stdio.h>

/*===========================================================================
 * Test Statistics (private)
 *===========================================================================*/
static uint32_t tests_passed = 0;
static uint32_t tests_failed = 0;
static uint32_t tests_total = 0;

/*===========================================================================
 * Test Framework Functions
 *===========================================================================*/

void test_init(void)
{
    tests_passed = 0;
    tests_failed = 0;
    tests_total = 0;
    printf("\n========================================\n");
    printf("       BACKEND UNIT TEST SUITE\n");
    printf("========================================\n");
}

void run_test(const char *test_name, TestFunction test_func)
{
    TestResult result;
    
    tests_total++;
    printf("  [%02u] %s ... ", tests_total, test_name);
    
    result = test_func();
    
    if (result == TEST_RESULT_PASS)
    {
        tests_passed++;
        printf("PASS\n");
    }
    else
    {
        tests_failed++;
        /* FAIL message already printed by TEST_ASSERT macros */
    }
}

void print_test_summary(void)
{
    printf("\n========================================\n");
    printf("            TEST SUMMARY\n");
    printf("========================================\n");
    printf("  Total:  %u\n", tests_total);
    printf("  Passed: %u\n", tests_passed);
    printf("  Failed: %u\n", tests_failed);
    printf("========================================\n");
    
    if (tests_failed == 0)
    {
        printf("  ALL TESTS PASSED!\n");
    }
    else
    {
        printf("  SOME TESTS FAILED!\n");
    }
    printf("========================================\n\n");
}

uint32_t get_tests_passed(void)
{
    return tests_passed;
}

uint32_t get_tests_failed(void)
{
    return tests_failed;
}

uint32_t get_tests_total(void)
{
    return tests_total;
}
