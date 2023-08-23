
#ifndef FIFO_LIST_H
#define FIFO_LIST_H

#include "stdint.h"


typedef struct _fifolist_t
{
	uint32_t handle;
	uint8_t* pbuf;
	uint32_t rxbufoffset;
	uint32_t rxcachereadoffset;
	uint32_t rxcachewriteoffset;
	uint32_t rxbufsize;

	struct _fifolist_t * pnext;
}fifolist_t;

int fifo_register(uint32_t handle,uint32_t fifolen);
int fifo_unregister(uint32_t handle);
void fifo_clear(uint32_t handle);
int fifo_get_char(uint32_t handle,uint8_t *pbuf,uint32_t size);
void fifo_fill(uint32_t handle,uint8_t* pbuf,uint32_t size);
#endif
