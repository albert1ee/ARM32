#include "main.h"
#include "fifolist.h"

#if 1	//UART IDLE IRQ
fifolist_t *pfifolist = 0;
int fifo_register(uint32_t handle,uint32_t fifolen)
{
	int r = 0;
	if (fifolen == 0)
		return -1;
	fifolist_t * pfifo = rt_malloc(sizeof(fifolist_t));
	if (pfifo == 0)
		return -2;
	pfifo->pnext = 0;
	pfifo->handle = handle;
	pfifo->pbuf = rt_malloc(fifolen);
	if (pfifo->pbuf == 0)
	{
		rt_free(pfifo);
		return -2;
	}
	pfifo->rxbufsize = fifolen;
	pfifo->rxbufoffset = 0;
	pfifo->rxcachereadoffset = 0;
	pfifo->rxcachewriteoffset = 0;

	fifolist_t *plist = pfifolist;
	if (plist == 0)
	{
		pfifolist = pfifo;
	}
	else
	{
		do
		{
			if (plist->pnext == 0)
			{
				plist->pnext = pfifo;
				break;
			}
			plist = plist->pnext;
		} while (1);
	}
	return r;
}

int fifo_unregister(uint32_t handle)
{
	fifolist_t * pfifo = pfifolist;
	fifolist_t * pnext = 0;
	fifolist_t * plast = 0;
	while(pfifo)
	{
		if (pfifo->handle == handle)
		{
			if (plast)
			{
				plast->pnext = pnext->pnext;
			}
			else
			{
				pfifolist = pnext->pnext;
			}
			if (pfifo->pbuf)
				rt_free(pfifo->pbuf);
			rt_free(pfifo);
			break;
		}
		plast = pfifo;
		pfifo = pfifo->pnext;
	}
	return 0;
}

void fifo_clear(uint32_t handle)
{
	fifolist_t * pfifo = pfifolist;
	while(pfifo)
	{
		if (pfifo->handle == handle)
		{
			pfifo->rxbufoffset = 0;
			pfifo->rxcachereadoffset = 0;
			pfifo->rxcachewriteoffset = 0;
			break;
		}
		pfifo = pfifo->pnext;
	}
}

int fifo_get_char(uint32_t handle,uint8_t *pbuf,uint32_t size)
{
	int c = 0;
	fifolist_t * pfifo = pfifolist;
	while(pfifo)
	{
		if (pfifo->handle == handle)
		{
			while(size)
			{
				if (pfifo->rxcachereadoffset != pfifo->rxcachewriteoffset)
				{
					*pbuf = pfifo->pbuf[pfifo->rxcachereadoffset++];
					if (pfifo->rxcachereadoffset >= pfifo->rxbufsize)
					{
						pfifo->rxcachereadoffset = 0;
					}
					c++;
				}
				else
				{
					break;
				}
				size--;
			}
			break;
		}
		pfifo = pfifo->pnext;
	}
	return c;
}

void fifo_fill(uint32_t handle,uint8_t* pbuf,uint32_t size)
{
	fifolist_t * pfifo = pfifolist;
	while(pfifo)
	{
		if (pfifo->handle == handle)
		{
			if (pfifo->pbuf)
			{
				while(size)
				{
					pfifo->pbuf[pfifo->rxcachewriteoffset++] = *pbuf++;
					if (pfifo->rxcachewriteoffset >= pfifo->rxbufsize)
					{
						pfifo->rxcachewriteoffset = 0;
					}
					size--;
				}
			}
			break;
		}
		pfifo = pfifo->pnext;
	}
}

#endif
