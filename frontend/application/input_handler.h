/******************************************************************************
 * File: input_handler.h
 * Module: Input Handler (Application Layer)
 * Description: Keypad input and password handling functions
 ******************************************************************************/

#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <stdint.h>
#include <stdbool.h>

#define PASSWORD_LENGTH 5

/**
 * @brief Wait for any key press (blocking)
 * @return The pressed key character
 */
char waitForKey(void);

/**
 * @brief Get a 5-digit password from keypad with LCD feedback
 * @param buffer Buffer to store the password (must be at least PASSWORD_LENGTH+1)
 * @return true if password entered successfully, false if cancelled
 */
bool getPasswordFromKeypad(char *buffer);

/**
 * @brief Compare two strings for equality
 * @param s1 First string
 * @param s2 Second string
 * @param len Number of characters to compare
 * @return true if strings match, false otherwise
 */
bool stringsMatch(const char *s1, const char *s2, uint8_t len);

#endif /* INPUT_HANDLER_H */
