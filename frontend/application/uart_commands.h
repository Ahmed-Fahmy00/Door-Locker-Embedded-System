/******************************************************************************
 * File: uart_commands.h
 * Module: UART Commands (Application Layer)
 * Description: High-level UART command API for door locker system
 ******************************************************************************/

#ifndef UART_COMMANDS_H
#define UART_COMMANDS_H

#include <stdint.h>

/* Password configuration */
#define PASSWORD_LENGTH 5
#define MAX_ATTEMPTS 3

/* Command IDs */
#define CMD_INIT_PASSWORD       0x01
#define CMD_AUTH                0x02
#define CMD_SET_TIMEOUT         0x03
#define CMD_CHANGE_PASSWORD     0x04
#define CMD_GET_TIMEOUT         0x05

/* Auth Modes */
#define AUTH_MODE_CHECK_ONLY    0x00
#define AUTH_MODE_OPEN_DOOR     0x01

/* Re-export status codes from protocol layer */
#include "uart_protocol.h"

/**
 * @brief Initialize UART communication
 */
void UART_Init(void);

/**
 * @brief CMD 0x01: Initialize password (signup)
 * @param password 5-digit password string
 * @return Status code
 */
uint8_t UART_InitPassword(const char *password);

/**
 * @brief CMD 0x02: Authenticate
 * @param password 5-digit password string
 * @param mode AUTH_MODE_CHECK_ONLY or AUTH_MODE_OPEN_DOOR
 * @param outTimeout Pointer to store timeout value (can be NULL)
 * @return Status code
 */
uint8_t UART_Authenticate(const char *password, uint8_t mode, uint8_t *outTimeout);

/**
 * @brief CMD 0x03: Set timeout (5-30 seconds)
 * @param seconds Timeout value
 * @return Status code
 */
uint8_t UART_SetTimeout(uint8_t seconds);

/**
 * @brief CMD 0x04: Change password
 * @param newPassword New 5-digit password string
 * @return Status code
 */
uint8_t UART_ChangePassword(const char *newPassword);

/**
 * @brief CMD 0x05: Get timeout value
 * @param outTimeout Pointer to store timeout value
 * @return Status code
 */
uint8_t UART_GetTimeout(uint8_t *outTimeout);

#endif /* UART_COMMANDS_H */
