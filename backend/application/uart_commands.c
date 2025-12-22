/******************************************************************************
 * File: uart_commands.c
 * Module: UART Commands (Application Layer)
 * Description: Command handlers for Backend UART protocol
 ******************************************************************************/

#include "uart_commands.h"
#include "uart_protocol.h"
#include "eeprom_handler.h"
#include "buzzer_service.h"
#include "door_controller.h"
#include <stddef.h>

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
 * Command Handlers
 *===========================================================================*/

 /* CMD 0x01: Initialize Password */
void CMD_InitPassword(uint8_t *buf, uint8_t len)
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
    
    UART_Protocol_SendResponse(CMD_INIT_PASSWORD, status, NULL, 0);
}

/* CMD 0x02: Authenticate */
void CMD_Auth(uint8_t *buf, uint8_t len)
{
    uint8_t status = UART_STATUS_ERROR;
    
    if (len == 7)  /* CMD(1) + MODE(1) + 5 digits */
    {
        uint8_t mode = buf[1];
        uint32_t pw = ascii_to_u32(&buf[2], 5);
        
        int result = authenticate(pw);
        
        if (result == STATUS_OK)
        {
            status = UART_STATUS_OK;
            
            /* If mode=1, get timeout and open door */
            if (mode == 0x01)
            {
                uint32_t timeout;
                if (get_auto_timeout(&timeout) == STATUS_OK)
                {
                    uint8_t timeout_val = (uint8_t)timeout;
                    /* Send timeout value to frontend */
                    UART_Protocol_SendResponse(CMD_AUTH, status, &timeout_val, 1);
                    /* Open door with this duration */
                    DoorController_OpenDoor(timeout);
                    return;
                }
            }
        }
        else
        {
            status = UART_STATUS_AUTH_FAIL;
        }
    }
    
    UART_Protocol_SendResponse(CMD_AUTH, status, NULL, 0);
}

/* CMD 0x03: Set Timeout */
void CMD_SetTimeout(uint8_t *buf, uint8_t len)
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
    
    UART_Protocol_SendResponse(CMD_SET_TIMEOUT, status, NULL, 0);
}

/* CMD 0x04: Change Password */
void CMD_ChangePassword(uint8_t *buf, uint8_t len)
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
    
    UART_Protocol_SendResponse(CMD_CHANGE_PASSWORD, status, NULL, 0);
}

/* CMD 0x05: Get Timeout 
 * Returns timeout value and activates buzzer for lockout mode.
 */
void CMD_GetTimeout(uint8_t *buf, uint8_t len)
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
            
            /* Activate buzzer for lockout */
            BuzzerService_Activate(timeout);
        }
    }
    
    UART_Protocol_SendResponse(CMD_GET_TIMEOUT, status, &timeout_val, 
                               (status == UART_STATUS_OK) ? 1 : 0);
}
