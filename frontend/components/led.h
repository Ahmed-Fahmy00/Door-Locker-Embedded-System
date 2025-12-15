/******************************************************************************
 * File: led.h
 * Description: RGB LED Driver for HMI_ECU (Frontend)
 * Uses onboard RGB LED on TM4C123G LaunchPad (Port F)
 * PF1 = Red, PF2 = Blue, PF3 = Green
 ******************************************************************************/
#ifndef LED_H_
#define LED_H_

#include <stdint.h>

#define LED_OFF         0x00
#define LED_RED         0x02    /* PF1 */
#define LED_BLUE        0x04    /* PF2 */
#define LED_GREEN       0x08    /* PF3 */
#define LED_YELLOW      0x0A    /* PF1 + PF3 (Red + Green) */
#define LED_CYAN        0x0C    /* PF2 + PF3 (Blue + Green) */
#define LED_MAGENTA     0x06    /* PF1 + PF2 (Red + Blue) */
#define LED_WHITE       0x0E    /* PF1 + PF2 + PF3 */

void LED_Init(void);

/* Set LED color (use LED_xxx defines) */
void LED_SetColor(uint8_t color);

/* Convenience functions */
void LED_Off(void);
void LED_Red(void);
void LED_Green(void);
void LED_Blue(void);
void LED_Yellow(void);
void LED_Cyan(void);

void LED_Blink(uint8_t color, uint8_t times, uint16_t delayMs);

#endif /* LED_H_ */
