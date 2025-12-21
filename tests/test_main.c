#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "uart_handler.h"
#include "eeprom_handler.h"
#include "components/timeout.h"
#include "components/motor.h"

/* TivaWare includes */
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "driverlib/eeprom.h"

/*=========================================================================== 
 * BUILD CONFIGURATION
 *===========================================================================*/
#define RUN_TESTS        1   // Set to 1 to enable tests, set to 0 to disable tests
#define MOTOR_TEST_MODE  0   // Set to 1 to run motor tests, set to 0 to disable

#if RUN_TESTS
#include "tests/test_common.h"
#endif

/*=========================================================================== 
 * UART0 printf redirection
 *===========================================================================*/
int putchar(int ch) {
    UARTCharPut(UART0_BASE, ch);
    return ch;
}

static void uart_init(void) {
    /* Enable UART0 and GPIOA */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    /* Configure PA0 (RX) and PA1 (TX) for UART0 */
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /* Set UART0 baud rate = 115200, 8N1 using current system clock */
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(),
                        115200, (UART_CONFIG_WLEN_8 |
                                 UART_CONFIG_STOP_ONE |
                                 UART_CONFIG_PAR_NONE));
}

/*=========================================================================== 
 * Main Function
 *===========================================================================*/
int main(void)
{
    /* System clock: 16 MHz (internal main osc with 16 MHz crystal) */
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC |
                   SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    /* Initialize EEPROM */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_EEPROM0)) {}
    if (EEPROMInit() != EEPROM_INIT_OK) {
        while (1) {}  /* EEPROM init failed - halt */
    }

#if RUN_TESTS
    /* Debug/test output via UART0 ? TeraTerm */
    uart_init();

    /* Initialize test framework */
    test_init();

    /* Run tests */
    run_eeprom_tests();
    run_buzzer_tests();
    run_motor_tests();
    run_ack_lights_tests();
    run_timer_tests();
    run_integration_tests();

    /* Summary */
    print_test_summary();
    printf("Tests complete. Check results above.\n");

    while (1) {}  /* Halt after tests */

#elif MOTOR_TEST_MODE
    /* Quick motor hardware test */
    uart_init(); /* optional: for status prints */
    Motor_GPIO_Init();

    Motor_Forward();
    SysCtlDelay(SysCtlClockGet() / 3 * 2);  /* ~2 sec at 16 MHz */

    Motor_Reverse();
    SysCtlDelay(SysCtlClockGet() / 3 * 2);  /* ~2 sec */

    Motor_Stop();
    while (1) {}

#else
    /* Normal operation: Door Locker System */
    uart_init();          /* enable printf to terminal for logs */
    set_default_auto_timeout();
    timeout_init();
    UART_Handler_Init();

    printf("Door Locker system started.\n");

    while (1) {
        /* main loop */
    }
#endif
}
