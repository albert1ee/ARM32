

// 在这里写C定义的函数声明

#ifndef FLASH_FS_H
#define FLASH_FS_H

#include "ext_flash.h"

#ifdef __cplusplus
extern "C" {
#endif


/* File function return code (FRESULT) */

typedef enum {
	FR_OK = 0,				/* (0) Succeeded */
	FR_DISK_ERR,			/* (1) A hard error occurred in the low level disk I/O layer */
	FR_INT_ERR,				/* (2) Assertion failed */
	FR_NOT_READY,			/* (3) The physical drive cannot work */
	FR_NO_FILE,				/* (4) Could not find the file */
	FR_NO_PATH,				/* (5) Could not find the path */
	FR_INVALID_NAME,		/* (6) The path name format is invalid */
	FR_DENIED,				/* (7) Access denied due to prohibited access or directory full */
	FR_EXIST,				/* (8) Access denied due to prohibited access */
	FR_INVALID_OBJECT,		/* (9) The file/directory object is invalid */
	FR_WRITE_PROTECTED,		/* (10) The physical drive is write protected */
	FR_INVALID_DRIVE,		/* (11) The logical drive number is invalid */
	FR_NOT_ENABLED,			/* (12) The volume has no work area */
	FR_NO_FILESYSTEM,		/* (13) There is no valid FAT volume */
	FR_MKFS_ABORTED,		/* (14) The f_mkfs() aborted due to any problem */
	FR_TIMEOUT,				/* (15) Could not get a grant to access the volume within defined period */
	FR_LOCKED,				/* (16) The operation is rejected according to the file sharing policy */
	FR_NOT_ENOUGH_CORE,		/* (17) LFN working buffer could not be allocated */
	FR_TOO_MANY_OPEN_FILES,	/* (18) Number of open files > FF_FS_LOCK */
	FR_INVALID_PARAMETER	/* (19) Given parameter is invalid */
} FRESULT;

/* File access mode and open method flags (3rd argument of f_open) */
#define	FA_READ			0x01
#define	FA_WRITE		0x02
#define	FA_OPEN_EXISTING	0x00
#define	FA_CREATE_NEW		0x04
#define	FA_CREATE_ALWAYS	0x08
#define	FA_OPEN_ALWAYS		0x10
#define	FA_OPEN_APPEND		0x30

#define FS_FAT_MARK		0x53A55A54
#define FS_FILE_MARK		0x2AA55A2A
#define FS_FOLDER_MARK		0x2AA55A2B
#define FS_MARK                 0x2AA55A00


#include "rtthread.h"
#define FF_FS_REENTRANT	1
#define FF_FS_TIMEOUT	1000
#define FF_SYNC_t		rt_mutex_t

/* Sync functions */
#if FF_FS_REENTRANT
int ff_cre_syncobj (int vol, FF_SYNC_t* sobj);	/* Create a sync object */
int ff_req_grant (FF_SYNC_t sobj);		/* Lock sync object */
void ff_rel_grant (FF_SYNC_t sobj);		/* Unlock sync object */
int ff_del_syncobj (FF_SYNC_t sobj);	/* Delete a sync object */
#endif


enum
{
        eFS_USED = 0xfffe,
        eFS_FREE = 0xffff,
};

//FATFS文件信息会保存在第0~n个clust
typedef struct {
    int dev;                //设备序号
    uint32_t clustsize;    //当前文件系统的clust大小
    uint32_t totalsize;    //当前文件系统的总大小
    uint32_t fatfsclustcnt; //fatfscnt与totalsize/clustsize有关，主要用于存放pdataclustmark数据类容
    
    uint32_t dirbaseclust;       //目录地址，默认为从第fatfsclustcnt个clust开始,fatfsclustcnt!=0
    uint32_t dirclustcnt;   //目录地址占用clust数

    uint32_t databaseclust;      //数据地址，database从(fatfsclustcnt+dirclustcnt)开始
    uint32_t dataclustcnt;  //数据地址，clust数量
    uint32_t mark;
    uint32_t crc;			//校验文件系统是否正常

    uint16_t * pclusttable;//用于标记database区域所有clust状态,0为无效，0xffff表示未使用，0xfffe表示结束，其他表示指向下一个clust
    #if FF_FS_REENTRANT
    FF_SYNC_t	sobj;		/* Identifier of sync object */
    #endif
    uint8_t ismount;
}FATFS;

typedef struct {
    FATFS*	fs;				/* Pointer to the hosting volume of this object */
    uint32_t	sclust;			/* Object data start cluster (0:no cluster or root directory) */
    uint32_t	objsize;		/* Object size (valid when sclust != 0) */
    uint32_t	dirbase;		//存储文件的目录地址
} FFOBJID;

typedef struct {
    FFOBJID obj;
    uint32_t fptr;
    uint32_t clust;			/* Current cluster of fpter (invalid when fptr is 0) */
    int		type;
}FIL;                       //文件信息在f_close是保存到flash

typedef struct 
{
    uint32_t marks;
    uint16_t len;       //当前文件名结构的长度
    uint16_t crc;
    uint32_t filesize;
    uint16_t filenamelen;    //当前文件名(包含后缀)长度
    uint16_t sclust;
    //  name		4字节对齐
}FILEDIR;

#define FS_FILE_NAME_MAX		(64)	//xxxxxxxxxxxx.bin

FRESULT f_mkfs(int dev);
FRESULT f_mount(int dev,FATFS * pfs);
FRESULT f_unmount(FATFS * pfs);
FRESULT f_open(FIL* fp,char * path,int mode);
FRESULT f_close(FIL* fp);
FRESULT f_read(FIL* fp, void* buff, uint32_t btr, uint32_t* br);
FRESULT f_readline(FIL* fp, void** buff,uint32_t *br);
FRESULT f_write(FIL* fp, void* buff, uint32_t btr, uint32_t* br);
FRESULT f_lseek(FIL* fp, uint32_t ofs);
FRESULT f_unlink(char *path);
FRESULT f_rename (const char* path_old, const char* path_new);
FRESULT f_getfree(FATFS * pfs,uint32_t *free_size);
uint32_t f_ls(FATFS *pfs,char * path,uint8_t * pbuf,uint32_t size,uint32_t * poffset);
#ifdef __cplusplus
}
#endif

#endif // !FLASH_FS_H
