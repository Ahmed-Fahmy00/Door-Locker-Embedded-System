/******************************************************************************
 * File: application.h
 * Module: Frontend Application
 * Description: Main frontend state machine for Door Locker Security System
 ******************************************************************************/

#ifndef APPLICATION_H
#define APPLICATION_H

#include <stdint.h>
#include <stdbool.h>

/* Frontend state machine states */
typedef enum {
    STATE_WELCOME,
    STATE_SIGNUP,
    STATE_MAIN_MENU,
    STATE_SIGNIN,
    STATE_CHANGE_PASSWORD,
    STATE_SET_TIMEOUT,
    STATE_LOCKOUT
} Frontend_State_t;

/**
 * @brief Start the frontend application
 * Initializes all peripherals and runs the main state machine loop.
 * This function never returns.
 */
void Frontend_Start(void);

#endif /* APPLICATION_H */
