/*****************************************************************************
 * File: keypad.c
 * Description: 4x4 Keypad Driver for TM4C123GH6PM
 * Rows: PC4-PC7 (inputs with pull-up)
 * Columns: PB4-PB7 (outputs)
 *****************************************************************************/

#include "keypad.h"
#include "../MCAL/dio.h"

const char keypad_codes[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

#define KEYPAD_ROW_PORT  PORTC
#define KEYPAD_COL_PORT  PORTB

/* Row pins: PC4, PC5, PC6, PC7 */
static const uint8_t row_pins[4] = {PIN4, PIN5, PIN6, PIN7};

/* Column pins: PB4, PB5, PB6, PB7 */
static const uint8_t col_pins[4] = {PIN4, PIN5, PIN6, PIN7};

void Keypad_Init(void)
{
    uint8_t i;

    /* Configure rows (PC4-PC7) as inputs with pull-up */
    for (i = 0; i < 4; i++) {
        DIO_Init(KEYPAD_ROW_PORT, row_pins[i], INPUT);
        DIO_SetPUR(KEYPAD_ROW_PORT, row_pins[i], ENABLE);
    }

    /* Configure columns (PB4-PB7) as outputs, set HIGH */
    for (i = 0; i < 4; i++) {
        DIO_Init(KEYPAD_COL_PORT, col_pins[i], OUTPUT);
        DIO_WritePin(KEYPAD_COL_PORT, col_pins[i], HIGH);
    }
}

char Keypad_GetKey(void)
{
    uint8_t col, row, c;

    for (col = 0; col < 4; col++) {
        /* Set all columns HIGH */
        for (c = 0; c < 4; c++) {
            DIO_WritePin(KEYPAD_COL_PORT, col_pins[c], HIGH);
        }

        /* Set current column LOW */
        DIO_WritePin(KEYPAD_COL_PORT, col_pins[col], LOW);

        /* Small delay for signal settling */
        for (volatile int d = 0; d < 100; d++);

        /* Check each row */
        for (row = 0; row < 4; row++) {
            if (DIO_ReadPin(KEYPAD_ROW_PORT, row_pins[row]) == LOW) {
                /* Wait for key release (debounce) */
                while (DIO_ReadPin(KEYPAD_ROW_PORT, row_pins[row]) == LOW);

                /* Small debounce delay */
                for (volatile int d = 0; d < 1000; d++);

                return keypad_codes[row][col];
            }
        }
    }

    return 0;  /* No key pressed */
}
