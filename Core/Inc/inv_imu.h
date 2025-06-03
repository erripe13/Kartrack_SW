/*
 * inv_imu.h
 *
 *  Created on: Jun 4, 2025
 *      Author: SEHTEL
 */

#ifndef _INV_IMU_H_
#define _INV_IMU_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup IMU IMU
 *  @brief Describes IMU
 *  @{
 */

/** @file inv_imu.h */


/* Device ID */
#define ICM45605

/* Device description */
#define INV_IMU_STRING_ID            "ICM45605"
#define INV_IMU_WHOAMI               0xE5
#define INV_IMU_HIGH_FSR_SUPPORTED   0
#define INV_IMU_FSYNC_SUPPORTED      1
#define INV_IMU_USE_BASIC_SMD        0
#define INV_IMU_INT2_PIN_SUPPORTED   1
#define INV_IMU_I2C_MASTER_SUPPORTED 1
#define INV_IMU_CLKIN_SUPPORTED      0
#define INV_IMU_AUX1_SUPPORTED       0
#define INV_IMU_AUX2_SUPPORTED       0

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _INV_IMU_H_ */

/** @} */
