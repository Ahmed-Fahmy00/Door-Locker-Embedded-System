/******************************************************************************
 * File: adc.c
 * Module: ADC (Analog to Digital Converter)
 * Description: Source file for TM4C123GH6PM ADC Driver
 * Author: Ahmedhh
 * Date: December 8, 2025
 ******************************************************************************/

#include "adc.h"
#include "tm4c123gh6pm.h"

/******************************************************************************
 *                         Function Definitions                                *
 ******************************************************************************/

/*
 * Description: Initializes ADC0 Sequencer 3 for single-ended sampling
 * Parameters:
 *   - channel: ADC input channel (0-11)
 * Returns: None
 * Note: PE3 corresponds to AIN0 (channel 0)
 */
void ADC_Init(uint8_t channel)
{
    volatile uint32_t delay;
    
    /* Enable ADC0 clock */
    SYSCTL_RCGCADC_R |= 0x01;           /* Enable ADC0 clock */
    delay = SYSCTL_RCGCADC_R;           /* Allow time for clock to stabilize */
    
    /* Enable GPIO Port E clock */
    SYSCTL_RCGCGPIO_R |= 0x10;          /* Enable Port E clock */
    delay = SYSCTL_RCGCGPIO_R;          /* Allow time for clock to stabilize */
    
    /* Configure PE3 as analog input */
    GPIO_PORTE_DIR_R &= ~0x08;          /* Set PE3 as input */
    GPIO_PORTE_AFSEL_R |= 0x08;         /* Enable alternate function on PE3 */
    GPIO_PORTE_DEN_R &= ~0x08;          /* Disable digital I/O on PE3 */
    GPIO_PORTE_AMSEL_R |= 0x08;         /* Enable analog function on PE3 */
    
    /* Configure ADC0 */
    ADC0_ACTSS_R &= ~0x08;              /* Disable Sample Sequencer 3 during configuration */
    ADC0_EMUX_R &= ~0xF000;             /* Software trigger for SS3 */
    ADC0_SSMUX3_R = (channel & 0x0F);   /* Set channel for SS3 */
    ADC0_SSCTL3_R = 0x06;               /* Set control bits: END0 and IE0 */
    ADC0_SAC_R = 0x00;                  /* Disable hardware averaging */
    ADC0_CTL_R &= ~0x01;                /* Clear VREF bit to use internal 3.3V reference */
    ADC0_ACTSS_R |= 0x08;               /* Enable Sample Sequencer 3 */
}

/*
 * Description: Reads a single sample from the ADC
 * Parameters: None
 * Returns: 12-bit ADC conversion result (0-4095)
 */
uint16_t ADC_Read(void)
{
    uint16_t result;
    volatile uint32_t delay;
    
    ADC0_PSSI_R = 0x08;                 /* Initiate SS3 conversion */
    while((ADC0_RIS_R & 0x08) == 0);    /* Wait for conversion complete */
    result = ADC0_SSFIFO3_R & 0xFFF;    /* Read 12-bit result */
    ADC0_ISC_R = 0x08;                  /* Clear completion flag */
    
    /* Small delay to allow settling */
    for(delay = 0; delay < 10; delay++);
    
    return result;
}

/*
 * Description: Converts ADC value to millivolts (assuming 3.3V reference)
 * Parameters:
 *   - adcValue: Raw ADC reading (0-4095)
 * Returns: Voltage in millivolts (0-3300)
 */
uint32_t ADC_ToMillivolts(uint16_t adcValue)
{
    /* Convert to millivolts: (adcValue * 3300) / 4095 */
    return (adcValue * 3300UL) / 4095UL;
}
