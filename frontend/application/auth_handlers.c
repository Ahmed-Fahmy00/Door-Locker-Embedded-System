/******************************************************************************
 * File: auth_handlers.c
 * Module: Auth Handlers (Application Layer)
 * Description: Authentication-related state handlers
 ******************************************************************************/

#include "auth_handlers.h"
#include "ui_display.h"
#include "input_handler.h"
#include "uart_commands.h"
#include "../HAL/lcd.h"
#include "../HAL/led.h"
#include "../MCAL/systick.h"
#include <stdio.h>

/* Local buffers for password entry */
static char passwordBuffer[PASSWORD_LENGTH + 1];
static char confirmBuffer[PASSWORD_LENGTH + 1];

void handleSignup(Frontend_State_t *currentState, bool *isFirstTime)
{
    bool done = false;
    
    while (!done) {
        showMessage("Create Password:", "");
        LCD_SetCursor(1, 0);
        if (!getPasswordFromKeypad(passwordBuffer)) continue;
        
        DelayMs(300);
        
        showMessage("Confirm Password", "");
        LCD_SetCursor(1, 0);
        if (!getPasswordFromKeypad(confirmBuffer)) continue;
        
        if (stringsMatch(passwordBuffer, confirmBuffer, PASSWORD_LENGTH)) {
            showMessage("Saving...", "");
            
            uint8_t status = UART_InitPassword(passwordBuffer);
            
            if (status == STATUS_OK) {
                LED_Green();
                showMessage("Password Saved!", "");
                DelayMs(1500);
                LED_Off();
                *isFirstTime = false;
                done = true;
                *currentState = STATE_MAIN_MENU;
            } else {
                LED_Red();
                showMessage("Save Failed!", "Try Again");
                DelayMs(1500);
                LED_Off();
            }
        } else {
            LED_Red();
            showMessage("Mismatch!", "Try Again");
            DelayMs(1500);
            LED_Off();
        }
    }
}

void handleSignin(Frontend_State_t *currentState, uint8_t *attemptCount)
{
    char buffer[17];
    
    showMessage("Enter Password:", "");
    LCD_SetCursor(1, 0);
    
    if (!getPasswordFromKeypad(passwordBuffer)) {
        *currentState = STATE_MAIN_MENU;
        return;
    }
    
    showMessage("Verifying...", "");
    
    uint8_t timeout = 0;
    uint8_t status = UART_Authenticate(passwordBuffer, AUTH_MODE_OPEN_DOOR, &timeout);
    
    if (status == STATUS_OK) {
        *attemptCount = 0;
        LED_Green();

        /* Use received timeout for countdown (default to 10 if invalid) */
        if (timeout < 5 || timeout > 30) timeout = 10;
        
        /* Show countdown while door is open */
        showMessage("Door Open", "");
        while (timeout > 0) {
            LCD_SetCursor(1, 0);
            snprintf(buffer, sizeof(buffer), "Closing in: %2d s", timeout);
            LCD_WriteString(buffer);
            DelayMs(1000);
            timeout--;
        }
        
        /* Door closing */
        LED_Off();
        showMessage("Door Closing...", "Please Wait");
        DelayMs(2000);
        
        LED_Off();
        showMessage("Door Locked", "");
        DelayMs(1500);
        *currentState = STATE_MAIN_MENU;
    } 
    else if (status == STATUS_UNKNOWN_CMD) {
        LED_Blink(LED_RED, 3, 200);
        showMessage("Comm Error!", "Try Again");
        DelayMs(1500);
        LED_Off();
        *currentState = STATE_MAIN_MENU;
    }
    else {
        (*attemptCount)++;
        LED_Red();
        
        if (*attemptCount >= MAX_ATTEMPTS) {
            showMessage("Too many tries!", "Locking out...");
            DelayMs(1500);
            *currentState = STATE_LOCKOUT;
        } else {
            char buffer[17];
            snprintf(buffer, sizeof(buffer), "%d tries left", MAX_ATTEMPTS - *attemptCount);
            showMessage("Wrong Password!", buffer);
            DelayMs(1500);
            LED_Off();
            *currentState = STATE_MAIN_MENU;
        }
    }
}


