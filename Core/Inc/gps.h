/*
 * gps.h
 *
 *  Created on: Apr 4, 2025
 *      Author: pierr
 */

#ifndef INC_GPS_H_
#define INC_GPS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f7xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @file gps_l76.h
 * @brief Driver for Quectel L76 GPS module using STM32F7 HAL and FreeRTOS.
 *
 * This driver provides functions to initialize and interface with the Quectel L76
 * GNSS module. It uses a UART interface (with HAL) to receive NMEA sentences and
 * parses GGA and RMC frames to retrieve position (latitude, longitude, altitude)
 * and navigation data (UTC time, date, speed, fix status).
 *
 * **Hardware Connections:**
 * - STM32F7 UART TX (e.g., PC6) -> L76 RX
 * - STM32F7 UART RX (e.g., PC7) -> L76 TX
 * - STM32F7 GPIO (PG6) -> L76 STANDBY pin (GPS_STDBY, active low)
 *
 * The code is compatible with STM32CubeIDE. It uses HAL functions and FreeRTOS (CMSIS v1)
 * and is written for easy integration into an existing FreeRTOS project.
 */

/* Define the STDBY control pin (adjust if needed for different hardware setup) */
#ifdef GPS_STDBY_Pin  // Use existing pin definitions if available (from main.h)
#define L76_STDBY_Pin        GPS_STDBY_Pin
#define L76_STDBY_GPIO_Port  GPS_STDBY_GPIO_Port
#else
#define L76_STDBY_Pin        GPIO_PIN_6     /* PG6 connected to L76 STDBY */
#define L76_STDBY_GPIO_Port  GPIOG
#endif

/* Maximum length of an NMEA sentence (including CR/LF and null terminator) */
#define L76_NMEA_MAX_LEN     128

/* Data structure to hold parsed GPS information */
typedef struct {
    double   latitude;       /**< Latitude in decimal degrees (positive = North, negative = South) */
    double   longitude;      /**< Longitude in decimal degrees (positive = East, negative = West) */
    float    altitude;       /**< Altitude in meters above sea level */
    uint8_t  fix_quality;    /**< GPS fix quality (0 = invalid, 1 = GPS fix, 2 = DGPS fix, etc.) */
    uint8_t  satellites;     /**< Number of satellites in use */
    float    speed;          /**< Speed over ground in knots */
    float    course;         /**< Course (track angle) in degrees */
    uint8_t  hours;          /**< UTC Time hours (0-23) */
    uint8_t  minutes;        /**< UTC Time minutes (0-59) */
    float    seconds;        /**< UTC Time seconds (including fractional part) */
    uint8_t  day;            /**< UTC Date day (1-31) */
    uint8_t  month;          /**< UTC Date month (1-12) */
    uint16_t year;           /**< UTC Date year (e.g., 2025) */
} L76_GPS_Data_t;

/**
 * @brief Initialize the L76 GPS module interface (UART and Standby control).
 * @param huart  Pointer to the UART handle connected to L76 (configured for 9600 8N1).
 *
 * This sets up the GPS standby pin (ensuring the module is active) and starts UART
 * reception in interrupt mode. Must be called before using other L76 functions.
 */
void L76_Init(UART_HandleTypeDef *huart);

/**
 * @brief FreeRTOS task function to handle GPS initialization and continuous NMEA reception.
 * @param argument FreeRTOS task argument (unused).
 *
 * This task initializes the L76 module (via L76_Init) and continuously waits for NMEA
 * sentences from the GPS. It parses GGA and RMC sentences as they arrive, updating an
 * internal L76_GPS_Data_t structure with the latest data. This task should be created
 * in the FreeRTOS scheduler (e.g., using osThreadCreate or xTaskCreate).
 */
void L76_Task(void *argument);

/**
 * @brief Get the latest parsed GPS data.
 * @param data Pointer to an L76_GPS_Data_t structure to receive the data.
 *
 * Copies the most recent GPS data (position, time, etc.) into the provided structure.
 * This function is thread-safe with respect to the L76 task (it uses a brief critical section).
 */
void L76_GetData(L76_GPS_Data_t *data);

/**
 * @brief Control the standby mode of the L76 module.
 * @param standby If true, put the module in standby (low power); if false, wake it up.
 *
 * Pulls the STDBY pin low to enter standby, or high to exit standby. Exiting standby
 * will also send a dummy byte over UART to ensure the module wakes up (per Quectel specs).
 *
 * @note Entering standby mode may require prior NMEA command (PMTK) to allow standby.
 * This function primarily toggles the hardware pin.
 */
void L76_SetStandby(bool standby);

/**
 * @brief Example function to demonstrate usage of the L76 driver (prints GPS data).
 *
 * This function retrieves the latest GPS data and prints the position (latitude, longitude, altitude)
 * and time/date/speed to the debug console. If no fix is acquired yet, it prints a message indicating so.
 *
 * @note Ensure that a UART or ITM is set up for printf output. This is just for demonstration and
 * would typically be replaced or removed in production code.
 */
void L76_PrintExample(void);

#ifdef __cplusplus
}
#endif

#endif
