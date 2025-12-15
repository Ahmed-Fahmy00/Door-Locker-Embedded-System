/*****************************************************************************
 * File: uart_comm.c
 * Description: UART Communication Layer for HMI_ECU (Frontend)
 * 
 * Protocol:
 *   Request:  [SOF=0x7E] [LEN] [CMD] [PAYLOAD...]
 *   Response: [SOF=0xFE] [LEN] [CMD] [STATUS] [DATA...]
 * 
 * Features:
 *   - Timeout protection on all receives
 *   - Automatic retry on communication failures
 *   - UART re-initialization on repeated failures
 *   - Robust error recovery
 *****************************************************************************/

#include "uart_comm.h"
#include "lib/tm4c123gh6pm.h"
#include <stddef.h>

/*===========================================================================
 * Constants
 *===========================================================================*/
#define UART_TIMEOUT_LOOPS   1000000   /* Timeout for receiving bytes */
#define UART_MAX_RETRIES     3         /* Retries per command */
#define UART_SOF_SEARCH_MAX  50        /* Max bytes to search for SOF */
#define UART_DELAY_SHORT     10000     /* Short delay loops */
#define UART_DELAY_LONG      100000    /* Longer delay for backend processing */

/*===========================================================================
 * Static variables
 *===========================================================================*/
static uint8_t consecutiveFailures = 0;

/*===========================================================================
 * Simple delay function
 *===========================================================================*/
static void UART_Delay(volatile uint32_t count)
{
    while (count > 0) {
        count--;
    }
}

/*===========================================================================
 * Re-initialize UART (call after communication failures)
 *===========================================================================*/
static void UART_Reinit(void)
{
    /* Disable UART */
    UART1_CTL_R &= ~0x01;
    
    /* Clear all errors */
    UART1_ECR_R = 0xFF;
    
    /* Small delay */
    UART_Delay(UART_DELAY_SHORT);
    
    /* Reconfigure baud rate (in case backend clock changed) */
    UART1_IBRD_R = 8;
    UART1_FBRD_R = 44;
    UART1_LCRH_R = 0x70;
    
    /* Re-enable UART */
    UART1_CTL_R = 0x301;
    
    /* Flush any garbage */
    while ((UART1_FR_R & 0x10) == 0) {
        (void)UART1_DR_R;
    }
    
    UART_Delay(UART_DELAY_SHORT);
}


/*===========================================================================
 * Flush RX FIFO - discard all pending received bytes
 *===========================================================================*/
static void UART_FlushRx(void)
{
    /* Only flush what's currently in the buffer, don't wait */
    while ((UART1_FR_R & 0x10) == 0) {
        (void)UART1_DR_R;
    }
    /* Clear any errors */
    UART1_ECR_R = 0xFF;
}

/*===========================================================================
 * Send a single byte (blocking with timeout)
 *===========================================================================*/
static uint8_t UART_SendByte(uint8_t data)
{
    uint32_t timeout = UART_TIMEOUT_LOOPS;
    
    while ((UART1_FR_R & 0x20) != 0) {  /* Wait while TX FIFO full */
        timeout--;
        if (timeout == 0) {
            return 0;  /* Timeout */
        }
    }
    UART1_DR_R = data;
    return 1;
}

/*===========================================================================
 * Wait for TX to complete
 *===========================================================================*/
static void UART_WaitTxComplete(void)
{
    uint32_t timeout = UART_TIMEOUT_LOOPS;
    /* Wait while UART is busy (BUSY flag = bit 3) */
    while ((UART1_FR_R & 0x08) != 0 && timeout > 0) {
        timeout--;
    }
    /* Also wait for TX FIFO to be empty (TXFE flag = bit 7) */
    timeout = UART_TIMEOUT_LOOPS;
    while ((UART1_FR_R & 0x80) == 0 && timeout > 0) {
        timeout--;
    }
}

/*===========================================================================
 * Receive a single byte with timeout
 * Returns: 1 on success, 0 on timeout/error
 *===========================================================================*/
static uint8_t UART_ReceiveByteTimeout(uint8_t *data)
{
    uint32_t timeout = UART_TIMEOUT_LOOPS;
    uint32_t dr;
    
    while ((UART1_FR_R & 0x10) != 0) {  /* While RX FIFO empty */
        timeout--;
        if (timeout == 0) {
            return 0;  /* Timeout */
        }
    }
    
    dr = UART1_DR_R;
    
    /* Check for errors (OE, BE, PE, FE) */
    if (dr & 0xF00) {
        UART1_ECR_R = 0xFF;  /* Clear errors */
        return 0;
    }
    
    *data = (uint8_t)(dr & 0xFF);
    return 1;
}

/*===========================================================================
 * Send a complete packet: [SOF=0x7E] [LEN] [CMD] [PAYLOAD...]
 *===========================================================================*/
