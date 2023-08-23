#ifndef _SYS_H_
#define _SYS_H_

#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"

void GpioInit(void);
void GpioUartGpioInit(uint32_t rxport,uint32_t rxpin,uint32_t txport,uint32_t txpin);
void GpioUartGpioDeInit(uint32_t rxport,uint32_t rxpin,uint32_t txport,uint32_t txpin);

void UsartIrqInit(uint32_t uart,uint32_t br,int tx_enable,int rx_enable);
void UsartIrqDeInit(uint32_t uart);
uint32_t GetUartPortByName(char * uart);

void Timer0Init(void);


#endif
