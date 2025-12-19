#ifndef FRONTEND_H
#define FRONTEND_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    STATE_WELCOME,
    STATE_SIGNUP,
    STATE_MAIN_MENU,
    STATE_SIGNIN,
    STATE_CHANGE_PASSWORD,
    STATE_SET_TIMEOUT,
    STATE_LOCKOUT
} Frontend_State_t;

void Frontend_Start(void);

#endif /* FRONTEND_H */