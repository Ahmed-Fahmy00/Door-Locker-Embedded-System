/*****************************************************************************
 * File: keypad.h
 * Description: Header for 4x4 Keypad Driver
 *****************************************************************************/

#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdint.h>

extern const char keypad_codes[4][4];

#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4

void Keypad_Init(void);
char Keypad_GetKey(void);

#endif