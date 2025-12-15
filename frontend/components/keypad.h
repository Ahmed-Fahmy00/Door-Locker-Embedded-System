/*****************************************************************************
 * File: keypad.h
 * Description: Header for 4x4 Keypad Driver
 * 
 * Pin Layout (ALL ON RIGHT SIDE - J4 + J2):
 *   Rows: PC4, PC5, PC6, PC7 (J4)
 *   Cols: PB6, PA4, PA3, PA2 (J2)
 * 
 * Keypad Layout:
 *   [1] [2] [3] [A]
 *   [4] [5] [6] [B]
 *   [7] [8] [9] [C]
 *   [*] [0] [#] [D]
 *****************************************************************************/

#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdint.h>

extern const char keypad_codes[4][4];

#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4

/* Initialize keypad GPIO pins */
void Keypad_Init(void);

/* Get pressed key (blocking with debounce), returns 0 if no key */
char Keypad_GetKey(void);

#endif /* KEYPAD_H */
