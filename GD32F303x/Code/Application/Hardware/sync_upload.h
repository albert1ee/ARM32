
#ifndef _SYNC_UPLOAD_H_
#define _SYNC_UPLOAD_H_


#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"


void sync_upload_set_motor_ticks(uint32_t tick);
uint32_t sync_upload_get_motor_ticks(void);
void sync_upload_set_motor_upload_type(uint8_t type);
uint8_t sync_upload_get_motor_upload_type(void);
void sync_upload_motor_data_upload(int parent,int type);

void sync_upload_proc(void);

#endif

