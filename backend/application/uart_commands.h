/******************************************************************************
 * File: uart_commands.h
 * Module: UART Commands (Application Layer)
 * Description: Command handlers for Backend UART protocol
 ******************************************************************************/

#ifndef UART_COMMANDS_H
#define UART_COMMANDS_H

#include <stdint.h>

/* Command IDs */
#define CMD_INIT_PASSWORD     0x01
#define CMD_AUTH              0x02
#define CMD_SET_TIMEOUT       0x03
#define CMD_CHANGE_PASSWORD   0x04
#define CMD_GET_TIMEOUT       0x05

/**
 * @brief CMD 0x01: Initialize Password
 */
void CMD_InitPassword(uint8_t *buf, uint8_t len);

/**
 * @brief CMD 0x02: Authenticate (and optionally open door)
 */
void CMD_Auth(uint8_t *buf, uint8_t len);

/**
 * @brief CMD 0x03: Set Timeout
 */
void CMD_SetTimeout(uint8_t *buf, uint8_t len);

/**
 * @brief CMD 0x04: Change Password
 */
void CMD_ChangePassword(uint8_t *buf, uint8_t len);

/**
 * @brief CMD 0x05: Get Timeout (and activate buzzer for lockout)
 */
void CMD_GetTimeout(uint8_t *buf, uint8_t len);

#endif /* UART_COMMANDS_H */
