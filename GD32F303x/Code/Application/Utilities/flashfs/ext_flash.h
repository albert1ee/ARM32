
// 在这里写C定义的函数声明

#ifndef _FLASH_H_
#define _FLASH_H_

#include "stdlib.h"
#include "stdio.h"
#include "stdint.h"
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FATFS_CNTS      (1)


#ifdef __CC_ARM
#include "userconfig.h"
#include "./spiflash/flashapp.h"
#include "./app/userconfig.h"

#define flash_sector_erase(addr,sector)		SPI_Flash_EraseSector(FAT_FS_FLASH_OFFSET + addr,sector)
#define flash_sector_read(addr,pbuf,size) SPI_Flash_Read(FAT_FS_FLASH_OFFSET + addr,pbuf,size)
#define flash_sector_write(addr,pbuf,size) SPI_Flash_Program(FAT_FS_FLASH_OFFSET + addr,pbuf,size)
#define flash_sector_size()								SPI_FLASH_SECTOR_SIZE
#define flash_memory_size()								FAT_FS_FLASH_SIZE

#include "rtthread.h"
#define fs_malloc(x)            rt_malloc(x)
#define fs_realloc(p,x)         rt_realloc(p,x)
#define fs_free(x)              rt_free(x)
#else


#define fs_malloc(x)            malloc(x)
#define fs_realloc(p,x)         realloc(p,x)
#define fs_free(x)              free(x)

#define FLASH_SECTOR_SIZE       (4096)
#define FLASH_MEMORY_SIZE       (128 * FLASH_SECTOR_SIZE)   //512KB - 4Mbits
int FlashInit(void);
int flash_sector_erase(uint32_t addr,int sector);
int flash_sector_read(uint32_t addr, uint8_t * pbuf,int size);
int flash_sector_write(uint32_t addr, uint8_t * pbuf, int size);
int flash_sector_size(void);
int flash_memory_size(void);

#endif

#ifdef __cplusplus
}
#endif

#endif //_FLASH_H_


