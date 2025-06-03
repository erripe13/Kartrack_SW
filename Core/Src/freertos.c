/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications using I2C and TDK driver
 ******************************************************************************
 */
/* USER CODE END Header */

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
#include "i2c.h"
#include "inv_imu_driver.h"
#include <stdio.h>



/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 4096 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

void StartDefaultTask(void *argument);
void StartTask02(void *argument);

/* Definitions for ICM45605_TASK */
osThreadId_t ICM45605_TASKHandle;
uint32_t ICM45605_TASKBuffer[4096];
StaticTask_t ICM45605_TASKControlBlock;
const osThreadAttr_t ICM45605_TASK_attributes = {
  .name = "ICM45605_TASK",
  .cb_mem = &ICM45605_TASKControlBlock,
  .cb_size = sizeof(ICM45605_TASKControlBlock),
  .stack_mem = &ICM45605_TASKBuffer[0],
  .stack_size = sizeof(ICM45605_TASKBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};

/* TDK Driver Instance */
static inv_imu_device_t imu_dev;

/* I2C read/write/delay hooks */



static int stm32_i2c_read(uint8_t reg, uint8_t *buf, uint32_t len) {
    return (HAL_I2C_Mem_Read(&hi2c1, (0x68 << 1), reg, I2C_MEMADD_SIZE_8BIT, buf, len, HAL_MAX_DELAY) == HAL_OK) ? 0 : -1;
}

static int stm32_i2c_write(uint8_t reg, const uint8_t *buf, uint32_t len) {
    return (HAL_I2C_Mem_Write(&hi2c1, (0x68 << 1), reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)buf, len, HAL_MAX_DELAY) == HAL_OK) ? 0 : -1;
}

static void stm32_delay_us(uint32_t us) {
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = us * (HAL_RCC_GetHCLKFreq() / 1000000);
    while ((DWT->CYCCNT - start) < cycles);
}

/* Init FreeRTOS objects */
void MX_FREERTOS_Init(void) {
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);
  ICM45605_TASKHandle = osThreadNew(StartTask02, NULL, &ICM45605_TASK_attributes);
}

void StartDefaultTask(void *argument) {
  for (;;) {
    osDelay(1);
  }
}

void StartTask02(void *argument) {
  inv_imu_sensor_data_t data;
  uint8_t whoami;
  uint32_t counter = 0;

  osDelay(500);

  printf("Initializing ICM45605 via TDK I2C driver...\r\n");

  imu_dev.transport.read_reg  = stm32_i2c_read;
  imu_dev.transport.write_reg = stm32_i2c_write;
  imu_dev.transport.sleep_us  = stm32_delay_us;
  imu_dev.transport.serif_type = UI_I2C;

  if (inv_imu_get_who_am_i(&imu_dev, &whoami) != 0 || whoami != 0xE5) {
    printf("WHO_AM_I failed or incorrect: 0x%02X\r\n", whoami);
    osThreadExit();
  }

  printf("WHO_AM_I: 0x%02X OK\r\n", whoami);
  inv_imu_soft_reset(&imu_dev);

  inv_imu_set_accel_mode(&imu_dev, PWR_MGMT0_ACCEL_MODE_LN);
  inv_imu_set_gyro_mode(&imu_dev, PWR_MGMT0_GYRO_MODE_LN);
  inv_imu_set_accel_frequency(&imu_dev, ACCEL_CONFIG0_ACCEL_ODR_200_HZ);
  inv_imu_set_gyro_frequency(&imu_dev, GYRO_CONFIG0_GYRO_ODR_200_HZ);

  inv_imu_set_accel_fsr(&imu_dev, ACCEL_CONFIG0_ACCEL_UI_FS_SEL_16_G);
  inv_imu_set_gyro_fsr(&imu_dev, GYRO_CONFIG0_GYRO_UI_FS_SEL_2000_DPS);
  printf("ICM45605 ready, streaming data...\r\n");

  for (;;) {
    if (inv_imu_get_register_data(&imu_dev, &data) == 0) {
      if (counter % 50 == 0) {
        printf("Accel X: %6d, Y: %6d, Z: %6d\r\n", data.accel_data[0], data.accel_data[1], data.accel_data[2]);
        printf("Gyro  X: %6d, Y: %6d, Z: %6d\r\n", data.gyro_data[0], data.gyro_data[1], data.gyro_data[2]);
        printf("----------------------------------\r\n");
      }
      counter++;
    } else {
      printf("Failed to read sensor data\r\n");
    }
    osDelay(5);
  }
}
