/*****************************************************************************
 * File: frontend.c
 * Description: Frontend Application Logic for Door Locker Security System
 * HMI_ECU - Handles LCD, Keypad, Potentiometer, User Interaction
 * 
 * Flow:
 *   Welcome -> Signup (first time) -> Main Menu
 *   Main Menu: A=Signin, B=Change Password, C=Set Timeout, #=Cancel
 *****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "frontend.h"
#include "uart_comm.h"
#include "components/lcd.h"
#include "components/keypad.h"
#include "components/potentiometer.h"
#include "MCAL/systick.h"

/* Current state */
static Frontend_State_t currentState = STATE_WELCOME;

/* Password buffers */
static char passwordBuffer[PASSWORD_LENGTH + 1];
static char confirmBuffer[PASSWORD_LENGTH + 1];

/* Attempt counter */
static uint8_t attemptCount = 0;

/*****************************************************************************
 * Helper Functions
 *****************************************************************************/
static void showMessage(const char *line1, const char *line2)
{
    LCD_Clear();
    LCD_SetCursor(0, 0);
    if (line1) LCD_WriteString(line1);
    if (line2) {
        LCD_SetCursor(1, 0);
        LCD_WriteString(line2);
    }
}

static char waitForKey(void)
{
    char key = 0;
    while (key == 0) {
        key = Keypad_GetKey();
    }
    return key;
}

/* Returns: true if password entered, false if cancelled (back) */
static bool getPasswordFromKeypad(char *buffer)
{
    uint8_t i = 0;
    char key;
    
    while (i < PASSWORD_LENGTH) {
        key = Keypad_GetKey();
        
        if (key == '#') {
            if (i > 0) {
                /* Delete last character */
                i--;
                LCD_SetCursor(1, i);
                LCD_WriteChar(' ');
                LCD_SetCursor(1, i);
            } else {
                /* Empty - go back */
                return false;
            }
        }
        else if (key >= '0' && key <= '9') {
            buffer[i] = key;
            LCD_WriteChar('*');
            i++;
        }
        /* Ignore other keys */
    }
    buffer[PASSWORD_LENGTH] = '\0';
    return true;
}

static bool stringsMatch(const char *s1, const char *s2, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        if (s1[i] != s2[i]) return false;
    }
    return true;
}

/*****************************************************************************
 * STATE: Welcome Screen
 *****************************************************************************/
static void handleWelcome(void)
{
    showMessage("Door Locker", "Security System");
    DelayMs(2000);
    
    /* Always go to signup first time, then main menu */
    /* Check with backend if password exists */
    if (UART_IsFirstTime()) {
        currentState = STATE_SIGNUP;
    } else {
        currentState = STATE_MAIN_MENU;
    }
}

/*****************************************************************************
 * STATE: Signup (First Time Password Setup)
 * Note: Cannot go back from signup - must create password
 *****************************************************************************/
static void handleSignup(void)
{
    bool done = false;
    
    while (!done) {
        /* Enter password */
        showMessage("Create Password:", "");
        LCD_SetCursor(1, 0);
        if (!getPasswordFromKeypad(passwordBuffer)) {
            /* Can't go back from signup, just restart */
            continue;
        }
        
        DelayMs(300);
        
        /* Confirm password */
        showMessage("Confirm Password", "");
        LCD_SetCursor(1, 0);
        if (!getPasswordFromKeypad(confirmBuffer)) {
            /* Go back to enter password */
            continue;
        }
        
        /* Check match */
        if (stringsMatch(passwordBuffer, confirmBuffer, PASSWORD_LENGTH)) {
            showMessage("Saving...", "");
            
            if (UART_SavePassword(passwordBuffer)) {
                showMessage("Password Saved!", "");
                DelayMs(1500);
                done = true;
                currentState = STATE_MAIN_MENU;
            } else {
                showMessage("Save Failed!", "Try Again");
                DelayMs(1500);
            }
        } else {
            showMessage("Mismatch!", "Try Again");
            DelayMs(1500);
        }
    }
}

/*****************************************************************************
 * STATE: Main Menu - Wait for A, B, C, or #
 *****************************************************************************/
