#include <stdint.h>
#include <stdbool.h>
#include "uart_handler.h"
#include "eeprom_handler.h"
#include "components/timeout.h"
#include "components/motor.h"

/* TivaWare includes */
#include "inc/hw_memmap.h"
#include "driverlib/eeprom.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

/* ========== MOTOR TEST MODE ========== */
/* Set to 1 to test motor at startup, 0 for normal operation */
#define MOTOR_TEST_MODE  0

int main(void)
{
    /* 
     * CRITICAL: Set system clock to 16 MHz and NEVER change it!
     * Changing clock speed will break UART baud rate.
     */
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC |
                   SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    
#if MOTOR_TEST_MODE
    /* Test: Forward 2 sec, then Backward 2 sec */
    Motor_GPIO_Init();
    
    /* Forward - Green LED */
    Motor_Forward();
    SysCtlDelay(SysCtlClockGet() / 3 * 2);
    
    /* Backward - Red LED */
    Motor_Reverse();
    SysCtlDelay(SysCtlClockGet() / 3 * 2);
    
    Motor_Stop();
    while (1) {}
    
#else
    /* Normal operation */
    
    /* Initialize EEPROM */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_EEPROM0)) {}
    
    if (EEPROMInit() != EEPROM_INIT_OK)
    {
        while (1) {}
    }
    
    set_default_auto_timeout();
    timeout_init();
    UART_Handler_Init();
    
    while (1)
    {
        UART_ProcessPending();
    }
#endif
}
