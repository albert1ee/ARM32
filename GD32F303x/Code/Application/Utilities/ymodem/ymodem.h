
#ifndef YMODEM
#define YMODEM

#include "stdint.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#define MODEM_SOH  0x01        //???????
#define MODEM_STX  0x02
#define MODEM_EOT  0x04
#define MODEM_ACK  0x06
#define MODEM_NAK  0x15
#define MODEM_CAN  0x18
#define MODEM_C    0x43


typedef enum
{
	eymHEAD,
	eymINDEX0,
	eymINDEX1,
	eymDATA,
	eymCRC0,
	eymCRC1,
}YMODEMSTATUS;

typedef enum
{
	eYM_FILESIZE,
	eYM_FILENAME,
	eYM_DATA,
}YMODEMWRITETYPE;

typedef void (*pYmodemTxFun)(uint8_t*p,uint32_t len);
typedef int  (*pYmodemRxFun)(uint8_t*p,uint32_t len);
typedef int  (*pYmodemWrite)(uint8_t*p,uint32_t len,YMODEMWRITETYPE type);
typedef void*(*pmalloc)(uint32_t len);
typedef void (*pfree)(void* p);

typedef struct _ymodem_t
{
	union
	{
		struct
		{
			unsigned int isYmodemStart	:		2;
			unsigned int isNeedCheckEnd	:		1;
			unsigned int isNeedSendC		:		1;
			unsigned int isEOT					:		1;
		}cfg;
		unsigned int config;
	}config;
	YMODEMSTATUS eymstatus;
	unsigned char head;
	unsigned char eotcnt;
	unsigned char index[2];
	unsigned short crc16;
	unsigned int timeout;
	
	pYmodemTxFun YmodemTx;
	pYmodemRxFun YmodemRx;
	pYmodemWrite YmodemWrite;
	pmalloc YmodemMalloc;
	pfree		YmodemFree;
	unsigned int *p;
	
	unsigned char *pbuf;
	unsigned char *pfilename;
	unsigned int filesize;
	unsigned int rxsize;
	unsigned int flashoffset;
	unsigned int errorcnt;
}ymodem_t;

int YmodemStart(pYmodemTxFun pTx,pYmodemRxFun pRx,pYmodemWrite pWrite,pmalloc malloc,pfree free);
void YmodemStop(void);
int Ymodem(void);
int isYmodemBusy(void);
#endif