void handleChangePassword(Frontend_State_t *currentState, uint8_t *attemptCount)
{
    showMessage("Old Password:", "");
    LCD_SetCursor(1, 0);
    
    if (!getPasswordFromKeypad(passwordBuffer)) {
        LED_Off();
        *currentState = STATE_MAIN_MENU;
        return;
    }
    
    showMessage("Verifying...", "");
    
    uint8_t status = UART_Authenticate(passwordBuffer, AUTH_MODE_CHECK_ONLY, NULL);
    
    if (status == STATUS_UNKNOWN_CMD) {
        LED_Blink(LED_RED, 3, 200);
        showMessage("Comm Error!", "Try Again");
        DelayMs(1500);
        LED_Off();
        *currentState = STATE_MAIN_MENU;
        return;
    }
    
    if (status != STATUS_OK) {
        (*attemptCount)++;
        LED_Red();
        if (*attemptCount >= MAX_ATTEMPTS) {
            *currentState = STATE_LOCKOUT;
            return;
        }
        showMessage("Wrong Password!", "");
        DelayMs(1500);
        LED_Off();
        *currentState = STATE_MAIN_MENU;
        return;
    }
    
    *attemptCount = 0;
    DelayMs(300);
    
    showMessage("New Password:", "");
    LCD_SetCursor(1, 0);
    if (!getPasswordFromKeypad(passwordBuffer)) {
        LED_Off();
        *currentState = STATE_MAIN_MENU;
        return;
    }
    
    DelayMs(300);
    
    showMessage("Confirm New Pwd:", "");
    LCD_SetCursor(1, 0);
    if (!getPasswordFromKeypad(confirmBuffer)) {
        LED_Off();
        *currentState = STATE_MAIN_MENU;
        return;
    }
    
    if (!stringsMatch(passwordBuffer, confirmBuffer, PASSWORD_LENGTH)) {
        LED_Red();
        showMessage("Mismatch!", "Not Changed");
        DelayMs(1500);
        LED_Off();
        *currentState = STATE_MAIN_MENU;
        return;
    }
    
    showMessage("Saving...", "");
    
    status = UART_ChangePassword(passwordBuffer);
    
    if (status == STATUS_OK) {
        LED_Green();
        showMessage("Password Changed", "");
    } else {
        LED_Red();
        showMessage("Change Failed!", "");
    }
    DelayMs(1500);
    LED_Off();
    *currentState = STATE_MAIN_MENU;
}

void handleLockout(Frontend_State_t *currentState, uint8_t *attemptCount)
{
    char buffer[17];
    uint8_t lockoutTime = 0;
    uint8_t status;
    uint8_t retryCount = 0;
    
    LED_Red();
    showMessage("!! LOCKED OUT !!", "Buzzer Active");
    DelayMs(300);
    
    while (retryCount < 5) {
        status = UART_GetTimeout(&lockoutTime);
        if (status == STATUS_OK && lockoutTime >= 5 && lockoutTime <= 30) break;
        retryCount++;
        snprintf(buffer, sizeof(buffer), "Retry %d/5...", retryCount);
        LCD_SetCursor(1, 0);
        LCD_WriteString(buffer);
        DelayMs(300);
    }
    
    while (lockoutTime < 5 || lockoutTime > 30) {
        showMessage("!! LOCKED OUT !!", "Getting time...");
        DelayMs(500);
        status = UART_GetTimeout(&lockoutTime);
        if (status == STATUS_OK && lockoutTime >= 5 && lockoutTime <= 30) break;
    }
    
    showMessage("!! LOCKED OUT !!", "");
    
    while (lockoutTime > 0) {
        LCD_SetCursor(1, 0);
        snprintf(buffer, sizeof(buffer), "Wait: %2d seconds", lockoutTime);
        LCD_WriteString(buffer);
        LED_Red();
        DelayMs(500);
        LED_Off();
        DelayMs(500);
        lockoutTime--;
    }
    
    *attemptCount = 0;
    LED_Green();
    showMessage("Lockout Over", "");
    DelayMs(1500);
    LED_Off();
    *currentState = STATE_MAIN_MENU;
}
