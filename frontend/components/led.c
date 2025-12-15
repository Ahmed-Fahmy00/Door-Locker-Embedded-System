/******************************************************************************
 * File: led.c
 * Description: RGB LED Driver for HMI_ECU (Frontend)
 * Uses onboard RGB LED on TM4C123G LaunchPad (Port F)
 * PF1 = Red, PF2 = Blue, PF3 = Green
 ******************************************************************************/
#include "led.h"
#include "../MCAL/dio.h"
#include "../MCAL/systick.h"

void LED_Init(void)
{
    /* Initialize PF1 (Red), PF2 (Blue), PF3 (Green) as outputs */
    DIO_Init(PORTF, PIN1, OUTPUT);
    DIO_Init(PORTF, PIN2, OUTPUT);
    DIO_Init(PORTF, PIN3, OUTPUT);
    
    LED_Off();
}

void LED_SetColor(uint8_t color)
{
    DIO_WritePin(PORTF, PIN1, (color & 0x02) ? HIGH : LOW);
    
    DIO_WritePin(PORTF, PIN2, (color & 0x04) ? HIGH : LOW);
    
    DIO_WritePin(PORTF, PIN3, (color & 0x08) ? HIGH : LOW);
}

void LED_Off(void)
{
    LED_SetColor(LED_OFF);
}

void LED_Red(void)
{
    LED_SetColor(LED_RED);
}

void LED_Green(void)
{
    LED_SetColor(LED_GREEN);
}

void LED_Blue(void)
{
    LED_SetColor(LED_BLUE);
}

void LED_Yellow(void)
{
    LED_SetColor(LED_YELLOW);
}

void LED_Cyan(void)
{
    LED_SetColor(LED_CYAN);
}

void LED_Blink(uint8_t color, uint8_t times, uint16_t delayMs)
{
    uint8_t i;
    
    for (i = 0; i < times; i++) {
        LED_SetColor(color);
        DelayMs(delayMs);
        LED_Off();
        if (i < times - 1) {
            DelayMs(delayMs);
        }
    }
}
