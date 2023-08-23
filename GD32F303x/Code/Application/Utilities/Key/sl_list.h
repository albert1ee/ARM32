
#ifndef _SINGLE_LINKED_LIST_H_
#define _SINGLE_LINKED_LIST_H_


#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"

typedef struct _sl_list
{
	void * p;
	struct _sl_list * pnext;
}sl_list_t;


void sl_list_init(sl_list_t * p);
int sl_list_cnt(sl_list_t *p);
void sl_list_append(sl_list_t* plist,sl_list_t *p);
void sl_list_insert(sl_list_t* plist,sl_list_t *p);
sl_list_t* sl_list_remove(sl_list_t *plist, sl_list_t *p);
sl_list_t *sl_list_tail(sl_list_t *plist);
sl_list_t *sl_list_next(sl_list_t *plist);
sl_list_t* sl_list_offet(sl_list_t *plist,int offset);
#endif
