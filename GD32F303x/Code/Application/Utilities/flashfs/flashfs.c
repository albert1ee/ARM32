
#include "flashfs.h"

FATFS *pfs_table[FATFS_CNTS] = {0};

static uint32_t f_get_free_clust(FATFS *pfs);
static uint32_t f_set_clust(FATFS *pfs,uint32_t curclust,uint32_t nextclust);
/*
数据格式
fat 
0-512               fatfs记录信息
512-n               fatfs database中所有clust的使用信息记录pdataclustmark
*/

/*************************************************************************
* *
* Function name : Crc8 *
* Returns : None *
* Parameters : *
* Purpose : x8+x5+x4+1 *
* *
*************************************************************************/
static unsigned char Crc8( unsigned char *Address, uint16_t Length )
{
    unsigned char crc = 0;
    unsigned char crcbuff;
    unsigned char i;

    while( Length-- )
     {
            crcbuff = *Address ++;
            for(i = 0; i< 8; i++)
             {
                 if( (crc ^ crcbuff) & 0x01 )
                 {
                        crc ^= 0x18;
                        crc>>= 1;
                        crc |= 0x80;
                 }
                 else
                 {
                        crc>>= 1;
                 }

                 crcbuff>>= 1;
             }
     }

    return crc;
}
static void f_savefs(FATFS * pfs)
{
    uint32_t len = (uint32_t)(&(pfs->crc)) - (uint32_t)pfs;
    uint32_t crc = Crc8((unsigned char*)pfs,len);
    crc += Crc8((uint8_t*)pfs->pclusttable,(pfs->totalsize/ pfs->clustsize) * sizeof(uint16_t));
    pfs->crc = crc;
    //将fatfs数据写到flash
    flash_sector_erase(0,pfs->fatfsclustcnt);
    flash_sector_write(0,(uint8_t*)pfs,len + 4);
    flash_sector_write(512,(uint8_t*)pfs->pclusttable,(pfs->totalsize/ pfs->clustsize) * sizeof(uint16_t));
    memset(pfs->pclusttable,0,(pfs->totalsize/ pfs->clustsize) * sizeof(uint16_t));
    flash_sector_read(512,(uint8_t*)pfs->pclusttable,(pfs->totalsize/ pfs->clustsize) * sizeof(uint16_t));
}
FRESULT f_mkfs(int dev)
{
    FATFS fs;
    memset(&fs,0,sizeof(FATFS));

    fs.dev = dev;
    fs.clustsize = flash_sector_size();    //暂定4096，便于spi flash操作
    fs.totalsize = flash_memory_size();    //暂定为4Mbitflasg
    
    //根据clustsize和totalsize计算fatfsclustcnt大小
    uint32_t tables = fs.totalsize / fs.clustsize * sizeof(uint16_t);    //计算最大的tables
    fs.pclusttable = (uint16_t*)fs_malloc(tables);
    memset(fs.pclusttable,0xff,tables);

    tables += 512;            //+fatfs记录信息
    fs.fatfsclustcnt = (tables + fs.clustsize - 1) / fs.clustsize;
    
    fs.dirbaseclust = fs.fatfsclustcnt;
    fs.dirclustcnt  =  1;           //至少占用1个clust

    //计算目录项最多需要多少个Bytes：根据最多文件数量和最大文件名长度
//    uint32_t maxdirlen = sizeof(FILEDIR) + FS_FILE_NAME_MAX;
//    maxdirlen = (maxdirlen + 15) & 0xFFFFFFF0;  //对齐

//    uint32_t maxfiles = fs.totalsize / fs.clustsize;
//    maxfiles -= fs.fatfsclustcnt;   //减去fatfs占用的clust
//    maxfiles -= 1;                  //dirbaseclustcnt至少为1

//    uint32_t dirbasebytes = maxdirlen * maxfiles;
//    if (dirbasebytes > fs.clustsize)
//    {
//        fs.dirclustcnt = (dirbasebytes + fs.clustsize - 1) / fs.clustsize;
//    }

    fs.databaseclust = fs.dirbaseclust + fs.dirclustcnt;
    fs.dataclustcnt = (fs.totalsize / fs.clustsize) - fs.fatfsclustcnt - fs.dirclustcnt;
    //将pdataclustmark中fatfsclust和dirclust设置used
    for (uint32_t i = 0; i < fs.fatfsclustcnt-1; i++)
    {
        fs.pclusttable[i] = i+1;
    }
    fs.pclusttable[fs.fatfsclustcnt-1] = eFS_USED;

    for (uint32_t i = fs.dirbaseclust; i < fs.dirbaseclust + fs.dirclustcnt - 1; i++)
    {
        fs.pclusttable[i] = i+1;
    }
    fs.pclusttable[fs.dirbaseclust + fs.dirclustcnt - 1] = eFS_USED;

    for (uint32_t i = fs.databaseclust; i < fs.databaseclust + fs.dataclustcnt; i++)
    {
        fs.pclusttable[i] = eFS_FREE;
    }
    
    fs.mark = FS_FAT_MARK;
    //擦除存储区域
    flash_sector_erase(0,10);
    //save fatfs
    f_savefs(&fs);

    fs_free((void*)fs.pclusttable);
    return FR_OK;
}
FRESULT f_mount(int dev,FATFS * pfs)
{
	if ((pfs_table[dev] != 0) && (pfs_table[dev]->ismount))
		return FR_INT_ERR;
    memset((void*)pfs,0,sizeof(FATFS));

    flash_sector_read(0,(uint8_t*)pfs,sizeof(FATFS)-4);
    pfs->pclusttable = 0;

    if(pfs->mark == FS_FAT_MARK)
    {
        uint32_t tables = pfs->totalsize / pfs->clustsize * sizeof(uint16_t);
        pfs->pclusttable = (uint16_t*)fs_malloc(tables);
        if (pfs->pclusttable == 0)
            return FR_INT_ERR;
        flash_sector_read(512,(uint8_t*)pfs->pclusttable,tables);
        uint32_t len = (uint32_t)(&(pfs->crc)) - (uint32_t)pfs;
        uint32_t crc = Crc8((unsigned char*)pfs,len);
        crc += Crc8((uint8_t*)pfs->pclusttable,(pfs->totalsize/ pfs->clustsize) * sizeof(uint16_t));
        if (pfs->crc != crc)
        {
            return FR_INT_ERR;
        }
        pfs_table[dev] = pfs;
        #ifdef FF_FS_REENTRANT
        ff_cre_syncobj(dev,&(pfs_table[dev]->sobj));
        #endif
        pfs->dev = dev;
        pfs_table[dev]->ismount = 1;
    }
    else
    {
        return FR_NO_FILESYSTEM;
    }
    return FR_OK;
}
FRESULT f_unmount(FATFS * pfs)
{
    if (pfs)
    {
        if (pfs->ismount)
        {
            if ((pfs_table[pfs->dev]) == pfs)
            {
                pfs->ismount = 0;
                fs_free(pfs->pclusttable);
                pfs->pclusttable = 0;
                #ifdef FF_FS_REENTRANT
                ff_del_syncobj(pfs->sobj);
                #endif
                return FR_OK;
            }
        }
    }
    return FR_OK;
}
static uint32_t f_get_cur_clust(FATFS *pfs,uint32_t sclust,uint32_t offset)
{
    if (sclust == 0)
        return 0;
    uint32_t clust = sclust;
    while (1)
    {
        if (offset < pfs->clustsize)
        {
            break;
        }
        offset -= pfs->clustsize;
        clust = pfs->pclusttable[sclust];
                sclust = clust;
    }
    return clust;
}

