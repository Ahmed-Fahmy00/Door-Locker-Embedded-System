#ifndef UART_HANDLER_H
#define UART_HANDLER_H

#include <stdint.h>

/* =========================
   RX Protocol
   ========================= */
#define UART_SOF_RX      0x7E
#define UART_MAX_LEN     32

/* =========================
   TX Protocol
   ========================= */
#define UART_SOF_TX      0xFE
#define UART_TX_LEN      2
#define UART_TX_LEN_WITH_DATA 3

/* =========================
   Command IDs
   ========================= */
#define CMD_INIT_PASSWORD     0x01
#define CMD_AUTH              0x02
#define CMD_SET_TIMEOUT       0x03
#define CMD_CHANGE_PASSWORD   0x04
#define CMD_TIMEOUT           0x05
#define CMD_CLOSE_DOOR        0x06
#define CMD_OPEN_DOOR         0x07

/* =========================
   Status Codes
   ========================= */
#define UART_STATUS_OK        0x00
#define UART_STATUS_ERROR     0x01
#define UART_STATUS_AUTH_FAIL 0x02

/* =========================
   Public API
   ========================= */
void UART_Handler_Init(void);
void UART1IntHandler(void);
void UART_ProcessPending(void);  /* Call from main loop */

#endif /* UART_HANDLER_H */