/******************************************************************************
 * File: application.c
 * Module: Frontend Application
 * Description: Main frontend state machine for Door Locker Security System
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "application.h"
#include "uart_commands.h"
#include "../HAL/lcd.h"
#include "../HAL/keypad.h"
#include "../HAL/potentiometer.h"
#include "../HAL/led.h"
#include "auth_handlers.h"
#include "menu_handlers.h"

/* State machine variables */
static Frontend_State_t currentState = STATE_WELCOME;
static uint8_t attemptCount = 0;
static bool isFirstTime = true;

void Frontend_Start(void)
{
    /* Initialize all peripherals */
    LCD_Init();
    Keypad_Init();
    Potentiometer_Init();
    UART_Init();
    LED_Init();
    
    /* Initialize state machine */
    currentState = STATE_WELCOME;
    attemptCount = 0;
    isFirstTime = true;
    
    /* Main state machine loop */
    while (1) {
        switch (currentState) {
            case STATE_WELCOME:         handleWelcome(&currentState, &isFirstTime); break;
            case STATE_SIGNUP:          handleSignup(&currentState, &isFirstTime); break;
            case STATE_MAIN_MENU:       handleMainMenu(&currentState); break;
            case STATE_SIGNIN:          handleSignin(&currentState, &attemptCount); break;
            case STATE_CHANGE_PASSWORD: handleChangePassword(&currentState, &attemptCount); break;
            case STATE_SET_TIMEOUT:     handleSetTimeout(&currentState, &attemptCount); break;
            case STATE_LOCKOUT:         handleLockout(&currentState, &attemptCount); break;
            default:                    currentState = STATE_MAIN_MENU; break;
        }
    }
}
