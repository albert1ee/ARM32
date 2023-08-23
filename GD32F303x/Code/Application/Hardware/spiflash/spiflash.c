
#include "spiflash.h"

#define SPI_ADDR(x)	x = (x >> 16) | ( (x&0xFF) << 16) | (x & 0x00FF00)

spiflash_t spiflash;

/**
* @brief
* spi flash init & read spifalsh ID
* @param 
cs: the call back for control pin of spi cs pin
p:spi flash read / write call back
* @retval None
*/
uint32_t spiflashInit(pspiflashcs cs,pspiflashfun p)
{
	spiflash.pcs = cs;
	spiflash.prwfun = p;
	spiflash.pcs(1);
	return 0;
}

/**
* @brief
* wait spi flash idle
* @param 
* @retval None
*/
int spiflashwaitbusy(void)
{
	int ret = -1;
	int timeout = 0;
	while(1)
	{
		uint8_t status = spiflashRDSR();
		if ((status & 0x01) == 0)
		{
			ret = 0;
			break;
		}
		for(int i=0;i<72*1000;i++);
		timeout++;
		if (timeout >= 100)
		{
			break;
		}
	}
	return ret;
}

/**
* @brief
* spi flash write enable
* @param 
* @retval None
*/
void spiflashWREN(void)
{
	spiflash.pcs(0);
	spiflash.prwfun(SPI_W,0x06,1,0,0);
	spiflash.pcs(1);
	spiflashwaitbusy();
}

/**
* @brief
* spi flash write disable
* @param 
* @retval None
*/
void spiflashWRDI(void)
{
	spiflash.pcs(0);
	spiflash.prwfun(SPI_W,0x04,1,0,0);
	spiflash.pcs(1);
}

/**
* @brief
* read spi flash ID
* @param 
* @retval spif flash ID
*/
uint32_t spiflashRDID(void)
{
	uint32_t rdid = 0;
	spiflash.pcs(0);
	spiflash.prwfun(SPI_R,0x9F,1,(uint8_t*)&rdid,3);
	spiflash.pcs(1);
	return rdid;
}

/**
* @brief
* read spi flash status
* @param 
* @retval None
*/
uint8_t spiflashRDSR(void)
{
	uint8_t rdsr = 0;
	spiflash.pcs(0);
	spiflash.prwfun(SPI_R,0x05,1,&rdsr,1);
	spiflash.pcs(1);
	return rdsr;
}

/**
* @brief
* write spi flash status
* @param 
wrsr: status
* @retval None
*/
void spiflashWRSR(uint8_t wrsr)
{
	spiflash.pcs(0);
	spiflash.prwfun(SPI_W,0x01,1,&wrsr,1);
	spiflash.pcs(1);
}

/**
* @brief
* read spi flash data
* @param 
addr:spi flash address
pbuf:the memory of chip for read
size:read size
* @retval None
*/
void spiflashREAD(uint32_t addr,uint8_t*pbuf,uint32_t size)
{
//	uint32_t len = 0;
//	uint32_t addrback;
	spiflash.pcs(0);
		SPI_ADDR(addr);
		spiflash.prwfun(SPI_W,0x03,1,(uint8_t*)&addr,3);
		spiflash.prwfun(SPI_R,0x00,0,pbuf,size);
//	while(size)
//	{
//		addrback = addr;
//		len = addr & 0xFF;
//		if ((len + size) > 0x100)//addr overflow
//		{
//			len = 0x100 - len;
//		}
//		else
//		{
//			len = size;
//		}
//		if (len > size)
//			len = size;
//		size -= len;
//		pbuf += len;
//		addr += len;
//	}
	spiflash.pcs(1);
}

/**
* @brief
* fast read spi flash data
* @param 
addr:spi flash address
pbuf:the memory of chip for read
size:read size
* @retval None
*/
void spiflashFREAD(uint32_t addr,uint8_t*pbuf,uint32_t size)
{
	addr <<= 8;
	spiflash.pcs(0);
	SPI_ADDR(addr);
	spiflash.prwfun(SPI_W,0x0B,1,(uint8_t*)&addr,4);
	spiflash.prwfun(SPI_R,0x00,0,pbuf,size);
	spiflash.pcs(1);
}

