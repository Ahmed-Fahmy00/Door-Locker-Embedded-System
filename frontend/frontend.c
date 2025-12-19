/* Frontend Application Logic for Door Locker Security System */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "frontend.h"
#include "uart_comm.h"
#include "HAL/lcd.h"
#include "HAL/keypad.h"
#include "HAL/potentiometer.h"
#include "HAL/led.h"
#include "MCAL/systick.h"

static Frontend_State_t currentState = STATE_WELCOME;
static char passwordBuffer[PASSWORD_LENGTH];
static char confirmBuffer[PASSWORD_LENGTH];
static uint8_t attemptCount = 0;
static bool isFirstTime = true;

/* Helper: Show message on LCD */
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

/* Helper: Wait for any key press */
static char waitForKey(void)
{
    char key = 0;
    while (key == 0) {
        key = Keypad_GetKey();
    }
    return key;
}

/* Helper: Get 5-digit password from keypad */
static bool getPasswordFromKeypad(char *buffer)
{
    uint8_t i = 0;
    char key;
    
    while (i < PASSWORD_LENGTH) {
        key = Keypad_GetKey();
        
        if (key == '#') {
            if (i > 0) {
                i--;
                LCD_SetCursor(1, i);
                LCD_WriteChar(' ');
                LCD_SetCursor(1, i);
            } else {
                return false;
            }
        }
        else if (key >= '0' && key <= '9') {
            buffer[i] = key;
            LCD_WriteChar('*');
            i++;
        }
    }
    buffer[PASSWORD_LENGTH] = '\0';
    return true;
}

/* Helper: Compare two strings */
static bool stringsMatch(const char *s1, const char *s2, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        if (s1[i] != s2[i]) return false;
    }
    return true;
}

/* STATE: Welcome Screen */
static void handleWelcome(void)
{
    LED_Blue();
    showMessage("Door Locker", "Security System");
    DelayMs(2000);
    LED_Off();
    
    if (isFirstTime) {
        currentState = STATE_SIGNUP;
    } else {
        currentState = STATE_MAIN_MENU;
    }
}

