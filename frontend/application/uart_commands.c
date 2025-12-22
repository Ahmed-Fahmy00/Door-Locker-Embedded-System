/******************************************************************************
 * File: uart_commands.c
 * Module: UART Commands (Application Layer)
 * Description: High-level UART command API for door locker system
 ******************************************************************************/

#include "uart_commands.h"
#include "uart_protocol.h"
#include <stddef.h>

void UART_Init(void)
{
    UART_Protocol_Init();
}

/* CMD 0x01: Initialize Password (Signup) */
uint8_t UART_InitPassword(const char *password)
{
    uint8_t payload[PASSWORD_LENGTH];
    for (uint8_t i = 0; i < PASSWORD_LENGTH; i++) {
        payload[i] = (uint8_t)password[i];
    }
    return UART_Protocol_SendCommand(CMD_INIT_PASSWORD, payload, PASSWORD_LENGTH, NULL, NULL);
}

/* CMD 0x02: Authenticate (mode: 0=check only, 1=open door) */
uint8_t UART_Authenticate(const char *password, uint8_t mode, uint8_t *outTimeout)
{
    uint8_t payload[1 + PASSWORD_LENGTH];
    payload[0] = mode;
    for (uint8_t i = 0; i < PASSWORD_LENGTH; i++) {
        payload[1 + i] = (uint8_t)password[i];
    }
    uint8_t data[4];
    uint8_t dataLen = 0;
    uint8_t status = UART_Protocol_SendCommand(CMD_AUTH, payload, 1 + PASSWORD_LENGTH, data, &dataLen);
    if (status == STATUS_OK && dataLen >= 1 && outTimeout != NULL) {
        *outTimeout = data[0];
    }
    return status;
}

/* CMD 0x03: Set Timeout (5-30 seconds) */
uint8_t UART_SetTimeout(uint8_t seconds)
{
    if (seconds < 5) seconds = 5;
    if (seconds > 30) seconds = 30;
    uint8_t payload[1] = {seconds};
    return UART_Protocol_SendCommand(CMD_SET_TIMEOUT, payload, 1, NULL, NULL);
}

/* CMD 0x04: Change Password */
uint8_t UART_ChangePassword(const char *newPassword)
{
    uint8_t payload[PASSWORD_LENGTH];
    for (uint8_t i = 0; i < PASSWORD_LENGTH; i++) {
        payload[i] = (uint8_t)newPassword[i];
    }
    return UART_Protocol_SendCommand(CMD_CHANGE_PASSWORD, payload, PASSWORD_LENGTH, NULL, NULL);
}

/* CMD 0x05: Get Timeout */
uint8_t UART_GetTimeout(uint8_t *outTimeout)
{
    uint8_t data[4];
    uint8_t dataLen = 0;
    uint8_t status = UART_Protocol_SendCommand(CMD_GET_TIMEOUT, NULL, 0, data, &dataLen);
    if (status == STATUS_OK && dataLen >= 1 && outTimeout != NULL) {
        *outTimeout = data[0];
    }
    return status;
}