static uint32_t f_get_free_clust(FATFS *pfs)
{
    uint32_t clusts = pfs->totalsize / pfs->clustsize;
    for (uint32_t i = pfs->databaseclust; i < clusts; i++)
    {
        if (pfs->pclusttable[i] == eFS_FREE)
        {
            return i;
        }
    }
    return 0;
}

static uint32_t f_set_clust(FATFS *pfs,uint32_t curclust,uint32_t nextclust)
{
    uint32_t clusts = pfs->totalsize / pfs->clustsize;
    if (curclust > clusts)
        return 1;
    if (nextclust > clusts)
        return 2;
    if (curclust)
    {
        pfs->pclusttable[curclust] = nextclust;
    }
    pfs->pclusttable[nextclust] = (uint16_t)0xfffe;
    return 0;
}

static uint32_t f_set_free_clust(FATFS *pfs,uint32_t sclust)
{
    if (sclust < pfs->databaseclust)
        return 1;
    uint32_t clust;
    while (1)
    {
        clust = pfs->pclusttable[sclust];
        pfs->pclusttable[sclust] = eFS_FREE;
        if (clust == 0xfffe)
            break;
        sclust = clust;
    }
    return 0;
}
//在pclust目录sector中从offset查找一个文件或文件夹，并返回名称，如果查找完成返回0
static char * f_file_scan(FATFS *pfs,FILEDIR * pfd,uint32_t * pclust,uint32_t* poffset)
{
    //根据clust查找对应path
    //当前目录的实际地址
    uint8_t* pfilename = 0;
    uint32_t clust = *pclust;
    uint32_t offset = *poffset;
    while(1)
    {
        while (offset >= pfs->clustsize)
        {
            offset -= pfs->clustsize;
            clust = pfs->pclusttable[clust];
            if ((clust == eFS_USED) || (clust == eFS_FREE))
            {
                break;
            }
        }
        if ((clust == eFS_USED) || (clust == eFS_FREE))
        {
            break;
        }
        uint32_t addr = clust * pfs->clustsize;
        for (; offset < pfs->clustsize;)
        {
            //读取FILEDIR信息
            flash_sector_read(addr + offset,(uint8_t*)pfd,sizeof(FILEDIR));
            if ((pfd->marks & 0xFFFFFF00) == FS_MARK)
            {
                //检查文件夹名是否匹配
                uint8_t* pfilename = (uint8_t*)fs_malloc(pfd->filenamelen + 4);
                memset(pfilename,0,pfd->filenamelen + 4);
                flash_sector_read(addr + offset + sizeof(FILEDIR),pfilename,pfd->filenamelen);
//                offset += (fd.len + 15) & 0xFFFFFFF0;
                //找到对应文件
                *pclust = clust;
                *poffset = offset;
                return (char*)pfilename;
//                if (strlen(path) == strlen((char*)pfilename))
//                {
//                    uint32_t len = strlen(path);
//                    int ret = memcmp((void*)pfilename,(void*)path,len);
//                    if (ret == 0)
//                    {
//                        fs_free(pfilename);
//                        return offset + clust * pfs->clustsize;
//                    }
//                }
//                fs_free(pfilename);
            }
            else
            {
                offset += 16;
            }
        }
    }
    return (char*)pfilename;
}
//在当前clust链表中查找是否有path
static uint32_t f_file_search(uint32_t clust,FATFS * pfs,char* path)
{
    //根据clust查找对应path
    //当前目录的实际地址
    uint32_t offset = 0;
    FILEDIR fd;
    while(1)
    {
        char *pfilename = f_file_scan(pfs,&fd,&clust,&offset);
        if (pfilename == 0)
            break;
        if (strlen(path) == strlen((char*)pfilename))
        {
            uint32_t len = strlen(path);
            int ret = memcmp((void*)pfilename,(void*)path,len);
            if (ret == 0)
            {
                fs_free(pfilename);
                return offset + clust * pfs->clustsize;
            }
        }
        fs_free(pfilename);
        offset += (fd.len + 15) & 0xFFFFFFF0;
    }
    return 0;
}

