
/******************************************************************************
 * File: input_handler.c
 * Module: Input Handler (Application Layer)
 * Description: Keypad input and password handling functions
 ******************************************************************************/

#include "input_handler.h"
#include "../HAL/keypad.h"
#include "../HAL/lcd.h"

char waitForKey(void)
{
    char key = 0;
    while (key == 0) {
        key = Keypad_GetKey();
    }
    return key;
}

bool getPasswordFromKeypad(char *buffer)
{
    uint8_t i = 0;
    char key;
    
    while (i < PASSWORD_LENGTH) {
        key = Keypad_GetKey();
        
        if (key == '#') {
            if (i > 0) {
                i--;
                LCD_SetCursor(1, i);
                LCD_WriteChar(' ');
                LCD_SetCursor(1, i);
            } else {
                return false;
            }
        }
        else if (key >= '0' && key <= '9') {
            buffer[i] = key;
            LCD_WriteChar('*');
            i++;
        }
    }
    buffer[PASSWORD_LENGTH] = '\0';
    return true;
}

bool stringsMatch(const char *s1, const char *s2, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        if (s1[i] != s2[i]) return false;
    }
    return true;
}
