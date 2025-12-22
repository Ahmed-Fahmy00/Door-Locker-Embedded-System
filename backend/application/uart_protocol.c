/******************************************************************************
 * File: uart_protocol.c
 * Module: UART Protocol (Application Layer)
 * Description: UART packet protocol handling for Backend
 ******************************************************************************/

#include "uart_protocol.h"
#include "uart_commands.h"
#include "../MCAL/uart.h"
#include "../HAL/status_led.h"
#include <stddef.h>

void UART_Protocol_SendResponse(uint8_t cmd, uint8_t status, uint8_t *data, uint8_t data_len)
{
    uint8_t len = 2 + data_len;  /* CMD + STATUS + data */
    
    UART_Driver_SendByte(UART_SOF_TX);   /* 0xFE */
    UART_Driver_SendByte(len);
    UART_Driver_SendByte(cmd);
    UART_Driver_SendByte(status);
    
    for (uint8_t i = 0; i < data_len; i++)
    {
        UART_Driver_SendByte(data[i]);
    }
    
    UART_Driver_WaitTxDone();
    
    /* Blink to show response sent */
    if (status == UART_STATUS_OK)
    {
        LED_BlinkGreen(2);  /* Green = OK */
    }
    else
    {
        LED_BlinkRed(2);    /* Red = Error */
    }
}

void UART_Protocol_HandlePacket(uint8_t *buf, uint8_t len)
{
    if (len == 0) return;
    
    uint8_t cmd = buf[0];
    
    LED_GreenOn();  /* Green LED on while processing */
    
    switch (cmd)
    {
        case CMD_INIT_PASSWORD:
            CMD_InitPassword(buf, len);
            break;
            
        case CMD_AUTH:
            CMD_Auth(buf, len);
            break;
            
        case CMD_SET_TIMEOUT:
            CMD_SetTimeout(buf, len);
            break;
            
        case CMD_CHANGE_PASSWORD:
            CMD_ChangePassword(buf, len);
            break;
            
        case CMD_GET_TIMEOUT:
            CMD_GetTimeout(buf, len);
            break;
            
        default:
            /* Unknown command - send error response */
            UART_Protocol_SendResponse(cmd, UART_STATUS_ERROR, NULL, 0);
            break;
    }
    
    LED_Off();
}
