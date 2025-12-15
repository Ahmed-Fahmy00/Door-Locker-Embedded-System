/*****************************************************************************
 * File: keypad.c
 * Description: 4x4 Keypad Driver for TM4C123GH6PM
 * 
 * PIN LAYOUT - ALL KEYPAD ON RIGHT SIDE:
 *   Rows: PC4-PC7 (J4, inputs with pull-up)
 *   Columns: PB6, PA4, PA3, PA2 (J2, outputs)
 * 
 * This keeps the ENTIRE keypad on the right side of the board.
 *****************************************************************************/

#include "keypad.h"
#include "../MCAL/dio.h"

const char keypad_codes[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

/* Row pins on Port C (PC4-PC7) - inputs */
#define KEYPAD_ROW_PORT  PORTC
static const uint8_t row_pins[4] = {PIN4, PIN5, PIN6, PIN7};

/* Column pins - outputs
 * Col 0 = PB6
 * Col 1 = PA4
 * Col 2 = PA3
 * Col 3 = PA2
 */
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

/* Debounce delay */
static void Keypad_Delay(volatile uint32_t count)
{
    while (count > 0) {
        count--;
    }
}

void Keypad_Init(void)
{
    uint8_t i;

    /* Configure rows (PC4-PC7) as inputs with pull-up */
    for (i = 0; i < 4; i++) {
        DIO_Init(KEYPAD_ROW_PORT, row_pins[i], INPUT);
        DIO_SetPUR(KEYPAD_ROW_PORT, row_pins[i], ENABLE);
    }

    /* Configure columns as outputs, set HIGH */
    for (i = 0; i < 4; i++) {
        DIO_Init(col_pins[i].port, col_pins[i].pin, OUTPUT);
        DIO_WritePin(col_pins[i].port, col_pins[i].pin, HIGH);
    }
}

/* Check if any key is currently pressed */
static uint8_t Keypad_AnyKeyPressed(void)
{
    uint8_t col, row;
    
    for (col = 0; col < 4; col++) {
        /* Set all columns HIGH first */
        for (uint8_t c = 0; c < 4; c++) {
            DIO_WritePin(col_pins[c].port, col_pins[c].pin, HIGH);
        }
        
        /* Set current column LOW */
        DIO_WritePin(col_pins[col].port, col_pins[col].pin, LOW);
        
        /* Small delay for signal settling */
        Keypad_Delay(100);
        
        /* Check each row */
        for (row = 0; row < 4; row++) {
            if (DIO_ReadPin(KEYPAD_ROW_PORT, row_pins[row]) == LOW) {
                return 1;  /* Key is pressed */
            }
        }
    }
    
    return 0;  /* No key pressed */
}

char Keypad_GetKey(void)
{
    uint8_t col, row, c;
    char key = 0;

    for (col = 0; col < 4; col++) {
        /* Set all columns HIGH */
        for (c = 0; c < 4; c++) {
            DIO_WritePin(col_pins[c].port, col_pins[c].pin, HIGH);
        }

        /* Set current column LOW */
        DIO_WritePin(col_pins[col].port, col_pins[col].pin, LOW);

        /* Small delay for signal settling */
        Keypad_Delay(100);

        /* Check each row */
        for (row = 0; row < 4; row++) {
            if (DIO_ReadPin(KEYPAD_ROW_PORT, row_pins[row]) == LOW) {
                /* Key detected - save it */
                key = keypad_codes[row][col];
                
                /* Wait while key is still pressed (prevents double detection) */
                while (DIO_ReadPin(KEYPAD_ROW_PORT, row_pins[row]) == LOW) {
                    Keypad_Delay(1000);
                }
                
                /* Additional debounce - wait for all keys to be released */
                Keypad_Delay(5000);
                
                /* Make sure no key is pressed anymore */
                while (Keypad_AnyKeyPressed()) {
                    Keypad_Delay(1000);
                }
                
                /* Final debounce delay after release */
                Keypad_Delay(10000);
                
                return key;
            }
        }
    }

    return 0;  /* No key pressed */
}
