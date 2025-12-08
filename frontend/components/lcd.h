/*****************************************************************************
 * File: lcd.h
 * Description: I2C LCD Driver Header (PCF8574 + HD44780) for TM4C123GH6PM
 * LCD: SDA on PB3, SCL on PB2 (I2C0)
 *****************************************************************************/

#ifndef LCD_H
#define LCD_H

#include <stdint.h>

/* PCF8574 I2C Address (typical: 0x27 or 0x3F) */
#define LCD_I2C_ADDR    0x27

/* LCD Commands */
#define LCD_CMD_CLEAR           0x01
#define LCD_CMD_HOME            0x02
#define LCD_CMD_ENTRY_MODE      0x06
#define LCD_CMD_DISPLAY_OFF     0x08
#define LCD_CMD_DISPLAY_ON      0x0C
#define LCD_CMD_FUNCTION_SET    0x28    /* 4-bit, 2 lines, 5x8 font */
#define LCD_CMD_SET_DDRAM       0x80

/* PCF8574 Pin Mapping for LCD */
#define LCD_RS      0x01    /* P0: Register Select */
#define LCD_RW      0x02    /* P1: Read/Write */
#define LCD_EN      0x04    /* P2: Enable */
#define LCD_BL      0x08    /* P3: Backlight */

/* Function Prototypes */
void LCD_Init(void);
void LCD_Clear(void);
void LCD_Home(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_WriteChar(char c);
void LCD_WriteString(const char *str);
void LCD_Command(uint8_t cmd);

#endif /* LCD_H */
