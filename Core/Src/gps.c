#include "gps.h"
#include "cmsis_os.h"      /* CMSIS-OS FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>   // Pour atoi, atof, strtof

/* ========= Static Variables (Module Scope) ========= */
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart6;
/* UART handle for the GPS module (set during initialization) */
static UART_HandleTypeDef *l76_huart = NULL;
/* Binary semaphore to signal line reception to the task */
static SemaphoreHandle_t l76_sem = NULL;
/* Buffer for incoming NMEA sentences (double buffer for safe ISR/task usage) */
static char l76_rxBuffer[2][L76_NMEA_MAX_LEN];
static uint16_t l76_rxIndex = 0; /* Current write index in the active buffer */
static uint8_t l76_currentBuf = 0; /* Index of active buffer being filled by ISR */
static volatile int8_t l76_readyBuf = -1; /* Index of buffer ready to be processed by task (-1 if none) */
/* Temporary storage for one received character (for HAL UART IT) */
static uint8_t l76_rxChar;

/* Structure to hold the latest parsed GPS data */
static L76_GPS_Data_t l76_data; /* This will be updated in the task and read by user via L76_GetData */

static uint8_t gsv_sat_count = 0;
static uint8_t gsv_max_snr = 0;

/* ========= Private Function Prototypes ========= */
static void L76_ProcessNMEA(char *nmea);
static void L76_ParseGGA(char *nmea, L76_GPS_Data_t *data);
static void L76_ParseRMC(char *nmea, L76_GPS_Data_t *data);
static double L76_ConvertNMEADegrees(const char *raw, char dir);
static void L76_ParseTime(const char *timestr, uint8_t *hour, uint8_t *min,
		float *sec);
static void L76_ParseDate(const char *datestr, uint8_t *day, uint8_t *month,
		uint16_t *year);

/* ========= Public API Functions ========= */

void L76_Init(UART_HandleTypeDef *huart) {
	/* Save the UART handle for use in ISR and other functions */
	l76_huart = huart;

	/* Configure the standby pin: ensure the GPS is powered on (STDBY pin high) */
	HAL_GPIO_WritePin(L76_STDBY_GPIO_Port, L76_STDBY_Pin, GPIO_PIN_SET);
	/* Small delay to ensure the pin state is registered (if needed) */
	HAL_Delay(10);

	/* Create a binary semaphore for line synchronization */
	l76_sem = xSemaphoreCreateBinary();
	if (l76_sem == NULL) {
		// Semaphore creation failed (should not happen under normal conditions)
		// In production, you might handle this with an error indicator.
	}

	/* Initialize the GPS data structure to all zeros */
	memset(&l76_data, 0, sizeof(L76_GPS_Data_t));
	l76_data.fix_quality = 0;
	l76_readyBuf = -1;
	l76_rxIndex = 0;
	l76_currentBuf = 0;

	/* Start UART reception in interrupt mode for one byte at a time */
	//HAL_UART_Receive_IT(l76_huart, &l76_rxChar, 1);
	HAL_UART_Receive_IT(l76_huart, &l76_rxChar, 1);

}

void L76_Task(void const *argument) {
	/* Initialize the GPS module (UART, pins, etc.) */
	UART_HandleTypeDef *uart = (UART_HandleTypeDef*) argument;
	L76_Init(uart);

	const char *uart_name = "not uart6 !";

	if (uart == &huart6)
		uart_name = "uart6";

	printf("L76_Task launched with UART: %s\r\n", uart_name);
	// Note: 'argument' can be used to pass the UART handle if desired.
	// Alternatively, one can call L76_Init before creating the task and ignore this parameter.

	/* Continuously wait for and process NMEA sentences */
	for (;;) {
		/* Wait indefinitely for a NMEA sentence to be received (signaled by ISR) */
		if (xSemaphoreTake(l76_sem, portMAX_DELAY) == pdTRUE) {
			// A full NMEA sentence has been captured and is ready in one of the buffers.
			if (l76_readyBuf >= 0) {
				/* Process the NMEA sentence in the ready buffer */
				L76_ProcessNMEA(l76_rxBuffer[l76_readyBuf]);
				/* Mark buffer as processed (readyBuf will be set by ISR for next line) */
				l76_readyBuf = -1;
			}
		}
		// Loop back to wait for the next sentence
	}
}

