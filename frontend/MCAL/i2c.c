/******************************************************************************
 * File: i2c.c
 * Module: I2C (Inter-Integrated Circuit)
 * Description: Source file for TM4C123GH6PM I2C Driver (MCAL Layer)
 ******************************************************************************/

#include "i2c.h"
#include "../lib/tm4c123gh6pm.h"

/******************************************************************************
 *                              Macros                                         *
 ******************************************************************************/

/* I2C Master Control/Status Register Bits */
#define I2C_MCS_BUSY    0x01  /* I2C Busy */
#define I2C_MCS_ERROR   0x02  /* Error */
#define I2C_MCS_ADRACK  0x04  /* Acknowledge Address */
#define I2C_MCS_DATACK  0x08  /* Acknowledge Data */
#define I2C_MCS_ARBLST  0x10  /* Arbitration Lost */
#define I2C_MCS_IDLE    0x20  /* I2C Idle */
#define I2C_MCS_BUSBSY  0x40  /* Bus Busy */
#define I2C_MCS_CLKTO   0x80  /* Clock Timeout Error */

/* I2C Master Control/Status Commands */
#define I2C_MCS_RUN     0x01  /* I2C Master Enable */
#define I2C_MCS_START   0x02  /* Generate START */
#define I2C_MCS_STOP    0x04  /* Generate STOP */
#define I2C_MCS_ACK     0x08  /* Data Acknowledge Enable */

/* Combined commands for common operations */
#define I2C_MASTER_CMD_SINGLE_SEND      (I2C_MCS_RUN | I2C_MCS_START | I2C_MCS_STOP)
#define I2C_MASTER_CMD_SINGLE_RECEIVE   (I2C_MCS_RUN | I2C_MCS_START | I2C_MCS_STOP)
#define I2C_MASTER_CMD_BURST_SEND_START (I2C_MCS_RUN | I2C_MCS_START)
#define I2C_MASTER_CMD_BURST_SEND_CONT  (I2C_MCS_RUN)
#define I2C_MASTER_CMD_BURST_SEND_FINISH (I2C_MCS_RUN | I2C_MCS_STOP)

/* System Clock Frequency (Hz) */
#define SYSTEM_CLOCK_FREQ   16000000UL

/* GPIO Configuration */
#define GPIO_LOCK_KEY       0x4C4F434B

/******************************************************************************
 *                         Private Function Prototypes                         *
 ******************************************************************************/

static void I2C_WaitBusy(uint8_t module);
static volatile uint32_t* I2C_GetMCRReg(uint8_t module);
static volatile uint32_t* I2C_GetMTPRReg(uint8_t module);
static volatile uint32_t* I2C_GetMSAReg(uint8_t module);
static volatile uint32_t* I2C_GetMDRReg(uint8_t module);
static volatile uint32_t* I2C_GetMCSReg(uint8_t module);

/******************************************************************************
 *                         Private Functions                                   *
 ******************************************************************************/

static void I2C_WaitBusy(uint8_t module) {
    volatile uint32_t *mcsReg = I2C_GetMCSReg(module);
    while (*mcsReg & I2C_MCS_BUSY);
}

static volatile uint32_t* I2C_GetMCRReg(uint8_t module) {
    switch(module) {
        case I2C_MODULE_0: return &I2C0_MCR_R;
        case I2C_MODULE_1: return &I2C1_MCR_R;
        case I2C_MODULE_2: return &I2C2_MCR_R;
        case I2C_MODULE_3: return &I2C3_MCR_R;
        default: return &I2C0_MCR_R;
    }
}

static volatile uint32_t* I2C_GetMTPRReg(uint8_t module) {
    switch(module) {
        case I2C_MODULE_0: return &I2C0_MTPR_R;
        case I2C_MODULE_1: return &I2C1_MTPR_R;
        case I2C_MODULE_2: return &I2C2_MTPR_R;
        case I2C_MODULE_3: return &I2C3_MTPR_R;
        default: return &I2C0_MTPR_R;
    }
}

