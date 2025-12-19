/*****************************************************************************
 * Description: UART Communication Layer for HMI_ECU <-> Control_ECU
 * UART1: PB0 (Rx), PB1 (Tx) - 115200 baud, 8N1
 * Packet Protocol:
 *   Request:  [SOF=0x7E] [LEN] [CMD] [PAYLOAD...]
 *   Response: [SOF=0xFE] [LEN=2] [CMD] [STATUS]
 *****************************************************************************/
#ifndef UART_COMM_H
#define UART_COMM_H

#include <stdint.h>
#include <stdbool.h>

/* Password configuration */
#define PASSWORD_LENGTH 5
#define MAX_ATTEMPTS 3

/* Protocol Constants */
#define SOF_REQUEST         0x7E    /* Start of frame for requests */
#define SOF_RESPONSE        0xFE    /* Start of frame for responses */

/* Command IDs (must match backend uart_handler.h) */
#define CMD_INIT_PASSWORD       0x01    /* Initialize password (signup) */
#define CMD_AUTH                0x02    /* Authenticate (signin) */
#define CMD_SET_TIMEOUT         0x03    /* Set timeout value */
#define CMD_CHANGE_PASSWORD     0x04    /* Change password */
#define CMD_GET_TIMEOUT         0x05    /* Get timeout value */

/* Auth Modes (for CMD_AUTH) */
#define AUTH_MODE_CHECK_ONLY    0x00    /* Just verify password */
#define AUTH_MODE_OPEN_DOOR     0x01    /* Verify + open door if correct */

/* Status Codes */
#define STATUS_OK               0x00    /* Success */
#define STATUS_ERROR            0x01    /* General error */
#define STATUS_AUTH_FAIL        0x02    /* Authentication failed / wrong password */
#define STATUS_UNKNOWN_CMD      0xFF    /* Communication timeout / unknown command */

/* Initialize UART1 */
void UART_Init(void);

/* CMD 0x01: Initialize password (signup) */
uint8_t UART_InitPassword(const char *password);
/* CMD 0x02: Authenticate */
uint8_t UART_Authenticate(const char *password, uint8_t mode, uint8_t *outTimeout);
/* CMD 0x03: Set timeout (5-30 seconds) */
uint8_t UART_SetTimeout(uint8_t seconds);
/* CMD 0x04: Change password */
uint8_t UART_ChangePassword(const char *newPassword);
/* CMD 0x05: Get timeout value from backend */
uint8_t UART_GetTimeout(uint8_t *outTimeout);

#endif /* UART_COMM_H */