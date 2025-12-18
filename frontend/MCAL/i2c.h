/******************************************************************************
 * File: i2c.h
 * Module: I2C (Inter-Integrated Circuit)
 * Description: Header file for TM4C123GH6PM I2C Driver (MCAL Layer)
 ******************************************************************************/

#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

/******************************************************************************
 *                              Definitions                                    *
 ******************************************************************************/

/* I2C Module Selection */
#define I2C_MODULE_0    0
#define I2C_MODULE_1    1
#define I2C_MODULE_2    2
#define I2C_MODULE_3    3

/* I2C Speed Configuration */
#define I2C_SPEED_100K  100000  /* Standard mode: 100 kHz */
#define I2C_SPEED_400K  400000  /* Fast mode: 400 kHz */

/* I2C Status Codes */
#define I2C_SUCCESS     0
#define I2C_ERROR       1
#define I2C_BUSY        2

/* Port/Pin Configuration for I2C Modules */
/* I2C0: PB2 (SCL), PB3 (SDA) */
/* I2C1: PA6 (SCL), PA7 (SDA) */
/* I2C2: PE4 (SCL), PE5 (SDA) */
/* I2C3: PD0 (SCL), PD1 (SDA) */

/******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/*
 * Description: Initialize I2C module with specified speed
 * Parameters:
 *   - module: I2C module number (0-3)
 *   - speed: I2C bus speed in Hz (e.g., I2C_SPEED_100K)
 */
void I2C_Init(uint8_t module, uint32_t speed);

/*
 * Description: Write a single byte to I2C slave device
 * Parameters:
 *   - module: I2C module number (0-3)
 *   - slaveAddr: 7-bit I2C slave address
 *   - data: Data byte to transmit
 * Returns: I2C_SUCCESS or I2C_ERROR
 */
uint8_t I2C_WriteByte(uint8_t module, uint8_t slaveAddr, uint8_t data);

/*
 * Description: Write multiple bytes to I2C slave device
 * Parameters:
 *   - module: I2C module number (0-3)
 *   - slaveAddr: 7-bit I2C slave address
 *   - data: Pointer to data buffer
 *   - length: Number of bytes to write
 * Returns: I2C_SUCCESS or I2C_ERROR
 */
uint8_t I2C_WriteMultipleBytes(uint8_t module, uint8_t slaveAddr, 
                                const uint8_t *data, uint8_t length);

/*
 * Description: Read a single byte from I2C slave device
 * Parameters:
 *   - module: I2C module number (0-3)
 *   - slaveAddr: 7-bit I2C slave address
 *   - data: Pointer to store received byte
 * Returns: I2C_SUCCESS or I2C_ERROR
 */
uint8_t I2C_ReadByte(uint8_t module, uint8_t slaveAddr, uint8_t *data);

/*
 * Description: Read multiple bytes from I2C slave device
 * Parameters:
 *   - module: I2C module number (0-3)
 *   - slaveAddr: 7-bit I2C slave address
 *   - data: Pointer to buffer for received data
 *   - length: Number of bytes to read
 * Returns: I2C_SUCCESS or I2C_ERROR
 */
uint8_t I2C_ReadMultipleBytes(uint8_t module, uint8_t slaveAddr, 
                               uint8_t *data, uint8_t length);

/*
 * Description: Check if I2C bus is busy
 * Parameters:
 *   - module: I2C module number (0-3)
 * Returns: 1 if busy, 0 if idle
 */
uint8_t I2C_IsBusy(uint8_t module);

#endif /* I2C_H_ */