static void handleMainMenu(void)
{
    char key;
    
    showMessage("A:Sign B:ChgPwd", "C:Time #:Cancel");
    
    /* Wait for valid menu key */
    while (1) {
        key = waitForKey();
        
        switch (key) {
            case 'A':
                currentState = STATE_SIGNIN;
                return;
            case 'B':
                currentState = STATE_CHANGE_PASSWORD;
                return;
            case 'C':
                currentState = STATE_SET_TIMEOUT;
                return;
            case '#':
                /* Cancel - just refresh menu */
                showMessage("A:Sign B:ChgPwd", "C:Time #:Cancel");
                break;
            default:
                /* Ignore other keys, stay in menu */
                break;
        }
    }
}

/*****************************************************************************
 * STATE: Signin (Password Verification) - Up to 3 attempts
 *****************************************************************************/
static void handleSignin(void)
{
    showMessage("Enter Password:", "");
    LCD_SetCursor(1, 0);
    
    if (!getPasswordFromKeypad(passwordBuffer)) {
        /* User pressed # when empty - go back to menu */
        currentState = STATE_MAIN_MENU;
        return;
    }
    
    showMessage("Verifying...", "");
    uint8_t response = UART_VerifyPassword(passwordBuffer);
    
    if (response == RESP_PASSWORD_CORRECT) {
        attemptCount = 0;
        showMessage("Access Granted!", "Door Opening...");
        currentState = STATE_DOOR_OPENING;
    } 
    else if (response == RESP_LOCKOUT) {
        attemptCount = 0;
        currentState = STATE_LOCKOUT;
    }
    else {
        /* RESP_PASSWORD_WRONG */
        attemptCount++;
        
        if (attemptCount >= MAX_ATTEMPTS) {
            /* 4th wrong attempt triggers lockout */
            currentState = STATE_LOCKOUT;
        } else {
            char buffer[17];
            snprintf(buffer, sizeof(buffer), "%d tries left", MAX_ATTEMPTS - attemptCount);
            showMessage("Wrong Password!", buffer);
            DelayMs(1500);
            currentState = STATE_MAIN_MENU;
        }
    }
}

/*****************************************************************************
 * STATE: Change Password
 *****************************************************************************/
static void handleChangePassword(void)
{
    /* Enter old password */
    showMessage("Old Password:", "");
    LCD_SetCursor(1, 0);
    
    if (!getPasswordFromKeypad(passwordBuffer)) {
        /* Go back to menu */
        currentState = STATE_MAIN_MENU;
        return;
    }
    
    DelayMs(300);
    
    /* Enter new password */
    showMessage("New Password:", "");
    LCD_SetCursor(1, 0);
    
    if (!getPasswordFromKeypad(confirmBuffer)) {
        /* Go back to menu */
        currentState = STATE_MAIN_MENU;
        return;
    }
    
    showMessage("Changing...", "");
    uint8_t response = UART_ChangePassword(passwordBuffer, confirmBuffer);
    
    if (response == RESP_OK) {
        showMessage("Password Changed", "");
        attemptCount = 0;
        DelayMs(1500);
    }
    else if (response == RESP_LOCKOUT) {
        currentState = STATE_LOCKOUT;
        return;
    }
    else {
        attemptCount++;
        if (attemptCount >= MAX_ATTEMPTS) {
            currentState = STATE_LOCKOUT;
            return;
        }
        showMessage("Wrong Old Pass!", "");
        DelayMs(1500);
    }
    
    currentState = STATE_MAIN_MENU;
}

/*****************************************************************************
 * STATE: Set Timeout (using Potentiometer)
 * D = Save, # = Cancel
 *****************************************************************************/
