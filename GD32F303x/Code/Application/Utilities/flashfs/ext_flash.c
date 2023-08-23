
#include "ext_flash.h"

#ifndef __CC_ARM
int FlashInit(void)
{
    FILE * p;
    p = fopen("flash.bin","rb");
    if (p == 0)
    {
        p = fopen("flash.bin","wb");
        uint8_t * flashmemory = (uint8_t*)malloc(FLASH_MEMORY_SIZE);
        memset(flashmemory,0xFF,FLASH_MEMORY_SIZE);
        fwrite(flashmemory,FLASH_MEMORY_SIZE,1,p);
        fclose(p);
        free(flashmemory);
    }
    else
    {
        fclose(p);
    }
    return 0;
}
int flash_sector_erase(uint32_t addr,int sector)
{
    (void)addr;
    (void)sector;
    uint8_t * p = (uint8_t*)malloc(FLASH_SECTOR_SIZE);
    memset(p,0xFF,FLASH_SECTOR_SIZE);

    FILE * pf;
    //以flash.bin当作文件系统存储单元
    pf = fopen("flash.bin","rb+");
    fseek(pf,addr,SEEK_SET);
    fwrite(p,FLASH_SECTOR_SIZE,1,pf);
    fclose(pf);
    return 0;
}

int flash_sector_read(uint32_t addr, uint8_t * pbuf,int size)
{
    FILE * p;
    //以flash.bin当作文件系统存储单元
    p = fopen("flash.bin","rb");
    fseek(p,addr,SEEK_SET);
    fread(pbuf,size,1,p);
    fclose(p);
    return 0;
}

int flash_sector_write(uint32_t addr, uint8_t * pbuf, int size)
{
    FILE * p;
    //以flash.bin当作文件系统存储单元
    p = fopen("flash.bin","rb+");
    fseek(p,addr,SEEK_SET);
    fwrite(pbuf,size,1,p);
    fclose(p);
    return 0;
}

int flash_sector_size(void)
{
    return FLASH_SECTOR_SIZE;
}

int flash_memory_size(void)
{
    return FLASH_MEMORY_SIZE;
}

#endif