static uint8_t UART_SendPacket(uint8_t cmd, const uint8_t *payload, uint8_t payloadLen)
{
    uint8_t len = 1 + payloadLen;
    uint8_t i;
    
    /* Flush stale RX data */
    UART_FlushRx();
    
    /* Send packet */
    if (!UART_SendByte(SOF_REQUEST)) return 0;
    if (!UART_SendByte(len)) return 0;
    if (!UART_SendByte(cmd)) return 0;
    
    for (i = 0; i < payloadLen; i++) {
        if (!UART_SendByte(payload[i])) return 0;
    }
    
    /* Wait for TX complete */
    UART_WaitTxComplete();
    
    /* Give backend time to process */
    UART_Delay(UART_DELAY_LONG);
    
    return 1;
}

/*===========================================================================
 * Receive response: [SOF=0xFE] [LEN] [CMD] [STATUS] [DATA...]
 * Returns status byte, or STATUS_UNKNOWN_CMD on error/timeout
 *===========================================================================*/
static uint8_t UART_ReceiveResponse(uint8_t *outData, uint8_t *outDataLen)
{
    uint8_t byte;
    uint8_t len;
    uint8_t cmd;
    uint8_t status;
    uint8_t i;
    uint8_t sofRetries = UART_SOF_SEARCH_MAX;
    
    /* Initialize output */
    if (outDataLen != NULL) {
        *outDataLen = 0;
    }
    
    /* Search for SOF = 0xFE */
    while (sofRetries > 0) {
        if (!UART_ReceiveByteTimeout(&byte)) {
            return STATUS_UNKNOWN_CMD;  /* Timeout */
        }
        if (byte == SOF_RESPONSE) {
            break;
        }
        sofRetries--;
    }
    
    if (sofRetries == 0) {
        return STATUS_UNKNOWN_CMD;  /* SOF not found */
    }
    
    /* Read LEN */
    if (!UART_ReceiveByteTimeout(&len)) {
        return STATUS_UNKNOWN_CMD;
    }
    
    /* Validate length */
    if (len < 2 || len > 32) {
        return STATUS_UNKNOWN_CMD;
    }
    
    /* Read CMD */
    if (!UART_ReceiveByteTimeout(&cmd)) {
        return STATUS_UNKNOWN_CMD;
    }
    
    /* Read STATUS */
    if (!UART_ReceiveByteTimeout(&status)) {
        return STATUS_UNKNOWN_CMD;
    }
    
    /* Read additional data bytes */
    uint8_t dataBytes = (len > 2) ? (len - 2) : 0;
    
    for (i = 0; i < dataBytes; i++) {
        if (!UART_ReceiveByteTimeout(&byte)) {
            break;
        }
        if (outData != NULL && outDataLen != NULL && i < 16) {
            outData[i] = byte;
            (*outDataLen)++;
        }
    }
    
    return status;
}

/*===========================================================================
 * Send command with automatic retry
 *===========================================================================*/
static uint8_t UART_SendCommandWithRetry(uint8_t cmd, const uint8_t *payload, 
                                          uint8_t payloadLen, uint8_t *outData, 
                                          uint8_t *outDataLen)
{
    uint8_t retry;
    uint8_t status;
    
    for (retry = 0; retry < UART_MAX_RETRIES; retry++) {
        /* Send packet */
        if (!UART_SendPacket(cmd, payload, payloadLen)) {
            UART_Delay(UART_DELAY_SHORT);
            continue;
        }
        
        /* Receive response */
        status = UART_ReceiveResponse(outData, outDataLen);
        
        /* Check if successful */
        if (status != STATUS_UNKNOWN_CMD) {
            consecutiveFailures = 0;
            return status;
        }
        
        /* Failed - prepare for retry */
        UART_FlushRx();
        UART_Delay(UART_DELAY_LONG);
    }
    
    /* All retries failed */
    consecutiveFailures++;
    
    /* Re-init UART after multiple consecutive failures */
    if (consecutiveFailures >= 2) {
        UART_Reinit();
        consecutiveFailures = 0;
    }
    
    return STATUS_UNKNOWN_CMD;
}


/*****************************************************************************
 * Initialize UART1 (PB0=Rx, PB1=Tx) - 115200 baud @ 16MHz
 *****************************************************************************/
void UART_Init(void)
{
    /* Enable UART1 and GPIO Port B clocks */
    SYSCTL_RCGCUART_R |= 0x02;
    SYSCTL_RCGCGPIO_R |= 0x02;
    
    /* Wait for peripherals to be ready */
    while ((SYSCTL_PRUART_R & 0x02) == 0) {}
    while ((SYSCTL_PRGPIO_R & 0x02) == 0) {}
    
    /* Configure PB0 and PB1 for UART */
    GPIO_PORTB_AFSEL_R |= 0x03;
    GPIO_PORTB_PCTL_R &= ~0x000000FF;
    GPIO_PORTB_PCTL_R |= 0x00000011;
    GPIO_PORTB_DEN_R |= 0x03;
    
    /* Disable UART before configuration */
    UART1_CTL_R &= ~0x01;
    
    /* Configure baud rate: 115200 @ 16MHz
     * BRD = 16,000,000 / (16 * 115200) = 8.6805
     * IBRD = 8
     * FBRD = int(0.6805 * 64 + 0.5) = 44
     */
    UART1_IBRD_R = 8;
    UART1_FBRD_R = 44;
    
    /* 8 data bits, 1 stop bit, no parity, FIFOs enabled */
    UART1_LCRH_R = 0x70;
    
    /* Clear any pending errors */
    UART1_ECR_R = 0xFF;
    
    /* Enable UART, TX, and RX */
    UART1_CTL_R = 0x301;
    
    /* Flush RX buffer */
    UART_FlushRx();
    
    /* Reset failure counter */
    consecutiveFailures = 0;
}

