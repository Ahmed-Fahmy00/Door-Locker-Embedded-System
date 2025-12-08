# Door Locker Security System

## TM4C123GH6PM - HMI_ECU (Frontend)

---

## System Flow

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   Welcome   │────>│   Signup    │────>│  Main Menu  │
│   Screen    │     │ (1st time)  │     │             │
└─────────────┘     └─────────────┘     └──────┬──────┘
                           │                   │
                    (if password exists)       │
                           │         ┌─────────┼─────────┐
                           v         v         v         v
                    ┌──────────┐ ┌────────┐ ┌────────┐ ┌────────┐
                    │  Signin  │ │ Change │ │  Set   │ │ Cancel │
                    │   (A)    │ │Pass (B)│ │Time (C)│ │  (#)   │
                    └────┬─────┘ └────────┘ └────────┘ └────────┘
                         │
            ┌────────────┼────────────┐
            v            v            v
      ┌──────────┐ ┌──────────┐ ┌──────────┐
      │ Correct  │ │  Wrong   │ │ Lockout  │
      │Door Opens│ │Try Again │ │(Buzzer)  │
      └──────────┘ └──────────┘ └──────────┘
```

---

## Menu Keys

| Key | Function                             |
| --- | ------------------------------------ |
| A   | Sign In (verify password, open door) |
| B   | Change Password                      |
| C   | Set Timeout (using potentiometer)    |
| D   | Save (confirm action)                |
| #   | Cancel                               |
| 0-9 | Password digits                      |

---

## Hardware Connections

### Pin Mapping

```
┌─────────────────────────────────────────────────────────┐
│                    TM4C123GH6PM                         │
│                                                         │
│  Potentiometer ──── PE3 (ADC)                           │
│                                                         │
│  LCD (I2C) ──────── PB2 (SCL)                           │
│                     PB3 (SDA)                           │
│                                                         │
│  Keypad Rows ────── PC4 (ROW0)                          │
│                     PC5 (ROW1)                          │
│                     PC6 (ROW2)                          │
│                     PC7 (ROW3)                          │
│                                                         │
│  Keypad Cols ────── PB4 (COL0)                          │
│                     PB5 (COL1)                          │
│                     PB6 (COL2)                          │
│                     PB7 (COL3)                          │
│                                                         │
│  UART1 ──────────── PB0 (Rx) ←── Control_ECU Tx         │
│  (to Backend)       PB1 (Tx) ──→ Control_ECU Rx         │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

---

## Component Wiring Table

| Component         | Pin  | Port | Type      | Notes                   |
| ----------------- | ---- | ---- | --------- | ----------------------- |
| **Potentiometer** |      |      |           |                         |
| Signal            | PE3  | E    | Analog In | ADC0 AIN0               |
| VCC               | 3.3V | -    | Power     |                         |
| GND               | GND  | -    | Ground    |                         |
| **LCD (I2C)**     |      |      |           |                         |
| SCL               | PB2  | B    | I2C Clock |                         |
| SDA               | PB3  | B    | I2C Data  |                         |
| VCC               | VBUS | -    | 5V Power  |                         |
| GND               | GND  | -    | Ground    |                         |
| **4x4 Keypad**    |      |      |           |                         |
| ROW0              | PC4  | C    | Input     | Pull-up                 |
| ROW1              | PC5  | C    | Input     | Pull-up                 |
| ROW2              | PC6  | C    | Input     | Pull-up                 |
| ROW3              | PC7  | C    | Input     | Pull-up                 |
| COL0              | PB4  | B    | Output    |                         |
| COL1              | PB5  | B    | Output    |                         |
| COL2              | PB6  | B    | Output    |                         |
| COL3              | PB7  | B    | Output    |                         |
| **UART1**         |      |      |           |                         |
| Rx                | PB0  | B    | UART Rx   | From Control_ECU Tx     |
| Tx                | PB1  | B    | UART Tx   | To Control_ECU Rx       |
| GND               | GND  | -    | Ground    | Shared with Control_ECU |

---

## Keypad Layout

```
        COL0    COL1    COL2    COL3
        (PB4)   (PB5)   (PB6)   (PB7)
         │       │       │       │
ROW0 ────┼───────┼───────┼───────┼──── PC4
        [1]     [2]     [3]     [A]
         │       │       │       │
ROW1 ────┼───────┼───────┼───────┼──── PC5
        [4]     [5]     [6]     [B]
         │       │       │       │
ROW2 ────┼───────┼───────┼───────┼──── PC6
        [7]     [8]     [9]     [C]
         │       │       │       │
ROW3 ────┼───────┼───────┼───────┼──── PC7
        [*]     [0]     [#]     [D]
```

---

## UART Protocol (Frontend <-> Backend)

### Commands (Frontend -> Backend)

| Code | Command          | Data Sent | Description              |
| ---- | ---------------- | --------- | ------------------------ |
| 0x10 | SAVE_PASSWORD    | 5 bytes   | Save password to EEPROM  |
| 0x11 | VERIFY_PASSWORD  | 5 bytes   | Check password (signin)  |
| 0x12 | CHANGE_PASSWORD  | 10 bytes  | Old + New password       |
| 0x13 | SAVE_TIMEOUT     | 1 byte    | Save timeout (5-30 sec)  |
| 0x14 | GET_TIMEOUT      | -         | Get current timeout      |
| 0x15 | CHECK_FIRST_TIME | -         | Check if password exists |

### Responses (Backend -> Frontend)

| Code | Response         | Description                  |
| ---- | ---------------- | ---------------------------- |
| 0x20 | OK               | Success                      |
| 0x21 | PASSWORD_CORRECT | Door opening                 |
| 0x22 | PASSWORD_WRONG   | Incorrect password           |
| 0x23 | LOCKOUT          | Too many attempts, buzzer on |
| 0x24 | DOOR_CLOSED      | Door has closed              |
| 0x25 | FIRST_TIME       | No password, need signup     |
| 0x26 | NOT_FIRST_TIME   | Password exists              |

---

## Wire Count

| Connection    | Wires                  |
| ------------- | ---------------------- |
| Potentiometer | 3 (VCC, GND, Signal)   |
| LCD (I2C)     | 4 (VCC, GND, SDA, SCL) |
| Keypad        | 8 (4 rows + 4 cols)    |
| UART          | 3 (Tx, Rx, GND)        |
| **Total**     | **18 wires**           |

---

## Quick Start

1. Connect all components as shown above
2. Connect UART to Control_ECU (crossover: Tx->Rx, Rx->Tx)
3. Power both Tiva boards
4. LCD shows "Door Locker Security System"
5. First time: Create 5-digit password
6. Use menu: A=Signin, B=Change Pass, C=Set Timeout
