/*****************************************************************************
 * File: timeout.c (DEPRECATED - DO NOT USE)
 * Description: This file has been deprecated and replaced by:
 * 
 * - MCAL/gptm.c - Low-level timer hardware control (Timer0)
 * - buzzer_service.c - High-level buzzer control logic (Application Layer)
 * 
 * Please use the new architecture:
 *   Application Layer: buzzer_service.c/h
 *   HAL Layer:        buzzer.c/h (GPIO only)
 *   MCAL Layer:       gptm.c/h (Timer hardware)
 * 
 * Date: December 19, 2025
 *****************************************************************************/

#error "This file is deprecated. Use buzzer_service.c and MCAL/gptm.c instead"

#if 0  /* Old code kept for reference only */

#include "timeout.h"
#include "../gptm0_oneshot.h"
#include "buzzer.h"

void timeout_init(void)
{
    /* Initialize buzzer first */
    buzzer_init();
    gptm0_oneshot_init();
}

void activate_timeout(uint32_t seconds)
{
    /* Start buzzer for specified duration */
    gptm0_oneshot_start_seconds(seconds);
}

void cancel_timeout(void)
{
    gptm0_oneshot_stop();
}

#endif /* Old code */
