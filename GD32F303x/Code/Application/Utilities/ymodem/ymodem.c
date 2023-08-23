
#include "ymodem.h"

ymodem_t ymodem=
{
	.pbuf = 0,
	.config = 0,
};
/*************************************************************************
* *
* Function name : Crc16_xmode *
* Returns : None *
* Parameters : *
* Purpose : x16+x12+x5+1 *
* *
*************************************************************************/
uint16_t Crc16_xmode(uint8_t *pbuf , uint16_t len )
{
	uint16_t i ,j;
	uint16_t crc = 0;
	for( i = 0;i < len ; i++)
	{
		crc ^= *pbuf ++ << 8;
		for(j = 0;j < 8;j++)
		{
			if( crc & 0x8000 )
			{
				crc = (crc << 1) ^ 0x1021;
			}
			else
			{
				crc <<= 0x01 ;
			}
		}
	}
	return crc;
}

int YmodemStart(pYmodemTxFun pTx,pYmodemRxFun pRx,pYmodemWrite pWrite,pmalloc malloc,pfree free)
{
	if (ymodem.config.cfg.isYmodemStart)
	{
		return -1;
	}

	memset(&ymodem,0,sizeof(ymodem_t));

	ymodem.YmodemTx = pTx;
	ymodem.YmodemRx = pRx;
	ymodem.YmodemWrite = pWrite;
	ymodem.YmodemMalloc = malloc;
	ymodem.YmodemFree = free;
	
	ymodem.config.cfg.isYmodemStart = 1;
	ymodem.config.cfg.isNeedSendC = 1;
	ymodem.eymstatus = eymHEAD;
	return 0;
}

void YmodemStop(void)
{
	if (ymodem.config.cfg.isYmodemStart)
	{
		if (ymodem.pfilename)
		{
			ymodem.YmodemFree(ymodem.pfilename);
		}
		if (ymodem.pbuf)
		{
			ymodem.YmodemFree(ymodem.pbuf);
		}
	}
	ymodem.config.cfg.isYmodemStart = 0;
}

