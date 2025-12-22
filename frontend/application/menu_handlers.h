/******************************************************************************
 * File: menu_handlers.h
 * Module: Menu Handlers (Application Layer)
 * Description: Navigation and settings state handlers
 ******************************************************************************/

#ifndef MENU_HANDLERS_H
#define MENU_HANDLERS_H

#include "application.h"

/**
 * @brief Handle welcome screen state
 */
void handleWelcome(Frontend_State_t *currentState, bool *isFirstTime);

/**
 * @brief Handle main menu state
 */
void handleMainMenu(Frontend_State_t *currentState);

/**
 * @brief Handle set timeout state
 */
void handleSetTimeout(Frontend_State_t *currentState, uint8_t *attemptCount);

#endif /* MENU_HANDLERS_H */
