/******************************************************************************
 * File: uart.h
 * Module: UART (MCAL Layer)
 * Description: Low-level UART driver for TM4C123GH6PM
 ******************************************************************************/

#ifndef UART_H
#define UART_H

#include <stdint.h>

/**
 * @brief Initialize UART1 (PB0=Rx, PB1=Tx) at 115200 baud
 */
void UART_Driver_Init(void);

/**
 * @brief Re-initialize UART after communication failures
 */
void UART_Driver_Reinit(void);

/**
 * @brief Flush the RX FIFO
 */
void UART_Driver_FlushRx(void);

/**
 * @brief Send a single byte (blocking with timeout)
 * @param data Byte to send
 * @return 1 on success, 0 on timeout
 */
uint8_t UART_Driver_SendByte(uint8_t data);

/**
 * @brief Wait for TX to complete
 */
void UART_Driver_WaitTxComplete(void);

/**
 * @brief Receive a single byte with timeout
 * @param data Pointer to store received byte
 * @return 1 on success, 0 on timeout/error
 */
uint8_t UART_Driver_ReceiveByte(uint8_t *data);

#endif /* UART_H */
