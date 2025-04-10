/*	Made By Adrien
 *
 * Copyright (c) [2020] by InvenSense, Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *	Driver I2C ICM45605
 */

#ifndef GYRO_H
#define GYRO_H


#include "stm32f7xx_hal.h"

// Adresse I2C par défaut du capteur ICM45605
#define ICM45605_I2C_ADDRESS 0x68

// Timeout pour les opérations I2C
#define I2C_TIMEOUT 100

// Registres ICM45605
#define ICM45605_WHO_AM_I         0x75
#define ICM45605_DEVICE_ID        0x68  // Valeur attendue dans WHO_AM_I

#define ICM45605_REG_SMPLRT_DIV   0x19
#define ICM45605_REG_CONFIG       0x1A
#define ICM45605_REG_GYRO_CONFIG  0x1B
#define ICM45605_REG_ACCEL_CONFIG 0x1C
#define ICM45605_REG_FIFO_EN      0x23
#define ICM45605_REG_INT_PIN_CFG  0x37
#define ICM45605_REG_INT_ENABLE   0x38

#define ICM45605_REG_ACCEL_XOUT_H 0x3B
#define ICM45605_REG_ACCEL_XOUT_L 0x3C
#define ICM45605_REG_ACCEL_YOUT_H 0x3D
#define ICM45605_REG_ACCEL_YOUT_L 0x3E
#define ICM45605_REG_ACCEL_ZOUT_H 0x3F
#define ICM45605_REG_ACCEL_ZOUT_L 0x40

#define ICM45605_REG_TEMP_OUT_H   0x41
#define ICM45605_REG_TEMP_OUT_L   0x42

#define ICM45605_REG_GYRO_XOUT_H  0x43
#define ICM45605_REG_GYRO_XOUT_L  0x44
#define ICM45605_REG_GYRO_YOUT_H  0x45
#define ICM45605_REG_GYRO_YOUT_L  0x46
#define ICM45605_REG_GYRO_ZOUT_H  0x47
#define ICM45605_REG_GYRO_ZOUT_L  0x48

#define ICM45605_REG_PWR_MGMT_1   0x6B
#define ICM45605_REG_PWR_MGMT_2   0x6C

// Bits de configuration
#define ICM45605_RESET            0x80
#define ICM45605_SLEEP            0x40

// Échelles accéléromètre
#define ICM45605_ACCEL_RANGE_2G   0x00
#define ICM45605_ACCEL_RANGE_4G   0x08
#define ICM45605_ACCEL_RANGE_8G   0x10
#define ICM45605_ACCEL_RANGE_16G  0x18

// Échelles gyroscope
#define ICM45605_GYRO_RANGE_250DPS   0x00
#define ICM45605_GYRO_RANGE_500DPS   0x08
#define ICM45605_GYRO_RANGE_1000DPS  0x10
#define ICM45605_GYRO_RANGE_2000DPS  0x18

// Filtre passe-bas
#define ICM45605_DLPF_BW_256HZ    0x00
#define ICM45605_DLPF_BW_188HZ    0x01
#define ICM45605_DLPF_BW_98HZ     0x02
#define ICM45605_DLPF_BW_42HZ     0x03
#define ICM45605_DLPF_BW_20HZ     0x04
#define ICM45605_DLPF_BW_10HZ     0x05
#define ICM45605_DLPF_BW_5HZ      0x06

// Structure pour les données de l'accéléromètre
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} ICM45605_Accel_t;

// Structure pour les données du gyroscope
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} ICM45605_Gyro_t;

// Prototypes des fonctions
HAL_StatusTypeDef ICM45605_WriteRegister(uint8_t reg, uint8_t data);
HAL_StatusTypeDef ICM45605_ReadRegister(uint8_t reg, uint8_t *data);
HAL_StatusTypeDef ICM45605_ReadRegisters(uint8_t regStart, uint8_t *buffer, uint8_t length);
HAL_StatusTypeDef ICM45605_Init(void);
HAL_StatusTypeDef ICM45605_ReadAccel(ICM45605_Accel_t *accel);
HAL_StatusTypeDef ICM45605_ReadGyro(ICM45605_Gyro_t *gyro);
HAL_StatusTypeDef ICM45605_ReadTemp(float *temp);
HAL_StatusTypeDef ICM45605_SetAccelRange(uint8_t range);
HAL_StatusTypeDef ICM45605_SetGyroRange(uint8_t range);
void ICM45605_Test(void);

#endif // GYRO_H
