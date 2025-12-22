/******************************************************************************
 * File: uart_protocol.h
 * Module: UART Protocol (Application Layer)
 * Description: Packet-level UART protocol handling
 ******************************************************************************/

#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H

#include <stdint.h>

/* Protocol Constants */
#define SOF_REQUEST         0x7E    /* Start of frame for requests */
#define SOF_RESPONSE        0xFE    /* Start of frame for responses */

/* Status Codes */
#define STATUS_OK               0x00
#define STATUS_ERROR            0x01
#define STATUS_AUTH_FAIL        0x02
#define STATUS_UNKNOWN_CMD      0xFF

/**
 * @brief Initialize the protocol layer (calls UART driver init)
 */
void UART_Protocol_Init(void);

/**
 * @brief Send a command packet with automatic retry
 * @param cmd Command ID
 * @param payload Payload data (can be NULL)
 * @param payloadLen Length of payload
 * @param outData Buffer for response data (can be NULL)
 * @param outDataLen Pointer to store response data length (can be NULL)
 * @return Status code from response
 */
uint8_t UART_Protocol_SendCommand(uint8_t cmd, const uint8_t *payload, 
                                   uint8_t payloadLen, uint8_t *outData, 
                                   uint8_t *outDataLen);

#endif /* UART_PROTOCOL_H */