static volatile uint32_t* I2C_GetMSAReg(uint8_t module) {
    switch(module) {
        case I2C_MODULE_0: return &I2C0_MSA_R;
        case I2C_MODULE_1: return &I2C1_MSA_R;
        case I2C_MODULE_2: return &I2C2_MSA_R;
        case I2C_MODULE_3: return &I2C3_MSA_R;
        default: return &I2C0_MSA_R;
    }
}

static volatile uint32_t* I2C_GetMDRReg(uint8_t module) {
    switch(module) {
        case I2C_MODULE_0: return &I2C0_MDR_R;
        case I2C_MODULE_1: return &I2C1_MDR_R;
        case I2C_MODULE_2: return &I2C2_MDR_R;
        case I2C_MODULE_3: return &I2C3_MDR_R;
        default: return &I2C0_MDR_R;
    }
}

static volatile uint32_t* I2C_GetMCSReg(uint8_t module) {
    switch(module) {
        case I2C_MODULE_0: return &I2C0_MCS_R;
        case I2C_MODULE_1: return &I2C1_MCS_R;
        case I2C_MODULE_2: return &I2C2_MCS_R;
        case I2C_MODULE_3: return &I2C3_MCS_R;
        default: return &I2C0_MCS_R;
    }
}

/******************************************************************************
 *                         Public Functions                                    *
 ******************************************************************************/

void I2C_Init(uint8_t module, uint32_t speed) {
    volatile uint32_t delay;
    uint8_t port;
    uint8_t sclPin, sdaPin;
    
    /* Enable I2C module clock */
    SYSCTL_RCGCI2C_R |= (1 << module);
    delay = SYSCTL_RCGCI2C_R; /* Wait for clock to stabilize */
    
    /* Determine GPIO port and pins based on I2C module */
    switch(module) {
        case I2C_MODULE_0: /* PB2 (SCL), PB3 (SDA) */
            port = 1; /* Port B */
            sclPin = 2;
            sdaPin = 3;
            break;
        case I2C_MODULE_1: /* PA6 (SCL), PA7 (SDA) */
            port = 0; /* Port A */
            sclPin = 6;
            sdaPin = 7;
            break;
        case I2C_MODULE_2: /* PE4 (SCL), PE5 (SDA) */
            port = 4; /* Port E */
            sclPin = 4;
            sdaPin = 5;
            break;
        case I2C_MODULE_3: /* PD0 (SCL), PD1 (SDA) */
            port = 3; /* Port D */
            sclPin = 0;
            sdaPin = 1;
            break;
        default:
            return;
    }
    
    /* Enable GPIO clock for the corresponding port */
    SYSCTL_RCGCGPIO_R |= (1 << port);
    delay = SYSCTL_RCGCGPIO_R;
    
    /* Wait for peripheral to be ready */
    while ((SYSCTL_PRGPIO_R & (1 << port)) == 0);
    
    /* Configure GPIO pins for I2C */
    switch(port) {
        case 0: /* Port A */
            GPIO_PORTA_AFSEL_R |= (1 << sclPin) | (1 << sdaPin);
            GPIO_PORTA_ODR_R |= (1 << sdaPin);  /* Open drain on SDA */
            GPIO_PORTA_DEN_R |= (1 << sclPin) | (1 << sdaPin);
            GPIO_PORTA_PCTL_R &= ~(0xFF << (sclPin * 4));
            GPIO_PORTA_PCTL_R |= (0x33 << (sclPin * 4)); /* I2C function */
            break;
        case 1: /* Port B */
            GPIO_PORTB_AFSEL_R |= (1 << sclPin) | (1 << sdaPin);
            GPIO_PORTB_ODR_R |= (1 << sdaPin);  /* Open drain on SDA */
            GPIO_PORTB_DEN_R |= (1 << sclPin) | (1 << sdaPin);
            GPIO_PORTB_PCTL_R &= ~(0xFF << (sclPin * 4));
            GPIO_PORTB_PCTL_R |= (0x33 << (sclPin * 4)); /* I2C function */
            break;
        case 3: /* Port D */
            GPIO_PORTD_LOCK_R = GPIO_LOCK_KEY;
            GPIO_PORTD_CR_R |= (1 << sclPin) | (1 << sdaPin);
            GPIO_PORTD_AFSEL_R |= (1 << sclPin) | (1 << sdaPin);
            GPIO_PORTD_ODR_R |= (1 << sdaPin);  /* Open drain on SDA */
            GPIO_PORTD_DEN_R |= (1 << sclPin) | (1 << sdaPin);
            GPIO_PORTD_PCTL_R &= ~(0xFF << (sclPin * 4));
            GPIO_PORTD_PCTL_R |= (0x33 << (sclPin * 4)); /* I2C function */
            break;
        case 4: /* Port E */
            GPIO_PORTE_AFSEL_R |= (1 << sclPin) | (1 << sdaPin);
            GPIO_PORTE_ODR_R |= (1 << sdaPin);  /* Open drain on SDA */
            GPIO_PORTE_DEN_R |= (1 << sclPin) | (1 << sdaPin);
            GPIO_PORTE_PCTL_R &= ~(0xFF << (sclPin * 4));
            GPIO_PORTE_PCTL_R |= (0x33 << (sclPin * 4)); /* I2C function */
            break;
    }
    
    /* Configure I2C Master */
    volatile uint32_t *mcrReg = I2C_GetMCRReg(module);
    volatile uint32_t *mtprReg = I2C_GetMTPRReg(module);
    
    *mcrReg = 0x10; /* Enable I2C Master function */
    
    /* Calculate and set Timer Period
     * TPR = (System Clock / (2 * SCL_LP * SCL_CLOCK)) - 1
     * Where SCL_LP = 10 (for standard mode)
     */
    uint32_t tpr = (SYSTEM_CLOCK_FREQ / (2 * 10 * speed)) - 1;
    *mtprReg = tpr;
}

