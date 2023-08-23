
#ifndef _POWER_BOARD_H_
#define _POWER_BOARD_H_


enum
{
	ePB_LED_NULL,
	ePB_LED_POWER_ONNING,
	ePB_LED_POWER_ON,
	ePB_LED_SET_NET,
	ePB_LED_SET_NET_SUCCESS,
	ePB_LED_FILE_DOWNLOAD,
	ePB_LED_FILE_DOWNLOAD_SUCCESS,
	ePB_LED_POWER_OFF,
	ePB_LED_GET_POWER,
	ePB_LED_GET_CHARGING,
};



#define POWER_BOARD_HEAD			0xA5

enum
{
	ePB_NULL,
	ePB_HEAD,
	ePB_CMD,
	ePB_DATA,
	ePB_SUM,
};

typedef struct _power_board_t
{
	unsigned char portocol[4];
	
	unsigned char status;
	
	unsigned char power_percent;
	unsigned char isCharging;
	unsigned char ledstatus;
	
	unsigned char ledstatus_backup;
	unsigned int backupticks;
	
	unsigned int getstatusticks;
}power_board_t;


void power_board_set_led(int type,int backupticks,int backuptype);
void power_board_get_status(int status);
void power_board_init(void);
void power_board_proc(void);
#endif

