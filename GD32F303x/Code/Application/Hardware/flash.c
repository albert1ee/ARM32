
#include "flash.h"
#include "main.h"
void Flash_ErasePage(uint32_t pageaddress,uint32_t erasePageCnt)
{
	fmc_unlock();
	
	/* erase the flash pages */
	for(uint32_t EraseCounter = 0; EraseCounter < erasePageCnt; EraseCounter++)
	{
		if ((pageaddress + (FMC_PAGE_SIZE * EraseCounter)) >= (512 * 1024))
		{
			/* clear all pending flags */
			fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR);
		}
		else
		{
			/* clear all pending flags */
			fmc_flag_clear(FMC_FLAG_BANK1_END | FMC_FLAG_BANK1_WPERR | FMC_FLAG_BANK1_PGERR);
		}
		fmc_page_erase(pageaddress + (FMC_PAGE_SIZE * EraseCounter));
	}

	/* lock the main FMC after the erase operation */
	fmc_lock();
}

int Flash_Program(uint32_t address,uint8_t* data,uint32_t len)
{
	uint64_t Write_Flash_Data = 0;
	uint8_t writeBytes;
	if (len & 0x01)
		return -1;
	fmc_unlock();
	while(len)
	{
		Write_Flash_Data = 0;
		if (len >= 4)
		{
			if ((address & 0x03) == 0)
			{
				writeBytes = 4;
				for(int i=0;i<4;i++)
				{
					Write_Flash_Data <<= 8;
					Write_Flash_Data |= data[3-i];
				}
			}
			else
			{
				writeBytes = 2;
				for(int i=0;i<2;i++)
				{
					Write_Flash_Data <<= 8;
					Write_Flash_Data |= data[1-i];
				}
			}
		}
		else
		{
			writeBytes = 2;
			for(int i=0;i<2;i++)
			{
				Write_Flash_Data <<= 8;
				Write_Flash_Data |= data[1-i];
			}
		}
		if (address >= (512 * 1024))
		{
			/* clear all pending flags */
			fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR);
		}
		else
		{
			/* clear all pending flags */
			fmc_flag_clear(FMC_FLAG_BANK1_END | FMC_FLAG_BANK1_WPERR | FMC_FLAG_BANK1_PGERR);
		}
		if (writeBytes == 4)
		{
			fmc_word_program(address,Write_Flash_Data);
		}
		else
		{
			fmc_halfword_program(address,Write_Flash_Data);
		}
		len -= writeBytes;
		address += writeBytes;
		data += writeBytes;
		
	}
	fmc_lock();
	return 0;
}