uint8_t I2C_WriteByte(uint8_t module, uint8_t slaveAddr, uint8_t data) {
    volatile uint32_t *msaReg = I2C_GetMSAReg(module);
    volatile uint32_t *mdrReg = I2C_GetMDRReg(module);
    volatile uint32_t *mcsReg = I2C_GetMCSReg(module);
    
    /* Wait for I2C to be idle */
    I2C_WaitBusy(module);
    
    /* Set slave address for write operation */
    *msaReg = (slaveAddr << 1) & 0xFE; /* Clear R/S bit for write */
    
    /* Place data in data register */
    *mdrReg = data;
    
    /* Initiate single byte transmission (START + RUN + STOP) */
    *mcsReg = I2C_MASTER_CMD_SINGLE_SEND;
    
    /* Wait for transmission to complete */
    I2C_WaitBusy(module);
    
    /* Check for errors */
    if (*mcsReg & I2C_MCS_ERROR) {
        *mcsReg = I2C_MCS_STOP; /* Send STOP condition */
        return I2C_ERROR;
    }
    
    return I2C_SUCCESS;
}

uint8_t I2C_WriteMultipleBytes(uint8_t module, uint8_t slaveAddr, 
                                const uint8_t *data, uint8_t length) {
    volatile uint32_t *msaReg = I2C_GetMSAReg(module);
    volatile uint32_t *mdrReg = I2C_GetMDRReg(module);
    volatile uint32_t *mcsReg = I2C_GetMCSReg(module);
    uint8_t i;
    
    if (length == 0) return I2C_ERROR;
    
    /* Wait for I2C to be idle */
    I2C_WaitBusy(module);
    
    /* Set slave address for write operation */
    *msaReg = (slaveAddr << 1) & 0xFE;
    
    /* Send first byte with START condition */
    *mdrReg = data[0];
    *mcsReg = I2C_MASTER_CMD_BURST_SEND_START;
    I2C_WaitBusy(module);
    
    if (*mcsReg & I2C_MCS_ERROR) {
        *mcsReg = I2C_MCS_STOP;
        return I2C_ERROR;
    }
    
    /* Send middle bytes */
    for (i = 1; i < length - 1; i++) {
        *mdrReg = data[i];
        *mcsReg = I2C_MASTER_CMD_BURST_SEND_CONT;
        I2C_WaitBusy(module);
        
        if (*mcsReg & I2C_MCS_ERROR) {
            *mcsReg = I2C_MCS_STOP;
            return I2C_ERROR;
        }
    }
    
    /* Send last byte with STOP condition */
    *mdrReg = data[length - 1];
    *mcsReg = I2C_MASTER_CMD_BURST_SEND_FINISH;
    I2C_WaitBusy(module);
    
    if (*mcsReg & I2C_MCS_ERROR) {
        *mcsReg = I2C_MCS_STOP;
        return I2C_ERROR;
    }
    
    return I2C_SUCCESS;
}

