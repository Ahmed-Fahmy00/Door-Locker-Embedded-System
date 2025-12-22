/******************************************************************************
 * File: status_led.h
 * Module: Status LED (HAL Layer)
 * Description: Status LED control for Backend (PF1=Red, PF3=Green)
 ******************************************************************************/

#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <stdint.h>

/**
 * @brief Initialize status LEDs (PF1=Red, PF3=Green)
 */
void LED_Init(void);

/**
 * @brief Turn off all LEDs
 */
void LED_Off(void);

/**
 * @brief Turn on green LED
 */
void LED_GreenOn(void);

/**
 * @brief Turn on red LED
 */
void LED_RedOn(void);

/**
 * @brief Blink green LED
 * @param times Number of blinks
 */
void LED_BlinkGreen(uint8_t times);

/**
 * @brief Blink red LED
 * @param times Number of blinks
 */
void LED_BlinkRed(uint8_t times);

#endif /* STATUS_LED_H */