void L76_GetData(L76_GPS_Data_t *data) {
	/* Copy the latest GPS data in a critical section to ensure consistency */
	taskENTER_CRITICAL();
	*data = l76_data;
	taskEXIT_CRITICAL();
}

void L76_SetStandby(bool standby) {
	if (standby) {
		/* Enter standby: pull STDBY pin low */
		HAL_GPIO_WritePin(L76_STDBY_GPIO_Port, L76_STDBY_Pin, GPIO_PIN_RESET);
	} else {
		/* Exit standby: pull STDBY high */
		HAL_GPIO_WritePin(L76_STDBY_GPIO_Port, L76_STDBY_Pin, GPIO_PIN_SET);
		/* Send a dummy character to wake the module (Quectel spec: a UART byte is needed) */
		uint8_t dummy = '\n';  // sending newline as a dummy byte
		HAL_UART_Transmit(l76_huart, &dummy, 1, 10);
		/* Restart UART reception in case it was halted during standby */
		l76_rxIndex = 0;
		l76_currentBuf = 0;
		HAL_UART_Receive_IT(l76_huart, &l76_rxChar, 1);
	}
}

void L76_PrintExample(void) {
	L76_GPS_Data_t data;
	L76_GetData(&data);

	char buf[256];

	if (data.fix_quality == 0) {
		snprintf(buf, sizeof(buf), "[GPS] No fix: %d satellites used, "
				"UTC %02d:%02d:%05.2f, date %02d/%02d/%04d\r\n"
				"[GSV] Satellites in view: %d, Max SNR: %ddB\r\n",
				data.satellites, data.hours, data.minutes, data.seconds,
				data.day, data.month, data.year, gsv_sat_count, gsv_max_snr);

		if (gsv_sat_count == 0 && data.hours == 0 && data.minutes == 0) {
			strcat(buf, "[GPS] Cold start likely (no GNSS signal)\r\n");
		} else if (gsv_sat_count < 4) {
			strcat(buf, "[GPS] Acquiring satellites (weak signal)\r\n");
		} else {
			strcat(buf, "[GPS] Satellites visible but not locked\r\n");
		}
	} else {
		snprintf(buf, sizeof(buf),
				"[GPS] FIX OK: Lat=%.5f, Lon=%.5f, Alt=%.1fm\r\n"
						"UTC=%02d:%02d:%05.2f, Date=%02d/%02d/%04d\r\n"
						"Speed=%.1f kt, Course=%.1f°, Satellites used=%d\r\n"
						"[GSV] Satellites in view: %d, Max SNR: %ddB\r\n",
				data.latitude, data.longitude, data.altitude, data.hours,
				data.minutes, data.seconds, data.day, data.month, data.year,
				data.speed, data.course, data.satellites, gsv_sat_count,
				gsv_max_snr);
	}

	printf("%s", buf);
}

/* ========= UART Interrupt Callback ========= */

