/*****************************************************************************
 * File: main.c
 * Description: Door Locker Security System - HMI_ECU Entry Point
 * TM4C123GH6PM - Frontend Microcontroller
 * 
 * Hardware Configuration:
 *   - LCD: I2C0 (PB2=SCL, PB3=SDA)
 *   - Keypad: Rows PC4-PC7, Columns PB4-PB7
 *   - Potentiometer: PE3 (ADC)
 *   - UART1: PB0=Rx, PB1=Tx (Communication with Control_ECU)
 *****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "driverlib/sysctl.h"
#include "MCAL/systick.h"
#include "frontend.h"

int main(void)
{
    /* Set system clock to 50 MHz */
    SysCtlClockSet(
        SYSCTL_SYSDIV_4 |
        SYSCTL_USE_PLL |
        SYSCTL_OSC_MAIN |
        SYSCTL_XTAL_16MHZ
    );

    /* Initialize SysTick for delays (50MHz / 50000 = 1ms tick) */
    SysTick_Init(50000, SYSTICK_NOINT);

    /* Start the frontend application */
    Frontend_Start();

    /* Should never reach here */
    while (1) {}
}
