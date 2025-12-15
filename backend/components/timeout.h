#ifndef TIMEOUT_H_
#define TIMEOUT_H_

#include <stdint.h>

void timeout_init(void);
void activate_timeout(uint32_t seconds);
void cancel_timeout(void);

#endif
