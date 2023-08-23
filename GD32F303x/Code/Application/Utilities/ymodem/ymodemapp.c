
#include "main.h"
#include "flashfs.h"
#include "fatfs.h"
#include "./motor_action_track/motor_action_track_fs.h"

/*
用户根据提示实现相应写flash/文件，内存分配与释放
*/

	extern void console_getchar_set(int enable);
	
#include "ymodemapp.h"


#ifdef YMODEM_APP_ENABLE
#define FLASH_PAGE_SIZE	0x800

ymodem_app_t ymodem_app =
{
	.freeflashaddr = 0,
	.maxflashsize = 0,
	.ymodemtimeout = 0,
	.pfile = 0,
	.pfilename = 0,
	.ymodem_mode = -1,
};

void YmodemTx(uint8_t*p,uint32_t len)
{
	//用户实现发送功能
	SEGGER_RTT_Write(0,p,len);
}
int YmodemRx(uint8_t*p,uint32_t len)
{
	//用户实现接收功能
	return SEGGER_RTT_Read(0,p,len);
}

int YmodemWriteFw(uint8_t*p,uint32_t len,YMODEMWRITETYPE type)
{
	if (type == eYM_FILESIZE)
	{
		if (*(uint32_t*)p > ymodem_app.maxflashsize)
		{
			return -1;
		}
		//erase
		uint32_t pagecnt = 0;
		pagecnt = *(uint32_t*)p / FLASH_PAGE_SIZE;
		if (*(uint32_t*)p & (FLASH_PAGE_SIZE - 1))
		{
			pagecnt++;
		}
		//获取当前文件大小，擦除对应flash
		Flash_ErasePage(ymodem_app.freeflashaddr,pagecnt);
	}
	else if (type == eYM_FILENAME)
	{
	}
	else if (type == eYM_DATA)
	{
		//数据写到对应flash地址
		Flash_Program(ymodem_app.freeflashaddr,p,len);
		ymodem_app.freeflashaddr += len;
	}
	return 0;
}

int YmodemWriteMotorFile(uint8_t*p,uint32_t len,YMODEMWRITETYPE type)
{
	if (type == eYM_FILESIZE)
	{
		// 传输文件大小
		if (*(uint32_t*)p > ymodem_app.maxflashsize)
		{
			return -1;
		}
	}
	else if (type == eYM_FILENAME)
	{
	}
	else if (type == eYM_DATA)
	{
	}
	return 0;
}

int YmodemWriteFatfsMotorFile(uint8_t*p,uint32_t len,YMODEMWRITETYPE type)
{
	if (type == eYM_FILESIZE)
	{
		//传输文件大小
		if (*(uint32_t*)p > ymodem_app.maxflashsize)
		{
			return -1;
		}
	}
	else if (type == eYM_FILENAME)
	{
		//根据文件名创建文件
		//用户实现文件结构体
		ymodem_app.pfile = rt_malloc(sizeof(FIL));
		if (ymodem_app.pfile)
		{
//			char *pfile = strstr((const char*)p,"/");
//			if (pfile == 0)
//			{
//				rt_free(ymodem_app.pfile);
//				return -4;
//			}
//			len = strlen(pfile+1);
			len = strlen((const char*)p);
			ymodem_app.pfilename = rt_malloc(len + 4 + strlen("0:/"));
			len = sprintf((char*)ymodem_app.pfilename,"0:/%s",p);
			ymodem_app.pfilename[len] = 0;
//			memset(ymodem_app.pfilename,0,len + 4);
//			memcpy((void*)(ymodem_app.pfilename),pfile+1,len);
//			memcpy((void*)(ymodem_app.pfilename),p,len);
			//用户实现创建文件功能
			FRESULT fr = f_open(ymodem_app.pfile, ymodem_app.pfilename, FA_CREATE_ALWAYS | FA_WRITE);
			if (fr != FR_OK)
			{
				rt_free(ymodem_app.pfile);
				ymodem_app.pfile = 0;
				rt_free(ymodem_app.pfilename);
				ymodem_app.pfilename = 0;
				return -2;
			}
		}
		else
		{
			return -1;
		}
	}
	else if (type == eYM_DATA)
	{
		if (ymodem_app.pfile)
		{
			uint32_t w = 0;
			//用户实现写数据到对应文件
			if (f_write(ymodem_app.pfile,p,len,&w) != FR_OK)
				return -1;
		}
	}
	return 0;
}
//启动ymodem，根据参数选择传输模式
//0:更新固件，1：传文件
void rb(int argc,char** argv)
{
	int index = 0;
	if (argc == 2)
	{
		index = atoi(argv[1]);
		if (index == 0)
		{
			//更新固件模式，启动ymodem，并设置对应参数，flash地址和flash大小
			ymodem_app.ymodem_mode = 0;
			ymodem_app.freeflashaddr = FLASH_OTA_START_ADDR;
			ymodem_app.maxflashsize = FLASH_OTA_MAX_SIZE;
			if (YmodemStart(YmodemTx,YmodemRx,YmodemWriteFw,(pmalloc)rt_malloc,rt_free) != 0)
				return;
		}
		else if (index == 1)
		{
			//传数据到文件系统
			ymodem_app.ymodem_mode = 1;

			//用户实现获取文件系统空闲size
			uint32_t freesize = 0;
			freesize = fatfs_freesize();
			
			if (freesize >= 0x1000)
			{
				ymodem_app.maxflashsize = freesize;
			}
			else
			{
				return;
			}
			//用户需实现rt_malloc,rt_free
			if (YmodemStart(YmodemTx,YmodemRx,YmodemWriteFatfsMotorFile,(pmalloc)rt_malloc,rt_free) != 0)
				return;
		}
	}
	//关闭打印输出，防止打印对主机ymode干扰
	console_getchar_set(0);
	ymodem_app.ymodemtimeout = rt_tick_get();
	return;
}
MSH_CMD_EXPORT(rb, ymodem app);



