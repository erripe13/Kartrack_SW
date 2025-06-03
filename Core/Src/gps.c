#include "gps.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart6;

static UART_HandleTypeDef *l76_huart = NULL;
static SemaphoreHandle_t l76_sem = NULL;
static char l76_rxBuffer[2][L76_NMEA_MAX_LEN];
static uint16_t l76_rxIndex = 0;
static uint8_t l76_currentBuf = 0;
static volatile int8_t l76_readyBuf = -1;
static uint8_t l76_rxChar;

static L76_GPS_Data_t l76_data;
static uint8_t gsv_sat_count = 0;
static uint8_t gsv_max_snr = 0;

static void L76_ProcessNMEA(char *nmea);
static void L76_ParseGGA(char *nmea, L76_GPS_Data_t *data);
static void L76_ParseRMC(char *nmea, L76_GPS_Data_t *data);
static double L76_ConvertNMEADegrees(const char *raw, char dir);
static void L76_ParseTime(const char *str, uint8_t *h, uint8_t *m, float *s);
static void L76_ParseDate(const char *str, uint8_t *d, uint8_t *mo, uint16_t *y);

void L76_Init(UART_HandleTypeDef *huart) {
	l76_huart = huart;
	HAL_GPIO_WritePin(L76_STDBY_GPIO_Port, L76_STDBY_Pin, GPIO_PIN_SET);
	HAL_Delay(10);
	l76_sem = xSemaphoreCreateBinary();
	memset(&l76_data, 0, sizeof(l76_data));
	l76_rxIndex = 0;
	l76_currentBuf = 0;
	l76_readyBuf = -1;
	HAL_UART_Receive_IT(l76_huart, &l76_rxChar, 1);
}

void L76_Task(void const *arg) {
	L76_Init((UART_HandleTypeDef*) arg);

	for (;;) {
		if (xSemaphoreTake(l76_sem, portMAX_DELAY) == pdTRUE && l76_readyBuf >= 0) {
			L76_ProcessNMEA(l76_rxBuffer[l76_readyBuf]);
			l76_readyBuf = -1;
		}
	}
}

void L76_GetData(L76_GPS_Data_t *data) {
	taskENTER_CRITICAL();
	*data = l76_data;
	taskEXIT_CRITICAL();
}

void L76_SetStandby(bool standby) {
	if (standby) {
		HAL_GPIO_WritePin(L76_STDBY_GPIO_Port, L76_STDBY_Pin, GPIO_PIN_RESET);
	} else {
		HAL_GPIO_WritePin(L76_STDBY_GPIO_Port, L76_STDBY_Pin, GPIO_PIN_SET);
		uint8_t dummy = '\n';
		HAL_UART_Transmit(l76_huart, &dummy, 1, 10);
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
		snprintf(buf, sizeof(buf),
			"[GPS] No fix: %d sats, UTC %02d:%02d:%05.2f, %02d/%02d/%04d\r\n"
			"[GSV] View: %d, Max SNR: %ddB\r\n",
			data.satellites, data.hours, data.minutes, data.seconds,
			data.day, data.month, data.year, gsv_sat_count, gsv_max_snr);

		if (gsv_sat_count == 0 && data.hours == 0 && data.minutes == 0)
			strcat(buf, "[GPS] Cold start?\r\n");
		else if (gsv_sat_count < 4)
			strcat(buf, "[GPS] Weak signal\r\n");
		else
			strcat(buf, "[GPS] No lock yet\r\n");
	} else {
		snprintf(buf, sizeof(buf),
			"[GPS] FIX OK: Lat=%.5f, Lon=%.5f, Alt=%.1fm\r\n"
			"UTC=%02d:%02d:%05.2f, Date=%02d/%02d/%04d\r\n"
			"Speed=%.1fkt, Course=%.1f°, Sats=%d\r\n"
			"[GSV] View: %d, Max SNR: %ddB\r\n",
			data.latitude, data.longitude, data.altitude,
			data.hours, data.minutes, data.seconds,
			data.day, data.month, data.year,
			data.speed, data.course, data.satellites,
			gsv_sat_count, gsv_max_snr);
	}

	printf("%s", buf);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	BaseType_t wake = pdFALSE;
	if (huart == l76_huart) {
		char c = (char) l76_rxChar;

		if (c == '$') {
			l76_rxIndex = 0;
			l76_currentBuf ^= 1;
			l76_rxBuffer[l76_currentBuf][l76_rxIndex++] = c;
		} else if (c == '\n') {
			if (l76_rxIndex < L76_NMEA_MAX_LEN)
				l76_rxBuffer[l76_currentBuf][l76_rxIndex] = '\0';
			else
				l76_rxBuffer[l76_currentBuf][L76_NMEA_MAX_LEN - 1] = '\0';

			if (l76_rxIndex > 0 && l76_rxBuffer[l76_currentBuf][l76_rxIndex - 1] == '\r')
				l76_rxBuffer[l76_currentBuf][l76_rxIndex - 1] = '\0';

			l76_readyBuf = l76_currentBuf;
			xSemaphoreGiveFromISR(l76_sem, &wake);
			portYIELD_FROM_ISR(wake);
		} else if (c != '\r') {
			if (l76_rxIndex < (L76_NMEA_MAX_LEN - 1))
				l76_rxBuffer[l76_currentBuf][l76_rxIndex++] = c;
			else
				l76_rxIndex = 0; // overflow safety
		}

		HAL_UART_Receive_IT(l76_huart, &l76_rxChar, 1);
	}
}

