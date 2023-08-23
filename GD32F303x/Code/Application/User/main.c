/*!
    \file    main.c
    \brief   running led

    \version 2017-02-10, V1.0.0, firmware for GD32F30x
    \version 2018-10-10, V1.1.0, firmware for GD32F30x
    \version 2018-12-25, V2.0.0, firmware for GD32F30x
    \version 2020-09-30, V2.1.0, firmware for GD32F30x
*/

/*
    Copyright (c) 2020, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "main.h"

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/

uint8_t console_isenable = 1;

void console_getchar_set(int enable)
{
	console_isenable = enable;
}

#ifdef ENABLE_SEGGER_RTT
int rt_hw_console_getchar(void)
{
	if (console_isenable)
	{
		int data = 0;
		int size = SEGGER_RTT_Read(0,&data,1);
		if (size)
			return data;
	}
   return -1;
}

void rt_hw_console_output(const char *str)
{
	SEGGER_RTT_Write(0,(uint8_t*)(str),strlen(str));
}
#endif

void reset(int argc,char**argv)
{
	NVIC_SystemReset();
}
MSH_CMD_EXPORT(reset, soft reset device);


int BoarInit(void)
{
//	SCB->VTOR |= BOOTLOADER_FLASH_SIZE + USER_APP_CONFIG_SIZE;
	systick_config();
	__enable_irq();
	GpioInit();
	return 0;
}
INIT_BOARD_EXPORT(BoarInit);

//	INIT_PREV_EXPORT;

int DeviceInit(void)
{
	SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
	KeyInit();
	PowerKeyInit();
	return 0;
}
INIT_DEVICE_EXPORT(DeviceInit);


int main(void)
{
	power_board_init();
	while (1){
		KeyProcess(rt_tick_get());
		fwdgt_counter_reload();
		power_board_proc();
		//YmodemProcess();
		//sync_upload_proc();
		rt_thread_delay(10);
	}
}
