#include "ff_gen_drv.h"
#include "sd_diskio_dma_rtos.h"
#include "cmsis_os.h"
#include "stm32f7xx_hal.h"

/* Definitions for SD card semaphore */
osSemaphoreId sdSemaphoreHandle;

/* Private variables ---------------------------------------------------------*/
static volatile DSTATUS Stat = STA_NOINIT;

/* Private function prototypes -----------------------------------------------*/
DSTATUS SD_initialize(BYTE lun);
DSTATUS SD_status(BYTE lun);
DRESULT SD_read(BYTE lun, BYTE *buff, DWORD sector, UINT count);
DRESULT SD_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count);
DRESULT SD_ioctl(BYTE lun, BYTE cmd, void *buff);

/* Diskio driver structure */
const Diskio_drvTypeDef SD_Driver = {
    SD_initialize,
    SD_status,
    SD_read,
    SD_write,
    SD_ioctl,
};

/* Callbacks for DMA completion */
void BSP_SD_ReadCpltCallback(void);
void BSP_SD_WriteCpltCallback(void);

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
    BSP_SD_CardInfo cardInfo;
    switch (cmd) {
        case CTRL_SYNC:
            return RES_OK;

        case GET_SECTOR_COUNT:
            BSP_SD_GetCardInfo(&cardInfo);
            *(DWORD *)buff = cardInfo.LogBlockNbr;
            return RES_OK;

        case GET_SECTOR_SIZE:
            BSP_SD_GetCardInfo(&cardInfo);
            *(WORD *)buff = cardInfo.LogBlockSize;
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
