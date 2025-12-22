/******************************************************************************
 * File: status_led.c
 * Module: Status LED (HAL Layer)
 * Description: Status LED control for Backend (PF1=Red, PF3=Green)
 ******************************************************************************/

#include "status_led.h"
#include <stdbool.h>
#include <stdint.h>

/* TivaWare Includes */
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

#define LED_RED     GPIO_PIN_1
#define LED_GREEN   GPIO_PIN_3
#define LED_ALL     (LED_RED | LED_GREEN)

void LED_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)) {}
    
    /* Configure PF1 (Red) and PF3 (Green) as outputs */
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, LED_ALL);
    GPIOPinWrite(GPIO_PORTF_BASE, LED_ALL, 0);
}

void LED_Off(void)
{
    GPIOPinWrite(GPIO_PORTF_BASE, LED_ALL, 0);
}

void LED_GreenOn(void)
{
    GPIOPinWrite(GPIO_PORTF_BASE, LED_ALL, LED_GREEN);
}

void LED_RedOn(void)
{
    GPIOPinWrite(GPIO_PORTF_BASE, LED_ALL, LED_RED);
}

void LED_BlinkGreen(uint8_t times)
{
    for (uint8_t i = 0; i < times; i++)
    {
        LED_GreenOn();
        SysCtlDelay(SysCtlClockGet() / 20);  /* 50ms on */
        LED_Off();
        if (i < times - 1)
        {
            SysCtlDelay(SysCtlClockGet() / 20);  /* 50ms off */
        }
    }
}

void LED_BlinkRed(uint8_t times)
{
    for (uint8_t i = 0; i < times; i++)
    {
        LED_RedOn();
        SysCtlDelay(SysCtlClockGet() / 20);  /* 50ms on */
        LED_Off();
        if (i < times - 1)
        {
            SysCtlDelay(SysCtlClockGet() / 20);  /* 50ms off */
        }
    }
}
