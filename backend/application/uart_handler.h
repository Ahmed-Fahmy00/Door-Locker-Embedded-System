/******************************************************************************
 * File: uart_handler.h
 * Module: UART Handler (Application Layer)
 * Description: Main UART handler API for Backend
 ******************************************************************************/

#ifndef UART_HANDLER_H
#define UART_HANDLER_H

#include <stdint.h>

/**
 * @brief Initialize UART handler (LEDs, UART driver, protocol)
 */
void UART_Handler_Init(void);

/**
 * @brief Process pending UART packets (call from main loop)
 */
void UART_ProcessPending(void);

/**
 * @brief UART1 Interrupt Handler (defined in MCAL/uart.c)
 */
void UART1IntHandler(void);

#endif /* UART_HANDLER_H */
