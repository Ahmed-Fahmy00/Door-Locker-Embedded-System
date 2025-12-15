/*****************************************************************************
 * File: uart_handler.c
 * Description: UART Communication Handler for Control_ECU (Backend)
 * 
 * Protocol:
 *   Request:  [SOF=0x7E] [LEN] [CMD] [PAYLOAD...]
 *   Response: [SOF=0xFE] [LEN] [CMD] [STATUS] [DATA...]
 * 
 * Debug LED (PF2 = Blue only, since PF1/PF3 used by motor):
 *   - 1 blink:  Command received
 *   - 2 blinks: Response sent OK
 *   - 3 blinks: Error
 *   - Solid ON: Processing
 *****************************************************************************/

#include "uart_handler.h"
#include "eeprom_handler.h"
#include "components/timeout.h"
#include "components/motor.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/* TivaWare Includes */
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"

/*===========================================================================
 * Debug LED (PF2 = Blue only - doesn't conflict with motor)
 *===========================================================================*/
#define LED_BLUE    GPIO_PIN_2

static void LED_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)) {}
    
    /* Only configure PF2 - leave PF1/PF3 for motor */
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, LED_BLUE);
    GPIOPinWrite(GPIO_PORTF_BASE, LED_BLUE, 0);
}

static void LED_On(void)
{
    GPIOPinWrite(GPIO_PORTF_BASE, LED_BLUE, LED_BLUE);
}

static void LED_Off(void)
{
    GPIOPinWrite(GPIO_PORTF_BASE, LED_BLUE, 0);
}

static void LED_Blink(uint8_t times)
{
    for (uint8_t i = 0; i < times; i++)
    {
        LED_On();
        SysCtlDelay(SysCtlClockGet() / 20);  /* 50ms on */
        LED_Off();
        if (i < times - 1)
        {
            SysCtlDelay(SysCtlClockGet() / 20);  /* 50ms off */
        }
    }
}

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

/* Flags for deferred operations (done outside ISR) */
static volatile bool pending_open_door = false;
static volatile bool packet_ready = false;
static uint8_t packet_buf[UART_MAX_LEN];
static uint8_t packet_len = 0;

/*===========================================================================
 * Helper: Convert 5 ASCII digits to uint32
 *===========================================================================*/
static uint32_t ascii_to_u32(const uint8_t *p, uint8_t len)
{
    uint32_t v = 0;
    for (uint8_t i = 0; i < len; i++)
    {
        if (p[i] < '0' || p[i] > '9')
            return 0;
        v = v * 10 + (p[i] - '0');
    }
    return v;
}

/*===========================================================================
 * UART TX Functions
 *===========================================================================*/
static void uart_send_byte(uint8_t b)
{
    while (UARTBusy(UART1_BASE)) {}
    UARTCharPut(UART1_BASE, b);
}

static void uart_wait_tx_done(void)
{
    while (UARTBusy(UART1_BASE)) {}
    SysCtlDelay(1000);
}

static void uart_send_response(uint8_t cmd, uint8_t status, uint8_t *data, uint8_t data_len)
{
    uint8_t len = 2 + data_len;  /* CMD + STATUS + data */
    
    uart_send_byte(UART_SOF_TX);   /* 0xFE */
    uart_send_byte(len);
    uart_send_byte(cmd);
    uart_send_byte(status);
    
    for (uint8_t i = 0; i < data_len; i++)
    {
        uart_send_byte(data[i]);
    }
    
    uart_wait_tx_done();
    
    /* Blink to show response sent */
    if (status == UART_STATUS_OK)
    {
        LED_Blink(2);  /* 2 blinks = OK */
    }
    else
    {
        LED_Blink(3);  /* 3 blinks = Error */
    }
}

/*===========================================================================
 * Command Handlers
 *===========================================================================*/

/* CMD 0x01: Initialize Password */
static void cmd_init_password(uint8_t *buf, uint8_t len)
{
    uint8_t status = UART_STATUS_ERROR;
    
    if (len == 6)  /* CMD(1) + 5 digits */
    {
        uint32_t pw = ascii_to_u32(&buf[1], 5);
        if (initialize_password(pw) == STATUS_OK)
        {
            status = UART_STATUS_OK;
        }
    }
    
    uart_send_response(CMD_INIT_PASSWORD, status, NULL, 0);
}

/* CMD 0x02: Authenticate */
static void cmd_auth(uint8_t *buf, uint8_t len)
{
    uint8_t status = UART_STATUS_ERROR;
    
    if (len == 7)  /* CMD(1) + MODE(1) + 5 digits */
    {
        uint8_t mode = buf[1];
        uint32_t pw = ascii_to_u32(&buf[2], 5);
        
        int result = authenticate(pw, 0);
        
        if (result == STATUS_OK)
        {
            status = UART_STATUS_OK;
            
            /* If mode=1, set flag to open door (done outside ISR) */
            if (mode == 0x01)
            {
                pending_open_door = true;
            }
        }
        else
        {
            status = UART_STATUS_AUTH_FAIL;
        }
    }
    
    uart_send_response(CMD_AUTH, status, NULL, 0);
}

