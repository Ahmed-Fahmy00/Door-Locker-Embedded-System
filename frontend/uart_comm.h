/*****************************************************************************
 * File: uart_comm.h
 * Description: UART Communication Layer for HMI_ECU <-> Control_ECU
 * UART1: PB0 (Rx), PB1 (Tx) - 9600 baud, 8N1
 *****************************************************************************/

#ifndef UART_COMM_H
#define UART_COMM_H

#include <stdint.h>
#include <stdbool.h>

/* Password configuration */
#define PASSWORD_LENGTH     5
#define MAX_ATTEMPTS        3

/*===========================================================================
 * UART Command Codes (HMI_ECU -> Control_ECU)
 *===========================================================================*/
#define CMD_SAVE_PASSWORD       0x10    /* Save password to EEPROM (signup) */
#define CMD_VERIFY_PASSWORD     0x11    /* Verify password (signin) */
#define CMD_CHANGE_PASSWORD     0x12    /* Change password (old + new) */
#define CMD_SAVE_TIMEOUT        0x13    /* Save timeout to EEPROM */
#define CMD_GET_TIMEOUT         0x14    /* Get timeout from EEPROM */
#define CMD_CHECK_FIRST_TIME    0x15    /* Check if first time (no password) */

/*===========================================================================
 * UART Response Codes (Control_ECU -> HMI_ECU)
 *===========================================================================*/
#define RESP_OK                 0x20    /* Success */
#define RESP_PASSWORD_CORRECT   0x21    /* Password correct, door opening */
#define RESP_PASSWORD_WRONG     0x22    /* Password incorrect */
#define RESP_LOCKOUT            0x23    /* Lockout activated (buzzer on) */
#define RESP_DOOR_CLOSED        0x24    /* Door has closed */
#define RESP_FIRST_TIME         0x25    /* First time, need signup */
#define RESP_NOT_FIRST_TIME     0x26    /* Password exists, go to signin */
#define RESP_ERROR              0x2F    /* Error */

/*===========================================================================
 * Function Prototypes
 *===========================================================================*/

/* Initialize UART1 */
void UART_Init(void);

/* Check if first time setup needed */
bool UART_IsFirstTime(void);

/* Save password to backend EEPROM (signup) */
bool UART_SavePassword(const char *password);

/* Verify password with backend (signin) */
/* Returns: RESP_PASSWORD_CORRECT, RESP_PASSWORD_WRONG, or RESP_LOCKOUT */
uint8_t UART_VerifyPassword(const char *password);

/* Change password (sends old + new) */
uint8_t UART_ChangePassword(const char *oldPassword, const char *newPassword);

/* Save timeout to backend EEPROM */
bool UART_SaveTimeout(uint8_t timeoutSeconds);

/* Get timeout from backend EEPROM */
uint8_t UART_GetTimeout(void);

/* Wait for door closed signal from backend */
void UART_WaitDoorClosed(void);

/* Get lockout time from backend */
uint8_t UART_GetLockoutTime(void);

#endif /* UART_COMM_H */
