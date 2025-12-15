#include "buzzer.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include <stdbool.h>

void buzzer_init(void)
{
    /* Buzzer on PA5 ONLY
     * DO NOT use PF1 - it's used by motor (IN2)!
     * Using PF1 here caused motor to spin during lockdown.
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)) {}
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_5);

    buzzer_off();
}

void buzzer_on(void)
{
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5, GPIO_PIN_5);
}

void buzzer_off(void)
{
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5, 0);
}
