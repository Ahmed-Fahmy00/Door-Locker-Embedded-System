/*****************************************************************************
 * File: potentiometer.c
 * Description: ADC Driver for Potentiometer on PB5 (AIN11)
 *****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "potentiometer.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/adc.h"
#include "driverlib/pin_map.h"

#define POT_MIN_VAL 0
#define POT_MAX_VAL 4095
#define TIMER_MIN_SEC 5
#define TIMER_MAX_SEC 30

void ADC0_Init_PE3(void)
{
    /* Enable Port B for PB5 */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));

    /* Enable ADC0 */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0));

    /* Configure PB5 as ADC input */
    GPIOPinTypeADC(GPIO_PORTB_BASE, GPIO_PIN_5);

    /* Configure ADC sequencer 3 for single sample on AIN11 (PB5) */
    ADCSequenceDisable(ADC0_BASE, 3);
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH11 | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 3);
    ADCIntClear(ADC0_BASE, 3);
}

uint32_t ReadPotentiometer(void)
{
    uint32_t value;

    ADCProcessorTrigger(ADC0_BASE, 3);
    while(!ADCIntStatus(ADC0_BASE, 3, false));
    ADCSequenceDataGet(ADC0_BASE, 3, &value);
    ADCIntClear(ADC0_BASE, 3);

    return value;
}

uint32_t GetScaledTimeout(void)
{
    uint32_t rawValue = ReadPotentiometer();
    uint32_t scaledValue = (rawValue * (TIMER_MAX_SEC - TIMER_MIN_SEC) / POT_MAX_VAL) + TIMER_MIN_SEC;
    return scaledValue;
}
