
#include "sl_list.h"

void sl_list_init(sl_list_t * p)
{
	if (p)
	{
		p->pnext = 0;
		p->p = 0;
	}
}

int sl_list_cnt(sl_list_t *p)
{
	int cnt = 0;
	if (p)
	{
		while(p)
		{
			cnt++;
			p = p->pnext;
		}
	}
	return cnt;
}
void sl_list_append(sl_list_t* plist,sl_list_t *p)
{
	while(plist->pnext) plist = plist->pnext;
	
	plist->pnext = p;
}

void sl_list_insert(sl_list_t* plist,sl_list_t *p)
{
	p->pnext = plist->pnext;
	plist->pnext = p;
}

sl_list_t* sl_list_remove(sl_list_t *plist, sl_list_t *p)
{
    /* remove slist head */
    while (plist->pnext && plist->pnext != p) plist = plist->pnext;

    /* remove node */
    if (plist->pnext != (sl_list_t *)0) plist->pnext = plist->pnext->pnext;

    return plist;
}

sl_list_t* sl_list_tail(sl_list_t *plist)
{
    while (plist->pnext) plist = plist->pnext;

    return plist;
}

sl_list_t* sl_list_next(sl_list_t *plist)
{
    return plist->pnext;
}

sl_list_t* sl_list_offet(sl_list_t *plist,int offset)
{
	while(plist && (offset > 0))
	{
		plist = plist->pnext;
		offset--;
	}
	return plist;
}

