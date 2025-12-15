# Door Locker Security System

A two-MCU embedded security system using TM4C123GH6PM (Tiva C LaunchPad).

## Overview

| MCU                   | Role           | Components                          |
| --------------------- | -------------- | ----------------------------------- |
| Frontend (HMI_ECU)    | User Interface | LCD, Keypad, RGB LED, Potentiometer |
| Backend (Control_ECU) | Security Logic | Motor, Buzzer, EEPROM, Status LEDs  |

---

## System Flow

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   Welcome   │────>│   Signup    │────>│  Main Menu  │
│   Screen    │     │ (1st time)  │     │             │
└─────────────┘     └─────────────┘     └──────┬──────┘
                                               │
                         ┌─────────────────────┼─────────────────────┐
                         v                     v                     v
                   ┌──────────┐          ┌──────────┐          ┌──────────┐
                   │ Sign In  │          │  Change  │          │   Set    │
                   │   (A)    │          │ Pass (B) │          │ Time (C) │
                   └────┬─────┘          └──────────┘          └──────────┘
                        │
           ┌────────────┼────────────┐
           v            v            v
     ┌──────────┐ ┌──────────┐ ┌──────────┐
     │ Success  │ │  Wrong   │ │ Lockout  │
     │Door Opens│ │ Password │ │ (Buzzer) │
     └────┬─────┘ └──────────┘ └──────────┘
          │
          v
     ┌──────────┐     ┌──────────┐     ┌──────────┐
     │  Motor   │────>│ Countdown│────>│  Motor   │
     │ Forward  │     │ (5-30s)  │     │ Reverse  │
     │  3 sec   │     │          │     │  3 sec   │
     └──────────┘     └──────────┘     └──────────┘
```

---

## Menu Keys

| Key | Function                    |
| --- | --------------------------- |
| A   | Sign In (open door)         |
| B   | Change Password             |
| C   | Set Timeout (potentiometer) |
| D   | Save/Confirm                |
| #   | Cancel/Backspace            |
| 0-9 | Password digits             |

---

## Hardware Connections

### Frontend (HMI_ECU)

#### UART

| Signal | Pin | Description     |
| ------ | --- | --------------- |
| RX     | PB0 | From Backend TX |
| TX     | PB1 | To Backend RX   |

#### LCD (I2C)

| Signal | Pin | Description |
| ------ | --- | ----------- |
| SCL    | PB2 | I2C Clock   |
| SDA    | PB3 | I2C Data    |

#### Keypad Rows (Input with Pull-up)

| Signal | Pin |
| ------ | --- |
| Row 0  | PC4 |
| Row 1  | PC5 |
| Row 2  | PC6 |
| Row 3  | PC7 |

#### Keypad Columns (Output)

| Signal | Pin |
| ------ | --- |
| Col 0  | PB6 |
| Col 1  | PA4 |
| Col 2  | PA3 |
| Col 3  | PA2 |

#### Potentiometer

| Signal | Pin | Description |
| ------ | --- | ----------- |
| Signal | PB5 | ADC (AIN11) |

#### RGB LED (Onboard)

| Color | Pin | Meaning       |
| ----- | --- | ------------- |
| Red   | PF1 | Error/Lockout |
| Blue  | PF2 | Processing    |
| Green | PF3 | Success       |

---

### Backend (Control_ECU)

#### UART

| Signal | Pin | Description      |
| ------ | --- | ---------------- |
| RX     | PB0 | From Frontend TX |
| TX     | PB1 | To Frontend RX   |

#### Motor (via H-Bridge on J4)

| Signal        | Pin | Description   |
| ------------- | --- | ------------- |
| IN1 (Forward) | PC4 | Motor forward |
| IN2 (Reverse) | PC5 | Motor reverse |

#### Buzzer

| Signal | Pin | Description       |
| ------ | --- | ----------------- |
| Signal | PA5 | Active buzzer (+) |

#### Status LEDs (Onboard)

| Color | Pin | Meaning      |
| ----- | --- | ------------ |
| Red   | PF1 | Door closing |
| Blue  | PF2 | Debug        |
| Green | PF3 | Door opening |

---

## UART Protocol

**Baud Rate:** 115200, 8N1  
**System Clock:** 16 MHz (DO NOT CHANGE)

### Packet Format

```
Request:  [SOF=0x7E] [LEN] [CMD] [PAYLOAD...]
Response: [SOF=0xFE] [LEN] [CMD] [STATUS] [DATA...]
```

### Commands

| CMD  | Name            | Payload         | Response         | Description        |
| ---- | --------------- | --------------- | ---------------- | ------------------ |
| 0x01 | INIT_PASSWORD   | 5 digits        | STATUS           | Create password    |
| 0x02 | AUTH            | MODE + 5 digits | STATUS           | Authenticate       |
| 0x03 | SET_TIMEOUT     | SECONDS         | STATUS           | Set timeout (5-30) |
| 0x04 | CHANGE_PASSWORD | 5 digits        | STATUS           | Change password    |
| 0x05 | GET_TIMEOUT     | -               | STATUS + TIMEOUT | Get timeout value  |
| 0x06 | CLOSE_DOOR      | -               | STATUS           | Motor reverse 3s   |
| 0x07 | OPEN_DOOR       | -               | STATUS           | Motor forward 3s   |

### Status Codes

| Code | Name      | Description    |
| ---- | --------- | -------------- |
| 0x00 | OK        | Success        |
| 0x01 | ERROR     | General error  |
| 0x02 | AUTH_FAIL | Wrong password |

### Auth Modes (CMD 0x02)

| Mode | Description                                  |
| ---- | -------------------------------------------- |
| 0x00 | Check only (for change password/set timeout) |
| 0x01 | Open door (sets pending_open_door flag)      |

---

## Door Open Sequence

```
Frontend                          Backend
   │                                 │
   │──── AUTH (mode=1) ─────────────>│ Verify password
   │<─── STATUS_OK ──────────────────│ Set pending_open_door
   │                                 │
   │──── GET_TIMEOUT ───────────────>│ Return timeout
   │<─── STATUS_OK + TIMEOUT ────────│ Clear pending_open_door
   │                                 │
   │──── OPEN_DOOR ─────────────────>│ Respond immediately
   │<─── STATUS_OK ──────────────────│ Motor FORWARD 3 sec
   │                                 │ (Green LED)
   │     [Wait 3.5 sec]              │
   │     [Show countdown]            │
   │                                 │
   │──── CLOSE_DOOR ────────────────>│ Respond immediately
   │<─── STATUS_OK ──────────────────│ Motor REVERSE 3 sec
   │                                 │ (Red LED)
   │     [Wait 3.5 sec]              │
   │                                 │
