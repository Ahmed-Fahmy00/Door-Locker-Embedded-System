/******************************************************************************
 * File: dio.c
 * Module: DIO (Digital Input/Output)
 * Description: Source file for TM4C123GH6PM GPIO Driver
 ******************************************************************************/

#include "dio.h"
#include "../lib/tm4c123gh6pm.h"

/******************************************************************************
 * Macros
 ******************************************************************************/

#define GPIO_LOCK_KEY           0x4C4F434B

/* Helper macros to get register address based on port (0-5) */
#define GET_GPIO_DATA(port)   ((port) == 0 ? &GPIO_PORTA_DATA_R : \
                               (port) == 1 ? &GPIO_PORTB_DATA_R : \
                               (port) == 2 ? &GPIO_PORTC_DATA_R : \
                               (port) == 3 ? &GPIO_PORTD_DATA_R : \
                               (port) == 4 ? &GPIO_PORTE_DATA_R : \
                               &GPIO_PORTF_DATA_R)

#define GET_GPIO_DIR(port)    ((port) == 0 ? &GPIO_PORTA_DIR_R : \
                               (port) == 1 ? &GPIO_PORTB_DIR_R : \
                               (port) == 2 ? &GPIO_PORTC_DIR_R : \
                               (port) == 3 ? &GPIO_PORTD_DIR_R : \
                               (port) == 4 ? &GPIO_PORTE_DIR_R : \
                               &GPIO_PORTF_DIR_R)

#define GET_GPIO_DEN(port)    ((port) == 0 ? &GPIO_PORTA_DEN_R : \
                               (port) == 1 ? &GPIO_PORTB_DEN_R : \
                               (port) == 2 ? &GPIO_PORTC_DEN_R : \
                               (port) == 3 ? &GPIO_PORTD_DEN_R : \
                               (port) == 4 ? &GPIO_PORTE_DEN_R : \
                               &GPIO_PORTF_DEN_R)

#define GET_GPIO_AFSEL(port)  ((port) == 0 ? &GPIO_PORTA_AFSEL_R : \
                               (port) == 1 ? &GPIO_PORTB_AFSEL_R : \
                               (port) == 2 ? &GPIO_PORTC_AFSEL_R : \
                               (port) == 3 ? &GPIO_PORTD_AFSEL_R : \
                               (port) == 4 ? &GPIO_PORTE_AFSEL_R : \
                               &GPIO_PORTF_AFSEL_R)

#define GET_GPIO_PUR(port)    ((port) == 0 ? &GPIO_PORTA_PUR_R : \
                               (port) == 1 ? &GPIO_PORTB_PUR_R : \
                               (port) == 2 ? &GPIO_PORTC_PUR_R : \
                               (port) == 3 ? &GPIO_PORTD_PUR_R : \
                               (port) == 4 ? &GPIO_PORTE_PUR_R : \
                               &GPIO_PORTF_PUR_R)

#define GET_GPIO_PDR(port)    ((port) == 0 ? &GPIO_PORTA_PDR_R : \
                               (port) == 1 ? &GPIO_PORTB_PDR_R : \
                               (port) == 2 ? &GPIO_PORTC_PDR_R : \
                               (port) == 3 ? &GPIO_PORTD_PDR_R : \
                               (port) == 4 ? &GPIO_PORTE_PDR_R : \
                               &GPIO_PORTF_PDR_R)

#define GET_GPIO_LOCK(port)   ((port) == 0 ? &GPIO_PORTA_LOCK_R : \
                               (port) == 1 ? &GPIO_PORTB_LOCK_R : \
                               (port) == 2 ? &GPIO_PORTC_LOCK_R : \
                               (port) == 3 ? &GPIO_PORTD_LOCK_R : \
                               (port) == 4 ? &GPIO_PORTE_LOCK_R : \
                               &GPIO_PORTF_LOCK_R)

#define GET_GPIO_CR(port)     ((port) == 0 ? &GPIO_PORTA_CR_R : \
                               (port) == 1 ? &GPIO_PORTB_CR_R : \
                               (port) == 2 ? &GPIO_PORTC_CR_R : \
                               (port) == 3 ? &GPIO_PORTD_CR_R : \
                               (port) == 4 ? &GPIO_PORTE_CR_R : \
                               &GPIO_PORTF_CR_R)

/******************************************************************************
 * Function Implementations
 ******************************************************************************/

void DIO_Init(uint8_t port, uint8_t pin, uint8_t direction) {
    volatile uint32_t delay;
    SYSCTL_RCGCGPIO_R |= (1 << port);
    delay = SYSCTL_RCGCGPIO_R;
    delay = SYSCTL_RCGCGPIO_R;

    *GET_GPIO_LOCK(port) = GPIO_LOCK_KEY;
    *GET_GPIO_CR(port) |= (1 << pin);
    *GET_GPIO_AFSEL(port) &= ~(1 << pin);
    
    if (port == 2 || port == 3) {
        volatile unsigned long *amsel = (port == 2) ? 
            (volatile unsigned long *)0x40006528 :
            (volatile unsigned long *)0x40007528;
        *amsel &= ~(1 << pin);
    }

    if (direction) {
        *GET_GPIO_DIR(port) |= (1 << pin);
    } else {
        *GET_GPIO_DIR(port) &= ~(1 << pin);
    }
    
    *GET_GPIO_DEN(port) |= (1 << pin);
    *GET_GPIO_LOCK(port) = 0;
}

void DIO_WritePin(uint8_t port, uint8_t pin, uint8_t value) {
    if (value) {
        *GET_GPIO_DATA(port) |= (1 << pin);
    } else {
        *GET_GPIO_DATA(port) &= ~(1 << pin);
    }
}

uint8_t DIO_ReadPin(uint8_t port, uint8_t pin) {
    return ((*GET_GPIO_DATA(port) & (1 << pin)) != 0);
}

void DIO_TogglePin(uint8_t port, uint8_t pin) {
    *GET_GPIO_DATA(port) ^= (1 << pin);
}

void DIO_SetPUR(uint8_t port, uint8_t pin, uint8_t enable) {
    if (enable) {
        *GET_GPIO_PUR(port) |= (1 << pin);
    } else {
        *GET_GPIO_PUR(port) &= ~(1 << pin);
    }
}

void DIO_SetPDR(uint8_t port, uint8_t pin, uint8_t enable) {
    if (enable) {
        *GET_GPIO_PDR(port) |= (1 << pin);
    } else {
        *GET_GPIO_PDR(port) &= ~(1 << pin);
    }
}
