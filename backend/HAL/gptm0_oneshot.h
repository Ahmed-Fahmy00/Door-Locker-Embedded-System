#ifndef GPTM0_ONESHOT_H_
#define GPTM0_ONESHOT_H_

#include <stdint.h>
#include <stdbool.h>

void gptm0_oneshot_init(void);

/*
 * Start buzzer and keep it ON for `seconds`
 * Returns false if duration cannot fit timer
 */
bool gptm0_oneshot_start_seconds(uint32_t seconds);

void gptm0_oneshot_stop(void);

#endif
