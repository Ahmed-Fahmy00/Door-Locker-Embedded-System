/******************************************************************************
 * File: uart.c
 * Module: UART (MCAL Layer)
 * Description: Low-level UART driver for TM4C123GH6PM
 ******************************************************************************/

#include "uart.h"
#include "../lib/tm4c123gh6pm.h"
#include "systick.h"

#define UART_TIMEOUT_LOOPS   2000000

void UART_Driver_Init(void)
{
    SYSCTL_RCGCUART_R |= 0x02;
    SYSCTL_RCGCGPIO_R |= 0x02;
    
    while ((SYSCTL_PRUART_R & 0x02) == 0) {}
    while ((SYSCTL_PRGPIO_R & 0x02) == 0) {}
    
    GPIO_PORTB_AFSEL_R |= 0x03;
    GPIO_PORTB_PCTL_R &= ~0x000000FF;
    GPIO_PORTB_PCTL_R |= 0x00000011;
    GPIO_PORTB_DEN_R |= 0x03;
    
    UART1_CTL_R &= ~0x01;
    UART1_IBRD_R = 8;
    UART1_FBRD_R = 44;
    UART1_LCRH_R = 0x70;
    UART1_ECR_R = 0xFF;
    UART1_CTL_R = 0x301;
    
    UART_Driver_FlushRx();
}

void UART_Driver_Reinit(void)
{
    UART1_CTL_R &= ~0x01;
    UART1_ECR_R = 0xFF;
    DelayMs(1);
    UART1_IBRD_R = 8;
    UART1_FBRD_R = 44;
    UART1_LCRH_R = 0x70;
    UART1_CTL_R = 0x301;
    while ((UART1_FR_R & 0x10) == 0) {
        (void)UART1_DR_R;
    }
    DelayMs(1);
}

void UART_Driver_FlushRx(void)
{
    while ((UART1_FR_R & 0x10) == 0) {
        (void)UART1_DR_R;
    }
    UART1_ECR_R = 0xFF;
}

uint8_t UART_Driver_SendByte(uint8_t data)
{
    uint32_t timeout = UART_TIMEOUT_LOOPS;
    while ((UART1_FR_R & 0x20) != 0) {
        timeout--;
        if (timeout == 0) return 0;
    }
    UART1_DR_R = data;
    return 1;
}

void UART_Driver_WaitTxComplete(void)
{
    uint32_t timeout = UART_TIMEOUT_LOOPS;
    while ((UART1_FR_R & 0x08) != 0 && timeout > 0) {
        timeout--;
    }
    timeout = UART_TIMEOUT_LOOPS;
    while ((UART1_FR_R & 0x80) == 0 && timeout > 0) {
        timeout--;
    }
}

uint8_t UART_Driver_ReceiveByte(uint8_t *data)
{
    uint32_t timeout = UART_TIMEOUT_LOOPS;
    uint32_t dr;
    
    while ((UART1_FR_R & 0x10) != 0) {
        timeout--;
        if (timeout == 0) return 0;
    }
    
    dr = UART1_DR_R;
    if (dr & 0xF00) {
        UART1_ECR_R = 0xFF;
        return 0;
    }
    
    *data = (uint8_t)(dr & 0xFF);
    return 1;
}
