/*****************************************************************************
 * File: frontend.h
 * Description: Frontend Application Logic for Door Locker Security System
 * HMI_ECU - Handles LCD, Keypad, Potentiometer, User Interaction
 *****************************************************************************/

#ifndef FRONTEND_H
#define FRONTEND_H

#include <stdint.h>
#include <stdbool.h>

/* Application States */
typedef enum {
    STATE_WELCOME,
    STATE_SIGNUP,
    STATE_SIGNIN,
    STATE_MAIN_MENU,
    STATE_CHANGE_PASSWORD,
    STATE_SET_TIMEOUT,
    STATE_DOOR_OPENING,
    STATE_DOOR_OPEN,
    STATE_DOOR_CLOSING,
    STATE_LOCKOUT
} Frontend_State_t;

/* Menu Keys */
#define KEY_SIGNIN          'A'
#define KEY_CHANGE_PASS     'B'
#define KEY_SET_TIMEOUT     'C'
#define KEY_SAVE            'D'
#define KEY_CANCEL          '#'

/* Initialize and start the frontend application */
void Frontend_Start(void);

/* Get current application state */
Frontend_State_t Frontend_GetState(void);

#endif /* FRONTEND_H */
