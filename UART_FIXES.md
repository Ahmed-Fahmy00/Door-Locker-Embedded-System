# UART Communication Fixes - December 19, 2025

## Problem
After implementing the 5 main UART commands, only `CMD_INIT_PASSWORD` was working. The other commands (`CMD_AUTH`, `CMD_SET_TIMEOUT`, `CMD_CHANGE_PASSWORD`, `CMD_TIMEOUT`) were returning "Comm Error" on the frontend.

## Root Causes
1. **Missing DoorController initialization** - `DoorController_Init()` was not called in backend's `main.c`
2. **UART timing issues** - Timeout values and delays were too short for reliable communication

---

## Changes Made

### Backend: `main.c`

**Added include:**
```c
#include "application/door_controller.h"
```

**Added initialization call:**
```c
set_default_auto_timeout();
BuzzerService_Init();
DoorController_Init();  // <-- ADDED
UART_Handler_Init();
```

---

### Frontend: `uart_comm.c`

**Increased timeout and retry parameters:**
```c
// Before:
#define UART_TIMEOUT_LOOPS   1000000
#define UART_SOF_SEARCH_MAX  50

// After:
#define UART_TIMEOUT_LOOPS   2000000
#define UART_SOF_SEARCH_MAX  100
```

**Increased delay after sending packet:**
```c
// Before:
DelayMs(10);  /* Give backend time to process */

// After:
DelayMs(50);  /* Give backend time to process */
```

**Increased retry delays:**
```c
// Before:
DelayMs(5);   // After failed send
DelayMs(20);  // Between retries

// After:
DelayMs(50);   // After failed send
DelayMs(100);  // Between retries
```

---

### Frontend: `frontend.c`

**Updated `handleSignin()` to display countdown:**

When authentication succeeds with `AUTH_MODE_OPEN_DOOR` (mode=1), the frontend now:
1. Receives the timeout value from the backend
2. Displays a countdown while the door is open
3. Shows "Door Closing..." message when countdown ends

```c
if (status == STATUS_OK) {
    attemptCount = 0;
    LED_Green();
    showMessage("Access Granted!", "Door Opening...");
    DelayMs(2000);
    
    /* Use received timeout for countdown (default to 10 if invalid) */
    if (timeout < 5 || timeout > 30) timeout = 10;
    
    /* Show countdown while door is open */
    showMessage("Door Open", "");
    while (timeout > 0) {
        LCD_SetCursor(1, 0);
        snprintf(buffer, sizeof(buffer), "Closing in: %2d s", timeout);
        LCD_WriteString(buffer);
        DelayMs(1000);
        timeout--;
    }
    
    /* Door closing */
    LED_Yellow();
    showMessage("Door Closing...", "Please Wait");
    DelayMs(2000);
    
    LED_Off();
    showMessage("Door Locked", "");
    DelayMs(1500);
    currentState = STATE_MAIN_MENU;
}
```

---

## Protocol Summary (5 Commands)

| Command | ID | Frontend → Backend | Backend → Frontend |
|---------|-----|-------------------|-------------------|
| INIT_PASSWORD | 0x01 | `[0x7E][6][0x01][5 ASCII chars]` | `[0xFE][2][0x01][status]` |
| AUTH | 0x02 | `[0x7E][7][0x02][mode][5 ASCII chars]` | `[0xFE][2 or 3][0x02][status][timeout?]` |
| SET_TIMEOUT | 0x03 | `[0x7E][2][0x03][seconds]` | `[0xFE][2][0x03][status]` |
| CHANGE_PASSWORD | 0x04 | `[0x7E][6][0x04][5 ASCII chars]` | `[0xFE][2][0x04][status]` |
| GET_TIMEOUT (Lockout) | 0x05 | `[0x7E][1][0x05]` | `[0xFE][3][0x05][status][timeout]` |

### Packet Format
- **Request:** `[SOF=0x7E] [LEN] [CMD] [PAYLOAD...]`
- **Response:** `[SOF=0xFE] [LEN] [CMD] [STATUS] [DATA...]`

### Status Codes
| Status | Value | Description |
|--------|-------|-------------|
| STATUS_OK | 0x00 | Success |
| STATUS_ERROR | 0x01 | General error |
| STATUS_AUTH_FAIL | 0x02 | Wrong password |
| STATUS_UNKNOWN_CMD | 0xFF | Communication timeout |

### Auth Modes
| Mode | Value | Description |
|------|-------|-------------|
| AUTH_MODE_CHECK_ONLY | 0x00 | Just verify password |
| AUTH_MODE_OPEN_DOOR | 0x01 | Verify + open door + return timeout |

---

## Hardware Configuration
- **UART1:** PB0 (RX), PB1 (TX)
- **Baud Rate:** 115200
- **Config:** 8N1 (8 data bits, no parity, 1 stop bit)
- **System Clock:** 16 MHz (both Tiva boards)
