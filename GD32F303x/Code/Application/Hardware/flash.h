
#ifndef _FLASH_H

#include "stdio.h"
#include "stdint.h"

#define FLASH_START_ADDRESS	0x08000000
#define FMC_PAGE_SIZE		0x800
void Flash_ErasePage(uint32_t pageaddress,uint32_t erasePageCnt);
int Flash_Program(uint32_t address,uint8_t* data,uint32_t len);
int Flash_Copy(uint32_t dest,uint32_t src,uint32_t size);

void SPI_Flash_EraseSector(uint32_t pageaddress,uint32_t eraseSectorCnt);
int SPI_Flash_Program(uint32_t address,uint8_t* data,uint32_t len);
int SPI_Flash_Copy(uint32_t dest,uint32_t src,uint32_t size);
#endif
