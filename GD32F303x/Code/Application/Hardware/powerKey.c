
#include "main.h"
#include "powerkey.h"
#include "fatfs.h"
#include "powerboard.h"
//#include "./clife_ble/clifeapp.h"
//#include "./motor_action_track/motor_action_track_app.h"
//#include "./serialmotor/serialmotorapp.h"

void PowerOnMotorCb(void)
{
}

void PowerOffMotorCb(void)
{
//	gpio_bit_reset(GPIOB,GPIO_PIN_14);
//	rt_enter_critical();
//	uint32_t ticks = rt_tick_get();
//	while(1)
//	{
//		fwdgt_counter_reload();
//		rt_thread_delay(1000);
//		
//		if (rt_tick_get() >= (ticks + 10 * 1000))
//		{
//			__disable_irq();
//			SCB->VTOR = NVIC_VECTTAB_FLASH;
//			NVIC_SystemReset();
//		}
//	}
}

int PowerKeyDetect(KeyInfo_t * pkey)
{
	return gpio_input_bit_get(GPIOA,GPIO_PIN_0);
}

int PowerKeyShortPress(KeyInfo_t * pkey)
{
//	if (gpio_output_bit_get(GPIOB,GPIO_PIN_14) == 0)
//	{
//		power_board_set_led(ePB_LED_POWER_ONNING,-1,ePB_LED_NULL);
//	}
	return 0;
}

int PowerKeyShortRelease(KeyInfo_t * pkey)
{
//	if (gpio_output_bit_get(GPIOB,GPIO_PIN_14) == 0)
//	{
//		power_board_set_led(ePB_LED_POWER_OFF,-1,ePB_LED_NULL);
//		return -1;
//	}
//	if (pkey->ucShortKeyTimes == 1)
//	{
//		int count = matfs_get_total_file_number();
//		if ((mat_get_mode() != eUserMode) && (mat_get_mode() != eFixedMode))
//		{
//			mat_set_index(mat_get_index());
//		}
//		else
//		{
//			mat_set_next();
//		}
//		mat_set_mode_loop(eLOOP_SELF,MOTOR_MODE_TICK);
//		mat_set_cb(0);
//	}
//	else if (pkey->ucShortKeyTimes == 2)
//	{
//		mat_set_mode(eNullMode);
//		mat_set_cb(0);
//	}
//	else if (pkey->ucShortKeyTimes == 5)
//	{
//		mat_set_index(0);
//		mat_set_mode_loop(eLOOP_SELF,72 * 60 * 60 * 1000);
//		mat_set_cb(0);
//	}
	return 0;
}

int PowerKeyLongPress(KeyInfo_t * pkey)
{
	#if LONG_CONTINUE
	pkey->ucLongContinueTimes = 1;
	if (pkey->ucLongContinueTimes == 1)
	{
//		if (gpio_output_bit_get(GPIOB,GPIO_PIN_14) == SET)
//		{
//			//enter power off
//			rt_kprintf("power off\n");
//			//LED
//			gpio_bit_reset(GPIOB,GPIO_PIN_2);
//			//BLE power
//			gpio_bit_set(GPIOB,GPIO_PIN_8);
//			power_board_set_led(ePB_LED_POWER_OFF,-1,ePB_LED_NULL);
//			
//			char * p = (char*)matf_get_sys_filepath_by_index(0);
//			if (p)
//			{
//				mat_set_mode(eSysMode);
//				mat_set_filename(p);
//				mat_set_mode_loop(eLOOP_NULL,(uint32_t)-1);
//				mat_set_cb(PowerOffMotorCb);
//			}
//			else{
//				PowerOffMotorCb();
//			}
//		}
//		else
//		{
//			//enter power on
//			rt_kprintf("power on\n");
//			//LED
//			gpio_bit_set(GPIOB,GPIO_PIN_2);
//			//SYS power
//			gpio_bit_set(GPIOB,GPIO_PIN_14);
//			power_board_set_led(ePB_LED_POWER_ON,-1,ePB_LED_NULL);
//			fatfs_init();
//			mat_thread_init();
//			serial_motor_app_init();
//			clife_app_init();
//			//初始化定时器
//			Timer0Init();
//			
//			char * p = (char*)matf_get_sys_filepath_by_index(0);
//			if (p)
//			{
//				mat_set_mode(eSysMode);
//				mat_set_filename(p);
//				mat_set_mode_loop(eLOOP_NULL,(uint32_t)-1);
//				mat_set_cb(PowerOnMotorCb);
//			}
//		}
	}
	#endif
	return 0;
}
int PowerKeyLongContinue(KeyInfo_t * pkey)
{
	pkey->ucLongContinueTimes++;
	if (pkey->ucLongContinueTimes == 3)
	{
		if (pkey->ucLongContinueTimes == 3)
		{
			
		}
		pkey->ucLongContinueTimes = 3;
	}
	return 0;
}
int PowerKeyLongRelease(KeyInfo_t * pkey)
{
	pkey->ucLongContinueTimes = 0;
	return 0;
}

const KeyCB_t PowerKeyCbBuf=
{
	(pKeyCallBackFunction)PowerKeyDetect,
	(pKeyCallBackFunction)PowerKeyShortPress,
	(pKeyCallBackFunction)PowerKeyShortRelease,
	(pKeyCallBackFunction)PowerKeyLongPress,
	#if LONG_CONTINUE
	(pKeyCallBackFunction)PowerKeyLongContinue,
	#endif
	(pKeyCallBackFunction)PowerKeyLongRelease,
};

sl_list_t power_key_list;
KeyInfo_t power_key_info;

void PowerKeyInit(void)
{
	power_key_info.ekey = eKeyNull;
	power_key_info.usDebounce = 0;
	power_key_info.usShortPressDebounce = 10;
	power_key_info.usLongPressDebounce = 100;
	#if MULTI_CLICK
	power_key_info.ucShortKeyTimes = 0;
	power_key_info.usReleaseDebounce = 0;
	power_key_info.usShortKeyReleaseDeobunce = 20;
	#endif
	#if LONG_CONTINUE
	power_key_info.usLongContinueDebounce = 0;
	power_key_info.ucLongContinueTimes = 0;
	#endif
	power_key_info.pKeyCB = (KeyCB_t*)&PowerKeyCbBuf;

	power_key_list.p = (void*)&power_key_info;
	power_key_list.pnext = 0;
	
	KeyRegister(&power_key_list);
}

