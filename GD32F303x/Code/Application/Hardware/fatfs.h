
#ifndef _FATFS_H_
#define _FATFS_H_

void fatfs_init(void);
unsigned int fatfs_freesize(void);

int sysfs_config_write(char * key,char * pvalue);
char* sysfs_config_read(char * key);

#endif