//获取当前目录clust中空闲的实际地址
static uint32_t f_file_free_scan(uint32_t clust,FATFS * pfs,uint32_t needsize)
{
    needsize = (needsize + 15) & 0xFFFFFFF0;    //16Bytes 对齐
    while(1)
    {
        uint32_t addr = clust * pfs->clustsize;
        uint32_t first_search = (uint32_t)-1;
        uint32_t second_search = 0;

        for (uint32_t offset = 0; offset < pfs->clustsize;)
        {
            //读取FILEDIR信息
            FILEDIR fd;
            flash_sector_read(addr + offset,(uint8_t*)&fd,sizeof(fd));
            if ((fd.marks & 0xFFFFFF00) == FS_MARK)
            {
                first_search = (uint32_t)-1;
                second_search = 0;
                offset += (fd.len + 15) & 0xFFFFFFF0;
            }
            else
            {
                //如果first_search == 0 表示当前为空
                if (first_search == (uint32_t)-1)
                {
                    first_search = offset;
                }
                else
                {
                    if ((offset + 16) - first_search >= needsize)
                    {
                        second_search = offset + 16;
                        break;
                    }
                }
                offset += 16;
            }
        }
        if (second_search > first_search)
        {
            if (second_search - first_search >= needsize)
            {
                return first_search + clust * pfs->clustsize;
            }
        }

        if ((pfs->pclusttable[clust] == eFS_USED) || (pfs->pclusttable[clust] == eFS_FREE))
        {
            uint32_t clustfree = f_get_free_clust(pfs);
            if (clustfree == 0)
            {
                //没有空闲clust
                break;
            }
            f_set_clust(pfs,clust,clustfree);
            clust = clustfree;
        }
        else
        {
            clust = pfs->pclusttable[clust];
        }
    }

    return 0;
}
static int f_floder_isfree(uint32_t clust,FATFS * pfs)
{
    while (1) {
        uint32_t addr = clust * pfs->clustsize;
        for(uint32_t offset = 0;offset < pfs->clustsize;)
        {
            //读取FILEDIR信息
            FILEDIR fd;
            flash_sector_read(addr + offset,(uint8_t*)&fd,sizeof(fd));
            if ((fd.marks & 0xFFFFFF00) == FS_MARK)
            {
                return 0;
            }
            else
            {
                offset += 16;
            }
        }
        clust = pfs->pclusttable[clust];
        if ((clust == eFS_USED) || (clust == eFS_FREE))
            break;
    }
    return 1;
}

