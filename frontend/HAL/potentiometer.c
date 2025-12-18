/*****************************************************************************
 * File: potentiometer.c
 * Description: Potentiometer HAL Driver (HAL Layer)
 * Note: Uses ADC MCAL for hardware communication
 *****************************************************************************/
#include <stdint.h>
#include "potentiometer.h"
#include "../MCAL/adc.h"

/******************************************************************************
 *                          Private Definitions                                *
 ******************************************************************************/


/* Potentiometer ADC Channel (PB5 = AIN11) */
#define POTENTIOMETER_ADC_CHANNEL   ADC_CHANNEL_11

/******************************************************************************
 *                          Public Functions                                   *
 ******************************************************************************/


void Potentiometer_Init(void)
{
    /* Initialize ADC for potentiometer on PB5 (AIN11) */
    ADC_Init(POTENTIOMETER_ADC_CHANNEL);
}


uint32_t Potentiometer_Read(void)
{
    /* Read and return ADC value (0-4095) */
    return ADC_Read();
}

uint32_t Potentiometer_GetTimeout(void)
{
    uint32_t rawValue = Potentiometer_Read();
    uint32_t scaledValue;
    
    /* Scale raw ADC value (0-4095) to timeout range (5-30 seconds) */
    scaledValue = (rawValue * (POTENTIOMETER_TIMEOUT_MAX_SEC - POTENTIOMETER_TIMEOUT_MIN_SEC)
                   / ADC_MAX_VALUE) + POTENTIOMETER_TIMEOUT_MIN_SEC;
    
    return scaledValue;
}
