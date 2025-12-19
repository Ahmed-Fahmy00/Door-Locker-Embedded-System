# Backend Architecture Documentation

## Overview
This document describes the layered architecture of the door locker backend system.

## Architecture Layers

### 1. MCAL Layer (Microcontroller Abstraction Layer)
**Location:** `MCAL/`

**Purpose:** Low-level hardware abstraction for microcontroller peripherals.

**Components:**
- **gptm.c/h** - General Purpose Timer Module
  - Timer0 (used by buzzer service) - Full 32-bit mode
  - Timer1 (used by door controller) - Full 32-bit mode
  - Provides one-shot timer functionality
  - No callbacks - handlers registered in startup file

- **dio.c/h** - Digital I/O control
- **systick.c/h** - System tick timer

### 2. HAL Layer (Hardware Abstraction Layer)
**Location:** `HAL/`

**Purpose:** Device-specific drivers that use MCAL primitives.

**Components:**
- **motor.c/h** - Motor GPIO control
  - Motor_Init() - Initialize motor pins
  - Motor_RotateCW() - Clockwise rotation (door opens)
  - Motor_RotateCCW() - Counter-clockwise rotation (door closes)
  - Motor_Stop() - Stop motor

- **buzzer.c/h** - Buzzer GPIO control
  - buzzer_init() - Initialize buzzer pin
  - buzzer_on() - Turn buzzer on
  - buzzer_off() - Turn buzzer off

- **timer.c/h** - ⚠️ DEPRECATED (use door_controller instead)
- **timeout.c/h** - ⚠️ DEPRECATED (use buzzer_service instead)

### 3. Application Layer
**Location:** `application/`

**Purpose:** High-level business logic and state machines.

**Components:**
- **door_controller.c/h** - Door automation service
- **buzzer_service.c/h** - Buzzer timeout service
- **uart_handler.c/h** - UART communication protocol
- **eeprom_handler.c/h** - Password & configuration storage

## File Organization

```
backend/
├── MCAL/                    # Microcontroller Abstraction Layer
│   ├── gptm.c/h            # Timer hardware (Timer0 & Timer1)
│   ├── dio.c/h             # Digital I/O
│   └── systick.c/h         # System tick
│
├── HAL/                     # Hardware Abstraction Layer
│   ├── motor.c/h           # Motor driver
│   ├── buzzer.c/h          # Buzzer driver
│   ├── timer.c/h           # ⚠️ DEPRECATED
│   └── timeout.c/h         # ⚠️ DEPRECATED
│
├── application/             # Application Layer
│   ├── door_controller.c/h
│   ├── buzzer_service.c/h
│   ├── uart_handler.c/h
│   └── eeprom_handler.c/h
│
└── main.c
```