/**
 * @brief UART RX Complete Callback (called from HAL IRQ handler when a byte is received).
 * @note This ISR appends incoming characters to a buffer and signals when a full line is received.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if (huart == l76_huart) {
		char c = (char) l76_rxChar;
		//HAL_UART_Transmit(&huart1, (uint8_t*) &c, 1, 1);
		//printf("callback triggered");
		if (c == '$') {
			/* Start of a new NMEA sentence */
			l76_rxIndex = 0;
			l76_currentBuf ^= 1;  // switch to the other buffer for new sentence
			l76_rxBuffer[l76_currentBuf][l76_rxIndex++] = c;
		} else if (c == '\n') {
			/* End of NMEA sentence (LF detected). Terminate the string. */
			if (l76_rxIndex < L76_NMEA_MAX_LEN) {
				l76_rxBuffer[l76_currentBuf][l76_rxIndex] = '\0'; // null-terminate line
			} else {
				l76_rxBuffer[l76_currentBuf][L76_NMEA_MAX_LEN - 1] = '\0';
			}
			// Remove any trailing CR if present (overwrite with null terminator)
			if (l76_rxIndex > 0
					&& l76_rxBuffer[l76_currentBuf][l76_rxIndex - 1] == '\r') {
				l76_rxBuffer[l76_currentBuf][l76_rxIndex - 1] = '\0';
			}
			/* The buffer l76_currentBuf now holds a complete sentence.
			 Signal the task that a line is ready in the OTHER buffer (the one just filled). */
			l76_readyBuf = l76_currentBuf;
			// Note: l76_currentBuf already switched to new buffer at sentence start.
			// Swap back to fill in the other buffer for the next sentence
			// Actually, we've toggled at '$', so at this point l76_currentBuf is the buffer just filled.
			// We will toggle it next time a new '$' comes.
			// So here l76_readyBuf is set to currentBuf, which is the filled buffer.

			/* Give semaphore to unblock the task waiting for a new line */
			xSemaphoreGiveFromISR(l76_sem, &xHigherPriorityTaskWoken);
			/* Optionally, yield to the GPS task immediately if it has higher priority */
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		} else if (c == '\r') {
			/* Carriage return: skip it (will handle on '\n') */
			// Do nothing, just ignore the CR
		} else {
			/* Regular character, add to current buffer if space */
			if (l76_rxIndex < (L76_NMEA_MAX_LEN - 1)) {
				l76_rxBuffer[l76_currentBuf][l76_rxIndex++] = c;
			} else {
				/* Buffer overflow - sentence too long, reset index to avoid overflow.
				 (In practice, NMEA sentences should not exceed buffer length.) */
				l76_rxIndex = 0;
			}
		}
		/* Re-arm the UART receive interrupt for the next character */
		HAL_UART_Receive_IT(l76_huart, &l76_rxChar, 1);
	}
}

/* ========= NMEA Sentence Processing ========= */

/**
 * @brief Determine the type of NMEA sentence and parse accordingly.
 * @param nmea Pointer to the NMEA sentence string (null-terminated, starting with '$').
 */
static void L76_ProcessNMEA(char *nmea) {
	if (nmea == NULL || nmea[0] != '$') {
		return; // invalid sentence
	}

	// Remove any checksum if present by cutting off at '*'
	char *checksum_start = strchr(nmea, '*');
	if (checksum_start) {
		*checksum_start = '\0'; // terminate string at '*' (discard checksum part)
	}

	// Identify sentence type by the 3-letter identifier after '$xx' (talker ID)
	if (strncmp(nmea + 3, "GGA", 3) == 0) {
		L76_ParseGGA(nmea, &l76_data);
	} else if (strncmp(nmea + 3, "RMC", 3) == 0) {
		L76_ParseRMC(nmea, &l76_data);
	} else if (strncmp(nmea, "$GPGSV", 6) == 0
			|| strncmp(nmea, "$GLGSV", 6) == 0) {
		// Display raw GSV line
		printf("[RAW GSV] %s\r\n", nmea);

		// Basic parsing to count satellites and max SNR
		char *token;
		uint8_t field = 0;
		uint8_t sat_seen = 0;
		uint8_t max_snr = 0;

		token = strtok(nmea, ",");
		while (token != NULL) {
			field++;
			if (field == 4) {
				sat_seen = (uint8_t) atoi(token);  // total satellites in view
			}
			if (field >= 8 && ((field - 8) % 4 == 0)) {
				uint8_t snr = (uint8_t) atoi(token);
				if (snr > max_snr)
					max_snr = snr;
			}
			token = strtok(NULL, ",");
		}

		// Update global GSV debug values
		gsv_sat_count = sat_seen;
		gsv_max_snr = max_snr;
	}
}

/**
 * @brief Parse a $GPGGA or $GNGGA NMEA sentence and update GPS data.
 * @param nmea The NMEA sentence string (null-terminated, without the checksum part).
 * @param data Pointer to GPS data structure to update.
 */
