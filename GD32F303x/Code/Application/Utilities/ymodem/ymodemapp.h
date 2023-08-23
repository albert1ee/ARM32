
#ifndef _YMODEM_APP_
#define _YMODEM_APP_

#include "ymodem.h"

#define YMODEM_APP_ENABLE

typedef struct _ymodem_app_t
{
	uint32_t freeflashaddr;
	uint32_t maxflashsize;
	uint32_t ymodemtimeout;
	void *pfile;
	char *pfilename;
	int	ymodem_mode;
	void *pfilelist;
}ymodem_app_t;

void YmodemProcess(void);

#endif


