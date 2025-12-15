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
