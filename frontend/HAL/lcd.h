/*****************************************************************************
 * File: lcd.h
 * Description: I2C LCD Driver Header (PCF8574 + HD44780) - HAL Layer
 * Note: Uses I2C MCAL for hardware communication
 *****************************************************************************/

#ifndef LCD_H
#define LCD_H

#include <stdint.h>

/******************************************************************************
 *                              Definitions                                    *
 ******************************************************************************/

/* LCD Configuration */
#define LCD_ROWS            2       /* Number of LCD rows */
#define LCD_COLS            16      /* Number of LCD columns */

/* LCD Commands */
#define LCD_CMD_CLEAR           0x01    /* Clear display */
#define LCD_CMD_HOME            0x02    /* Return home */
#define LCD_CMD_ENTRY_MODE      0x06    /* Entry mode: increment, no shift */
#define LCD_CMD_DISPLAY_OFF     0x08    /* Display off */
#define LCD_CMD_DISPLAY_ON      0x0C    /* Display on, cursor off, blink off */
#define LCD_CMD_FUNCTION_SET    0x28    /* 4-bit mode, 2 lines, 5x8 font */
#define LCD_CMD_SET_DDRAM       0x80    /* Set DDRAM address */

/******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/*
 * Description: Initialize LCD display via I2C
 * Parameters: None
 * Returns: None
 */
void LCD_Init(void);

/*
 * Description: Clear LCD display and return cursor to home position
 * Parameters: None
 * Returns: None
 */
void LCD_Clear(void);

/*
 * Description: Set cursor position on LCD
 * Parameters:
 *   - row: Row number (0 or 1)
 *   - col: Column number (0-15)
 * Returns: None
 */
void LCD_SetCursor(uint8_t row, uint8_t col);

/*
 * Description: Write a single character to LCD at current cursor position
 * Parameters:
 *   - c: Character to display
 * Returns: None
 */
void LCD_WriteChar(char c);

/*
 * Description: Write a null-terminated string to LCD
 * Parameters:
 *   - str: Pointer to string to display
 * Returns: None
 */
void LCD_WriteString(const char *str);

/*
 * Description: Send command to LCD
 * Parameters:
 *   - cmd: Command byte to send
 * Returns: None
 * Note: This is a low-level function, prefer using higher-level functions
 */
void LCD_Command(uint8_t cmd);

#endif /* LCD_H */
