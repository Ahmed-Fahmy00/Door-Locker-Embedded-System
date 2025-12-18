/*****************************************************************************
 * 4x4 Keypad 
 * Rows: PC4-PC7 (J4, inputs with pull-up)
 * Column pins - outputs
 *    Col 0 = PB6
 *    Col 1 = PA4
 *    Col 2 = PA3
 *    Col 3 = PA2
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
static const uint8_t row_pins[4] = {PIN4, PIN5, PIN6, PIN7};

typedef struct { 
    uint8_t port;
    uint8_t pin;
} ColPin_t;

static const ColPin_t col_pins[4] = {
    {PORTB, PIN6},  /* Col 0 = PB6 */
    {PORTA, PIN4},  /* Col 1 = PA4 */
    {PORTA, PIN3},  /* Col 2 = PA3 */
    {PORTA, PIN2}   /* Col 3 = PA2 */
};

void Keypad_Init(void)
{
    /* Configure rows (PC4-PC7) as inputs with pull-up */
    for (uint8_t i = 0; i < 4; i++) {
        DIO_Init(KEYPAD_ROW_PORT, row_pins[i], INPUT);
        DIO_SetPUR(KEYPAD_ROW_PORT, row_pins[i], ENABLE);
    }

    /* Configure columns as outputs, set HIGH */
    for (uint8_t i = 0; i < 4; i++) {
        DIO_Init(col_pins[i].port, col_pins[i].pin, OUTPUT);
        DIO_WritePin(col_pins[i].port, col_pins[i].pin, HIGH);
    }
}

char Keypad_GetKey(void)
{
    uint8_t col, row, c;

    for (col = 0; col < 4; col++) {
        /* Set all columns HIGH */
        for (c = 0; c < 4; c++) {
            DIO_WritePin(col_pins[c].port, col_pins[c].pin, HIGH);
        }

        /* Set current column LOW */
        DIO_WritePin(col_pins[col].port, col_pins[col].pin, LOW);

        /* Check each row */
        for (row = 0; row < 4; row++) {
            if (DIO_ReadPin(KEYPAD_ROW_PORT, row_pins[row]) == LOW) {
                /* Wait for key release */
                while (DIO_ReadPin(KEYPAD_ROW_PORT, row_pins[row]) == LOW);
                
                return keypad_codes[row][col];
            }
        }
    }
    return 0;  /* No key pressed */
}
