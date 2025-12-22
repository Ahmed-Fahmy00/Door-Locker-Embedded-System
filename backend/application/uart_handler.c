/******************************************************************************
 * File: uart_handler.c
 * Module: UART Handler (Application Layer)
 * Description: Main UART handler for Backend - ties together all UART modules
 ******************************************************************************/

#include "uart_handler.h"
#include "uart_protocol.h"
#include "../MCAL/uart.h"
#include "../HAL/status_led.h"

/* Packet buffer */
static uint8_t packet_buf[UART_MAX_LEN];
static uint8_t packet_len = 0;

void UART_Handler_Init(void)
{
    /* Initialize status LEDs */
    LED_Init();
    LED_BlinkGreen(1);  /* Green blink = starting */
    
    /* Initialize UART driver */
    UART_Driver_Init();
    
    LED_BlinkGreen(2);  /* Green = ready */
}

void UART_ProcessPending(void)
{
    /* Check if a packet is ready */
    if (UART_Driver_IsPacketReady())
    {
        /* Get the packet */
        UART_Driver_GetPacket(packet_buf, &packet_len);
        
        /* Process it */
        UART_Protocol_HandlePacket(packet_buf, packet_len);
    }
}
