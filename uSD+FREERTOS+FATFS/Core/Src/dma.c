/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    dma.c
 * @brief   This file provides code for the configuration
 *          of all the requested memory to memory DMA transfers.
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
#include "dma.h"

/* USER CODE BEGIN 0 */
#include "sdmmc.h"
#include "main.h"
#include <stdio.h>

extern SD_HandleTypeDef hsd1;
extern DMA_HandleTypeDef hdma_sdmmc1_tx;
extern DMA_HandleTypeDef hdma_sdmmc1_rx;

/* Callback manuelle pour DMA TX → SDMMC */
void SD_TxDMACallback(DMA_HandleTypeDef *hdma) {
	printf("[DEBUG] SD_TxDMACallback fired\r\n");
	HAL_SD_TxCpltCallback(hdma->Parent);
}

/* Callback manuelle pour DMA RX → SDMMC */
void SD_RxDMACallback(DMA_HandleTypeDef *hdma) {
	printf("[DEBUG] SD_RxDMACallback fired\r\n");
	HAL_SD_RxCpltCallback(hdma->Parent);
}
/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure DMA                                                              */
/*----------------------------------------------------------------------------*/

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
 * Enable DMA controller clock
 */
void MX_DMA_Init(void) {

	/* DMA controller clock enable */
	__HAL_RCC_DMA2_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA2_Stream3_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
	/* DMA2_Stream6_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

	hdma_sdmmc1_tx.XferCpltCallback = SD_TxDMACallback;
	hdma_sdmmc1_tx.Parent = &hsd1;

	hdma_sdmmc1_rx.XferCpltCallback = SD_RxDMACallback;
	hdma_sdmmc1_rx.Parent = &hsd1;

	printf("[DEBUG] XferCpltCallback addr = 0x%08lX\r\n", (uint32_t)hdma_sdmmc1_tx.XferCpltCallback);
	printf("[DEBUG] SD_TxDMACallback address = 0x%08lX\r\n", (uint32_t)SD_TxDMACallback);


}

/* USER CODE BEGIN 2 */
//to be put in the INIT function if MX generation breaks it :
//hdma_sdmmc1_tx.XferCpltCallback = SD_TxDMACallback;
//hdma_sdmmc1_tx.Parent = &hsd1;
//
//hdma_sdmmc1_rx.XferCpltCallback = SD_RxDMACallback;
//hdma_sdmmc1_rx.Parent = &hsd1;
/* USER CODE END 2 */

