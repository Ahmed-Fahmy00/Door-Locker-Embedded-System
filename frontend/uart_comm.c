/*****************************************************************************
 * File: uart_comm.c
 * Description: UART Communication Layer for HMI_ECU <-> Control_ECU
 * 
 * TEMPORARY: Local storage for testing without backend
 * TODO: Replace with actual UART communication when backend is ready
 *****************************************************************************/

#include "uart_comm.h"
#include "lib/tm4c123gh6pm.h"
#include <string.h>

/*===========================================================================
 * TEMPORARY LOCAL STORAGE (for testing without backend)
 *===========================================================================*/
static char storedPassword[PASSWORD_LENGTH + 1] = {0};
static uint8_t storedTimeout = 15;  /* Default 15 seconds */
static bool passwordExists = false;
static uint8_t wrongAttempts = 0;

/*****************************************************************************
 * Initialize UART1 (PB0=Rx, PB1=Tx)
 * For now, just initialize the GPIO - actual UART not used yet
 *****************************************************************************/
void UART_Init(void)
{
    /* Enable UART1 and PORTB clocks */
    SYSCTL_RCGCUART_R |= 0x02;
    SYSCTL_RCGCGPIO_R |= 0x02;
    
    /* Wait for clocks */
    while ((SYSCTL_PRUART_R & 0x02) == 0) {}
    while ((SYSCTL_PRGPIO_R & 0x02) == 0) {}
    
    /* Configure PB0, PB1 for UART */
    GPIO_PORTB_AFSEL_R |= 0x03;
    GPIO_PORTB_PCTL_R &= ~0x000000FF;
    GPIO_PORTB_PCTL_R |= 0x00000011;
    GPIO_PORTB_DEN_R |= 0x03;
    
    /* Configure UART1: 9600 baud, 8N1 @ 50MHz */
    UART1_CTL_R &= ~0x01;
    UART1_IBRD_R = 325;
    UART1_FBRD_R = 33;
    UART1_LCRH_R = 0x70;
    UART1_CTL_R = 0x301;
    
    /* Reset local storage */
    passwordExists = false;
    wrongAttempts = 0;
}

/*****************************************************************************
 * Check if first time (no password saved)
 * TEMPORARY: Uses local variable
 *****************************************************************************/
bool UART_IsFirstTime(void)
{
    return !passwordExists;
}

/*****************************************************************************
 * Save password
 * TEMPORARY: Stores locally
 *****************************************************************************/
bool UART_SavePassword(const char *password)
{
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
        storedPassword[i] = password[i];
    }
    storedPassword[PASSWORD_LENGTH] = '\0';
    passwordExists = true;
    wrongAttempts = 0;
    return true;
}

/*****************************************************************************
 * Verify password
 * TEMPORARY: Compares with local storage
 *****************************************************************************/
uint8_t UART_VerifyPassword(const char *password)
{
    /* Check if password matches */
    bool match = true;
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
        if (password[i] != storedPassword[i]) {
            match = false;
            break;
        }
    }
    
    if (match) {
        wrongAttempts = 0;
        return RESP_PASSWORD_CORRECT;
    } else {
        wrongAttempts++;
        if (wrongAttempts >= MAX_ATTEMPTS) {
            return RESP_LOCKOUT;
        }
        return RESP_PASSWORD_WRONG;
    }
}

/*****************************************************************************
 * Change password
 * TEMPORARY: Verifies old, saves new locally
 *****************************************************************************/
uint8_t UART_ChangePassword(const char *oldPassword, const char *newPassword)
{
    /* Verify old password first */
    bool match = true;
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
        if (oldPassword[i] != storedPassword[i]) {
            match = false;
            break;
        }
    }
    
    if (!match) {
        wrongAttempts++;
        if (wrongAttempts >= MAX_ATTEMPTS) {
            return RESP_LOCKOUT;
        }
        return RESP_PASSWORD_WRONG;
    }
    
    /* Old password correct - reset attempts */
    wrongAttempts = 0;
    
    /* Save new password (even if same as old, that's allowed) */
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
        storedPassword[i] = newPassword[i];
    }
    storedPassword[PASSWORD_LENGTH] = '\0';
    return RESP_OK;
}

/*****************************************************************************
 * Save timeout
 * TEMPORARY: Stores locally
 *****************************************************************************/
bool UART_SaveTimeout(uint8_t timeoutSeconds)
{
    storedTimeout = timeoutSeconds;
    return true;
}

/*****************************************************************************
 * Get timeout
 * TEMPORARY: Returns local value
 *****************************************************************************/
uint8_t UART_GetTimeout(void)
{
    return storedTimeout;
}

/*****************************************************************************
 * Wait for door closed
 * TEMPORARY: Just delays
 *****************************************************************************/
void UART_WaitDoorClosed(void)
{
    /* Simulate door closing time */
    for (volatile int i = 0; i < 1000000; i++);
}

/*****************************************************************************
 * Get lockout time
 * TEMPORARY: Returns fixed value
 *****************************************************************************/
uint8_t UART_GetLockoutTime(void)
{
    wrongAttempts = 0;  /* Reset after lockout */
    return 30;  /* 30 second lockout */
}