/* STATE: Signup - Create password */
static void handleSignup(void)
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
            LED_Yellow();
            showMessage("Saving...", "");
            
            uint8_t status = UART_InitPassword(passwordBuffer);
            
            if (status == STATUS_OK) {
                LED_Green();
                showMessage("Password Saved!", "");
                DelayMs(1500);
                LED_Off();
                isFirstTime = false;
                done = true;
                currentState = STATE_MAIN_MENU;
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

/* STATE: Main Menu */
static void handleMainMenu(void)
{
    char key;
    showMessage("A:Sign B:ChgPwd", "C:Time #:Cancel");
    
    while (1) {
        key = waitForKey();
        
        switch (key) {
            case 'A': currentState = STATE_SIGNIN; return;
            case 'B': currentState = STATE_CHANGE_PASSWORD; return;
            case 'C': currentState = STATE_SET_TIMEOUT; return;
            case '#': showMessage("A:Sign B:ChgPwd", "C:Time #:Cancel"); break;
            default: break;
        }
    }
}

/* STATE: Signin - Authenticate and open door */
static void handleSignin(void)
{
    showMessage("Enter Password:", "");
    LCD_SetCursor(1, 0);
    
    if (!getPasswordFromKeypad(passwordBuffer)) {
        currentState = STATE_MAIN_MENU;
        return;
    }
    
    LED_Yellow();
    showMessage("Verifying...", "");
    
    uint8_t timeout = 0;
    uint8_t status = UART_Authenticate(passwordBuffer, AUTH_MODE_OPEN_DOOR, &timeout);
    
    if (status == STATUS_OK) {
        attemptCount = 0;
        LED_Green();
        showMessage("Access Granted!", "Door Opening...");
        DelayMs(2000);
        LED_Off();
        currentState = STATE_MAIN_MENU;
    } 
    else if (status == STATUS_UNKNOWN_CMD) {
        LED_Blink(LED_YELLOW, 3, 200);
        showMessage("Comm Error!", "Try Again");
        DelayMs(1500);
        LED_Off();
        currentState = STATE_MAIN_MENU;
    }
    else {
        attemptCount++;
        LED_Red();
        
        if (attemptCount >= MAX_ATTEMPTS) {
            showMessage("Too many tries!", "Locking out...");
            DelayMs(1500);
            currentState = STATE_LOCKOUT;
        } else {
            char buffer[17];
            snprintf(buffer, sizeof(buffer), "%d tries left", MAX_ATTEMPTS - attemptCount);
            showMessage("Wrong Password!", buffer);
            DelayMs(1500);
            LED_Off();
            currentState = STATE_MAIN_MENU;
        }
    }
}

/* STATE: Change Password */
static void handleChangePassword(void)
{
    LED_Blue();
    showMessage("Old Password:", "");
    LCD_SetCursor(1, 0);
    
    if (!getPasswordFromKeypad(passwordBuffer)) {
        LED_Off();
        currentState = STATE_MAIN_MENU;
        return;
    }
    
    LED_Yellow();
    showMessage("Verifying...", "");
    
    uint8_t status = UART_Authenticate(passwordBuffer, AUTH_MODE_CHECK_ONLY, NULL);
    
    if (status == STATUS_UNKNOWN_CMD) {
        LED_Blink(LED_YELLOW, 3, 200);
        showMessage("Comm Error!", "Try Again");
        DelayMs(1500);
        LED_Off();
        currentState = STATE_MAIN_MENU;
        return;
    }
    
    if (status != STATUS_OK) {
        attemptCount++;
        LED_Red();
        if (attemptCount >= MAX_ATTEMPTS) {
            currentState = STATE_LOCKOUT;
            return;
        }
        showMessage("Wrong Password!", "");
        DelayMs(1500);
        LED_Off();
        currentState = STATE_MAIN_MENU;
        return;
    }
    
    attemptCount = 0;
    LED_Blue();
    DelayMs(300);
    
    showMessage("New Password:", "");
    LCD_SetCursor(1, 0);
    if (!getPasswordFromKeypad(passwordBuffer)) {
        LED_Off();
        currentState = STATE_MAIN_MENU;
        return;
    }
    
    DelayMs(300);
    
    showMessage("Confirm New Pwd:", "");
    LCD_SetCursor(1, 0);
    if (!getPasswordFromKeypad(confirmBuffer)) {
        LED_Off();
        currentState = STATE_MAIN_MENU;
        return;
    }
    
    if (!stringsMatch(passwordBuffer, confirmBuffer, PASSWORD_LENGTH)) {
        LED_Red();
        showMessage("Mismatch!", "Not Changed");
        DelayMs(1500);
        LED_Off();
        currentState = STATE_MAIN_MENU;
        return;
    }
    
    LED_Yellow();
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
    currentState = STATE_MAIN_MENU;
}

/* STATE: Set Timeout */
static void handleSetTimeout(void)
{
    char buffer[17];
    char key = 0;
    uint32_t newTimeout;
    
    LED_Cyan();
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
            currentState = STATE_MAIN_MENU;
            return;
        }
        
        LED_Yellow();
        showMessage("Verifying...", "");
        
        uint8_t status = UART_Authenticate(passwordBuffer, AUTH_MODE_CHECK_ONLY, NULL);
        
        if (status == STATUS_UNKNOWN_CMD) {
            LED_Blink(LED_YELLOW, 3, 200);
            showMessage("Comm Error!", "Try Again");
            DelayMs(1500);
            LED_Off();
            currentState = STATE_MAIN_MENU;
            return;
        }
        
        if (status == STATUS_OK) {
            attemptCount = 0;
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
            attemptCount++;
            LED_Red();
            if (attemptCount >= MAX_ATTEMPTS) {
                currentState = STATE_LOCKOUT;
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
    
    currentState = STATE_MAIN_MENU;
}

/* STATE: Lockout */
static void handleLockout(void)
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
    
    attemptCount = 0;
    LED_Green();
    showMessage("Lockout Over", "");
    DelayMs(1500);
    LED_Off();
    currentState = STATE_MAIN_MENU;
}

/* Main Entry Point */
void Frontend_Start(void)
{
    LCD_Init();
    Keypad_Init();
    Potentiometer_Init();
    UART_Init();
    LED_Init();
    
    currentState = STATE_WELCOME;
    attemptCount = 0;
    isFirstTime = true;
    
    while (1) {
        switch (currentState) {
            case STATE_WELCOME:         handleWelcome(); break;
            case STATE_SIGNUP:          handleSignup(); break;
            case STATE_MAIN_MENU:       handleMainMenu(); break;
            case STATE_SIGNIN:          handleSignin(); break;
            case STATE_CHANGE_PASSWORD: handleChangePassword(); break;
            case STATE_SET_TIMEOUT:     handleSetTimeout(); break;
            case STATE_LOCKOUT:         handleLockout(); break;
            default:                    currentState = STATE_MAIN_MENU; break;
        }
    }
}
