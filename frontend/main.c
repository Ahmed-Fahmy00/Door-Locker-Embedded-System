/*****************************************************************************
 * Hardware Configuration:
 *   - LCD: I2C0 (PB2=SCL, PB3=SDA)
 *   - Keypad: Rows PC4-PC7, Columns PB6, PA4, PA3, PA2
 *   - Potentiometer: PB5 (ADC AIN11)
 *   - UART1: PB0=Rx, PB1=Tx (Communication with Control_ECU)
 *   - RGB LED: PF1, PF2, PF3 (onboard)
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/sysctl.h"
#include "MCAL/systick.h"
#include "frontend.h"

int main(void)
{
    /* Wait for hardware to stabilize after flashing/power-on */
    SysCtlDelay(SysCtlClockGet() / 3);  /* ~1 second delay using default clock */
    
    /* Set system clock to 16 MHz (to match backend) */
    SysCtlClockSet(
        SYSCTL_SYSDIV_1 |
        SYSCTL_USE_OSC |
        SYSCTL_OSC_MAIN |
        SYSCTL_XTAL_16MHZ
    );

    /* Initialize SysTick for delays (16MHz / 16000 = 1ms tick) */
    SysTick_Init(16000, SYSTICK_NOINT);
    
    /* Additional stabilization delay for I2C LCD */
    DelayMs(200);

    Frontend_Start(); /* Start the frontend application */

    while (1) {}/* Should never reach here */
}
