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
#include "fatfs.h"         // pour FATFS, FIL, SDPath, FA_*
#include "ff.h"            // pour les constantes FatFs comme FA_CREATE_ALWAYS, f_mount, f_open...
#include "string.h"        // pour strlen
#include "diskio.h"  		// pour SD_Driver
#include "bsp_driver_sd.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//extern char SDPath[];
extern SD_HandleTypeDef hsd1;
extern FATFS SDFatFs;
extern FIL MyFile;
extern char SDPath[4];

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
uint32_t defaultTaskBuffer[4096];
osStaticThreadDef_t defaultTaskControlBlock;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
		StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize);

/* Hook prototypes */
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 2 */
void vApplicationIdleHook(void) {
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
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName) {
	/* Run time stack overflow checking is performed if
	 configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
	 called if a stack overflow is detected. */
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
void vApplicationMallocFailedHook(void) {
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
	osThreadStaticDef(defaultTask, StartDefaultTask, osPriorityHigh, 0, 4096,
			defaultTaskBuffer, &defaultTaskControlBlock);
	defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const *argument) {
	/* USER CODE BEGIN StartDefaultTask */
	FRESULT res;
	uint32_t byteswritten, bytesread;
	uint8_t wtext[] = "This is STM32 working with FatFs\r\n";
	uint8_t rtext[100];
	BYTE workBuffer[512];

	printf("[INFO] StartDefaultTask running (FreeRTOS OK)\r\n");

	// Step 1 - Mount the filesystem
	printf("[INFO] Mounting filesystem...\r\n");
	res = f_mount(&SDFatFs, (TCHAR const*) SDPath, 1);
	printf("[DEBUG] f_mount returned: %d\r\n", res);

	// Step 2 - Format if needed
	if (res == FR_NO_FILESYSTEM) {
		printf("[WARN] No filesystem found. Formatting...\r\n");
		res = f_mkfs(SDPath, FM_ANY, 0, workBuffer, sizeof(workBuffer));
		printf("[DEBUG] f_mkfs returned: %d\r\n", res);
		if (res != FR_OK)
			Error_Handler();

		res = f_mount(&SDFatFs, (TCHAR const*) SDPath, 1);
		if (res != FR_OK)
			Error_Handler();
	} else if (res != FR_OK) {
		printf("[ERROR] f_mount failed. res = %d\r\n", res);
		Error_Handler();
	}

	// Step 3 - Open for write
	printf("[INFO] Opening file for writing...\r\n");
	res = f_open(&MyFile, "STM32.TXT", FA_CREATE_ALWAYS | FA_WRITE);
	if (res != FR_OK) {
		printf("[ERROR] f_open failed. res = %d\r\n", res);
		Error_Handler();
	}

	// Step 4 - Write
	res = f_write(&MyFile, wtext, sizeof(wtext), (void*) &byteswritten);
	if ((byteswritten == 0) || (res != FR_OK)) {
		printf("[ERROR] f_write failed. res = %d, bytes = %lu\r\n", res,
				byteswritten);
		Error_Handler();
	}
	f_close(&MyFile);
	printf("[INFO] Write OK (%lu bytes)\r\n", byteswritten);

	// Step 5 - Read back
	printf("[INFO] Reading back the file...\r\n");
	res = f_open(&MyFile, "STM32.TXT", FA_READ);
	if (res != FR_OK) {
		printf("[ERROR] f_open for read failed. res = %d\r\n", res);
		Error_Handler();
	}

	res = f_read(&MyFile, rtext, sizeof(rtext) - 1, (UINT*) &bytesread);
	rtext[bytesread] = '\0';
	if ((bytesread == 0) || (res != FR_OK)) {
		printf("[ERROR] f_read failed. res = %d, bytesread = %lu\r\n", res,
				bytesread);
		Error_Handler();
	}
	f_close(&MyFile);
	printf("[INFO] Read OK (%lu bytes): '%s'\r\n", bytesread, rtext);

	// Step 6 - Success LED
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, GPIO_PIN_RESET);
	/* Infinite loop */
	for (;;) {
		osDelay(1);
	}
	/* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