/* CMD 0x03: Set Timeout */
static void cmd_set_timeout(uint8_t *buf, uint8_t len)
{
    uint8_t status = UART_STATUS_ERROR;
    
    if (len == 2)  /* CMD(1) + SECONDS(1) */
    {
        uint8_t seconds = buf[1];
        
        /* Validate range 5-30 */
        if (seconds >= 5 && seconds <= 30)
        {
            if (change_auto_timeout(seconds) == STATUS_OK)
            {
                status = UART_STATUS_OK;
            }
        }
    }
    
    uart_send_response(CMD_SET_TIMEOUT, status, NULL, 0);
}

/* CMD 0x04: Change Password */
static void cmd_change_password(uint8_t *buf, uint8_t len)
{
    uint8_t status = UART_STATUS_ERROR;
    
    if (len == 6)  /* CMD(1) + 5 digits */
    {
        uint32_t new_pw = ascii_to_u32(&buf[1], 5);
        if (change_password(new_pw) == STATUS_OK)
        {
            status = UART_STATUS_OK;
        }
    }
    
    uart_send_response(CMD_CHANGE_PASSWORD, status, NULL, 0);
}

/* CMD 0x05: Get Timeout 
 * If pending_open_door is set (from AUTH mode=1), this is for door countdown.
 * Otherwise, this is for lockout (activate buzzer).
 */
static void cmd_get_timeout(uint8_t *buf, uint8_t len)
{
    uint8_t status = UART_STATUS_ERROR;
    uint8_t timeout_val = 0;
    
    if (len == 1)  /* CMD only */
    {
        uint32_t timeout;
        if (get_auto_timeout(&timeout) == STATUS_OK)
        {
            status = UART_STATUS_OK;
            timeout_val = (uint8_t)timeout;
            
            /* Check if this is for door opening or lockout */
            if (pending_open_door)
            {
                /* Door opening - just return timeout, don't activate buzzer */
                /* Motor will be started by CMD_OPEN_DOOR */
                pending_open_door = false;
            }
            else
            {
                /* Lockout - activate buzzer */
                activate_timeout(timeout);
            }
        }
    }
    
    uart_send_response(CMD_TIMEOUT, status, &timeout_val, 
                       (status == UART_STATUS_OK) ? 1 : 0);
}

/* CMD 0x07: Open Door - Frontend calls this to start motor forward */
static void cmd_open_door(uint8_t *buf, uint8_t len)
{
    uint8_t status = UART_STATUS_OK;
    
    /* Send response FIRST so frontend doesn't timeout */
    uart_send_response(CMD_OPEN_DOOR, status, NULL, 0);
    
    /* THEN start motor forward (3 sec) - blocks */
    open_door_start();
}

/* CMD 0x06: Close Door - Frontend calls this after timeout countdown */
static void cmd_close_door(uint8_t *buf, uint8_t len)
{
    uint8_t status = UART_STATUS_OK;
    
    /* Send response first */
    uart_send_response(CMD_CLOSE_DOOR, status, NULL, 0);
    
    /* Then close door (3 sec reverse) */
    close_door();
}

/*===========================================================================
 * Packet Dispatcher (called from main loop, NOT from ISR)
 *===========================================================================*/
static void handle_packet(uint8_t *buf, uint8_t len)
{
    if (len == 0) return;
    
    uint8_t cmd = buf[0];
    
    LED_On();  /* LED on while processing */
    
    switch (cmd)
    {
        case CMD_INIT_PASSWORD:
            cmd_init_password(buf, len);
            break;
            
        case CMD_AUTH:
            cmd_auth(buf, len);
            break;
            
        case CMD_SET_TIMEOUT:
            cmd_set_timeout(buf, len);
            break;
            
        case CMD_CHANGE_PASSWORD:
            cmd_change_password(buf, len);
            break;
            
        case CMD_TIMEOUT:
            cmd_get_timeout(buf, len);
            break;
            
        case CMD_CLOSE_DOOR:
            cmd_close_door(buf, len);
            break;
            
        case CMD_OPEN_DOOR:
            cmd_open_door(buf, len);
            break;
            
        default:
            /* Unknown command - send error response */
            uart_send_response(cmd, UART_STATUS_ERROR, NULL, 0);
            break;
    }
    
    LED_Off();
}

/*===========================================================================
 * UART Initialization
 *===========================================================================*/
void UART_Handler_Init(void)
{
    /* Initialize debug LED (PF2 only) */
    LED_Init();
    LED_Blink(1);  /* 1 blink = starting */
    
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
    pending_open_door = false;
    packet_ready = false;
    
    LED_Blink(2);  /* 2 blinks = ready */
}

/*===========================================================================
 * Process Pending Operations (call from main loop)
 *===========================================================================*/
void UART_ProcessPending(void)
{
    /* Process received packet outside ISR */
    if (packet_ready)
    {
        packet_ready = false;
        handle_packet(packet_buf, packet_len);
    }
    
    /* Door opening is now triggered by GET_TIMEOUT command, not here */
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
