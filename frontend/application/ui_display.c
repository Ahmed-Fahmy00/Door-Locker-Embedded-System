/******************************************************************************
 * File: ui_display.c
 * Module: UI Display (Application Layer)
 * Description: LCD display helper functions
 ******************************************************************************/

#include "ui_display.h"
#include "../HAL/lcd.h"

void showMessage(const char *line1, const char *line2)
{
    LCD_Clear();
    LCD_SetCursor(0, 0);
    if (line1) LCD_WriteString(line1);
    if (line2) {
        LCD_SetCursor(1, 0);
        LCD_WriteString(line2);
    }
}