static void L76_ParseGGA(char *nmea, L76_GPS_Data_t *data) {
	// Example GGA: $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,
	char *token;
	token = strtok(nmea, ",");       // token0: "$GPGGA" (with talker)
	token = strtok(NULL, ",");       // token1: UTC time (hhmmss.sss)
	if (token != NULL) {
		L76_ParseTime(token, &data->hours, &data->minutes, &data->seconds);
	}
	token = strtok(NULL, ",");       // token2: latitude (ddmm.mmmm)
	const char *lat_str = token;
	token = strtok(NULL, ",");       // token3: N/S
	char lat_dir = (token != NULL ? token[0] : 0);
	token = strtok(NULL, ",");       // token4: longitude (dddmm.mmmm)
	const char *lon_str = token;
	token = strtok(NULL, ",");       // token5: E/W
	char lon_dir = (token != NULL ? token[0] : 0);
	token = strtok(NULL, ","); // token6: Fix quality (0 = invalid, 1 = GPS fix, 2 = DGPS fix, ...)
	data->fix_quality = (token != NULL ? (uint8_t) atoi(token) : 0);
	token = strtok(NULL, ",");       // token7: Number of satellites
	data->satellites = (token != NULL ? (uint8_t) atoi(token) : 0);
	token = strtok(NULL, ",");       // token8: HDOP
	// We can parse HDOP if needed: float hdop = token ? strtof(token, NULL) : 0.0f;
	token = strtok(NULL, ",");       // token9: Altitude
	data->altitude = (token != NULL ? strtof(token, NULL) : 0.0f);
	// token10: Altitude unit (usually "M")
	token = strtok(NULL, ",");
	// token11: Geoidal separation (not used)
	token = strtok(NULL, ",");
	// token12: Geoidal separation unit (usually "M")
	token = strtok(NULL, ",");
	// token13: DGPS age (if any, not used)
	token = strtok(NULL, ",");
	// token14: DGPS reference station ID (if any, not used)
	// (Note: strtok already stopped at '*' if there was a checksum, since we cut it in ProcessNMEA)

	// Convert latitude and longitude to decimal degrees
	if (lat_str && lat_dir) {
		data->latitude = L76_ConvertNMEADegrees(lat_str, lat_dir);
	}
	if (lon_str && lon_dir) {
		data->longitude = L76_ConvertNMEADegrees(lon_str, lon_dir);
	}
	// No return value; data is updated in the provided structure
}

/**
 * @brief Parse a $GPRMC or $GNRMC NMEA sentence and update GPS data.
 * @param nmea The NMEA sentence string (null-terminated, without the checksum).
 * @param data Pointer to GPS data structure to update.
 */
static void L76_ParseRMC(char *nmea, L76_GPS_Data_t *data) {
	// Example RMC: $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,A
	char *token;
	token = strtok(nmea, ",");      // token0: "$GPRMC"
	token = strtok(NULL, ",");      // token1: UTC time
	if (token != NULL) {
		L76_ParseTime(token, &data->hours, &data->minutes, &data->seconds);
	}
	token = strtok(NULL, ",");      // token2: Status (A=active, V=void)
	char status = (token != NULL ? token[0] : 'V');
	token = strtok(NULL, ",");      // token3: Latitude
	const char *lat_str = token;
	token = strtok(NULL, ",");      // token4: N/S
	char lat_dir = (token != NULL ? token[0] : 0);
	token = strtok(NULL, ",");      // token5: Longitude
	const char *lon_str = token;
	token = strtok(NULL, ",");      // token6: E/W
	char lon_dir = (token != NULL ? token[0] : 0);
	token = strtok(NULL, ",");      // token7: Speed in knots
	data->speed = (token != NULL ? strtof(token, NULL) : 0.0f);
	token = strtok(NULL, ",");      // token8: Course (track angle in degrees)
	data->course = (token != NULL ? strtof(token, NULL) : 0.0f);
	token = strtok(NULL, ",");      // token9: Date (ddmmyy)
	if (token != NULL) {
		L76_ParseDate(token, &data->day, &data->month, &data->year);
	}
	// token10: Magnetic variation (optional, may be empty)
	token = strtok(NULL, ",");
	// token11: Mag var direction (E/W, optional)
	token = strtok(NULL, ",");

	// Update latitude/longitude if available
	if (lat_str && lat_dir) {
		data->latitude = L76_ConvertNMEADegrees(lat_str, lat_dir);
	}
	if (lon_str && lon_dir) {
		data->longitude = L76_ConvertNMEADegrees(lon_str, lon_dir);
	}
	// Update fix status based on Status field
	if (status == 'V') {
		// 'V' = navigation receiver warning (no valid fix)
		data->fix_quality = 0;
		data->satellites = 0;
	}
}

