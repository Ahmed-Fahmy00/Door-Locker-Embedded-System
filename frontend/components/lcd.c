/*****************************************************************************
 * lcd.c - I2C LCD Driver for TM4C123GH6PM
 * I2C0: PB2 (SCL), PB3 (SDA)
 * PCF8574 backpack + HD44780 LCD in 4-bit mode
 *****************************************************************************/

#include <stdint.h>
#include "../lib/tm4c123gh6pm.h"
#include "../MCAL/systick.h"
#include "lcd.h"

/* I2C Master commands */
#define I2C_CMD_SINGLE_SEND  0x07  /* START + RUN + STOP */

/* I2C MCS register bits */
#define MCS_BUSY   0x01
#define MCS_ERROR  0x02
#define MCS_STOP   0x04

/* Forward declarations */
static void I2C0_Init(void);
static void I2C0_Write(uint8_t data);
static void LCD_WriteNibble(uint8_t nibble, uint8_t rs);
static void LCD_WriteByte(uint8_t data, uint8_t rs);

static void I2C0_Init(void)
{
    volatile uint32_t delay;
    
    /* Enable clocks */
    SYSCTL_RCGCI2C_R |= 0x01;   /* I2C0 */
    SYSCTL_RCGCGPIO_R |= 0x02;  /* Port B */
    
    /* Wait for clocks */
    delay = SYSCTL_RCGCI2C_R;
    delay = SYSCTL_RCGCGPIO_R;
    while ((SYSCTL_PRGPIO_R & 0x02) == 0) {}
    
    /* Configure PB2, PB3 for I2C */
    GPIO_PORTB_AFSEL_R |= 0x0C;
    GPIO_PORTB_ODR_R |= 0x08;        /* Open drain on SDA */
    GPIO_PORTB_DEN_R |= 0x0C;
    GPIO_PORTB_PCTL_R &= ~0x0000FF00;
    GPIO_PORTB_PCTL_R |= 0x00003300; /* I2C function */
    
    /* I2C Master init */
    I2C0_MCR_R = 0x10;
    I2C0_MTPR_R = 7;  /* 100kHz @ 16MHz: TPR = (16MHz / (2*10*100kHz)) - 1 = 7 */
}

static void I2C0_Write(uint8_t data)
{
    while (I2C0_MCS_R & MCS_BUSY) {}
    
    I2C0_MSA_R = (LCD_I2C_ADDR << 1);  /* Write mode */
    I2C0_MDR_R = data;
    I2C0_MCS_R = I2C_CMD_SINGLE_SEND;
    
    while (I2C0_MCS_R & MCS_BUSY) {}
    
    if (I2C0_MCS_R & MCS_ERROR) {
        I2C0_MCS_R = MCS_STOP;
    }
}

static void LCD_WriteNibble(uint8_t nibble, uint8_t rs)
{
    uint8_t data = (nibble & 0xF0) | LCD_BL;
    if (rs) data |= LCD_RS;
    
    I2C0_Write(data | LCD_EN);  /* EN high */
    DelayMs(1);
    I2C0_Write(data);           /* EN low */
    DelayMs(1);
}

static void LCD_WriteByte(uint8_t data, uint8_t rs)
{
    LCD_WriteNibble(data & 0xF0, rs);        /* High nibble */
    LCD_WriteNibble((data << 4) & 0xF0, rs); /* Low nibble */
}

void LCD_WriteChar(char c)
{
    LCD_WriteByte((uint8_t)c, 1);
}

void LCD_WriteString(const char *str)
{
    while (*str) {
        LCD_WriteChar(*str++);
    }
}

void LCD_Command(uint8_t cmd)
{
    LCD_WriteByte(cmd, 0);
    if (cmd == LCD_CMD_CLEAR || cmd == LCD_CMD_HOME) {
        DelayMs(2);
    }
}

void LCD_Init(void)
{
    I2C0_Init();
    DelayMs(50);
    
    /* 4-bit mode init sequence */
    LCD_WriteNibble(0x30, 0);
    DelayMs(5);
    LCD_WriteNibble(0x30, 0);
    DelayMs(1);
    LCD_WriteNibble(0x30, 0);
    DelayMs(1);
    LCD_WriteNibble(0x20, 0);
    DelayMs(1);
    
    LCD_Command(LCD_CMD_FUNCTION_SET);
    LCD_Command(LCD_CMD_DISPLAY_ON);
    LCD_Clear();
    LCD_Command(LCD_CMD_ENTRY_MODE);
}

void LCD_Clear(void)
{
    LCD_Command(LCD_CMD_CLEAR);
    DelayMs(2);
}

void LCD_Home(void)
{
    LCD_Command(LCD_CMD_HOME);
}

void LCD_SetCursor(uint8_t row, uint8_t col)
{
    uint8_t addr = (row == 0) ? col : (0x40 + col);
    LCD_Command(LCD_CMD_SET_DDRAM | addr);
}