void YmodemProcess(void)
{
	int c;
	if (isYmodemBusy())
	{
		int ret = Ymodem();
		if (ret == 0)
		{
			if (rt_tick_get() > (ymodem_app.ymodemtimeout + 5000))
			{
				while(YmodemRx((uint8_t*)&c,1));
				console_getchar_set(1);
				YmodemStop();
				rt_kprintf("ymodem rx timeout\n");
				if (ymodem_app.ymodem_mode == 0)
				{
					//用户实现删除数据
					Flash_ErasePage(FLASH_OTA_START_ADDR,1);
				}
				else if (ymodem_app.ymodem_mode == 1)
				{
					//用户实现关闭文件，并删除文件
					if (ymodem_app.pfile)
					{
						f_close(ymodem_app.pfile);
						rt_free(ymodem_app.pfile);
						ymodem_app.pfile = 0;
					}
					f_unlink(ymodem_app.pfilename);
					rt_free(ymodem_app.pfilename);
					ymodem_app.pfilename = 0;
				}
			}
		}
		else if (ret == 1)
		{
			//更新Time out时间点
			ymodem_app.ymodemtimeout = rt_tick_get();
		}
		else if (ret == 2)
		{
			while(YmodemRx((uint8_t*)&c,1));
			//打开打印
			console_getchar_set(1);
			//ymodem成功
			YmodemStop();
			rt_kprintf("ymodem success\n");
			if (ymodem_app.ymodem_mode == 0)
			{
			}
			else if (ymodem_app.ymodem_mode == 1)
			{
				//成功，关闭文件
				if (ymodem_app.pfile)
				{
					f_close(ymodem_app.pfile);
					rt_free(ymodem_app.pfile);
					ymodem_app.pfile = 0;
				}
				int len = strlen(ymodem_app.pfilename);
				if (memcmp((void*)(ymodem_app.pfilename + len - 4),".hmf",4) == 0)
				{
					matfs_add_file(ymodem_app.pfilename,0);
				}
				else
				{
					//删除文件
					f_unlink(ymodem_app.pfilename);
				}
				rt_free(ymodem_app.pfilename);
				ymodem_app.pfilename = 0;
			}
		}
		else if (ret < 0)
		{
			while(YmodemRx((uint8_t*)&c,1));
			rt_kprintf("ymodem fail %d\n",ret);
			//打开打印
			console_getchar_set(1);
			//失败
			YmodemStop();
			if (ymodem_app.ymodem_mode == 0)
			{
				//删除数据
				Flash_ErasePage(FLASH_OTA_START_ADDR,1);
			}
			
			else if (ymodem_app.ymodem_mode == 1)
			{
				//失败，关闭文件，并删除文件
				if (ymodem_app.pfile)
				{
					f_close(ymodem_app.pfile);
					rt_free(ymodem_app.pfile);
					ymodem_app.pfile = 0;
				}
				f_unlink(ymodem_app.pfilename);
				rt_free(ymodem_app.pfilename);
				ymodem_app.pfilename = 0;
			}
		}
	}
}
#endif

