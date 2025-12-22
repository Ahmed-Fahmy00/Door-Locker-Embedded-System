/******************************************************************************
 * File: uart.h
 * Module: UART (MCAL Layer)
 * Description: Low-level UART driver for TM4C123GH6PM (Backend)
 ******************************************************************************/

#ifndef UART_MCAL_H
#define UART_MCAL_H

#include <stdint.h>
#include <stdbool.h>

#define UART_MAX_LEN     32

/**
 * @brief Initialize UART1 hardware (PB0=RX, PB1=TX) at 115200 baud
 */
void UART_Driver_Init(void);

/**
 * @brief Send a single byte (blocking)
 * @param b Byte to send
 */
void UART_Driver_SendByte(uint8_t b);

/**
 * @brief Wait for TX to complete
 */
void UART_Driver_WaitTxDone(void);

/**
 * @brief Check if a complete packet is ready
 * @return true if packet is ready
 */
bool UART_Driver_IsPacketReady(void);

/**
 * @brief Get the received packet
 * @param buf Buffer to copy packet into
 * @param len Pointer to store packet length
 */
void UART_Driver_GetPacket(uint8_t *buf, uint8_t *len);

/**
 * @brief UART1 Interrupt Handler (must be registered in startup)
 */
void UART1IntHandler(void);

#endif /* UART_MCAL_H */