FRESULT f_open(FIL* fp,char * path,int mode)
{
    int dev = -1;
    char buf[4];
    for(int i = 0;i < FATFS_CNTS;i++)
    {
        memset(buf,0,4);
        sprintf(buf,"%d:",i);
        //根据path路径查找dev
        if (strstr(path,buf) == path)
        {
            dev = i;
            break;
        }
    }
    if (dev == -1)
        return FR_DISK_ERR;
	FRESULT ret;

    if ((pfs_table[dev] == 0) || (pfs_table[dev]->ismount == 0))
        return FR_INT_ERR;

    #ifdef FF_FS_REENTRANT
    ff_req_grant(pfs_table[dev]->sobj);
    #endif

    //根目录clust 和 对应实际目录地址
    uint32_t clust = pfs_table[dev]->dirbaseclust;
    uint32_t diraddr = 0;//pfs_table[dev]->dirbaseclust * pfs_table[dev]->clustsize;
    #if 1
        char * p = path;
        char * p1 = 0;
        char * p2 = 0;
        p += 2;
        //分离目录项，如果*p!=0 表示
        while(*p)
        {
            // 查找 '/' 或 '\\'，判断是否是文件夹
            while ((*p == '/') || (*p == '\\'))
            {
                p++;
            }
            p1 = strstr(p,"/");
            if (p1 == 0)
            {
                p1 = strstr(p,"\\");
            }
            if (p2)
            {
                fs_free(p2);
                p2 = 0;
            }
            if (p1 != 0)
            {
                //p剩余内容包含文件夹,p2表示文件夹名
                int len = p1 - p + 1;
                p2 = fs_malloc(len);
                if (p2)
                {
                    memset(p2,0,len);
                    memcpy(p2,p,len-1);
                }
                p = p1;
            }
            else
            {
                //p剩余内容为文件，p2表示文件名
                int len = strlen(p) + 1;
                p2 = fs_malloc(len);
                if (p2)
                {
                    memset(p2,0,len);
                    memcpy(p2,p,len-1);
                }
                p += strlen(p);
            }
            if (p2 != 0)
            {
                FILEDIR fd;
                //查找p2对应的文件或文件夹的实际地址
                uint32_t addr = f_file_search(clust,pfs_table[dev],p2);
                //p1 !=0 表示当前是文件夹
                if (p1 != 0)
                {
                    //从实际地址中读出文件夹信息
                    if (addr != 0)
                    {
                        flash_sector_read(addr,(uint8_t*)&fd,sizeof(FILEDIR));
                        //更新目录地址
                        clust = fd.sclust;
                    }
                    else
                    {
                        //当前文件夹不存在，如果是创建文件，则创建该文件夹
                        if (mode & FA_CREATE_ALWAYS)
                        {
                            fd.marks = FS_FOLDER_MARK;
                            fd.filesize = 0;
                            fd.filenamelen = strlen(p2);
                            fd.filenamelen = (fd.filenamelen + 3) & (0xFFFFFFFC);
                            fd.sclust = f_get_free_clust(pfs_table[dev]);
                            if (fd.sclust == 0)
                            {
                                ret = FR_NOT_READY;
                                diraddr = 0;
                                break;
                            }
                            f_set_clust(pfs_table[dev],0,fd.sclust);
                            fd.len = sizeof(FILEDIR) + fd.filenamelen;
                            //获取到当前clust的实际地址
                            addr = f_file_free_scan(clust,pfs_table[dev],fd.len);
                            if (addr == 0)
                            {
                                ret = FR_NOT_READY;
                                diraddr = 0;
                                break;
                            }

                            uint32_t offset = addr & (pfs_table[dev]->clustsize - 1);
                            //当前clust的起始地址
                            addr -= offset;
                            #if 0
                            //由于sram不够用，改为内部flash最后4KB当作临时存储
                            uint8_t * p = (uint8_t*)fs_malloc(256);
                            if (p)
                            {
                                Flash_ErasePage(0x08000000 + 64*1024 - 4096,4096 / FMC_PAGE_SIZE);
                                for(int cnts=0;cnts<pfs_table[dev]->clustsize;)
                                {
                                    flash_sector_read(addr + cnts,p,256);
                                    Flash_Program(0x08000000 + 64*1024 - 4096 + cnts,p,256);
                                    cnts+=256;
                                }
                                flash_sector_erase(addr,1);
                                flash_sector_write(addr,(uint8_t*)(0x08000000 + 64*1024 - 4096),offset);
                                flash_sector_write(addr + offset,(void*)&fd,sizeof(FILEDIR));
                                flash_sector_write(addr + offset + sizeof(FILEDIR),(uint8_t*)p2,fd.filenamelen);
                                flash_sector_write(addr + offset + sizeof(FILEDIR) + fd.filenamelen,	\
                                    (uint8_t*)(0x08000000 + 64*1024 - 4096 + offset + sizeof(FILEDIR) + fd.filenamelen),	\
                                    pfs_table[dev]->clustsize - (offset + sizeof(FILEDIR) + fd.filenamelen));
                                fs_free(p);
                            }
                            #endif
                            #if 1
                            //使用内存分配
                            uint8_t * p = (uint8_t*)fs_malloc(pfs_table[dev]->clustsize);
                            if (p)
                            {
                                flash_sector_read(addr ,p,pfs_table[dev]->clustsize);
                                flash_sector_erase(addr,1);
                                memcpy((void*)(p+offset),(void*)&fd,sizeof(FILEDIR));
                                memcpy((void*)(p+offset + sizeof(FILEDIR)),(uint8_t*)p2,fd.filenamelen);
                                flash_sector_write(addr,p,pfs_table[dev]->clustsize);
                                fs_free(p);
                            }
                            #endif
                            clust = fd.sclust;
                        }
						else
						{
							//当前文件夹不存在，不可读，直接退出
							diraddr = addr;
							break;
						}
                    }
                }
                else
                {
                    diraddr = addr;
                    break;
                }
            }
        }
    #endif
    if (p2)
    {
        if (strlen(p2) == 0)
            path = 0;
        else
        {
            path = strstr(path,p2);
        }
        fs_free(p2);
        p2 = 0;
    }
    else
    {
        path = 0;
    }
    if ((diraddr == 0) && (path))
    {
        if (mode & FA_CREATE_ALWAYS)
        {
            //创建文件,先将marks len crc filesize filenamelen clustcnt 和 filename 写入到文件系统
            FILEDIR fd;
            fd.marks = FS_FILE_MARK;
            fd.filesize = 0;
            fd.filenamelen = strlen(path);
            fd.filenamelen = (fd.filenamelen + 3) & (0xFFFFFFFC);
            fd.sclust = 0;
            fd.len = sizeof(FILEDIR) + fd.filenamelen;

            diraddr = f_file_free_scan(clust,pfs_table[dev],fd.len);
            if (diraddr == 0)
            {
                ret = FR_NOT_READY;
                goto open_end;
            }
            
            fp->fptr = 0;
            fp->obj.fs  = pfs_table[dev];
            fp->obj.dirbase = diraddr;
            fp->clust = 0;
            fp->obj.objsize = 0;
            fp->obj.sclust = 0;

            uint32_t offset = diraddr & (pfs_table[dev]->clustsize - 1);
            diraddr -= offset;
            #if 0
            //由于sram不够用，改为内部flash最后4KB当作临时存储
            uint8_t * p = (uint8_t*)fs_malloc(256);
            if (p)
            {
                Flash_ErasePage(0x08000000 + 64*1024 - 4096,4096 / FMC_PAGE_SIZE);
                for(int cnts=0;cnts<pfs_table[dev]->clustsize;)
                {
                    flash_sector_read(diraddr + cnts,p,256);
                    Flash_Program(0x08000000 + 64*1024 - 4096 + cnts,p,256);
                    cnts+=256;
                }
                flash_sector_erase(diraddr,1);
                flash_sector_write(diraddr,(uint8_t*)(0x08000000 + 64*1024 - 4096),offset);
                flash_sector_write(diraddr + offset,(void*)&fd,sizeof(FILEDIR));
                flash_sector_write(diraddr + offset + sizeof(FILEDIR),(uint8_t*)path,fd.filenamelen);
                flash_sector_write(diraddr + offset + sizeof(FILEDIR) + fd.filenamelen,	\
                    (uint8_t*)(0x08000000 + 64*1024 - 4096 + offset + sizeof(FILEDIR) + fd.filenamelen),	\
                    pfs_table[dev]->clustsize - (offset + sizeof(FILEDIR) + fd.filenamelen));
                fs_free(p);
            }
            #endif
            #if 1
            //使用内存分配
            uint8_t * p = (uint8_t*)fs_malloc(pfs_table[dev]->clustsize);
            if (p)
            {
                flash_sector_read(diraddr ,p,pfs_table[dev]->clustsize);
                flash_sector_erase(diraddr,1);
                memcpy((void*)(p+offset),(void*)&fd,sizeof(FILEDIR));
                memcpy((void*)(p+offset + sizeof(FILEDIR)),(uint8_t*)path,fd.filenamelen);
                flash_sector_write(diraddr,p,pfs_table[dev]->clustsize);
                fs_free(p);
            }
            #endif
        }
        else
        {
            ret = FR_NO_FILE;
            goto open_end;
        }
    }
    else if ((diraddr == 0) && (path == 0))
    {
        //根目录
        fp->fptr = 0;
        fp->clust = 0;
        fp->obj.fs = pfs_table[dev];
        fp->obj.dirbase = diraddr;
        fp->obj.objsize = 0;
        fp->obj.sclust = pfs_table[dev]->dirbaseclust;
    }
    else//读取文件
    {
//        if (mode & FA_READ)
        {
            fp->fptr = 0;
            fp->obj.fs = pfs_table[dev];
            fp->obj.dirbase = diraddr;
            //先读取文件前16Bytes：marks len crc filesize
            FILEDIR fd;
            flash_sector_read(diraddr,(uint8_t*)&fd,sizeof(FILEDIR));
            if (fd.sclust == 0)
            {
                fp->clust = 0;
                fp->obj.objsize = 0;
                fp->obj.sclust = 0;
            }
            else
            {
                fp->clust = fd.sclust;
                fp->obj.objsize = fd.filesize;
                fp->obj.sclust = fd.sclust;
            }
        }
    }
    fp->type = mode;
    ret = FR_OK;
    open_end:
    #ifdef FF_FS_REENTRANT
    ff_rel_grant(pfs_table[dev]->sobj);
    #endif
    return ret;
}
FRESULT f_close(FIL* fp)
{
		if ((fp->obj.fs == 0) || (fp->obj.fs->ismount == 0))
			return FR_INT_ERR;
    if (fp->type == FA_READ)
    {
        return FR_OK;
    }
    #ifdef FF_FS_REENTRANT
		ff_req_grant(fp->obj.fs->sobj);
    #endif
    //读取当前目录项
    FILEDIR fd;
    flash_sector_read(fp->obj.dirbase,(uint8_t*)&fd,sizeof(FILEDIR));
    fd.filesize = fp->obj.objsize;
    fd.sclust = fp->obj.sclust;
    //更新目录项
    uint32_t offset = fp->obj.dirbase & (fp->obj.fs->clustsize - 1);
    uint32_t clustbase = fp->obj.dirbase & (~(fp->obj.fs->clustsize - 1));
		#if 0
		//由于sram不够用，改为内部flash最后4KB当作临时存储
		uint8_t * p = (uint8_t*)fs_malloc(256);
		if (p)
		{
			Flash_ErasePage(0x08000000 + 64*1024 - 4096,4096 / FMC_PAGE_SIZE);
			for(int cnts=0;cnts<fp->obj.fs->clustsize;)
			{
				flash_sector_read(clustbase + cnts,p,256);
				Flash_Program(0x08000000 + 64*1024 - 4096 + cnts,p,256);
				cnts+=256;
			}
			flash_sector_erase(clustbase,1);
			flash_sector_write(clustbase,(uint8_t*)(0x08000000 + 64*1024 - 4096),offset);
			flash_sector_write(clustbase + offset,(void*)&fd,sizeof(FILEDIR));
			
			flash_sector_write(clustbase + offset + sizeof(FILEDIR),	\
				(uint8_t*)(0x08000000 + 64*1024 - 4096 + offset + sizeof(FILEDIR)),	\
				fp->obj.fs->clustsize - (offset + sizeof(FILEDIR)));
			fs_free(p);
		}
		#endif
		#if 1
		//使用内存分配
		uint8_t * p = (uint8_t*)fs_malloc(fp->obj.fs->clustsize);
		if (p)
		{
			flash_sector_read(clustbase ,p,fp->obj.fs->clustsize);
			flash_sector_erase(clustbase,1);
			memcpy((void*)(p + offset),(void*)&fd,sizeof(FILEDIR));
			flash_sector_write(clustbase,p,fp->obj.fs->clustsize);
			fs_free(p);
		}
		#endif
    f_savefs(fp->obj.fs);
    #ifdef FF_FS_REENTRANT
		ff_rel_grant(fp->obj.fs->sobj);
    #endif
    return FR_OK;
}
FRESULT f_read(FIL* fp, void* buff, uint32_t btr, uint32_t* br)
{
	if ((fp->obj.fs == 0) || (fp->obj.fs->ismount == 0))
		return FR_INT_ERR;
    uint32_t size = 0;
    if (fp->fptr >= fp->obj.objsize)
        return FR_NOT_READY;
    #ifdef FF_FS_REENTRANT
		ff_req_grant(fp->obj.fs->sobj);
    #endif
    size = fp->obj.objsize - fp->fptr;
    if (size >= btr)
    {
        size = btr;
    }
    
    uint32_t sectors = f_get_cur_clust(fp->obj.fs,fp->obj.sclust,fp->fptr);
    uint32_t offset = fp->fptr & (fp->obj.fs->clustsize - 1);
    offset += sectors * fp->obj.fs->clustsize;
    
    for (uint32_t i = 0; i < size; )
    {
		if ((offset & (fp->obj.fs->clustsize - 1)) == 0)
		{
			sectors = f_get_cur_clust(fp->obj.fs,fp->obj.sclust,fp->fptr);
			offset = 0;
			offset += sectors * fp->obj.fs->clustsize;
		}
        uint32_t cnts = fp->obj.fs->clustsize - (offset & (fp->obj.fs->clustsize - 1));
        if (cnts > (size-i))
        {
            cnts = size - i;
        }
        flash_sector_read(offset,(uint8_t*)buff + i,cnts);
		fp->fptr += cnts;
        offset += cnts;
        i += cnts;
		*br += cnts;
    }
    fp->clust = f_get_cur_clust(fp->obj.fs,fp->obj.sclust,fp->fptr);
    #ifdef FF_FS_REENTRANT
		ff_rel_grant(fp->obj.fs->sobj);
    #endif
    return FR_OK;
}
FRESULT f_readline(FIL* fp, void** buff,uint32_t *br)
{
	*br = 0;
	*buff = 0;
	if ((fp->obj.fs == 0) || (fp->obj.fs->ismount == 0))
		return FR_INT_ERR;
    uint32_t size = 0;
    if (fp->fptr >= fp->obj.objsize)
        return FR_NOT_READY;
    #ifdef FF_FS_REENTRANT
		ff_req_grant(fp->obj.fs->sobj);
    #endif
    size = fp->obj.objsize - fp->fptr;
    
    uint32_t sectors = f_get_cur_clust(fp->obj.fs,fp->obj.sclust,fp->fptr);
    uint32_t offset = fp->fptr & (fp->obj.fs->clustsize - 1);
    offset += sectors * fp->obj.fs->clustsize;
    char * pbuf = fs_malloc(64+4);
	if (pbuf == 0)
		return FR_NOT_READY;
	uint32_t sfptr = fp->fptr;
	uint32_t efptr = 0;
    for (uint32_t i = 0; i < size; )
    {
		if ((offset & (fp->obj.fs->clustsize - 1)) == 0)
		{
			sectors = f_get_cur_clust(fp->obj.fs,fp->obj.sclust,fp->fptr);
			offset = 0;
			offset += sectors * fp->obj.fs->clustsize;
		}
        uint32_t cnts = fp->obj.fs->clustsize - (offset & (fp->obj.fs->clustsize - 1));
        if (cnts > (size-i))
        {
            cnts = size - i;
        }
		if (cnts >= 64)
		{
			cnts = 64;
		}
        flash_sector_read(offset,(uint8_t*)pbuf + *br,cnts);
		for(int c = 0;c < cnts;c++)
		{
			if (pbuf[c] == '\n')
			{
				pbuf[c+1] = 0;
				efptr = fp->fptr + c + 1;
				break;
			}
		}
		#if 0
		if (efptr)
		{
			fp->fptr = sfptr;
			fs_free(pbuf);
			*br = efptr - sfptr;
			pbuf = fs_malloc(*br);
			if (pbuf)
			{
				memset(pbuf,0,*br);
				uint32_t sectors = f_get_cur_clust(fp->obj.fs,fp->obj.sclust,fp->fptr);
				uint32_t offset = fp->fptr & (fp->obj.fs->clustsize - 1);
				offset += sectors * fp->obj.fs->clustsize;
				size = *br;
				for (uint32_t i = 0; i < size; )
				{
					if ((offset & (fp->obj.fs->clustsize - 1)) == 0)
					{
						sectors = f_get_cur_clust(fp->obj.fs,fp->obj.sclust,fp->fptr);
						offset = 0;
						offset += sectors * fp->obj.fs->clustsize;
					}
					uint32_t cnts = fp->obj.fs->clustsize - (offset & (fp->obj.fs->clustsize - 1));
					if (cnts > (size-i))
					{
						cnts = size - i;
					}
					flash_sector_read(offset,(uint8_t*)pbuf + i,cnts);
					fp->fptr += cnts;
					offset += cnts;
					i += cnts;
				}
				*buff = pbuf;
			}
			break;
		}
		#endif
		if (efptr)
		{
			fp->fptr = efptr;
			*br = efptr - sfptr;
			*buff = pbuf;
			break;
		}
		fp->fptr += cnts;
        offset += cnts;
        i += cnts;
		*br += 64;
		char * p = fs_realloc(pbuf,64);
		if (p == 0)
		{
			fs_free(pbuf);
			*buff = 0;
			*br = 0;
			fp->fptr = sfptr;
			break;
		}
    }
    fp->clust = f_get_cur_clust(fp->obj.fs,fp->obj.sclust,fp->fptr);
    #ifdef FF_FS_REENTRANT
		ff_rel_grant(fp->obj.fs->sobj);
    #endif
    return FR_OK;
}
FRESULT f_write(FIL* fp, void* buff, uint32_t btr, uint32_t* br)
{
	FRESULT ret;
    if ((fp->obj.fs == 0) || (fp->obj.fs->ismount == 0))
        return FR_INT_ERR;
    if ((fp->type & FA_WRITE) == 0)
        return FR_NOT_ENABLED;
    #ifdef FF_FS_REENTRANT
		ff_req_grant(fp->obj.fs->sobj);
    #endif
    for (uint32_t i = 0; i < btr; )
    {
        if ((fp->fptr == fp->obj.objsize) && ((fp->fptr & (fp->obj.fs->clustsize - 1)) == 0))
        {
            //需要分配新的clust
            uint32_t clust = f_get_free_clust(fp->obj.fs);
            if (clust == 0)
            {
                ret = FR_NOT_READY;
                goto write_end;
            }
            f_set_clust(fp->obj.fs,fp->clust,clust);
            fp->clust = clust;
            if (fp->obj.sclust == 0)
                fp->obj.sclust = clust;
            //擦除新分配的clust
            flash_sector_erase(clust * fp->obj.fs->clustsize,1);
        }
        
        uint32_t sectors = f_get_cur_clust(fp->obj.fs,fp->obj.sclust,fp->fptr) * fp->obj.fs->clustsize;
        uint32_t offset = fp->fptr & (fp->obj.fs->clustsize - 1);
        uint32_t cnts = fp->obj.fs->clustsize - (offset & (fp->obj.fs->clustsize - 1));
        if (cnts > (btr-i))
        {
            cnts = btr - i;
        }
        if (fp->obj.objsize > fp->fptr)
        {
            if (cnts > (fp->obj.objsize - fp->fptr))
            {
                cnts = fp->obj.objsize - fp->fptr;
            }
            //需要擦除offset后cnts部分
						#if 0
            //由于sram不够用，改为内部flash最后4KB当作临时存储
						uint8_t * p = (uint8_t*)fs_malloc(256);
						if (p)
						{
							Flash_ErasePage(0x08000000 + 64*1024 - 4096,4096 / FMC_PAGE_SIZE);
							for(int cnts=0;cnts<fp->obj.fs->clustsize;)
							{
								flash_sector_read(sectors + cnts,p,256);
								Flash_Program(0x08000000 + 64*1024 - 4096 + cnts,p,256);
								cnts+=256;
							}
							flash_sector_erase(sectors,1);
							flash_sector_write(sectors,(uint8_t*)(0x08000000 + 64*1024 - 4096),offset);
							
							flash_sector_write(sectors + offset + cnts,	\
							(uint8_t*)(0x08000000 + 64*1024 - 4096 + offset + cnts),fp->obj.fs->clustsize - offset - cnts);
							fs_free(p);
            }
						#endif
						#if 1
						//使用内存分配
						uint8_t * p = (uint8_t*)fs_malloc(fp->obj.fs->clustsize);
						if (p)
						{
							flash_sector_read(sectors ,p,fp->obj.fs->clustsize);
							flash_sector_erase(sectors,1);
							
							flash_sector_write(sectors,p,offset);
							
							flash_sector_write(sectors + offset + cnts,	\
							(uint8_t*)(p + offset + cnts),fp->obj.fs->clustsize - offset - cnts);
							fs_free(p);
						}
						#endif
        }
        
        offset += sectors;
        flash_sector_write(offset,(uint8_t*)buff + i,cnts);
        offset += cnts;
        i += cnts;
        *br += cnts;
        if (fp->fptr == fp->obj.objsize)
        {
            fp->fptr += cnts;
            fp->obj.objsize += cnts;
        }
        else
        {
            fp->fptr += cnts;
        }
    }
		ret = FR_OK;
		write_end:
        #ifdef FF_FS_REENTRANT
		ff_rel_grant(fp->obj.fs->sobj);
        #endif
    return ret;
}
FRESULT f_lseek(FIL* fp, uint32_t ofs)
{
    if ((fp->obj.fs == 0) || (fp->obj.fs->ismount == 0))
        return FR_INT_ERR;
    if (ofs > fp->obj.objsize)
        return FR_NOT_READY;
    #ifdef FF_FS_REENTRANT
		ff_req_grant(fp->obj.fs->sobj);
    #endif
    fp->fptr = ofs;
    fp->clust = f_get_cur_clust(fp->obj.fs,fp->obj.sclust,fp->fptr);
    #ifdef FF_FS_REENTRANT
		ff_rel_grant(fp->obj.fs->sobj);
    #endif
    return FR_OK;
}
FRESULT f_unlink(char *path)
{
    int dev = -1;
    char buf[4];
    for(int i = 0;i < FATFS_CNTS;i++)
    {
        memset(buf,0,4);
        sprintf(buf,"%d:",i);
        //根据path路径查找dev
        if (strstr(path,buf) == path)
        {
            dev = i;
            break;
        }
    }
    if (dev == -1)
        return FR_DISK_ERR;

    if ((pfs_table[dev] == 0) || (pfs_table[dev]->ismount == 0))
        return FR_INT_ERR;
    #ifdef FF_FS_REENTRANT
    ff_req_grant(pfs_table[dev]->sobj);
    #endif

    //根目录clust 和 对应实际目录地址
    uint32_t clust = pfs_table[dev]->dirbaseclust;
    uint32_t diraddr = 0;//pfs_table[dev]->dirbaseclust * pfs_table[dev]->clustsize;
    #if 1
        char * p = path;
        p += 2;
        //分离目录项，如果*p!=0 表示
        while(*p)
        {
            // 查找 '/' 或 '\\'，判断是否是文件夹
            char * p1 = 0;
            char * p2 = 0;
            while ((*p == '/') || (*p == '\\'))
            {
                p++;
            }
            p1 = strstr(p,"/");
            if (p1 == 0)
            {
                p1 = strstr(p,"\\");
            }
            if (p1 != 0)
            {
                //p剩余内容包含文件夹,p2表示文件夹名
                int len = p1 - p + 1;
                p2 = fs_malloc(len);
                if (p2)
                {
                    memset(p2,0,len);
                    memcpy(p2,p,len-1);
                }
                p = p1;
            }
            else
            {
                //p剩余内容为文件，p2表示文件名
                int len = strlen(p) + 1;
                p2 = fs_malloc(len);
                if (p2)
                {
                    memset(p2,0,len);
                    memcpy(p2,p,len-1);
                }
                p += strlen(p);
            }
            if (p2 != 0)
            {
                FILEDIR fd;
                //查找p2对应的文件或文件夹的实际地址
                uint32_t addr = f_file_search(clust,pfs_table[dev],p2);
                fs_free(p2);
                p2 = 0;
                //p1 !=0 表示当前是文件夹
                if (p1 != 0)
                {
                    //从实际地址中读出文件夹信息
                    if (addr != 0)
                    {
                        flash_sector_read(addr,(uint8_t*)&fd,sizeof(FILEDIR));
                        //更新目录地址
                        clust = fd.sclust;
                    }
                    else
                    {
                        //当前文件夹不存在，
                        diraddr = 0;
                        break;
                    }
                }
                else
                {
                    diraddr = addr;
                    break;
                }
            }
        }
    #endif
    if (diraddr == 0)
    {
        #ifdef FF_FS_REENTRANT
        ff_rel_grant(pfs_table[dev]->sobj);
        #endif
        return FR_NO_FILE;
    }
    //根据clustcnt 清除fatfs tab 表
    FILEDIR fd;
    flash_sector_read(diraddr,(uint8_t*)&fd,sizeof(FILEDIR));
    if (fd.marks == FS_FILE_MARK)
    {
        f_set_free_clust(pfs_table[dev],fd.sclust);
    }
    else
    {
        //检查当前目录是否还有文件，如果有文件则不能删除
        if (f_floder_isfree(fd.sclust,pfs_table[dev]))
        {
            f_set_free_clust(pfs_table[dev],fd.sclust);
        }
        else
        {
            #ifdef FF_FS_REENTRANT
            ff_rel_grant(pfs_table[dev]->sobj);
            #endif
            return FR_WRITE_PROTECTED;
        }
    }
    f_savefs(pfs_table[dev]);
    //删除目录项
    uint32_t offset = diraddr & (pfs_table[dev]->clustsize - 1);
    diraddr -= offset;
    #if 0
    //由于sram不够用，改为内部flash最后4KB当作临时存储
    uint8_t * p = (uint8_t*)fs_malloc(256);
    if (p)
    {
        Flash_ErasePage(0x08000000 + 64*1024 - 4096,4096 / FMC_PAGE_SIZE);
        for(int cnts=0;cnts<pfs_table[dev]->clustsize;)
        {
            flash_sector_read(diraddr + cnts,p,256);
            Flash_Program(0x08000000 + 64*1024 - 4096 + cnts,p,256);
            cnts+=256;
        }
        flash_sector_erase(diraddr,1);
        flash_sector_write(diraddr,(uint8_t*)(0x08000000 + 64*1024 - 4096),offset);
        uint32_t mark = 0;
        flash_sector_write(diraddr + offset,(uint8_t*)&mark,4);
        flash_sector_write(diraddr + offset + 4,(uint8_t*)(0x08000000 + 64*1024 - 4096 + offset + 4),	\
        pfs_table[dev]->clustsize - (offset + 4));
        fs_free(p);
    }
    #endif
    #if 1
    //使用内存分配
    p = (char*)fs_malloc(pfs_table[dev]->clustsize);
    if (p)
    {
        flash_sector_read(diraddr ,(uint8_t*)p,pfs_table[dev]->clustsize);
        flash_sector_erase(diraddr,1);
        memset(p+offset,0,4);
        flash_sector_write(diraddr,(uint8_t*)p,pfs_table[dev]->clustsize);
        fs_free(p);
    }
    #endif
    #ifdef FF_FS_REENTRANT
    ff_rel_grant(pfs_table[dev]->sobj);
    #endif
    return FR_OK;
}
FRESULT f_rename (const char* path_old, const char* path_new)
{
    int dev = -1;
    char buf[4];
    if (path_new[0] != path_old[0])
        return FR_INVALID_OBJECT;
    for(int i = 0;i < FATFS_CNTS;i++)
    {
        memset(buf,0,4);
        sprintf(buf,"%d:",i);
        //根据path路径查找dev
        if (strstr(path_old,buf) == path_old)
        {
            dev = i;
            break;
        }
    }
    if (dev == -1)
      return FR_DISK_ERR;
    if ((pfs_table[dev] == 0) || (pfs_table[dev]->ismount == 0))
        return FR_INT_ERR;
    #ifdef FF_FS_REENTRANT
    ff_req_grant(pfs_table[dev]->sobj);
    #endif
    //打开旧文件
    FIL fold;
    if (f_open(&fold,(char*)path_old,FA_READ) != FR_OK)
    {
        #ifdef FF_FS_REENTRANT
        ff_rel_grant(pfs_table[dev]->sobj);
        #endif
        return FR_INT_ERR;
    }
    //创建新文件
    FIL f;
    if (f_open(&f,(char*)path_new,FA_CREATE_ALWAYS | FA_WRITE | FA_READ) != FR_OK)
    {
        #ifdef FF_FS_REENTRANT
        ff_rel_grant(pfs_table[dev]->sobj);
        #endif
        return FR_INT_ERR;
    }
    f.clust = fold.obj.sclust;
    f.fptr = 0;
    f.obj.objsize = fold.obj.objsize;
    f.obj.sclust = fold.obj.sclust;
    f_close(&f);
    //根目录clust 和 对应实际目录地址
    uint32_t clust = pfs_table[dev]->dirbaseclust;
    uint32_t diraddr = 0;//pfs_table[dev]->dirbaseclust * pfs_table[dev]->clustsize;
    #if 1
        char * p = (char*)path_old;
        p += 2;
        //分离目录项，如果*p!=0 表示
        while(*p)
        {
            // 查找 '/' 或 '\\'，判断是否是文件夹
            char * p1 = 0;
            char * p2 = 0;
            while ((*p == '/') || (*p == '\\'))
            {
                p++;
            }
            p1 = strstr(p,"/");
            if (p1 == 0)
            {
                p1 = strstr(p,"\\");
            }
            if (p1 != 0)
            {
                //p剩余内容包含文件夹,p2表示文件夹名
                int len = p1 - p + 1;
                p2 = fs_malloc(len);
                if (p2)
                {
                    memset(p2,0,len);
                    memcpy(p2,p,len-1);
                }
                p = p1;
            }
            else
            {
                //p剩余内容为文件，p2表示文件名
                int len = strlen(p) + 1;
                p2 = fs_malloc(len);
                if (p2)
                {
                    memset(p2,0,len);
                    memcpy(p2,p,len-1);
                }
                p += strlen(p);
            }
            if (p2 != 0)
            {
                FILEDIR fd;
                //查找p2对应的文件或文件夹的实际地址
                uint32_t addr = f_file_search(clust,pfs_table[dev],p2);
                fs_free(p2);
                p2 = 0;
                //p1 !=0 表示当前是文件夹
                if (p1 != 0)
                {
                    //从实际地址中读出文件夹信息
                    if (addr != 0)
                    {
                        flash_sector_read(addr,(uint8_t*)&fd,sizeof(FILEDIR));
                        //更新目录地址
                        clust = fd.sclust;
                    }
                    else
                    {
                        //当前文件夹不存在，
                        diraddr = 0;
                        break;
                    }
                }
                else
                {
                    diraddr = addr;
                    break;
                }
            }
        }
    #endif

        //将文件mark清0
        uint32_t temp = 0;
    flash_sector_write(diraddr,(uint8_t*)&temp,4);
    #ifdef FF_FS_REENTRANT
    ff_rel_grant(pfs_table[dev]->sobj);
    #endif
    return FR_OK;
}
FRESULT f_getfree(FATFS * pfs,uint32_t *free_size)
{
    if (pfs->ismount == 0)
        return FR_INT_ERR;
    #ifdef FF_FS_REENTRANT
    ff_req_grant(pfs->sobj);
    #endif
    *free_size = 0;
    for (uint32_t i = 0; i < pfs->totalsize / pfs->clustsize; i++)
    {
        if (pfs->pclusttable[i] == eFS_FREE)
        {
            *free_size += pfs->clustsize;
        }
    }
    #ifdef FF_FS_REENTRANT
        ff_rel_grant(pfs->sobj);
    #endif
    return FR_OK;
}

uint32_t f_ls(FATFS *pfs,char * path,uint8_t * pbuf,uint32_t size,uint32_t * poffset)
{
	uint32_t outsize = 0;
	memset(pbuf,0,size);
    FIL fd;
	FILEDIR fdir;
	//获取当前目录信息
	if (f_open(&fd,path,FA_READ) == FR_OK)
	{
		f_close(&fd);
		while(1)
		{
			//根据offset
			#ifdef FF_FS_REENTRANT
				ff_req_grant(pfs->sobj);
			#endif
			uint32_t clust = fd.obj.sclust;
			clust = f_get_cur_clust(pfs,clust,*poffset);
			char * pfilename = f_file_scan(pfs,&fdir,&clust,poffset);
			if (pfilename == 0)
				break;
			#ifdef FF_FS_REENTRANT
				ff_rel_grant(pfs->sobj);
			#endif
			*poffset += (fdir.len + 15) & 0xFFFFFFF0;
			if (strlen(pfilename) >= (size - outsize))
			{
				fs_free(pfilename);
				break;
			}
			sprintf((char*)(pbuf + outsize),"%s\n",pfilename);
			outsize += strlen(pfilename) + 1;
			fs_free(pfilename);
		}
	}
	return outsize;
}
