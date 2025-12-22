/******************************************************************************
 * File: uart.c
 * Module: UART (MCAL Layer)
 * Description: Low-level UART driver for TM4C123GH6PM (Backend)
 ******************************************************************************/

#include "uart.h"
#include <stdbool.h>

/* TivaWare Includes */
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"

/* Protocol Constants */
#define UART_SOF_RX      0x7E

/*===========================================================================
 * RX State Machine
 *===========================================================================*/
typedef enum {
    RX_WAIT_SOF = 0,
    RX_READ_LEN,
    RX_READ_BODY
} rx_state_t;

static volatile rx_state_t rx_state = RX_WAIT_SOF;
static volatile uint8_t rx_len = 0;
static volatile uint8_t rx_index = 0;
static uint8_t rx_buf[UART_MAX_LEN];

/* Packet buffer for main loop processing */
static volatile bool packet_ready = false;
static uint8_t packet_buf[UART_MAX_LEN];
static uint8_t packet_len = 0;

/*===========================================================================
 * Public Functions
 *===========================================================================*/

void UART_Driver_Init(void)
{
    /* Enable UART1 and GPIO Port B */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_UART1)) {}
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)) {}
    
    /* Configure PB0=RX, PB1=TX */
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    
    /* Disable UART before configuration */
    UARTDisable(UART1_BASE);
    
    /* Configure UART: 115200 baud, 8N1 */
    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 115200,
                        UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE);
    
    /* Enable FIFO */
    UARTFIFOEnable(UART1_BASE);
    UARTFIFOLevelSet(UART1_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);
    
    /* Clear any pending errors */
    UARTRxErrorClear(UART1_BASE);
    
    /* Clear and enable interrupts */
    UARTIntDisable(UART1_BASE, 0xFFFFFFFF);
    UARTIntClear(UART1_BASE, 0xFFFFFFFF);
    UARTIntRegister(UART1_BASE, UART1IntHandler);
    UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);
    
    /* Enable UART */
    UARTEnable(UART1_BASE);
    
    IntEnable(INT_UART1);
    IntMasterEnable();
    
    /* Initialize state */
    rx_state = RX_WAIT_SOF;
    packet_ready = false;
}

void UART_Driver_SendByte(uint8_t b)
{
    while (UARTBusy(UART1_BASE)) {}
    UARTCharPut(UART1_BASE, b);
}

void UART_Driver_WaitTxDone(void)
{
    while (UARTBusy(UART1_BASE)) {}
    SysCtlDelay(1000);
}

bool UART_Driver_IsPacketReady(void)
{
    return packet_ready;
}

void UART_Driver_GetPacket(uint8_t *buf, uint8_t *len)
{
    for (uint8_t i = 0; i < packet_len; i++)
    {
        buf[i] = packet_buf[i];
    }
    *len = packet_len;
    packet_ready = false;
}

/*===========================================================================
 * UART ISR - Only collects bytes, doesn't process commands
 *===========================================================================*/
void UART1IntHandler(void)
{
    uint32_t int_status = UARTIntStatus(UART1_BASE, true);
    UARTIntClear(UART1_BASE, int_status);
    
    /* Clear any RX errors */
    if (UARTRxErrorGet(UART1_BASE))
    {
        UARTRxErrorClear(UART1_BASE);
        rx_state = RX_WAIT_SOF;
    }
    
    /* Process received bytes */
    while (UARTCharsAvail(UART1_BASE))
    {
        uint8_t byte = (uint8_t)UARTCharGetNonBlocking(UART1_BASE);
        
        switch (rx_state)
        {
            case RX_WAIT_SOF:
                if (byte == UART_SOF_RX)
                {
                    rx_state = RX_READ_LEN;
                }
                break;
                
            case RX_READ_LEN:
                if (byte > 0 && byte <= UART_MAX_LEN)
                {
                    rx_len = byte;
                    rx_index = 0;
                    rx_state = RX_READ_BODY;
                }
                else
                {
                    rx_state = RX_WAIT_SOF;
                }
                break;
                
            case RX_READ_BODY:
                rx_buf[rx_index++] = byte;
                if (rx_index >= rx_len)
                {
                    /* Copy to packet buffer for main loop processing */
                    if (!packet_ready)  /* Don't overwrite unprocessed packet */
                    {
                        for (uint8_t i = 0; i < rx_len; i++)
                        {
                            packet_buf[i] = rx_buf[i];
                        }
                        packet_len = rx_len;
                        packet_ready = true;
                    }
                    rx_state = RX_WAIT_SOF;
                }
                break;
                
            default:
                rx_state = RX_WAIT_SOF;
                break;
        }
    }
}
