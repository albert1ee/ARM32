

#ifndef _KEY_H_
#define _KEY_H_

#include "stdio.h"
#include "stdint.h"
#include "sl_list.h"

#define MULTI_CLICK		1
#define LONG_CONTINUE	1


typedef enum
{
	eKeyLow,
	eKeyHigh,
}eKey;

typedef enum
{
	eKeyNull,
	eKeyPress,
	eKeyRelease,
	eKeyLongPress,
	#if LONG_CONTINUE
	eKeyLongContinnue,
	#endif
	eKeyLongRelease,
}eKeyStatus;

typedef int (*pKeyCallBackFunction)(void* parm);

typedef struct _Key_CB_t
{
	pKeyCallBackFunction pKeyIoDetect;
	pKeyCallBackFunction pKeyPressCB;
	pKeyCallBackFunction pKeyReleaseCB;
	pKeyCallBackFunction pKeyLongPressCB;
	#if LONG_CONTINUE
	pKeyCallBackFunction pKeyLongContinueCB;
	#endif
	pKeyCallBackFunction pKeyLongReleaseCB;
}KeyCB_t;

typedef struct _Key_t
{
	uint8_t uId;
	eKeyStatus ekey;
	uint16_t usDebounce;
	uint16_t usShortPressDebounce;
	uint16_t usLongPressDebounce;
	#if MULTI_CLICK
	uint8_t ucShortKeyTimes;
	uint16_t usReleaseDebounce;
	uint16_t usShortKeyReleaseDeobunce;
	#endif
	#if LONG_CONTINUE
	uint16_t usLongContinueDebounce;
	uint8_t ucLongContinueTimes;
	#endif
	KeyCB_t* pKeyCB;
}KeyInfo_t;

void KeyInit(void);
void KeyProcess(unsigned int systick);

void KeyRegister(sl_list_t * pkey);
void KeyUnRegister(sl_list_t * pkey);
#endif
