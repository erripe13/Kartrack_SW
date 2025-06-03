/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gps.h"
#include "LoRa.h"
#include "spi.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
extern UART_HandleTypeDef huart6;
extern void safe_printf(const char *format, ...);

LoRa myLoRa;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osThreadId gpsTaskHandle;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;
uint32_t defaultTaskBuffer[ 1024 ];
osStaticThreadDef_t defaultTaskControlBlock;
osThreadId LoRa_initHandle;
uint32_t LoRa_initBuffer[ 1024 ];
osStaticThreadDef_t LoRa_initControlBlock;
osMutexId printf_mutexHandle;
osStaticMutexDef_t printf_mutexControlBlock;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void LoRa_init_Task(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* Hook prototypes */
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 2 */
__weak void vApplicationIdleHook(void) {
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	 to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
	 task. It is essential that code added to this hook function never attempts
	 to block in any way (for example, call xQueueReceive() with a block time
	 specified, or call vTaskDelay()). If the application makes use of the
	 vTaskDelete() API function (as this demo application does) then it is also
	 important that vApplicationIdleHook() is permitted to return to its calling
	 function, because it is the responsibility of the idle task to clean up
	 memory allocated by the kernel to any task that has since been deleted. */
}
/* USER CODE END 2 */

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(xTaskHandle xTask,
		signed char *pcTaskName) {
	/* Run time stack overflow checking is performed if
	 configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
	 called if a stack overflow is detected. */
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
__weak void vApplicationMallocFailedHook(void) {
	/* vApplicationMallocFailedHook() will only be called if
	 configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
	 function that will get called if a call to pvPortMalloc() fails.
	 pvPortMalloc() is called internally by the kernel whenever a task, queue,
	 timer or semaphore is created. It is also called by various parts of the
	 demo application. If heap_1.c or heap_2.c are used, then the size of the
	 heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	 FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	 to query the size of free heap space that remains (although it does not
	 provide information on how the remaining heap might be fragmented). */
}
/* USER CODE END 5 */

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
		StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
	*ppxIdleTaskStackBuffer = &xIdleStack[0];
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
	/* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* definition and creation of printf_mutex */
  osMutexStaticDef(printf_mutex, &printf_mutexControlBlock);
  printf_mutexHandle = osMutexCreate(osMutex(printf_mutex));

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadStaticDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 1024, defaultTaskBuffer, &defaultTaskControlBlock);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of LoRa_init */
  osThreadStaticDef(LoRa_init, LoRa_init_Task, osPriorityHigh, 0, 1024, LoRa_initBuffer, &LoRa_initControlBlock);
  LoRa_initHandle = osThreadCreate(osThread(LoRa_init), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	if (LoRa_initHandle == NULL) {
		printf("LoRa creation FAILED\r\n");
	} else {
		printf("LoRa created OK\r\n");
	}

	osThreadDef(gpsTask, L76_Task, osPriorityHigh, 0, 2048);
	gpsTaskHandle = osThreadCreate(osThread(gpsTask), &huart6);
	if (gpsTaskHandle == NULL) {
		printf("gpsTask creation FAILED\r\n");
	} else {
		printf("gpsTask created OK\r\n");
	}

  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
	/* Infinite loop */
	for (;;) {

		osDelay(2000);
	}
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_LoRa_init_Task */
/**
 * @brief Function implementing the LoRa_init thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_LoRa_init_Task */
void LoRa_init_Task(void const * argument)
{
  /* USER CODE BEGIN LoRa_init_Task */
	myLoRa = newLoRa(); //cree un objet LoRa
	myLoRa.CS_port = GPIOI;
	myLoRa.CS_pin = GPIO_PIN_0;
	myLoRa.reset_port = GPIOF;
	myLoRa.reset_pin = GPIO_PIN_8;
	myLoRa.DIO0_port = GPIOI;
	myLoRa.DIO0_pin = GPIO_PIN_3;
	myLoRa.hSPIx = &hspi2;

	//	myLoRa.frequency = 433;
	//	myLoRa.spredingFactor = SF_9;
	//	myLoRa.bandWidth = BW_250KHz;
	//	myLoRa.crcRate = CR_4_8;
	//	myLoRa.power = POWER_11db;
	//	myLoRa.overCurrentProtection = 130;
	//	myLoRa.preamble = 10;
	myLoRa.frequency = 433;
	myLoRa.spredingFactor = SF_7;
	myLoRa.bandWidth = BW_125KHz;
	myLoRa.crcRate = CR_4_5;
	myLoRa.preamble = 8;
	myLoRa.power = POWER_20db;

	uint16_t status = LoRa_init(&myLoRa);

	if (status != LORA_OK) {
		//debug uart error message
		printf("LoRa crashed with output : %d\n", status);
	} else {
		printf("LoRa OK ! Chip ID 0x12 detected.\n");
	}
	char *send_data = "Hello Kart !";
	/* Infinite loop */
	for (;;) {
		uint8_t ok = LoRa_transmit(&myLoRa, (uint8_t*) send_data,
				sizeof("Hello Kart !"), 100);
		if (ok) {
			printf("LoRa sent : %s\n", send_data);
		} else {
			printf("LoRa timeout !\n");
		}
		L76_PrintExample();
		osDelay(2000);
	}
  /* USER CODE END LoRa_init_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