static void handleSetTimeout(void)
{
    char buffer[17];
    char key = 0;
    uint32_t timeout;
    
    showMessage("Adjust Timeout", "D:Save #:Cancel");
    DelayMs(1000);
    
    /* Live display potentiometer value */
    while (key != 'D' && key != '#') {
        timeout = GetScaledTimeout();
        
        LCD_SetCursor(1, 0);
        snprintf(buffer, sizeof(buffer), "Time: %2lu sec   ", timeout);
        LCD_WriteString(buffer);
        
        key = Keypad_GetKey();
        DelayMs(100);
    }
    
    if (key == 'D') {
        /* Save - verify password first */
        showMessage("Enter Password:", "");
        LCD_SetCursor(1, 0);
        
        if (!getPasswordFromKeypad(passwordBuffer)) {
            /* Go back to menu */
            currentState = STATE_MAIN_MENU;
            return;
        }
        
        uint8_t response = UART_VerifyPassword(passwordBuffer);
        
        if (response == RESP_PASSWORD_CORRECT) {
            attemptCount = 0;
            if (UART_SaveTimeout((uint8_t)timeout)) {
                showMessage("Timeout Saved!", "");
            } else {
                showMessage("Save Failed!", "");
            }
            DelayMs(1500);
        }
        else if (response == RESP_LOCKOUT) {
            currentState = STATE_LOCKOUT;
            return;
        }
        else {
            attemptCount++;
            if (attemptCount >= MAX_ATTEMPTS) {
                currentState = STATE_LOCKOUT;
                return;
            }
            showMessage("Wrong Password!", "Not Saved");
            DelayMs(1500);
        }
    } else {
        showMessage("Cancelled", "");
        DelayMs(1000);
    }
    
    currentState = STATE_MAIN_MENU;
}

/*****************************************************************************
 * STATE: Door Opening (backend controls motor)
 *****************************************************************************/
static void handleDoorOpening(void)
{
    showMessage("Door Opening...", "Please Wait");
    DelayMs(3000);
    currentState = STATE_DOOR_OPEN;
}

/*****************************************************************************
 * STATE: Door Open (countdown)
 *****************************************************************************/
static void handleDoorOpen(void)
{
    char buffer[17];
    uint8_t timeout = UART_GetTimeout();
    
    if (timeout == 0) timeout = 15;  /* Default */
    
    showMessage("Door Open", "");
    
    while (timeout > 0) {
        LCD_SetCursor(1, 0);
        snprintf(buffer, sizeof(buffer), "Closing in: %2d s", timeout);
        LCD_WriteString(buffer);
        
        DelayMs(1000);
        timeout--;
    }
    
    currentState = STATE_DOOR_CLOSING;
}

/*****************************************************************************
 * STATE: Door Closing
 *****************************************************************************/
static void handleDoorClosing(void)
{
    showMessage("Door Closing...", "Please Wait");
    
    /* Wait for backend signal */
    UART_WaitDoorClosed();
    
    showMessage("Door Locked", "");
    DelayMs(1500);
    
    currentState = STATE_MAIN_MENU;
}

/*****************************************************************************
 * STATE: Lockout (buzzer activated by backend)
 *****************************************************************************/
static void handleLockout(void)
{
    char buffer[17];
    uint8_t remaining = UART_GetLockoutTime();
    
    if (remaining == 0) remaining = 30;
    
    showMessage("!! LOCKED OUT !!", "Buzzer Active");
    
    while (remaining > 0) {
        LCD_SetCursor(1, 0);
        snprintf(buffer, sizeof(buffer), "Wait: %2d seconds", remaining);
        LCD_WriteString(buffer);
        
        DelayMs(1000);
        remaining--;
    }
    
    attemptCount = 0;
    showMessage("Lockout Over", "");
    DelayMs(1500);
    
    currentState = STATE_MAIN_MENU;
}

/*****************************************************************************
 * Main Entry Point
 *****************************************************************************/
void Frontend_Start(void)
{
    /* Initialize peripherals */
    LCD_Init();
    Keypad_Init();
    ADC0_Init_PE3();
    UART_Init();
    
    currentState = STATE_WELCOME;
    attemptCount = 0;
    
    /* Main loop */
    while (1) {
        switch (currentState) {
            case STATE_WELCOME:
                handleWelcome();
                break;
            case STATE_SIGNUP:
                handleSignup();
                break;
            case STATE_MAIN_MENU:
                handleMainMenu();
                break;
            case STATE_SIGNIN:
                handleSignin();
                break;
            case STATE_CHANGE_PASSWORD:
                handleChangePassword();
                break;
            case STATE_SET_TIMEOUT:
                handleSetTimeout();
                break;
            case STATE_DOOR_OPENING:
                handleDoorOpening();
                break;
            case STATE_DOOR_OPEN:
                handleDoorOpen();
                break;
            case STATE_DOOR_CLOSING:
                handleDoorClosing();
                break;
            case STATE_LOCKOUT:
                handleLockout();
                break;
            default:
                currentState = STATE_MAIN_MENU;
                break;
        }
    }
}

Frontend_State_t Frontend_GetState(void)
{
    return currentState;
}
