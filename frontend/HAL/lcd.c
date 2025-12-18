/*****************************************************************************
 * lcd.c - I2C LCD Driver for PCF8574 Backpack + HD44780 LCD (HAL Layer)
 * Description: Hardware Abstraction Layer for LCD communication via I2C
 * Note: Uses I2C MCAL for low-level I2C operations
 *****************************************************************************/
#include <stdint.h>
#include "../MCAL/systick.h"
#include "../MCAL/i2c.h"
#include "lcd.h"

/******************************************************************************
 *                          Private Definitions                                *
 ******************************************************************************/

/* PCF8574 I2C Address (typical: 0x27 or 0x3F) */
#define LCD_I2C_ADDR    0x27

/* I2C Module Used for LCD */
#define LCD_I2C_MODULE  I2C_MODULE_0

/* PCF8574 Pin Mapping for LCD */
#define LCD_RS      0x01    /* P0: Register Select (0=Command, 1=Data) */
#define LCD_RW      0x02    /* P1: Read/Write (0=Write, 1=Read) */
#define LCD_EN      0x04    /* P2: Enable */
#define LCD_BL      0x08    /* P3: Backlight */

/******************************************************************************
 *                         Private Function Prototypes                         *
 ******************************************************************************/

static void LCD_WriteNibble(uint8_t nibble, uint8_t rs);
static void LCD_WriteByte(uint8_t data, uint8_t rs);

/******************************************************************************
 *                         Private Functions                                   *
 ******************************************************************************/

/*
 * Description: Send a 4-bit nibble to LCD via I2C
 * Parameters:
 *   - nibble: 4-bit data in upper nibble (bits 7-4)
 *   - rs: Register Select (0=command, 1=data)
 */
static void LCD_WriteNibble(uint8_t nibble, uint8_t rs)
{
    uint8_t data = (nibble & 0xF0) | LCD_BL;  /* Keep backlight on */
    if (rs) data |= LCD_RS;  /* Set RS bit if writing data */
    
    /* Enable pulse: high -> low */
    I2C_WriteByte(LCD_I2C_MODULE, LCD_I2C_ADDR, data | LCD_EN);  /* EN high */
    DelayMs(1);
    I2C_WriteByte(LCD_I2C_MODULE, LCD_I2C_ADDR, data);           /* EN low */
    DelayMs(1);
}

/*
 * Description: Send a full byte to LCD in 4-bit mode (two nibbles)
 * Parameters:
 *   - data: 8-bit data to send
 *   - rs: Register Select (0=command, 1=data)
 */
static void LCD_WriteByte(uint8_t data, uint8_t rs)
{
    LCD_WriteNibble(data & 0xF0, rs);        /* High nibble */
    LCD_WriteNibble((data << 4) & 0xF0, rs); /* Low nibble */
}

/******************************************************************************
 *                          Public Functions                                   *
 ******************************************************************************/

void LCD_WriteChar(char c)
{
    LCD_WriteByte((uint8_t)c, 1);  /* RS=1 for data */
}

void LCD_WriteString(const char *str)
{
    while (*str) {
        LCD_WriteChar(*str++);
    }
}

void LCD_Command(uint8_t cmd)
{
    LCD_WriteByte(cmd, 0);  /* RS=0 for command */
    
    /* Clear and Home commands require longer delay */
    if (cmd == LCD_CMD_CLEAR || cmd == LCD_CMD_HOME) {
        DelayMs(2);
    }
}

void LCD_Init(void)
{
    /* Initialize I2C MCAL driver */
    I2C_Init(LCD_I2C_MODULE, I2C_SPEED_100K);
    DelayMs(50);  /* Wait for LCD power-up */
    
    /* HD44780 initialization sequence for 4-bit mode */
    LCD_WriteNibble(0x30, 0);  /* Function set: 8-bit mode (initial) */
    DelayMs(5);
    LCD_WriteNibble(0x30, 0);  /* Function set: 8-bit mode (repeat) */
    DelayMs(1);
    LCD_WriteNibble(0x30, 0);  /* Function set: 8-bit mode (repeat) */
    DelayMs(1);
    LCD_WriteNibble(0x20, 0);  /* Function set: Switch to 4-bit mode */
    DelayMs(1);
    
    /* Configure LCD in 4-bit mode */
    LCD_Command(LCD_CMD_FUNCTION_SET);  /* 4-bit, 2 lines, 5x8 font */
    LCD_Command(LCD_CMD_DISPLAY_ON);    /* Display on, cursor off */
    LCD_Clear();                        /* Clear display */
    LCD_Command(LCD_CMD_ENTRY_MODE);    /* Entry mode: increment, no shift */
}

void LCD_Clear(void)
{
    LCD_Command(LCD_CMD_CLEAR);
    DelayMs(2);  /* Clear command requires ~2ms */
}

void LCD_SetCursor(uint8_t row, uint8_t col)
{
    uint8_t addr;
    
    /* Calculate DDRAM address based on row */
    if (row == 0) {
        addr = col;              /* Row 0: 0x00-0x0F */
    } else {
        addr = 0x40 + col;       /* Row 1: 0x40-0x4F */
    }
    
    /* Send Set DDRAM Address command */
    LCD_Command(LCD_CMD_SET_DDRAM | addr);
}