static void L76_ProcessNMEA(char *nmea) {
	if (!nmea || nmea[0] != '$') return;

	char *star = strchr(nmea, '*');
	if (star) *star = '\0';

	if (strncmp(nmea + 3, "GGA", 3) == 0) {
		L76_ParseGGA(nmea, &l76_data);
	} else if (strncmp(nmea + 3, "RMC", 3) == 0) {
		L76_ParseRMC(nmea, &l76_data);
	} else if (strncmp(nmea, "$GPGSV", 6) == 0 || strncmp(nmea, "$GLGSV", 6) == 0) {
		char *token;
		uint8_t field = 0, sat = 0, snr = 0;

		token = strtok(nmea, ",");
		while (token != NULL) {
			field++;
			if (field == 4) sat = atoi(token);
			if (field >= 8 && ((field - 8) % 4 == 0)) {
				uint8_t val = atoi(token);
				if (val > snr) snr = val;
			}
			token = strtok(NULL, ",");
		}

		gsv_sat_count = sat;
		gsv_max_snr = snr;
	}
}

static void L76_ParseGGA(char *nmea, L76_GPS_Data_t *data) {
	char *token = strtok(nmea, ","); // skip GGA
	token = strtok(NULL, ","); L76_ParseTime(token, &data->hours, &data->minutes, &data->seconds);
	const char *lat = strtok(NULL, ","); char lat_dir = *strtok(NULL, ",");
	const char *lon = strtok(NULL, ","); char lon_dir = *strtok(NULL, ",");
	data->fix_quality = atoi(strtok(NULL, ","));
	data->satellites = atoi(strtok(NULL, ","));
	strtok(NULL, ","); // skip HDOP
	data->altitude = strtof(strtok(NULL, ","), NULL);

	if (lat && lat_dir) data->latitude = L76_ConvertNMEADegrees(lat, lat_dir);
	if (lon && lon_dir) data->longitude = L76_ConvertNMEADegrees(lon, lon_dir);
}

static void L76_ParseRMC(char *nmea, L76_GPS_Data_t *data) {
	char *token = strtok(nmea, ","); // skip RMC
	token = strtok(NULL, ","); L76_ParseTime(token, &data->hours, &data->minutes, &data->seconds);
	char status = *strtok(NULL, ",");
	const char *lat = strtok(NULL, ","); char lat_dir = *strtok(NULL, ",");
	const char *lon = strtok(NULL, ","); char lon_dir = *strtok(NULL, ",");
	data->speed = strtof(strtok(NULL, ","), NULL);
	data->course = strtof(strtok(NULL, ","), NULL);
	L76_ParseDate(strtok(NULL, ","), &data->day, &data->month, &data->year);

	if (lat && lat_dir) data->latitude = L76_ConvertNMEADegrees(lat, lat_dir);
	if (lon && lon_dir) data->longitude = L76_ConvertNMEADegrees(lon, lon_dir);
	if (status == 'V') {
		data->fix_quality = 0;
		data->satellites = 0;
	}
}

static double L76_ConvertNMEADegrees(const char *raw, char dir) {
	double val = atof(raw);
	int deg = (int)(val / 100);
	double min = val - deg * 100;
	double res = deg + min / 60.0;
	return (dir == 'S' || dir == 'W') ? -res : res;
}

static void L76_ParseTime(const char *s, uint8_t *h, uint8_t *m, float *sec) {
	if (strlen(s) < 6) { *h = *m = 0; *sec = 0; return; }
	char b[3] = {0}; memcpy(b, s, 2); *h = atoi(b);
	memcpy(b, s + 2, 2); *m = atoi(b);
	memcpy(b, s + 4, 2); uint8_t si = atoi(b); float sf = 0.0f;
	if (s[6] == '.') {
		const char *frac = s + 7;
		if (*frac) {
			int fi = atoi(frac), len = strlen(frac);
			sf = (float)fi; while (len--) sf /= 10.0f;
		}
	}
	*sec = si + sf;
}

static void L76_ParseDate(const char *s, uint8_t *d, uint8_t *mo, uint16_t *y) {
	if (strlen(s) != 6) { *d = *mo = 0; *y = 0; return; }
	char b[3] = {0}; memcpy(b, s, 2); *d = atoi(b);
	memcpy(b, s + 2, 2); *mo = atoi(b);
	memcpy(b, s + 4, 2); uint8_t yy = atoi(b);
	*y = (yy < 90) ? (2000 + yy) : (1900 + yy);
}
