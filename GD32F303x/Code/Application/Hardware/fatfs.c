
#include "main.h"
#include "fatfs.h"
#include "flashfs.h"

FATFS fs;

void fatfs_init(void)
{
	FRESULT res;
	FlashAppInit();
	rt_kprintf("SPI Flash ID = 0x%08x\n", spiflashRDID());
	if (f_mount(0,&fs) != FR_OK)
	{
		rt_kprintf("f_mount fail\n");
		res = f_mkfs(0);
		rt_kprintf("f_mkfs res=%d\n",res);
		f_mount(0,&fs);
	}
}
void fatfs_format(int argc,char** argv)
{
	FRESULT res;
	res = f_mkfs(0);
	rt_kprintf("format res = %d\n",res);
	if (res == FR_OK)
	{
		fatfs_init();
	}
}
MSH_CMD_EXPORT(fatfs_format, format fatfs);

uint32_t fatfs_freesize(void)
{
	uint32_t freesize = 0;
	f_getfree(&fs,&freesize);
	rt_kprintf("freesize = %dB\n",freesize);
	return freesize;
}
MSH_CMD_EXPORT(fatfs_freesize, get fatfs free size);

FRESULT ls(int argc,char** argv)
{
	char * pbuf = rt_malloc(64+4);
	uint32_t offset = 0;
	while(pbuf)
	{
		memset(pbuf,0,64+4);
		if (f_ls(&fs,argv[1],(uint8_t*)pbuf,64,&offset) == 0)
		{
			break;
		}
		rt_kprintf("file=%s\n",pbuf);
	}
	rt_free(pbuf);
	return FR_OK;
}
MSH_CMD_EXPORT(ls, scan fatfs folder);

//删除文件
int funlink(int argc,char ** argv)
{
	if (argc == 2)
	{
		FRESULT fr = f_unlink(argv[1]);
		rt_kprintf("remove file = %s, ret = %d\n",argv[1],fr);
	}
	return 0;
}
MSH_CMD_EXPORT(funlink,del file from fatfs)



#if 1	//系统配置文件，0:/sysconfig.ini
int sysfs_config_write(char * key,char * pvalue)
{
	char * pbuf = 0;
	uint32_t br = 0;
	uint32_t keylen = strlen(key);
	FIL fr;
	FIL fw;
	uint32_t size = 0;
	if (f_open(&fr,"0:/sysconfig.ini",FA_READ) == FR_OK)
	{
		size = fr.obj.objsize;
	}

	if (f_open(&fw,"0:/sysconfig_temp.ini",FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
	{
		if (size)
		while (f_readline(&fr,(void*)&pbuf,&br) == FR_OK)
		{
			if (pbuf)
			{
				rt_kprintf("%s",pbuf);
				if ((strstr(pbuf,key) == pbuf) && (pbuf[keylen] == ':'))
				{
					if (key && pvalue)
					{
						//更换当前行
						br = strlen(key) + strlen(pvalue) + 4;
						char *p = rt_malloc(br);
						if (p)
						{
							memset(p,0,br);
							sprintf(p,"%s:%s\n",key,pvalue);
							f_write(&fw,p,br,&br);
							rt_free(p);
						}
					}
					key = 0;
					pvalue = 0;
				}
				else
				{
					//当前行写入临时文件
					f_write(&fw,pbuf,br,&br);
				}
				//释放内存
				rt_free(pbuf);
			}
			else
			{
				break;
			}
		}
		if (key && pvalue)
		{
			//没有写入，最后写入
			br = strlen(key) + strlen(pvalue) + 4;
			pbuf = rt_malloc(br);
			if (pbuf)
			{
				memset(pbuf,0,br);
				sprintf(pbuf,"%s:%s\n",key,pvalue);
				f_write(&fw,pbuf,strlen(pbuf),&br);
				rt_free(pbuf);
			}
		}
		f_close(&fw);
		if (size)
			f_close(&fr);
		f_unlink("0:/sysconfig.ini");
		f_rename("0:/sysconfig_temp.ini","0:/sysconfig.ini");
	}
	return 0;
}

char * sysfs_config_read(char * key)
{
	char * pbuf = 0;
	uint32_t br = 0;
	uint32_t keylen = strlen(key);
	FIL f;
	if (f_open(&f,"0:/sysconfig.ini",FA_READ) == FR_OK)
	{
		while (f_readline(&f,(void*)&pbuf,&br) == FR_OK)
		{
			if (pbuf)
			{
				rt_kprintf("%s",pbuf);
				if ((strstr(pbuf,key) == pbuf) && (pbuf[keylen] == ':'))
				{
					break;
				}
				rt_free(pbuf);
			}
			else
			{
				break;
			}
		}
		f_close(&f);
	}
	return pbuf;
}

#endif

