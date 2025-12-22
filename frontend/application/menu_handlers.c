/******************************************************************************
 * File: menu_handlers.c
 * Module: Menu Handlers (Application Layer)
 * Description: Navigation and settings state handlers
 ******************************************************************************/

#include "menu_handlers.h"
#include "ui_display.h"
#include "input_handler.h"
#include "uart_commands.h"
#include "../HAL/lcd.h"
#include "../HAL/led.h"
#include "../HAL/keypad.h"
#include "../HAL/potentiometer.h"
#include "../MCAL/systick.h"
#include <stdio.h>

/* Local buffer for password entry */
static char passwordBuffer[PASSWORD_LENGTH + 1];

void handleWelcome(Frontend_State_t *currentState, bool *isFirstTime)
{
    LED_Blue();
    showMessage("Door Locker", "Security System");
    DelayMs(2000);
    LED_Off();
    
    if (*isFirstTime) {
        *currentState = STATE_SIGNUP;
    } else {
        *currentState = STATE_MAIN_MENU;
    }
}

void handleMainMenu(Frontend_State_t *currentState)
{
    char key;
    showMessage("A:Sign *:ChgPwd", "C:Time #:Cancel");
    
    while (1) {
        key = waitForKey();
        
        switch (key) {
            case 'A': *currentState = STATE_SIGNIN; return;
            case '*': *currentState = STATE_CHANGE_PASSWORD; return;
            case 'C': *currentState = STATE_SET_TIMEOUT; return;
            case '#': showMessage("A:Sign *:ChgPwd", "C:Time #:Cancel"); break;
            default: break;
        }
    }
}

void handleSetTimeout(Frontend_State_t *currentState, uint8_t *attemptCount)
{
    char buffer[17];
    char key = 0;
    uint32_t newTimeout;
    
    showMessage("Adjust Timeout", "D:Save #:Cancel");
    DelayMs(500);
    
    while (key != 'D' && key != '#') {
        newTimeout = Potentiometer_GetTimeout();
        LCD_SetCursor(1, 0);
        snprintf(buffer, sizeof(buffer), "Time: %2lu sec   ", newTimeout);
        LCD_WriteString(buffer);
        key = Keypad_GetKey();
        DelayMs(100);
    }
    
    if (key == 'D') {
        showMessage("Enter Password:", "");
        LCD_SetCursor(1, 0);
        
        if (!getPasswordFromKeypad(passwordBuffer)) {
            LED_Off();
            *currentState = STATE_MAIN_MENU;
            return;
        }
        
        showMessage("Verifying...", "");
        
        uint8_t status = UART_Authenticate(passwordBuffer, AUTH_MODE_CHECK_ONLY, NULL);
        
        if (status == STATUS_UNKNOWN_CMD) {
            showMessage("Comm Error!", "Try Again");
            DelayMs(1500);
            *currentState = STATE_MAIN_MENU;
            return;
        }
        
        if (status == STATUS_OK) {
            *attemptCount = 0;
            status = UART_SetTimeout((uint8_t)newTimeout);
            
            if (status == STATUS_OK) {
                LED_Green();
                showMessage("Timeout Saved!", "");
            } else {
                LED_Red();
                showMessage("Save Failed!", "Try Again");
            }
            DelayMs(1500);
            LED_Off();
        }
        else {
            (*attemptCount)++;
            LED_Red();
            if (*attemptCount >= MAX_ATTEMPTS) {
                *currentState = STATE_LOCKOUT;
                return;
            }
            showMessage("Wrong Password!", "Not Saved");
            DelayMs(1500);
            LED_Off();
        }
    } else {
        LED_Off();
        showMessage("Cancelled", "");
        DelayMs(1000);
    }

    *currentState = STATE_MAIN_MENU;
}