/* ========= Helper Parsing Functions ========= */

/**
 * @brief Convert an NMEA coordinate string (degrees and minutes) into decimal degrees.
 * @param raw The coordinate string in NMEA format (ddmm.mmmm or dddmm.mmmm).
 * @param dir The direction character ('N','S','E','W').
 * @return The coordinate in decimal degrees, with sign indicating hemisphere.
 */
static double L76_ConvertNMEADegrees(const char *raw, char dir) {
	// NMEA format: latitude: ddmm.mmmm, longitude: dddmm.mmmm
	// Convert to decimal degrees.
	double val = atof(raw);
	int degrees = (int) (val / 100);         // extract whole degrees
	double minutes = val - (degrees * 100); // extract minutes (including fractional part)
	double dec_deg = (double) degrees + minutes / 60.0;
	if (dir == 'S' || dir == 'W') {
		dec_deg = -dec_deg;
	}
	return dec_deg;
}

/**
 * @brief Parse a UTC time string from NMEA (hhmmss.sss) into hour, minute, second.
 * @param timestr Time string (e.g., "123519" or "123519.00").
 * @param hour [out] Parsed hour.
 * @param min  [out] Parsed minute.
 * @param sec  [out] Parsed seconds (including fractional part).
 */
static void L76_ParseTime(const char *timestr, uint8_t *hour, uint8_t *min,
		float *sec) {
	if (strlen(timestr) < 6) {
		// Invalid time string
		*hour = *min = 0;
		*sec = 0.0f;
		return;
	}
	// Parse hour, minute, second
	char buf[3] = { 0 };
	buf[0] = timestr[0];
	buf[1] = timestr[1]; // HH
	*hour = (uint8_t) atoi(buf);
	buf[0] = timestr[2];
	buf[1] = timestr[3]; // MM
	*min = (uint8_t) atoi(buf);
	buf[0] = timestr[4];
	buf[1] = timestr[5]; // SS
	uint8_t sec_int = (uint8_t) atoi(buf);
	float sec_frac = 0.0f;
	if (timestr[6] == '.') {
		// Fractional part present
		const char *frac_str = timestr + 7;  // part after the decimal point
		if (*frac_str) {
			int frac_int = atoi(frac_str);
			int frac_len = strlen(frac_str);
			sec_frac = (float) frac_int;
			// Divide by 10^frac_len to get the fractional seconds
			while (frac_len-- > 0) {
				sec_frac /= 10.0f;
			}
		}
	}
	*sec = sec_int + sec_frac;
}

/**
 * @brief Parse a date string from NMEA (ddmmyy) into day, month, year.
 * @param datestr Date string (e.g., "230394" for 23rd March 1994).
 * @param day   [out] Parsed day.
 * @param month [out] Parsed month.
 * @param year  [out] Parsed year (full year, e.g., 1994 or 2025).
 */
static void L76_ParseDate(const char *datestr, uint8_t *day, uint8_t *month,
		uint16_t *year) {
	if (strlen(datestr) != 6) {
		*day = *month = 0;
		*year = 0;
		return;
	}
	char buf[3] = { 0 };
	buf[0] = datestr[0];
	buf[1] = datestr[1];
	*day = (uint8_t) atoi(buf);
	buf[0] = datestr[2];
	buf[1] = datestr[3];
	*month = (uint8_t) atoi(buf);
	buf[0] = datestr[4];
	buf[1] = datestr[5];
	uint8_t yy = (uint8_t) atoi(buf);
	// NMEA year is two digits (00-99). Assume 2000-2099 for 00-99 up to 89, and 1900s for 90-99.
	if (yy < 90) {
		*year = 2000 + yy;
	} else {
		*year = 1900 + yy;
	}
}

//void L76_Update(void) {
//    if (xSemaphoreTake(l76_sem, 0) == pdTRUE) {
//        if (l76_readyBuf >= 0) {
//            L76_ProcessNMEA(l76_rxBuffer[l76_readyBuf]);
//            l76_readyBuf = -1;
//        }
//    }
//}

