/*****************************************************************************
 * File: frontend.c
 * Description: Frontend Application Logic for Door Locker Security System
 * HMI_ECU - Handles LCD, Keypad, Potentiometer, User Interaction
 *****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "frontend.h"
#include "uart_comm.h"
#include "components/lcd.h"
#include "components/keypad.h"
#include "components/potentiometer.h"
#include "components/led.h"
#include "MCAL/systick.h"

/* Current state */
static Frontend_State_t currentState = STATE_WELCOME;

/* Password buffers */
static char passwordBuffer[PASSWORD_LENGTH + 1];
static char confirmBuffer[PASSWORD_LENGTH + 1];

/* Attempt counter */
static uint8_t attemptCount = 0;

/* Door timeout - received from backend in AUTH response */
static uint8_t doorTimeout = 10;

/* First time flag */
static bool isFirstTime = true;

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
    LED_Blue();  /* Blue = System starting */
    showMessage("Door Locker", "Security System");
    DelayMs(2000);
    LED_Off();
    
    if (isFirstTime) {
        currentState = STATE_SIGNUP;
    } else {
        currentState = STATE_MAIN_MENU;
    }
}

/*****************************************************************************
 * STATE: Signup - Enter password twice, verify match, send CMD 0x01
 *****************************************************************************/
static void handleSignup(void)
{
    bool done = false;
    
    while (!done) {
        showMessage("Create Password:", "");
        LCD_SetCursor(1, 0);
        if (!getPasswordFromKeypad(passwordBuffer)) {
            continue;
        }
        
        DelayMs(300);
        
        showMessage("Confirm Password", "");
        LCD_SetCursor(1, 0);
        if (!getPasswordFromKeypad(confirmBuffer)) {
            continue;
        }
        
        if (stringsMatch(passwordBuffer, confirmBuffer, PASSWORD_LENGTH)) {
            LED_Yellow();  /* Yellow = Processing */
            showMessage("Saving...", "");
            
            uint8_t status = UART_InitPassword(passwordBuffer);
            
            if (status == STATUS_OK) {
                LED_Green();  /* Green = Success */
                showMessage("Password Saved!", "");
                DelayMs(1500);
                LED_Off();
                isFirstTime = false;
                done = true;
                currentState = STATE_MAIN_MENU;
            } else {
                LED_Red();  /* Red = Error */
                showMessage("Save Failed!", "Try Again");
                DelayMs(1500);
                LED_Off();
            }
        } else {
            LED_Red();  /* Red = Mismatch */
            showMessage("Mismatch!", "Try Again");
            DelayMs(1500);
            LED_Off();
        }
    }
}

/*****************************************************************************
 * STATE: Main Menu
 *****************************************************************************/
static void handleMainMenu(void)
{
    char key;
    
    showMessage("A:Sign B:ChgPwd", "C:Time #:Cancel");
    
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
                showMessage("A:Sign B:ChgPwd", "C:Time #:Cancel");
                break;
            default:
                break;
        }
    }
}


/*****************************************************************************
 * STATE: Signin - Send CMD 0x02 (MODE=1)
 *****************************************************************************/
