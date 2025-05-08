
#ifndef __SD_DISKIO_DMA_RTOS_H__
#define __SD_DISKIO_DMA_RTOS_H__

#include "ff_gen_drv.h"
#include "stm32f7xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const Diskio_drvTypeDef SD_Driver;

DSTATUS SD_initialize(BYTE lun);
DSTATUS SD_status(BYTE lun);
DRESULT SD_read(BYTE lun, BYTE *buff, DWORD sector, UINT count);
DRESULT SD_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count);
DRESULT SD_ioctl(BYTE lun, BYTE cmd, void *buff);

void BSP_SD_ReadCpltCallback(void);
void BSP_SD_WriteCpltCallback(void);

#ifdef __cplusplus
}
#endif

#endif /* __SD_DISKIO_DMA_RTOS_H__ */
