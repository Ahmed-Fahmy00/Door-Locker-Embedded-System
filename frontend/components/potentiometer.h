/*****************************************************************************
 * File: potentiometer.h
 * Description: ADC Driver Header for Potentiometer
 *****************************************************************************/

#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#include <stdint.h>

/* Initialize ADC0 on PE3 (AIN0) */
void ADC0_Init_PE3(void);

/* Read raw ADC value (0-4095) */
uint32_t ReadPotentiometer(void);

/* Get scaled timer value (5-30 seconds) */
uint32_t GetScaledTimeout(void);

#endif /* POTENTIOMETER_H */
