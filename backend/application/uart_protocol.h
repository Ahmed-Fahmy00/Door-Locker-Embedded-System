/******************************************************************************
 * File: uart_protocol.h
 * Module: UART Protocol (Application Layer)
 * Description: UART packet protocol handling for Backend
 ******************************************************************************/

#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H

#include <stdint.h>

/* Protocol Constants */
#define UART_SOF_RX      0x7E
#define UART_SOF_TX      0xFE

/* Status Codes */
#define UART_STATUS_OK        0x00
#define UART_STATUS_ERROR     0x01
#define UART_STATUS_AUTH_FAIL 0x02

/**
 * @brief Send a response packet
 * @param cmd Command ID
 * @param status Status code
 * @param data Response data (can be NULL)
 * @param data_len Length of response data
 */
void UART_Protocol_SendResponse(uint8_t cmd, uint8_t status, uint8_t *data, uint8_t data_len);

/**
 * @brief Process a received packet (dispatches to command handlers)
 * @param buf Packet buffer
 * @param len Packet length
 */
void UART_Protocol_HandlePacket(uint8_t *buf, uint8_t len);

#endif /* UART_PROTOCOL_H */
