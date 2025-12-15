/*
 * UART Communication Layer for HMI_ECU (Frontend)
 * Protocol:
 *   Request:  [SOF=0x7E] [LEN] [CMD] [PAYLOAD...]
 *   Response: [SOF=0xFE] [LEN] [CMD] [STATUS] [DATA...]
 */
#include "uart_comm.h"
#include "lib/tm4c123gh6pm.h"
#include "MCAL/systick.h"
#include <stddef.h>

#define UART_TIMEOUT_LOOPS   1000000
#define UART_MAX_RETRIES     3
#define UART_SOF_SEARCH_MAX  50

static uint8_t consecutiveFailures = 0;

/* Re-initialize UART (call after communication failures) */
static void UART_Reinit(void)
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

/* Flush RX FIFO */
static void UART_FlushRx(void)
{
    while ((UART1_FR_R & 0x10) == 0) {
        (void)UART1_DR_R;
    }
    UART1_ECR_R = 0xFF;
}

/* Send a single byte (blocking with timeout) */
static uint8_t UART_SendByte(uint8_t data)
{
    uint32_t timeout = UART_TIMEOUT_LOOPS;
    while ((UART1_FR_R & 0x20) != 0) {
        timeout--;
        if (timeout == 0) return 0;
    }
    UART1_DR_R = data;
    return 1;
}

/* Wait for TX to complete */
static void UART_WaitTxComplete(void)
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

/* Receive a single byte with timeout */
static uint8_t UART_ReceiveByteTimeout(uint8_t *data)
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

/* Send a complete packet: [SOF=0x7E] [LEN] [CMD] [PAYLOAD...] */
static uint8_t UART_SendPacket(uint8_t cmd, const uint8_t *payload, uint8_t payloadLen)
{
    uint8_t len = 1 + payloadLen;
    uint8_t i;
    
    UART_FlushRx();
    
    if (!UART_SendByte(SOF_REQUEST)) return 0;
    if (!UART_SendByte(len)) return 0;
    if (!UART_SendByte(cmd)) return 0;
    
    for (i = 0; i < payloadLen; i++) {
        if (!UART_SendByte(payload[i])) return 0;
    }
    
    UART_WaitTxComplete();
    DelayMs(10);  /* Give backend time to process */
    
    return 1;
}

/* Receive response: [SOF=0xFE] [LEN] [CMD] [STATUS] [DATA...] */
static uint8_t UART_ReceiveResponse(uint8_t *outData, uint8_t *outDataLen)
{
    uint8_t byte, len, cmd, status, i;
    uint8_t sofRetries = UART_SOF_SEARCH_MAX;
    
    if (outDataLen != NULL) *outDataLen = 0;
    
    while (sofRetries > 0) {
        if (!UART_ReceiveByteTimeout(&byte)) return STATUS_UNKNOWN_CMD;
        if (byte == SOF_RESPONSE) break;
        sofRetries--;
    }
    
    if (sofRetries == 0) return STATUS_UNKNOWN_CMD;
    if (!UART_ReceiveByteTimeout(&len)) return STATUS_UNKNOWN_CMD;
    if (len < 2 || len > 32) return STATUS_UNKNOWN_CMD;
    if (!UART_ReceiveByteTimeout(&cmd)) return STATUS_UNKNOWN_CMD;
    if (!UART_ReceiveByteTimeout(&status)) return STATUS_UNKNOWN_CMD;
    
    uint8_t dataBytes = (len > 2) ? (len - 2) : 0;
    for (i = 0; i < dataBytes; i++) {
        if (!UART_ReceiveByteTimeout(&byte)) break;
        if (outData != NULL && outDataLen != NULL && i < 16) {
            outData[i] = byte;
            (*outDataLen)++;
        }
    }
    
    return status;
}

/* Send command with automatic retry */
static uint8_t UART_SendCommandWithRetry(uint8_t cmd, const uint8_t *payload, uint8_t payloadLen, uint8_t *outData, uint8_t *outDataLen)
{
    uint8_t retry, status;
    
    for (retry = 0; retry < UART_MAX_RETRIES; retry++) {
        if (!UART_SendPacket(cmd, payload, payloadLen)) {
            DelayMs(5);
            continue;
        }
        
        status = UART_ReceiveResponse(outData, outDataLen);
        if (status != STATUS_UNKNOWN_CMD) {
            consecutiveFailures = 0;
            return status;
        }
        
        UART_FlushRx();
        DelayMs(20);
    }
    
    consecutiveFailures++;
    if (consecutiveFailures >= 2) {
        UART_Reinit();
        consecutiveFailures = 0;
    }
    
    return STATUS_UNKNOWN_CMD;
}

/* Initialize UART1 (PB0=Rx, PB1=Tx) - 115200 baud @ 16MHz */
void UART_Init(void)
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
    
    UART_FlushRx();
    consecutiveFailures = 0;
}

/* CMD 0x01: Initialize Password (Signup) */
uint8_t UART_InitPassword(const char *password)
{
    uint8_t payload[PASSWORD_LENGTH];
    for (uint8_t i = 0; i < PASSWORD_LENGTH; i++) {
        payload[i] = (uint8_t)password[i];
    }
    return UART_SendCommandWithRetry(CMD_INIT_PASSWORD, payload, PASSWORD_LENGTH, NULL, NULL);
}

/* CMD 0x02: Authenticate (mode: 0=check only, 1=open door) */
uint8_t UART_Authenticate(const char *password, uint8_t mode)
{
    uint8_t payload[1 + PASSWORD_LENGTH];
    payload[0] = mode;
    for (uint8_t i = 0; i < PASSWORD_LENGTH; i++) {
        payload[1 + i] = (uint8_t)password[i];
    }
    return UART_SendCommandWithRetry(CMD_AUTH, payload, 1 + PASSWORD_LENGTH, NULL, NULL);
}

/* CMD 0x03: Set Timeout (5-30 seconds) */
uint8_t UART_SetTimeout(uint8_t seconds)
{
    if (seconds < 5) seconds = 5;
    if (seconds > 30) seconds = 30;
    uint8_t payload[1] = {seconds};
    return UART_SendCommandWithRetry(CMD_SET_TIMEOUT, payload, 1, NULL, NULL);
}

/* CMD 0x04: Change Password */
uint8_t UART_ChangePassword(const char *newPassword)
{
    uint8_t payload[PASSWORD_LENGTH];
    for (uint8_t i = 0; i < PASSWORD_LENGTH; i++) {
        payload[i] = (uint8_t)newPassword[i];
    }
    return UART_SendCommandWithRetry(CMD_CHANGE_PASSWORD, payload, PASSWORD_LENGTH, NULL, NULL);
}

/* CMD 0x05: Get Timeout */
uint8_t UART_GetTimeout(uint8_t *outTimeout)
{
    uint8_t data[4];
    uint8_t dataLen = 0;
    uint8_t status = UART_SendCommandWithRetry(CMD_GET_TIMEOUT, NULL, 0, data, &dataLen);
    if (status == STATUS_OK && dataLen >= 1 && outTimeout != NULL) {
        *outTimeout = data[0];
    }
    return status;
}

/* CMD 0x06: Close Door */
uint8_t UART_CloseDoor(void)
{
    return UART_SendCommandWithRetry(CMD_CLOSE_DOOR, NULL, 0, NULL, NULL);
}

/* CMD 0x07: Open Door */
uint8_t UART_OpenDoor(void)
{
    return UART_SendCommandWithRetry(CMD_OPEN_DOOR, NULL, 0, NULL, NULL);
}
