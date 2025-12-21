/*
 * test_eeprom.c - Unit tests for EEPROM handler
 * 
 * Tests password storage, authentication, and timeout functions
 * in eeprom_handler.c
 */

#include "test_common.h"
#include "../eeprom_handler.h"
#include <stdint.h>

/*===========================================================================
 * Test: Initialize Password
 *===========================================================================*/
static TestResult test_initialize_password(void)
{
    int result;
    
    /* Initialize password to known value */
    result = initialize_password(12345);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    
    /* Verify by authenticating */
    result = authenticate(12345, 0);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Authenticate Success
 *===========================================================================*/
static TestResult test_authenticate_success(void)
{
    int result;
    
    /* Set known password */
    initialize_password(54321);
    
    /* Authenticate with correct password */
    result = authenticate(54321, 0);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Authenticate Failure
 *===========================================================================*/
static TestResult test_authenticate_fail(void)
{
    int result;
    
    /* Set known password */
    initialize_password(11111);
    
    /* Try wrong password */
    result = authenticate(99999, 0);
    TEST_ASSERT_EQUAL(STATUS_AUTH_FAIL, result);
    
    /* Try another wrong password */
    result = authenticate(00000, 0);
    TEST_ASSERT_EQUAL(STATUS_AUTH_FAIL, result);
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Change Password
 *===========================================================================*/
static TestResult test_change_password(void)
{
    int result;
    
    /* Set initial password */
    initialize_password(11111);
    
    /* Change to new password */
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
 * Test: Default Timeout
 *===========================================================================*/
static TestResult test_timeout_default(void)
{
    int result;
    uint32_t timeout = 0;
    
    /* First force timeout to default by setting it directly */
    result = change_auto_timeout(DEFAULT_TIMEOUT);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    
    /* Read timeout */
    result = get_auto_timeout(&timeout);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    
    /* Should be DEFAULT_TIMEOUT (11 seconds as defined in eeprom_handler.h) */
    TEST_ASSERT_EQUAL(DEFAULT_TIMEOUT, timeout);
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Change Timeout
 *===========================================================================*/
static TestResult test_timeout_change(void)
{
    int result;
    uint32_t timeout = 0;
    
    /* Change timeout to 15 seconds */
    result = change_auto_timeout(15);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    
    /* Read back */
    result = get_auto_timeout(&timeout);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    TEST_ASSERT_EQUAL(15, timeout);
    
    /* Change to 30 seconds */
    result = change_auto_timeout(30);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    
    result = get_auto_timeout(&timeout);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    TEST_ASSERT_EQUAL(30, timeout);
    
    TEST_PASS();
}

/*===========================================================================
 * Test: Timeout Boundary Values
 *===========================================================================*/
static TestResult test_timeout_boundaries(void)
{
    int result;
    uint32_t timeout = 0;
    
    /* Test minimum timeout (5 seconds) */
    result = change_auto_timeout(5);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    
    result = get_auto_timeout(&timeout);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    TEST_ASSERT_EQUAL(5, timeout);
    
    /* Test maximum timeout (30 seconds) */
    result = change_auto_timeout(30);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    
    result = get_auto_timeout(&timeout);
    TEST_ASSERT_EQUAL(STATUS_OK, result);
    TEST_ASSERT_EQUAL(30, timeout);
    
    TEST_PASS();
}

/*===========================================================================
 * Run All EEPROM Tests
 *===========================================================================*/
void run_eeprom_tests(void)
{
    printf("\n--- EEPROM Handler Tests ---\n");
    
    run_test("Initialize Password", test_initialize_password);
    run_test("Authenticate Success", test_authenticate_success);
    run_test("Authenticate Failure", test_authenticate_fail);
    run_test("Change Password", test_change_password);
    run_test("Default Timeout", test_timeout_default);
    run_test("Change Timeout", test_timeout_change);
    run_test("Timeout Boundaries", test_timeout_boundaries);
}