/*****************************************************************************
 * CMD 0x01: Initialize Password (Signup)
 *****************************************************************************/
uint8_t UART_InitPassword(const char *password)
{
    uint8_t payload[PASSWORD_LENGTH];
    uint8_t i;
    
    for (i = 0; i < PASSWORD_LENGTH; i++) {
        payload[i] = (uint8_t)password[i];
    }
    
    return UART_SendCommandWithRetry(CMD_INIT_PASSWORD, payload, PASSWORD_LENGTH, NULL, NULL);
}

/*****************************************************************************
 * CMD 0x02: Authenticate
 * mode: 0 = check only, 1 = open door
 * outTimeout: if mode=1 and auth OK, backend returns timeout value here
 *****************************************************************************/
uint8_t UART_Authenticate(const char *password, uint8_t mode)
{
    uint8_t payload[1 + PASSWORD_LENGTH];
    uint8_t i;
    
    payload[0] = mode;
    for (i = 0; i < PASSWORD_LENGTH; i++) {
        payload[1 + i] = (uint8_t)password[i];
    }
    
    return UART_SendCommandWithRetry(CMD_AUTH, payload, 1 + PASSWORD_LENGTH, NULL, NULL);
}

/*****************************************************************************
 * CMD 0x02: Authenticate with timeout return (for door open)
 * mode: 0 = check only, 1 = open door
 * outTimeout: if mode=1 and auth OK, backend returns timeout value here
 *****************************************************************************/
uint8_t UART_AuthenticateWithTimeout(const char *password, uint8_t mode, uint8_t *outTimeout)
{
    uint8_t payload[1 + PASSWORD_LENGTH];
    uint8_t data[4];
    uint8_t dataLen = 0;
    uint8_t status;
    uint8_t i;
    
    payload[0] = mode;
    for (i = 0; i < PASSWORD_LENGTH; i++) {
        payload[1 + i] = (uint8_t)password[i];
    }
    
    status = UART_SendCommandWithRetry(CMD_AUTH, payload, 1 + PASSWORD_LENGTH, data, &dataLen);
    
    /* If mode=1 and auth OK, backend sends timeout in response */
    if (status == STATUS_OK && mode == 1 && dataLen >= 1 && outTimeout != NULL) {
        *outTimeout = data[0];
    }
    
    return status;
}

/*****************************************************************************
 * CMD 0x03: Set Timeout
 *****************************************************************************/
uint8_t UART_SetTimeout(uint8_t seconds)
{
    uint8_t payload[1];
    
    /* Clamp to valid range */
    if (seconds < 5) seconds = 5;
    if (seconds > 30) seconds = 30;
    
    payload[0] = seconds;
    
    return UART_SendCommandWithRetry(CMD_SET_TIMEOUT, payload, 1, NULL, NULL);
}

/*****************************************************************************
 * CMD 0x04: Change Password
 *****************************************************************************/
uint8_t UART_ChangePassword(const char *newPassword)
{
    uint8_t payload[PASSWORD_LENGTH];
    uint8_t i;
    
    for (i = 0; i < PASSWORD_LENGTH; i++) {
        payload[i] = (uint8_t)newPassword[i];
    }
    
    return UART_SendCommandWithRetry(CMD_CHANGE_PASSWORD, payload, PASSWORD_LENGTH, NULL, NULL);
}

/*****************************************************************************
 * CMD 0x05: Get Timeout
 * Response: [0xFE] [3] [0x05] [STATUS] [TIMEOUT_VALUE]
 *****************************************************************************/
uint8_t UART_GetTimeout(uint8_t *outTimeout)
{
    uint8_t data[4];
    uint8_t dataLen = 0;
    uint8_t status;
    
    status = UART_SendCommandWithRetry(CMD_GET_TIMEOUT, NULL, 0, data, &dataLen);
    
    if (status == STATUS_OK && dataLen >= 1 && outTimeout != NULL) {
        *outTimeout = data[0];
    }
    
    return status;
}

/*****************************************************************************
 * CMD 0x06: Close Door (after timeout countdown)
 *****************************************************************************/
uint8_t UART_CloseDoor(void)
{
    return UART_SendCommandWithRetry(CMD_CLOSE_DOOR, NULL, 0, NULL, NULL);
}

/*****************************************************************************
 * CMD 0x07: Open Door (motor forward 3 sec)
 *****************************************************************************/
uint8_t UART_OpenDoor(void)
{
    return UART_SendCommandWithRetry(CMD_OPEN_DOOR, NULL, 0, NULL, NULL);
}
