
#include "main.h"

#include "flashapp.h"

	static struct rt_mutex spi_flash_mutex;
/**
* @brief
* This function is the spi flash call back for read/write
* @param 
SPI_RW 	:for read or write
cmd		 	:for spi command
withcmd :send cmd or not
pbuf		:the buf of read or write
size		:the size of buf for read or write
* @retval None
*/
int spiflashRW(SPI_RW rw,uint8_t cmd,uint8_t withcmd,uint8_t *pbuf,uint32_t size)
{
	if (withcmd)	//send cmd
	{
		spi_i2s_data_transmit(SPI0,cmd);
		while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE));
		for(int i=0;i<120;i++);
	}
	if (rw == SPI_W)
	{
		for(int i=0;i<size;i++)
		{
			spi_i2s_data_transmit(SPI0,pbuf[i]);
			while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE));
		}
	}
	else if (rw == SPI_R)
	{
		for(int i=0;i<size;i++)
		{
			spi_i2s_data_transmit(SPI0,spi_i2s_data_receive(SPI0));
			while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE));
			while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE));
			pbuf[i] = spi_i2s_data_receive(SPI0);
		}
	}
	for(int i=0;i<120;i++);
	return 0;
}
/**
* @brief
* This function is spi flash chip select pin call back
* @param
* @retval None
*/
void spiflashcs(uint8_t level)
{
	gpio_bit_write(GPIOA,GPIO_PIN_4,(bit_status)level);
}

void SpiInit(void)
{
	rcu_periph_clock_enable(RCU_GPIOA);
	//PA4 SPI CS PIN
	gpio_bit_set(GPIOA,GPIO_PIN_4);
	gpio_init(GPIOA,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_4);
	
	rcu_periph_clock_enable(RCU_SPI0);
	spi_nss_output_disable(SPI0);
	
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5 | GPIO_PIN_7);
	gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
	spi_parameter_struct spi_init_struct;

	/* SPI0 parameter config */
	spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
	spi_init_struct.device_mode          = SPI_MASTER;
	spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
	spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
	spi_init_struct.nss                  = SPI_NSS_SOFT;
	spi_init_struct.prescale             = SPI_PSC_8;
	spi_init_struct.endian               = SPI_ENDIAN_MSB;
	spi_init(SPI0, &spi_init_struct);
	
	spi_enable(SPI0);
}

void FlashAppInit(void)
{
	static uint8_t isInit = 0;
	
	if (isInit)
		return;
	
	isInit = 1;
	SpiInit();
	spiflashInit(spiflashcs,spiflashRW);
	
	rt_mutex_init(&spi_flash_mutex,"spi_mut",RT_IPC_FLAG_FIFO);
}

/**
* @brief
* erase flash page(exteral flash)
* @param 
pageaddress: the flash address, aglin page
erasepagecnt: the page counts
* @retval None
*/
void SPI_Flash_EraseSector(uint32_t pageaddress,uint32_t eraseSectorCnt)
{
	if (rt_mutex_take(&spi_flash_mutex,100) != RT_EOK)
		return;
	//erase
	for(int i=0;i<eraseSectorCnt;i++)
	{
		spiflashSE(pageaddress + i*SPI_FLASH_SECTOR_SIZE);
	}
	rt_mutex_release(&spi_flash_mutex);
}
/*
*/
int SPI_Flash_Read(uint32_t address,uint8_t* data,int len)
{
	if (len <= 0)
	{
		return -1;
	}
	if (rt_mutex_take(&spi_flash_mutex,100) != RT_EOK)
		return -2;
	uint32_t maxsize = 0;
	while(len > 0)
	{
		maxsize = SPI_FLASH_PAGE_SIZE - (address & (SPI_FLASH_PAGE_SIZE - 1));
		if (maxsize >= len)
		{
			maxsize = len;
		}
		spiflashREAD(address,(uint8_t*)data,maxsize);
		address += maxsize;
		data += maxsize;
		len -= maxsize;
	}
	rt_mutex_release(&spi_flash_mutex);
	return 0;
}
/**
* @brief
* program flash(internal flash)
* @param 
address:flash address
data:data point
len: flash program length
* @retval None
*/
int SPI_Flash_Program(uint32_t address,uint8_t* data,uint32_t len)
{
	uint32_t end_addr, current_size, current_addr;
	
	if (rt_mutex_take(&spi_flash_mutex,100) != RT_EOK)
		return -1;
	/* Calculation of the size between the write address and the end of the page */
  current_addr = 0;

  while (current_addr <= address)
  {
    current_addr += SPI_FLASH_PAGE_SIZE;
  }
  current_size = current_addr - address;

  /* Check if the size of the data is less than the remaining place in the page */
  if (current_size > len)
  {
    current_size = len;
  }

  /* Initialize the adress variables */
  current_addr = address;
  end_addr = address + len;
	
  /* Perform the write page by page */
  do
  {
		spiflashPP(current_addr,data,current_size);
    /* Update the address and size variables for next page programming */
    current_addr += current_size;
    data += current_size;
    current_size = ((current_addr + SPI_FLASH_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : SPI_FLASH_PAGE_SIZE;
  } while (current_addr < end_addr);
	
	rt_mutex_release(&spi_flash_mutex);
	return 0;
}

/**
* @brief
* flash copy to flash
* @param 
dest:flash address
src:	flash address
size:	copy size
* @retval 
0:success
-1:malloc fail
-2:the address error
-3:dest not aglin 4
-4:src not aglin 4
*/
int SPI_Flash_Copy(uint32_t dest,uint32_t src,uint32_t size)
{
//	if (size == 0)
//		return 0;
//	if (dest & (SPI_FLASH_SECTOR_SIZE - 1))
//		return -3;
//	if (src & (SPI_FLASH_SECTOR_SIZE - 1))
//		return -4;
//	uint32_t pagebuf = (uint32_t)mymalloc(SPI_FLASH_PAGE_SIZE);
//	if (pagebuf == 0)
//		return -1;
//	uint32_t offset = 0;
//	while(offset < size)
//	{
//		spiflashSE(dest);
//		for(int i=0; (i< SPI_FLASH_SECTOR_SIZE) && (offset < size) ;)
//		{
//			spiflashREAD(src,(uint8_t*)pagebuf,SPI_FLASH_PAGE_SIZE);
//			spiflashPP(dest,(uint8_t*)pagebuf,SPI_FLASH_PAGE_SIZE);
//			i += SPI_FLASH_PAGE_SIZE;
//			offset += SPI_FLASH_PAGE_SIZE;
//			src += SPI_FLASH_PAGE_SIZE;
//			dest += SPI_FLASH_PAGE_SIZE;
//		}
//	}
//	myfree((void*)pagebuf);
	return 0;
}

