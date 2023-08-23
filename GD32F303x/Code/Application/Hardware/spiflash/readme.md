## 简介
 基于4线spi对spi flash的读写控制，按照以下步骤实现。

### 硬件接口
 用户需提供2个硬件接口：spi cs IO硬件控制接口和4线spi读写接口，将这2个接口通过spiflashInit初始化进行设置。
 #### spi cs IO
 spi cs IO控制函数类型满足typedef void(*pspiflashcs)(uint8_t level);
 level 为0 表示设置cs io为低电平，如果为1表示设置cs io为高电平。
 #### spi读写
 spi读写使用typedef int(*pspiflashfun)(SPI_RW rw,uint8_t cmd,uint8_t withcmd,uint8_t *pbuf,uint32_t size);
 参数说明：
  
  rw:表示当前操作是读还是写

  cmd：当前操作要发送的命令

  withcmd：表示当前操作是否要发送cmd

  pbuf：读或写的缓存
  
  size：读或写的数据量

## 示例
 ### GD32F SPI0 示例
 实现spiflash接口读写回调函数及CS pin的设置函数
 实现spiflash内容读写与擦除
 本文中有使用rt thread互斥向量，如果不需要请自行屏蔽
 ```
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
 ```