```

---

## Lockout Sequence

Triggered after 3 wrong password attempts:

```
Frontend                          Backend
   │                                 │
   │──── GET_TIMEOUT ───────────────>│ No pending_open_door
   │<─── STATUS_OK + TIMEOUT ────────│ Activate BUZZER
   │                                 │
   │     [Show lockout countdown]    │ [Buzzer sounds]
   │     [Red LED blinks]            │
   │                                 │
```

---

## LED Indicators

### Frontend RGB LED

| Color  | Meaning                       |
| ------ | ----------------------------- |
| Blue   | System starting/Settings mode |
| Yellow | Processing/Verifying          |
| Green  | Success/Access granted        |
| Red    | Error/Wrong password          |
| Cyan   | Timeout adjustment mode       |

### Backend LEDs

| LED         | Meaning                      |
| ----------- | ---------------------------- |
| Green (PF3) | Motor forward (door opening) |
| Red (PF1)   | Motor reverse (door closing) |
| Blue (PF2)  | Debug (blinks on commands)   |

---

## Timing

| Operation                  | Duration                       |
| -------------------------- | ------------------------------ |
| Door Open (motor forward)  | 3 seconds                      |
| Door Close (motor reverse) | 3 seconds                      |
| Door Open Countdown        | 5-30 seconds (configurable)    |
| Lockout Duration           | 5-30 seconds (same as timeout) |

---

## Project Structure

```
Door-Locker-Embedded-System/
├── frontend/
│   ├── main.c
│   ├── frontend.c/h          # State machine, UI logic
│   ├── uart_comm.c/h         # UART communication
│   ├── components/
│   │   ├── lcd.c/h
│   │   ├── keypad.c/h
│   │   ├── led.c/h
│   │   └── potentiometer.c/h
│   └── MCAL/
│
├── backend/
│   ├── main.c
│   ├── uart_handler.c/h      # UART protocol, commands
│   ├── eeprom_handler.c/h    # Password & timeout storage
│   ├── components/
│   │   ├── motor.c/h
│   │   ├── buzzer.c/h
│   │   └── timeout.c/h
│   └── MCAL/
│
├── SYSTEM_DOCUMENTATION.txt  # Full system documentation
├── data sheet frontend.txt   # Frontend wiring details
└── README.md                 # This file
```

---

## Quick Start

1. **Wire both LaunchPads** as shown in hardware tables
2. **Cross-connect UART:** Frontend TX → Backend RX, Frontend RX → Backend TX
3. **Common ground** between both boards
4. **Power both boards** via USB
5. **First boot:** Create 5-digit password
6. **Use menu:** A=Sign In, B=Change Password, C=Set Timeout

---

## Documentation Files

| File                             | Description                     |
| -------------------------------- | ------------------------------- |
| `SYSTEM_DOCUMENTATION.txt`       | Complete system documentation   |
| `data sheet frontend.txt`        | Frontend pin assignments/wiring |
| `backend/data sheet backend.txt` | Backend pin assignments/wiring  |
| `frontend/WIRING_GUIDE.txt`      | Frontend wiring checklist       |
