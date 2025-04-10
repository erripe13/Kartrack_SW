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
//#include "stm32f4xx_hal.h"
#include "gyro.h"

// Variables globales
extern I2C_HandleTypeDef hi2c1;    // Déclarer l'instance I2C (définie dans le fichier généré par STM32CubeMX)

// Fonction pour écrire dans un registre du capteur
HAL_StatusTypeDef ICM45605_WriteRegister(uint8_t reg, uint8_t data) {
    uint8_t buffer[2];
    buffer[0] = reg;   // Adresse du registre
    buffer[1] = data;  // Donnée à écrire

    return HAL_I2C_Master_Transmit(&hi2c1, ICM45605_I2C_ADDRESS << 1, buffer, 2, I2C_TIMEOUT);
}

// Fonction pour lire un registre du capteur
HAL_StatusTypeDef ICM45605_ReadRegister(uint8_t reg, uint8_t *data) {
    // Envoyer l'adresse du registre à lire
    if (HAL_I2C_Master_Transmit(&hi2c1, ICM45605_I2C_ADDRESS << 1, &reg, 1, I2C_TIMEOUT) != HAL_OK) {
        return HAL_ERROR;
    }

    // Lire la donnée du registre
    return HAL_I2C_Master_Receive(&hi2c1, ICM45605_I2C_ADDRESS << 1, data, 1, I2C_TIMEOUT);
}

// Fonction pour lire plusieurs registres consécutifs
HAL_StatusTypeDef ICM45605_ReadRegisters(uint8_t regStart, uint8_t *buffer, uint8_t length) {
    // Envoyer l'adresse du premier registre à lire
    if (HAL_I2C_Master_Transmit(&hi2c1, ICM45605_I2C_ADDRESS << 1, &regStart, 1, I2C_TIMEOUT) != HAL_OK) {
        return HAL_ERROR;
    }

    // Lire les données des registres consécutifs
    return HAL_I2C_Master_Receive(&hi2c1, ICM45605_I2C_ADDRESS << 1, buffer, length, I2C_TIMEOUT);
}

// Fonction pour initialiser le capteur ICM45605
HAL_StatusTypeDef ICM45605_Init(void) {
    HAL_StatusTypeDef status;
    uint8_t who_am_i = 0;
    
    // Délai de démarrage
    HAL_Delay(100);

    // Lire le registre WHO_AM_I pour vérifier la communication
    if (ICM45605_ReadRegister(ICM45605_WHO_AM_I, &who_am_i) != HAL_OK) {
        return HAL_ERROR;
    }

    // Vérifier si le capteur répond correctement
    if (who_am_i != ICM45605_DEVICE_ID) {
        return HAL_ERROR;
    }

    // Reset du capteur
    if (ICM45605_WriteRegister(ICM45605_REG_PWR_MGMT_1, ICM45605_RESET) != HAL_OK) {
        return HAL_ERROR;
    }
    
    // Attendre que le reset soit terminé
    HAL_Delay(100);
    
    // Sortir du mode sleep
    if (ICM45605_WriteRegister(ICM45605_REG_PWR_MGMT_1, 0x01) != HAL_OK) {
        return HAL_ERROR;
    }
    
    HAL_Delay(10);
    
    // Configuration du capteur
    // Configuration de l'accéléromètre (±2g par défaut)
    status = ICM45605_WriteRegister(ICM45605_REG_ACCEL_CONFIG, ICM45605_ACCEL_RANGE_2G);
    if (status != HAL_OK) return status;
    
    // Configuration du gyroscope (±250 dps par défaut)
    status = ICM45605_WriteRegister(ICM45605_REG_GYRO_CONFIG, ICM45605_GYRO_RANGE_250DPS);
    if (status != HAL_OK) return status;
    
    // Configuration de la fréquence d'échantillonnage (100 Hz par défaut)
    status = ICM45605_WriteRegister(ICM45605_REG_CONFIG, ICM45605_DLPF_BW_98HZ);
    if (status != HAL_OK) return status;
    
    // Configuration de la fréquence de division (100 Hz)
    status = ICM45605_WriteRegister(ICM45605_REG_SMPLRT_DIV, 0x09);
    if (status != HAL_OK) return status;

    return HAL_OK;
}

