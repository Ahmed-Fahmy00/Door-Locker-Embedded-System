/*****************************************************************************
 * File: potentiometer.h
 * Description: Potentiometer HAL Driver Header
 * Note: Uses ADC MCAL for hardware communication
 *****************************************************************************/

#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#include <stdint.h>

/******************************************************************************
 *                              Definitions                                    *
 ******************************************************************************/

/* Timeout range in seconds */
#define POTENTIOMETER_TIMEOUT_MIN_SEC   5
#define POTENTIOMETER_TIMEOUT_MAX_SEC   30

/******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/*
 * Description: Initialize potentiometer ADC interface
 * Parameters: None
 * Returns: None
 */
void Potentiometer_Init(void);

/*
 * Description: Read raw potentiometer ADC value
 * Parameters: None
 * Returns: Raw ADC value (0-4095)
 */
uint32_t Potentiometer_Read(void);

/*
 * Description: Get scaled timeout value from potentiometer
 * Parameters: None
 * Returns: Timeout value in seconds (5-30)
 */
uint32_t Potentiometer_GetTimeout(void);

#endif /* POTENTIOMETER_H */
