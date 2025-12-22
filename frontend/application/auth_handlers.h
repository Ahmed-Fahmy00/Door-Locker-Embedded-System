/******************************************************************************
 * File: auth_handlers.h
 * Module: Auth Handlers (Application Layer)
 * Description: Authentication-related state handlers
 ******************************************************************************/

#ifndef AUTH_HANDLERS_H
#define AUTH_HANDLERS_H

#include "application.h"

/**
 * @brief Handle signup (password creation) state
 */
void handleSignup(Frontend_State_t *currentState, bool *isFirstTime);

/**
 * @brief Handle signin (door open) state
 */
void handleSignin(Frontend_State_t *currentState, uint8_t *attemptCount);

/**
 * @brief Handle change password state
 */
void handleChangePassword(Frontend_State_t *currentState, uint8_t *attemptCount);

/**
 * @brief Handle lockout state
 */
void handleLockout(Frontend_State_t *currentState, uint8_t *attemptCount);

#endif /* AUTH_HANDLERS_H */