// Fonction pour lire les données de l'accéléromètre
HAL_StatusTypeDef ICM45605_ReadAccel(ICM45605_Accel_t *accel) {
    uint8_t buffer[6];
    HAL_StatusTypeDef status;
    
    status = ICM45605_ReadRegisters(ICM45605_REG_ACCEL_XOUT_H, buffer, 6);
    if (status != HAL_OK) return status;
    
    // Conversion en valeurs signées 16 bits
    accel->x = (int16_t)(buffer[0] << 8 | buffer[1]);
    accel->y = (int16_t)(buffer[2] << 8 | buffer[3]);
    accel->z = (int16_t)(buffer[4] << 8 | buffer[5]);
    
    return HAL_OK;
}

// Fonction pour lire les données du gyroscope
HAL_StatusTypeDef ICM45605_ReadGyro(ICM45605_Gyro_t *gyro) {
    uint8_t buffer[6];
    HAL_StatusTypeDef status;
    
    status = ICM45605_ReadRegisters(ICM45605_REG_GYRO_XOUT_H, buffer, 6);
    if (status != HAL_OK) return status;
    
    // Conversion en valeurs signées 16 bits
    gyro->x = (int16_t)(buffer[0] << 8 | buffer[1]);
    gyro->y = (int16_t)(buffer[2] << 8 | buffer[3]);
    gyro->z = (int16_t)(buffer[4] << 8 | buffer[5]);
    
    return HAL_OK;
}

// Fonction pour lire la température
HAL_StatusTypeDef ICM45605_ReadTemp(float *temp) {
    uint8_t buffer[2];
    int16_t raw_temp;
    HAL_StatusTypeDef status;
    
    status = ICM45605_ReadRegisters(ICM45605_REG_TEMP_OUT_H, buffer, 2);
    if (status != HAL_OK) return status;
    
    raw_temp = (int16_t)(buffer[0] << 8 | buffer[1]);
    // Formule de conversion selon la fiche technique
    *temp = ((float)raw_temp / 327.68f) + 21.0f;
    
    return HAL_OK;
}

// Fonction pour configurer l'échelle de l'accéléromètre
HAL_StatusTypeDef ICM45605_SetAccelRange(uint8_t range) {
    if (range > ICM45605_ACCEL_RANGE_16G) return HAL_ERROR;
    
    return ICM45605_WriteRegister(ICM45605_REG_ACCEL_CONFIG, range);
}

// Fonction pour configurer l'échelle du gyroscope
HAL_StatusTypeDef ICM45605_SetGyroRange(uint8_t range) {
    if (range > ICM45605_GYRO_RANGE_2000DPS) return HAL_ERROR;
    
    return ICM45605_WriteRegister(ICM45605_REG_GYRO_CONFIG, range);
}

// Exemple d'utilisation
void ICM45605_Test(void) {
    ICM45605_Accel_t accel;
    ICM45605_Gyro_t gyro;
    float temp;
    
    if (ICM45605_Init() == HAL_OK) {
        // Lire les données de l'accéléromètre, du gyroscope et de la température
        if (ICM45605_ReadAccel(&accel) == HAL_OK && 
            ICM45605_ReadGyro(&gyro) == HAL_OK &&
            ICM45605_ReadTemp(&temp) == HAL_OK) {
            
            // À ce stade, vous pouvez utiliser les données accel, gyro et temp
            // Par exemple, les transmettre via UART, les afficher sur un écran LCD, etc.
        }
    } else {
        // Gérer l'erreur d'initialisation
    }
}