static void handleSignin(void)
{
    showMessage("Enter Password:", "");
    LCD_SetCursor(1, 0);
    
    if (!getPasswordFromKeypad(passwordBuffer)) {
        currentState = STATE_MAIN_MENU;
        return;
    }
    
    LED_Yellow();  /* Yellow = Verifying */
    showMessage("Verifying...", "");
    
    uint8_t status = UART_Authenticate(passwordBuffer, AUTH_MODE_OPEN_DOOR);
    
    if (status == STATUS_OK) {
        attemptCount = 0;
        LED_Green();  /* Green = Access granted */
        showMessage("Access Granted!", "Door Opening...");
        DelayMs(1500);
        currentState = STATE_DOOR_OPEN;
    } 
    else if (status == STATUS_UNKNOWN_CMD) {
        /* Communication error - don't count as wrong password */
        LED_Blink(LED_YELLOW, 3, 200);  /* Yellow blink = Comm error */
        showMessage("Comm Error!", "Try Again");
        DelayMs(1500);
        LED_Off();
        currentState = STATE_MAIN_MENU;
    }
    else {
        /* Wrong password (STATUS_AUTH_FAIL or STATUS_ERROR) */
        attemptCount++;
        LED_Red();  /* Red = Wrong password */
        
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

/*****************************************************************************
 * STATE: Change Password - CMD 0x02 (MODE=0) then CMD 0x04
 *****************************************************************************/
static void handleChangePassword(void)
{
    LED_Blue();  /* Blue = Change password mode */
    showMessage("Old Password:", "");
    LCD_SetCursor(1, 0);
    
    if (!getPasswordFromKeypad(passwordBuffer)) {
        LED_Off();
        currentState = STATE_MAIN_MENU;
        return;
    }
    
    LED_Yellow();  /* Yellow = Verifying */
    showMessage("Verifying...", "");
    
    uint8_t status = UART_Authenticate(passwordBuffer, AUTH_MODE_CHECK_ONLY);
    
    if (status == STATUS_UNKNOWN_CMD) {
        /* Communication error */
        LED_Blink(LED_YELLOW, 3, 200);
        showMessage("Comm Error!", "Try Again");
        DelayMs(1500);
        LED_Off();
        currentState = STATE_MAIN_MENU;
        return;
    }
    
    if (status != STATUS_OK) {
        attemptCount++;
        LED_Red();  /* Red = Wrong password */
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
    LED_Blue();  /* Blue = Enter new password */
    DelayMs(300);
    
    /* Enter new password */
    showMessage("New Password:", "");
    LCD_SetCursor(1, 0);
    
    if (!getPasswordFromKeypad(passwordBuffer)) {
        LED_Off();
        currentState = STATE_MAIN_MENU;
        return;
    }
    
    DelayMs(300);
    
    /* Confirm new password */
    showMessage("Confirm New Pwd:", "");
    LCD_SetCursor(1, 0);
    
    if (!getPasswordFromKeypad(confirmBuffer)) {
        LED_Off();
        currentState = STATE_MAIN_MENU;
        return;
    }
    
    /* Check if passwords match */
    if (!stringsMatch(passwordBuffer, confirmBuffer, PASSWORD_LENGTH)) {
        LED_Red();  /* Red = Mismatch */
        showMessage("Mismatch!", "Not Changed");
        DelayMs(1500);
        LED_Off();
        currentState = STATE_MAIN_MENU;
        return;
    }
    
    LED_Yellow();  /* Yellow = Saving */
    showMessage("Saving...", "");
    
    status = UART_ChangePassword(passwordBuffer);
    
    if (status == STATUS_OK) {
        LED_Green();  /* Green = Success */
        showMessage("Password Changed", "");
    } else {
        LED_Red();  /* Red = Failed */
        showMessage("Change Failed!", "");
    }
    DelayMs(1500);
    LED_Off();
    
    currentState = STATE_MAIN_MENU;
}


/*****************************************************************************
 * STATE: Set Timeout - Adjust with potentiometer, verify password, CMD 0x03
 *****************************************************************************/
static void handleSetTimeout(void)
{
    char buffer[17];
    char key = 0;
    uint32_t newTimeout;
    
    LED_Cyan();  /* Cyan = Settings mode */
    showMessage("Adjust Timeout", "D:Save #:Cancel");
    DelayMs(500);
    
    while (key != 'D' && key != '#') {
        newTimeout = GetScaledTimeout();
        
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
        
        LED_Yellow();  /* Yellow = Verifying */
        showMessage("Verifying...", "");
        
        uint8_t status = UART_Authenticate(passwordBuffer, AUTH_MODE_CHECK_ONLY);
        
        if (status == STATUS_UNKNOWN_CMD) {
            /* Communication error */
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
                LED_Green();  /* Green = Success */
                showMessage("Timeout Saved!", "");
            } else {
                LED_Red();  /* Red = Failed */
                showMessage("Save Failed!", "Try Again");
            }
            DelayMs(1500);
            LED_Off();
        }
        else {
            attemptCount++;
            LED_Red();  /* Red = Wrong password */
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

/*****************************************************************************
 * STATE: Door Open - Get timeout, open door, then countdown
 * Flow: GET_TIMEOUT -> OPEN_DOOR (3 sec) -> countdown -> CLOSE_DOOR
 *****************************************************************************/
static void handleDoorOpen(void)
{
    char buffer[17];
    uint8_t remaining = 0;
    uint8_t status;
    uint8_t retryCount = 0;
    
    /* Step 1: Get timeout from backend */
    showMessage("Door Opening...", "Getting time...");
    
    while (retryCount < 5) {
        status = UART_GetTimeout(&remaining);
        
        if (status == STATUS_OK && remaining >= 5 && remaining <= 30) {
            break;
        }
        
        retryCount++;
        snprintf(buffer, sizeof(buffer), "Retry %d/5...", retryCount);
        LCD_SetCursor(1, 0);
        LCD_WriteString(buffer);
        DelayMs(300);
    }
    
    /* Default if failed */
    if (remaining < 5 || remaining > 30) {
        remaining = 10;
    }
    
    /* Step 2: Open door (motor forward 3 sec) */
    LED_Green();
    showMessage("Door Opening...", "Please Wait");
    
    UART_OpenDoor();  /* Backend responds immediately, then runs motor */
    
    /* Wait for motor to finish opening (3 seconds) */
    DelayMs(3500);
    
    /* Step 3: Door is now open - show countdown */
    showMessage("Door Open", "");
    
    while (remaining > 0) {
        LCD_SetCursor(1, 0);
        snprintf(buffer, sizeof(buffer), "Closing in: %2d s", remaining);
        LCD_WriteString(buffer);
        
        DelayMs(1000);
        remaining--;
    }
    
    currentState = STATE_DOOR_CLOSING;
}

/*****************************************************************************
 * STATE: Door Closing - Send CMD 0x06 to backend to close door
 *****************************************************************************/
static void handleDoorClosing(void)
{
    LED_Yellow();  /* Yellow = Door closing */
    showMessage("Door Closing...", "Please Wait");
    
    /* Send close door command to backend - backend will run motor reverse 3 sec */
    UART_CloseDoor();
    
    /* Wait for motor to finish (3 seconds) */
    DelayMs(3500);
    
    LED_Off();  /* Off = Door locked/idle */
    showMessage("Door Locked", "");
    DelayMs(1500);
    
    currentState = STATE_MAIN_MENU;
}

/*****************************************************************************
 * STATE: Lockout - Request timeout from backend for lockout duration
 *****************************************************************************/
static void handleLockout(void)
{
    char buffer[17];
    uint8_t lockoutTime = 0;
    uint8_t status;
    uint8_t retryCount = 0;
    
    LED_Red();  /* Red = Lockout/Alarm */
    showMessage("!! LOCKED OUT !!", "Buzzer Active");
    
    /* Small delay before UART request */
    DelayMs(300);
    
    /* Keep trying to get timeout from backend - NEVER use local fallback */
    while (retryCount < 5) {
        status = UART_GetTimeout(&lockoutTime);
        
        /* Check if we got a valid timeout */
        if (status == STATUS_OK && lockoutTime >= 5 && lockoutTime <= 30) {
            break;  /* Success! */
        }
        
        retryCount++;
        snprintf(buffer, sizeof(buffer), "Retry %d/5...", retryCount);
        LCD_SetCursor(1, 0);
        LCD_WriteString(buffer);
        DelayMs(300);
    }
    
    /* If still no valid timeout after retries, keep retrying indefinitely */
    while (lockoutTime < 5 || lockoutTime > 30) {
        showMessage("!! LOCKED OUT !!", "Getting time...");
        DelayMs(500);
        status = UART_GetTimeout(&lockoutTime);
        if (status == STATUS_OK && lockoutTime >= 5 && lockoutTime <= 30) {
            break;
        }
    }
    
    showMessage("!! LOCKED OUT !!", "");
    
    /* Blink red during lockout countdown */
    while (lockoutTime > 0) {
        LCD_SetCursor(1, 0);
        snprintf(buffer, sizeof(buffer), "Wait: %2d seconds", lockoutTime);
        LCD_WriteString(buffer);
        
        /* Blink red LED */
        LED_Red();
        DelayMs(500);
        LED_Off();
        DelayMs(500);
        lockoutTime--;
    }
    
    attemptCount = 0;
    LED_Green();  /* Green = Lockout over */
    showMessage("Lockout Over", "");
    DelayMs(1500);
    LED_Off();
    
    currentState = STATE_MAIN_MENU;
}


/*****************************************************************************
 * Main Entry Point
 *****************************************************************************/
void Frontend_Start(void)
{
    LCD_Init();
    Keypad_Init();
    ADC0_Init_PE3();
    UART_Init();
    LED_Init();
    
    currentState = STATE_WELCOME;
    attemptCount = 0;
    isFirstTime = true;
    
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
