/******************************************************************************
 * File: uart_protocol.c
 * Module: UART Protocol (Application Layer)
 * Description: Packet-level UART protocol handling
 ******************************************************************************/

#include "uart_protocol.h"
#include "../MCAL/uart.h"
#include "../MCAL/systick.h"
#include <stddef.h>

#define UART_MAX_RETRIES     3
#define UART_SOF_SEARCH_MAX  100

static uint8_t consecutiveFailures = 0;

/* Send a complete packet: [SOF=0x7E] [LEN] [CMD] [PAYLOAD...] */
static uint8_t SendPacket(uint8_t cmd, const uint8_t *payload, uint8_t payloadLen)
{
    uint8_t len = 1 + payloadLen;
    uint8_t i;
    
    UART_Driver_FlushRx();
    
    if (!UART_Driver_SendByte(SOF_REQUEST)) return 0;
    if (!UART_Driver_SendByte(len)) return 0;
    if (!UART_Driver_SendByte(cmd)) return 0;
    
    for (i = 0; i < payloadLen; i++) {
        if (!UART_Driver_SendByte(payload[i])) return 0;
    }
    
    UART_Driver_WaitTxComplete();
    DelayMs(50);  /* Give backend time to process */
    
    return 1;
}

/* Receive response: [SOF=0xFE] [LEN] [CMD] [STATUS] [DATA...] */
static uint8_t ReceiveResponse(uint8_t *outData, uint8_t *outDataLen)
{
    uint8_t byte, len, cmd, status, i;
    uint8_t sofRetries = UART_SOF_SEARCH_MAX;
    
    if (outDataLen != NULL) *outDataLen = 0;
    
    while (sofRetries > 0) {
        if (!UART_Driver_ReceiveByte(&byte)) return STATUS_UNKNOWN_CMD;
        if (byte == SOF_RESPONSE) break;
        sofRetries--;
    }
    
    if (sofRetries == 0) return STATUS_UNKNOWN_CMD;
    if (!UART_Driver_ReceiveByte(&len)) return STATUS_UNKNOWN_CMD;
    if (len < 2 || len > 32) return STATUS_UNKNOWN_CMD;
    if (!UART_Driver_ReceiveByte(&cmd)) return STATUS_UNKNOWN_CMD;
    if (!UART_Driver_ReceiveByte(&status)) return STATUS_UNKNOWN_CMD;
    
    uint8_t dataBytes = (len > 2) ? (len - 2) : 0;
    for (i = 0; i < dataBytes; i++) {
        if (!UART_Driver_ReceiveByte(&byte)) break;
        if (outData != NULL && outDataLen != NULL && i < 16) {
            outData[i] = byte;
            (*outDataLen)++;
        }
    }
    
    return status;
}

void UART_Protocol_Init(void)
{
    UART_Driver_Init();
    consecutiveFailures = 0;
}

uint8_t UART_Protocol_SendCommand(uint8_t cmd, const uint8_t *payload, uint8_t payloadLen, uint8_t *outData, uint8_t *outDataLen)
{
    uint8_t retry, status;
    
    for (retry = 0; retry < UART_MAX_RETRIES; retry++) {
        if (!SendPacket(cmd, payload, payloadLen)) {
            DelayMs(50);
            continue;
        }
        
        status = ReceiveResponse(outData, outDataLen);
        if (status != STATUS_UNKNOWN_CMD) {
            consecutiveFailures = 0;
            return status;
        }
        
        UART_Driver_FlushRx();
        DelayMs(100);  /* Longer delay between retries */
    }
    
    consecutiveFailures++;
    if (consecutiveFailures >= 2) {
        UART_Driver_Reinit();
        consecutiveFailures = 0;
    }
    
    return STATUS_UNKNOWN_CMD;
}