uint8_t I2C_ReadByte(uint8_t module, uint8_t slaveAddr, uint8_t *data) {
    volatile uint32_t *msaReg = I2C_GetMSAReg(module);
    volatile uint32_t *mdrReg = I2C_GetMDRReg(module);
    volatile uint32_t *mcsReg = I2C_GetMCSReg(module);
    
    /* Wait for I2C to be idle */
    I2C_WaitBusy(module);
    
    /* Set slave address for read operation */
    *msaReg = (slaveAddr << 1) | 0x01; /* Set R/S bit for read */
    
    /* Initiate single byte reception (START + RUN + STOP) */
    *mcsReg = I2C_MASTER_CMD_SINGLE_RECEIVE;
    
    /* Wait for reception to complete */
    I2C_WaitBusy(module);
    
    /* Check for errors */
    if (*mcsReg & I2C_MCS_ERROR) {
        *mcsReg = I2C_MCS_STOP;
        return I2C_ERROR;
    }
    
    /* Read received data */
    *data = *mdrReg & 0xFF;
    
    return I2C_SUCCESS;
}

uint8_t I2C_ReadMultipleBytes(uint8_t module, uint8_t slaveAddr, 
                               uint8_t *data, uint8_t length) {
    volatile uint32_t *msaReg = I2C_GetMSAReg(module);
    volatile uint32_t *mdrReg = I2C_GetMDRReg(module);
    volatile uint32_t *mcsReg = I2C_GetMCSReg(module);
    uint8_t i;
    
    if (length == 0) return I2C_ERROR;
    
    /* Wait for I2C to be idle */
    I2C_WaitBusy(module);
    
    /* Set slave address for read operation */
    *msaReg = (slaveAddr << 1) | 0x01;
    
    /* Receive first byte with START and ACK */
    *mcsReg = (I2C_MCS_RUN | I2C_MCS_START | I2C_MCS_ACK);
    I2C_WaitBusy(module);
    
    if (*mcsReg & I2C_MCS_ERROR) {
        *mcsReg = I2C_MCS_STOP;
        return I2C_ERROR;
    }
    
    data[0] = *mdrReg & 0xFF;
    
    /* Receive middle bytes with ACK */
    for (i = 1; i < length - 1; i++) {
        *mcsReg = (I2C_MCS_RUN | I2C_MCS_ACK);
        I2C_WaitBusy(module);
        
        if (*mcsReg & I2C_MCS_ERROR) {
            *mcsReg = I2C_MCS_STOP;
            return I2C_ERROR;
        }
        
        data[i] = *mdrReg & 0xFF;
    }
    
    /* Receive last byte with STOP (no ACK) */
    *mcsReg = (I2C_MCS_RUN | I2C_MCS_STOP);
    I2C_WaitBusy(module);
    
    if (*mcsReg & I2C_MCS_ERROR) {
        *mcsReg = I2C_MCS_STOP;
        return I2C_ERROR;
    }
    
    data[length - 1] = *mdrReg & 0xFF;
    
    return I2C_SUCCESS;
}

uint8_t I2C_IsBusy(uint8_t module) {
    volatile uint32_t *mcsReg = I2C_GetMCSReg(module);
    return (*mcsReg & I2C_MCS_BUSY) ? 1 : 0;
}
