/******************************************************************************
 * File: adc.h
 * Module: ADC (Analog to Digital Converter)
 * Description: Header file for TM4C123GH6PM ADC Driver
 * Author: Ahmedhh
 * Date: December 8, 2025
 ******************************************************************************/

#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>

/******************************************************************************
 *                              Definitions                                    *
 ******************************************************************************/

/*
 * ADC Module Definitions
 */
#define ADC_MODULE_0        0
#define ADC_MODULE_1        1

/*
 * ADC Sample Sequencer Definitions
 */
#define ADC_SS0             0
#define ADC_SS1             1
#define ADC_SS2             2
#define ADC_SS3             3

/*
 * ADC Channel Definitions (for PE3, we use AIN0)
 * PE3 = AIN0
 * PE2 = AIN1
 * PE1 = AIN2
 * PE0 = AIN3
 */
#define ADC_CHANNEL_0       0
#define ADC_CHANNEL_1       1
#define ADC_CHANNEL_2       2
#define ADC_CHANNEL_3       3
#define ADC_CHANNEL_4       4
#define ADC_CHANNEL_5       5
#define ADC_CHANNEL_6       6
#define ADC_CHANNEL_7       7
#define ADC_CHANNEL_8       8
#define ADC_CHANNEL_9       9
#define ADC_CHANNEL_10      10
#define ADC_CHANNEL_11      11

/*
 * ADC Resolution: 12-bit ADC (0-4095)
 */
#define ADC_MAX_VALUE       4095

/******************************************************************************
 *                         Function Prototypes                                 *
 ******************************************************************************/

/*
 * Description: Initializes ADC0 Sequencer 3 for single-ended sampling
 * Parameters:
 *   - channel: ADC input channel (0-11)
 * Returns: None
 * Note: PE3 corresponds to AIN0 (channel 0)
 */
void ADC_Init(uint8_t channel);

/*
 * Description: Reads a single sample from the ADC
 * Parameters: None
 * Returns: 12-bit ADC conversion result (0-4095)
 */
uint16_t ADC_Read(void);

/*
 * Description: Converts ADC value to millivolts (assuming 3.3V reference)
 * Parameters:
 *   - adcValue: Raw ADC reading (0-4095)
 * Returns: Voltage in millivolts (0-3300)
 */
uint32_t ADC_ToMillivolts(uint16_t adcValue);

#endif /* ADC_H_ */
