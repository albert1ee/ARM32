
#ifndef _FLASH_APP_H_
#define _FLASH_APP_H_

#include "spiflash.h"

#define SPI_FLASH_PAGE_SIZE			(0x100)
#define SPI_FLASH_SECTOR_SIZE		(0x1000)

void FlashAppInit(void);
void SPI_Flash_EraseSector(uint32_t pageaddress,uint32_t eraseSectorCnt);
int SPI_Flash_Read(uint32_t address,uint8_t* data,int len);
int SPI_Flash_Program(uint32_t address,uint8_t* data,uint32_t len);


#endif
