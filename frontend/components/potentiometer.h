/*****************************************************************************
 * File: potentiometer.h
 * Description: ADC Driver Header for Potentiometer on PB5 (AIN11)
 *****************************************************************************/

#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#include <stdint.h>

/* Initialize ADC0 on PB5 (AIN11) */
void ADC0_Init_PE3(void);

/* Read raw ADC value (0-4095) */
uint32_t ReadPotentiometer(void);

/* Get scaled timer value (5-30 seconds) */
uint32_t GetScaledTimeout(void);

#endif /* POTENTIOMETER_H */