/**
* @brief
* spi flash sector erase
* @param 
addr:	erase address
* @retval None
*/
void spiflashSE(uint32_t addr)
{
	//write enable
	spiflashWREN();
	spiflash.pcs(0);
	SPI_ADDR(addr);
	spiflash.prwfun(SPI_W,0x20,1,(uint8_t*)&addr,3);
	spiflash.pcs(1);
	//busy
	spiflashwaitbusy();
}

/**
* @brief
* spi flash block erase
* @param 
addr: erase address
* @retval None
*/
void spiflashBE(uint32_t addr)
{
	//write enable
	spiflashWREN();
	spiflash.pcs(0);
	SPI_ADDR(addr);
	spiflash.prwfun(SPI_W,0x52,1,(uint8_t*)&addr,3);
	spiflash.pcs(1);
	//busy
	spiflashwaitbusy();
}

/**
* @brief
* spi flash chip erase
* @param 
* @retval None
*/
void spiflashCE(void)
{
	//write enable
	spiflashWREN();
	spiflash.pcs(0);
	spiflash.prwfun(SPI_W,0x60,1,0,0);
	spiflash.pcs(1);
	//busy
	spiflashwaitbusy();
}

/**
* @brief
* spi flash program
* @param 
addr:spi flash address
pbuf:the memory of chip for write
size:read size
* @retval None
*/
void spiflashPP(uint32_t addr,uint8_t*pbuf,uint32_t size)
{
	uint32_t len = 0;
	uint32_t addrback;
	//write enable
	spiflashWREN();
	spiflash.pcs(0);
//	SPI_ADDR(addr);
//	spiflash.prwfun(SPI_W,0x02,1,0,0);//cmd
//	spiflash.prwfun(SPI_W,0x02,0,(uint8_t*)&addr,3);	//write addr
//	spiflash.prwfun(SPI_W,0x02,0,pbuf,size);//write data
	while(size)
	{
		addrback = addr;
		SPI_ADDR(addrback);
		len = addr & 0xFF;
		if ((len + size) > 0x100)//addr overflow
		{
			len = 0x100 - len;
		}
		else
		{
			len = size;
		}
		if (len > size)
			len = size;
		spiflash.prwfun(SPI_W,0x02,1,0,0);//cmd
		spiflash.prwfun(SPI_W,0x02,0,(uint8_t*)&addrback,3);	//write addr
		spiflash.prwfun(SPI_W,0x02,0,pbuf,len);//write data
		size -= len;
		pbuf += len;
		addr += len;
	}
	spiflash.pcs(1);
	//busy
	spiflashwaitbusy();
}

/**
* @brief
* spi flash deep power down
* @param 
* @retval None
*/
void spiflashDP(void)
{
	//write enable
	spiflashWREN();
	spiflash.pcs(0);
	spiflash.prwfun(SPI_W,0xB9,1,0,0);
	spiflash.pcs(1);
	//busy
	spiflashwaitbusy();
}

/**
* @brief
* spi flash wake-up
* @param 
* @retval None
*/
void spiflashWKP(void)
{
	spiflash.pcs(0);
	spiflash.prwfun(SPI_W,0xAB,1,0,0);
	spiflash.pcs(1);
}

/**
* @brief
* read rems
* @param 
* @retval None
*/
uint32_t spiflashREMS(void)
{
	uint32_t rems = 0;
	spiflash.pcs(0);
	spiflash.prwfun(SPI_W,0x90,1,(uint8_t*)&rems,3);
	spiflash.prwfun(SPI_R,0x90,0,(uint8_t*)&rems,2);
	spiflash.pcs(1);
	return rems;
}

/**
* @brief
* This function 
* @param 
* @retval None
*/
void spiflashWPSEL(void)
{
	spiflash.pcs(0);
	spiflash.prwfun(SPI_W,0x68,1,0,0);
	spiflash.pcs(1);
}