int Ymodem(void)
{
	int ret = 0;
	int c;
	uint8_t buf[2];
	if (ymodem.config.cfg.isNeedSendC)
	{
		buf[0] = MODEM_C;
		buf[1] = 0;
		ymodem.YmodemTx((uint8_t*)buf,strlen((const char*)buf));
		ymodem.config.cfg.isNeedSendC = 0;
		ret = 1;
	}
	do
	{
		c = -1;
		if (ymodem.YmodemRx((uint8_t*)&c,1))
		{
			c &= 0xFF;
			switch (ymodem.eymstatus)
			{
				case eymHEAD:
				{
					if (c == MODEM_SOH)
					{
						//data size = 128;
						ymodem.pbuf = (uint8_t*)ymodem.YmodemMalloc(128);
						if (ymodem.pbuf == 0)
						{
							ret = -2;
							goto RET;
						}
						ymodem.head = c;
						ymodem.eymstatus = eymINDEX0;
						ymodem.eotcnt = 0;
					}
					else if (c == MODEM_STX)
					{
						//data size = 1024
						ymodem.pbuf = (uint8_t*)ymodem.YmodemMalloc(1024);
						if (ymodem.pbuf == 0)
						{
							ret = -2;
							goto RET;
						}
						ymodem.head = c;
						ymodem.eymstatus = eymINDEX0;
					}
					else if (c == MODEM_EOT)
					{
						//master end of transf
						//slave send nack
						if (ymodem.eotcnt == 0)
						{
							buf[0] = MODEM_NAK;
						}
						else if (ymodem.eotcnt == 1)
						{
							buf[0] = MODEM_ACK;
							ymodem.config.cfg.isEOT = 1;
							ymodem.config.cfg.isNeedSendC = 1;
						}
						buf[1] = 0;
						ymodem.YmodemTx((uint8_t*)buf,strlen((const char*)buf));
						ymodem.eotcnt++;
					}
					break;
				}
				case eymINDEX0:
				{
					ymodem.index[0] = c;
					ymodem.eymstatus = eymINDEX1;
					break;
				}
				case eymINDEX1:
				{
					ymodem.index[1] = c;
					if (ymodem.index[1] + ymodem.index[0] == 0xff)
					{
						ymodem.eymstatus = eymDATA;
						ymodem.rxsize = 0;
					}
					else
					{
						ret = -3;
					}
					break;
				}
				case eymDATA:
				{
					ymodem.pbuf[ymodem.rxsize++] = c;
					unsigned int size = ymodem.head == MODEM_SOH? 128 : 1024;
					if (ymodem.rxsize >= size)
					{
						ymodem.eymstatus = eymCRC0;
					}
					break;
				}
				case eymCRC0:
				{
					ymodem.crc16 = c;
					ymodem.eymstatus = eymCRC1;
					break;
				}
				case eymCRC1:
				{
					ymodem.crc16 <<= 8;
					ymodem.crc16 |= c;
					//do crc check
					uint16_t crc16 = Crc16_xmode(ymodem.pbuf,ymodem.rxsize);
					if (crc16 == ymodem.crc16)
					{
						//if ok send ack ,than write flash, and send 'C'
						if (ymodem.filesize == 0)
						{
							int filenamelen = strlen((const char*)ymodem.pbuf);
							if (ymodem.pfilename == 0)
							{
								ymodem.pfilename = ymodem.YmodemMalloc(filenamelen + 4);
								memset(ymodem.pfilename,0,filenamelen + 4);
								if (ymodem.pfilename)
								{
									memcpy(ymodem.pfilename,ymodem.pbuf,filenamelen);
								}
							}
							unsigned char *p = ymodem.pbuf + filenamelen + 1;
							ymodem.filesize = 0;
							while((*p!=0x20) &&(*p!=0))
							{
								ymodem.filesize *= 10;
								ymodem.filesize += *p++ - 0x30;
							}
							if (ymodem.filesize == 0)
							{
								ret = -4;
								goto RET;
							}
							if (ymodem.YmodemWrite((uint8_t*)&ymodem.filesize,4,eYM_FILESIZE) != 0)
							{
								ret = -5;
								goto RET;
							}
							if (ymodem.YmodemWrite(ymodem.pfilename,strlen((const char*)ymodem.pfilename),eYM_FILENAME) != 0)
							{
								ret = -6;
								goto RET;
							}
							ymodem.config.cfg.isNeedSendC = 1;
						}
						else
						{
							if (ymodem.flashoffset < ymodem.filesize)
							{
								ymodem.YmodemWrite(ymodem.pbuf,ymodem.rxsize,eYM_DATA);
								ymodem.flashoffset += ymodem.rxsize;
							}
							else
							{
								//rx null data
								if (ymodem.config.cfg.isNeedCheckEnd == 0)
								{
									ymodem.config.cfg.isNeedCheckEnd = 1;
								}
							}
						}
						buf[0] = MODEM_ACK;
						buf[1] = 0;
						ymodem.YmodemTx((uint8_t*)buf,strlen((const char*)buf));
						if (ymodem.pbuf)
						{
							ymodem.YmodemFree(ymodem.pbuf);
							ymodem.pbuf = 0;
						}
						
						ymodem.eymstatus = eymHEAD;
						
						
						if (ymodem.config.cfg.isNeedCheckEnd)
						{
							if ((ymodem.index[0] == 0) && (ymodem.index[1] == 0xFF))
							{
								//end
								if (ymodem.config.cfg.isEOT)
								{
									ret = 2;
									ymodem.config.cfg.isNeedSendC = 0;
									buf[0] = MODEM_CAN;
									buf[1] = 0;
									ymodem.YmodemTx((uint8_t*)buf,strlen((const char*)buf));
									goto RET;
								}
							}
						}
						ymodem.errorcnt = 0;
						ret = 1;
					}
					else
					{
						buf[0] = MODEM_NAK;
						buf[1] = 0;
						ymodem.YmodemTx((uint8_t*)buf,strlen((const char*)buf));
						ymodem.errorcnt++;
						if (ymodem.errorcnt >= 5)
						{
							ret = -7;
							goto RET;
						}
					}
					break;
				}
				default:break;
			}
		}
	}while(c != -1);
	RET:
	return ret;
}

int isYmodemBusy(void)
{
	return ymodem.config.cfg.isYmodemStart;
}
