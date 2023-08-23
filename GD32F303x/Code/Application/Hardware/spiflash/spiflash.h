

#ifndef _SPI_FLASH_H_
#define _SPI_FLASH_H_

#include "stdint.h"

typedef enum
{
	SPI_W,
	SPI_R,
}SPI_RW;

typedef int(*pspiflashfun)(SPI_RW rw,uint8_t cmd,uint8_t withcmd,uint8_t *pbuf,uint32_t size);
typedef void(*pspiflashcs)(uint8_t level);
typedef struct _spiflash_t
{
	pspiflashcs pcs;
	pspiflashfun prwfun;
}spiflash_t;
extern spiflash_t spiflash;

uint32_t spiflashInit(pspiflashcs cs,pspiflashfun p);
int spiflashwaitbusy(void);
void spiflashWREN(void);
void spiflashWRDI(void);
uint32_t spiflashRDID(void);
uint8_t spiflashRDSR(void);
void spiflashWRSR(uint8_t wrsr);
void spiflashREAD(uint32_t addr,uint8_t*pbuf,uint32_t size);
void spiflashFREAD(uint32_t addr,uint8_t*pbuf,uint32_t size);
void spiflashSE(uint32_t addr);
void spiflashBE(uint32_t addr);
void spiflashCE(void);
void spiflashPP(uint32_t addr,uint8_t*pbuf,uint32_t size);
void spiflashDP(void);
void spiflashWKP(void);
uint32_t spiflashREMS(void);
void spiflashWPSEL(void);
#endif

