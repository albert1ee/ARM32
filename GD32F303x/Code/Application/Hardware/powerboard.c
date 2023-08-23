#include "main.h"
#include "powerboard.h"

power_board_t power_board;

void power_board_set_led(int type,int backupticks,int backuptype)
{
	if (type < ePB_LED_NULL)
		return;
	if (type >= ePB_LED_GET_POWER)
		return;
	uint8_t buf[4];
	buf[1] = 1;
	buf[2] = type;
	buf[3] = 0;
	for(int i=0;i<3;i++)
	{
		buf[3] += buf[i];
	}
	for(int i=0;i<4;i++)
	{
		usart_data_transmit(USART0,buf[i]);
		while(usart_flag_get(USART0,USART_FLAG_TBE) == RESET);
	}
	power_board.ledstatus_backup = backuptype;
	power_board.ledstatus = type;
	if (backupticks == -1)
		power_board.backupticks = 0;
	else
	{
		power_board.backupticks = backupticks + rt_tick_get();
	}
}

void power_board_get_status(int status)
{
	if (status < ePB_LED_GET_POWER)
		return;
	if (status > ePB_LED_GET_CHARGING)
		return;
	uint8_t buf[4];
	buf[0] = POWER_BOARD_HEAD;
	if (status == ePB_LED_GET_POWER)
	{
		buf[1] = 0x02;
		buf[2] = 0x01;
	}
	else if (status == ePB_LED_GET_CHARGING)
	{
		buf[1] = 0x03;
		buf[2] = 0x01;
	}
	buf[3] = 0;
	for(int i=0;i<3;i++)
	{
		buf[3] += buf[i];
	}
	for(int i=0;i<4;i++)
	{
			usart_data_transmit(USART0,buf[i]);
			while(usart_flag_get(USART0,USART_FLAG_TBE) == RESET);
	}
}

void power_board_init(void)
{
	UsartIrqInit(USART0,9600,1,1);
	fifo_register(USART0,0x400);

	power_board.status = ePB_NULL;
	
	power_board.isCharging = 0;
	power_board.power_percent = 0;
	power_board.ledstatus = 0;
	power_board.backupticks = 0;
	power_board.getstatusticks = rt_tick_get();
}

void power_board_proc(void)
{
	if ((rt_tick_get() - (power_board.getstatusticks + 10 * 1000)) <= (0xFFFFFFFF / 2))
	{
		power_board.getstatusticks = rt_tick_get();
		power_board_get_status(ePB_LED_GET_POWER);
//		rt_thread_delay(40);
//		power_board_get_status(ePB_LED_GET_CHARGING);
	}
	if (power_board.backupticks)
	{
		if (power_board.backupticks != (uint32_t)-1)
		{
			if ((rt_tick_get() - power_board.backupticks) <= (0xFFFFFFFF / 2))
			{
				power_board_set_led(power_board.ledstatus_backup,-1,ePB_LED_NULL);
			}
		}
	}
	
	int c = 0;
	do
	{
		if (fifo_get_char(USART0,(uint8_t*)&c,1) == 0)
			break;
		if (c != -1)
		{
			if (power_board.status == ePB_NULL)
			{
				if (c == POWER_BOARD_HEAD)
				{
					power_board.portocol[power_board.status] = POWER_BOARD_HEAD;
					power_board.status++;
				}
			}
			else if (power_board.status == ePB_HEAD)
			{
				power_board.portocol[power_board.status] = c;
				power_board.status++;
			}
			else if (power_board.status == ePB_CMD)
			{
				power_board.portocol[power_board.status] = c;
				power_board.status++;
			}
			else if (power_board.status == ePB_DATA)
			{
				power_board.portocol[power_board.status] = c;
				uint8_t sum = 0;
				for(int i=0;i<power_board.status;i++)
					sum += power_board.portocol[i];
				if (sum == power_board.portocol[power_board.status])
				{
					if (power_board.portocol[1] == 0x02)
					{
						power_board.power_percent = power_board.portocol[2];
					}
					else if (power_board.portocol[1] == 0x03)
					{
						power_board.isCharging = power_board.portocol[2];;
					}
				}
				power_board.status = ePB_NULL;
			}
		}
	}while(c != -1);
}
