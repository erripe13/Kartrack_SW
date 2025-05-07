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
#include "fatfs.h"
#include "sdmmc.h"
#include <stdio.h>
#include <string.h>
#include "ff_gen_drv.h"
#include "sd_diskio.h"
#include "stm32f7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "LoRa.h"
#include "spi.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
LoRa myLoRa;

/* Definitions for SD card semaphore */
osSemaphoreId sdSemaphoreHandle;

/* Private variables ---------------------------------------------------------*/
static volatile DSTATUS Stat = STA_NOINIT;

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId LoRa_initHandle;
osThreadId SDCardTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void SDCard_Task(void const * argument);
DSTATUS SD_initialize(BYTE lun);
DSTATUS SD_status(BYTE lun);
DRESULT SD_read(BYTE lun, BYTE *buff, DWORD sector, UINT count);
DRESULT SD_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count);
DRESULT SD_ioctl(BYTE lun, BYTE cmd, void *buff);
void BSP_SD_ReadCpltCallback(void);
void BSP_SD_WriteCpltCallback(void);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void LoRa_init_Task(void const * argument);

extern void MX_USB_HOST_Init(void);
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

  /* USER CODE BEGIN RTOS_MUTEX */
  /* Add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* Add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* Start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* Add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* Definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 4096);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* Definition and creation of LoRa_init */
  osThreadDef(LoRa_init, LoRa_init_Task, osPriorityNormal, 0, 128);
  LoRa_initHandle = osThreadCreate(osThread(LoRa_init), NULL);

  /* Definition and creation of SDCardTask */
  osThreadDef(SDCardTask, SDCard_Task, osPriorityNormal, 0, 512);
  SDCardTaskHandle = osThreadCreate(osThread(SDCardTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* Add threads, ... */
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
  /* init code for USB_HOST */
  MX_USB_HOST_Init();
  /* USER CODE BEGIN StartDefaultTask */
	/* Infinite loop */
	for (;;) {
		osDelay(1);
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

	myLoRa.frequency = 434;
	myLoRa.spredingFactor = SF_9;
	myLoRa.bandWidth = BW_250KHz;
	myLoRa.crcRate = CR_4_8;
	myLoRa.power = POWER_17db;
	myLoRa.overCurrentProtection = 130;
	myLoRa.preamble = 10;

	uint16_t status = LoRa_init(&myLoRa);

	if (status != LORA_OK) {
		//debug uart error message
		printf("LoRa crashed with output : %d\n", status);
	}
	char *send_data = "Hello Kart !";
	/* Infinite loop */
	for (;;) {
		LoRa_transmit(&myLoRa, (uint8_t*)send_data, sizeof(send_data), 100);
		osDelay(1000);
	}
  /* USER CODE END LoRa_init_Task */
}

/* USER CODE BEGIN Header_SDCard_Task */
/**
 * @brief Function implementing the SDCardTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_SDCard_Task */
void SDCard_Task(void const * argument)
{
  /* USER CODE BEGIN SDCard_Task */
  FATFS fs;          // File system object
  FIL file;          // File object
  FRESULT res;       // FatFs function common result code
  UINT bytesWritten; // Number of bytes written
  UINT bytesRead;    // Number of bytes read
  char buffer[100];  // Buffer for reading/writing
  char *folderName = "KartData";
  char *fileName = "KartData/log.txt";

  // Mount the file system
  res = f_mount(&fs, SDPath, 1);
  if (res != FR_OK) {
    printf("Error mounting SD card: %d\r\n", res);
    vTaskDelete(NULL); // Exit the task if mounting fails
  }
  printf("SD card mounted successfully.\r\n");

  // Create a folder
  res = f_mkdir(folderName);
  if (res == FR_OK) {
    printf("Folder '%s' created successfully.\r\n", folderName);
  } else if (res == FR_EXIST) {
    printf("Folder '%s' already exists.\r\n", folderName);
  } else {
    printf("Error creating folder '%s': %d\r\n", folderName, res);
  }

  // Create and write to a file
  res = f_open(&file, fileName, FA_WRITE | FA_CREATE_ALWAYS);
  if (res == FR_OK) {
    printf("File '%s' opened successfully.\r\n", fileName);

    // Write data to the file
    char *data = "Hello Kart! This is a test log.\r\n";
    res = f_write(&file, data, strlen(data), &bytesWritten);
    if (res == FR_OK) {
      printf("Data written to file: %s\r\n", data);
    } else {
      printf("Error writing to file: %d\r\n", res);
    }

    // Close the file
    f_close(&file);
  } else {
    printf("Error opening file '%s': %d\r\n", fileName, res);
  }

  // Read the file
  res = f_open(&file, fileName, FA_READ);
  if (res == FR_OK) {
    printf("File '%s' opened for reading.\r\n", fileName);

    // Read data from the file
    res = f_read(&file, buffer, sizeof(buffer) - 1, &bytesRead);
    if (res == FR_OK) {
      buffer[bytesRead] = '\0'; // Null-terminate the string
      printf("Data read from file:\r\n%s\r\n", buffer);
    } else {
      printf("Error reading from file: %d\r\n", res);
    }

    // Close the file
    f_close(&file);
  } else {
    printf("Error opening file '%s' for reading: %d\r\n", fileName, res);
  }

  // Unmount the file system
  f_mount(NULL, SDPath, 1);
  printf("SD card unmounted.\r\n");

  // End the task
  vTaskDelete(NULL); // Correctly terminate the task
  /* USER CODE END SDCard_Task */
}

/* USER CODE BEGIN SD_DRIVER */
/* Diskio driver structure */
const Diskio_drvTypeDef SD_Driver = {
    SD_initialize,
    SD_status,
    SD_read,
    SD_write,
    SD_ioctl,
};

/* Semaphore for DMA completion */
static osSemaphoreId sdDmaSemaphore;

/* Initialize SD card */
DSTATUS SD_initialize(BYTE lun) {
    if (BSP_SD_Init() == MSD_OK) {
        Stat &= ~STA_NOINIT;
    } else {
        Stat = STA_NOINIT;
    }

    /* Create semaphore for DMA completion */
    if (sdDmaSemaphore == NULL) {
        osSemaphoreDef(sdDmaSemaphore);
        sdDmaSemaphore = osSemaphoreCreate(osSemaphore(sdDmaSemaphore), 1);
        osSemaphoreWait(sdDmaSemaphore, 0); // Initialize to unavailable
    }

    return Stat;
}

/* Get SD card status */
DSTATUS SD_status(BYTE lun) {
    return Stat;
}

/* Read sectors using DMA */
DRESULT SD_read(BYTE lun, BYTE *buff, DWORD sector, UINT count) {
    if (BSP_SD_ReadBlocks_DMA((uint32_t *)buff, sector, count) == MSD_OK) {
        /* Wait for DMA transfer to complete */
        if (osSemaphoreWait(sdDmaSemaphore, 1000) == osOK) {
            return RES_OK;
        } else {
            return RES_ERROR;
        }
    }
    return RES_ERROR;
}

/* Write sectors using DMA */
DRESULT SD_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count) {
    if (BSP_SD_WriteBlocks_DMA((uint32_t *)buff, sector, count) == MSD_OK) {
        /* Wait for DMA transfer to complete */
        if (osSemaphoreWait(sdDmaSemaphore, 1000) == osOK) {
            return RES_OK;
        } else {
            return RES_ERROR;
        }
    }
    return RES_ERROR;
}

/* IOCTL function for SD card */
DRESULT SD_ioctl(BYTE lun, BYTE cmd, void *buff) {
    BSP_SD_CardInfo CardInfo; // Déclarez une structure pour stocker les informations de la carte SD

    switch (cmd) {
        case CTRL_SYNC:
            return RES_OK;

        case GET_SECTOR_COUNT:
            BSP_SD_GetCardInfo(&CardInfo); // Passez l'adresse de la structure
            *(DWORD *)buff = CardInfo.LogBlockNbr; // Utilisez les informations de la structure
            return RES_OK;

        case GET_SECTOR_SIZE:
            BSP_SD_GetCardInfo(&CardInfo); // Passez l'adresse de la structure
            *(WORD *)buff = CardInfo.LogBlockSize; // Utilisez les informations de la structure
            return RES_OK;

        case GET_BLOCK_SIZE:
            *(DWORD *)buff = 1; // Erase block size in units of sectors
            return RES_OK;

        default:
            return RES_PARERR;
    }
}

/* Callback for read completion */
void BSP_SD_ReadCpltCallback(void) {
    osSemaphoreRelease(sdDmaSemaphore);
}

/* Callback for write completion */
void BSP_SD_WriteCpltCallback(void) {
    osSemaphoreRelease(sdDmaSemaphore);
}
/* USER CODE END SD_DRIVER */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

