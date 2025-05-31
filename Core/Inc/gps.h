/* gps.h */
#ifndef INC_GPS_H_
#define INC_GPS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f7xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define L76_STDBY_Pin        GPIO_PIN_6
#define L76_STDBY_GPIO_Port  GPIOG
#define L76_NMEA_MAX_LEN     128

typedef struct {
    double latitude;
    double longitude;
    float altitude;
    uint8_t fix_quality;
    uint8_t satellites;
    float speed;
    float course;
    uint8_t hours;
    uint8_t minutes;
    float seconds;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} L76_GPS_Data_t;

void L76_Init(UART_HandleTypeDef *huart);
void L76_Task(void const *argument);
void L76_GetData(L76_GPS_Data_t *data);
void L76_SetStandby(bool standby);
void L76_PrintExample(void);
void L76_Update(void);


#ifdef __cplusplus
}
#endif

#endif /* INC_GPS_H_ */
