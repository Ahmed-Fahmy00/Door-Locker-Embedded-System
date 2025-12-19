/*****************************************************************************
 * File: timeout.h (DEPRECATED - DO NOT USE)
 * Description: This file has been deprecated and replaced by:
 * 
 * - MCAL/gptm.h - Low-level timer hardware control
 * - buzzer_service.h - High-level buzzer control logic (Application Layer)
 * 
 * Please use the new architecture:
 *   Application Layer: buzzer_service.c/h
 *   HAL Layer:        buzzer.c/h (GPIO only)
 *   MCAL Layer:       gptm.c/h (Timer hardware)
 * 
 * Date: December 19, 2025
 *****************************************************************************/

#ifndef TIMEOUT_H_DEPRECATED
#define TIMEOUT_H_DEPRECATED

#error "This file is deprecated. Use buzzer_service.h and MCAL/gptm.h instead"

#if 0  /* Old interface kept for reference only */

#include <stdint.h>

void timeout_init(void);
void activate_timeout(uint32_t seconds);
void cancel_timeout(void);

#endif /* Old interface */

#endif /* TIMEOUT_H_DEPRECATED */
