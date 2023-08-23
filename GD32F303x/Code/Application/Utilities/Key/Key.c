#include "Key.h"

sl_list_t * key_sl_list;

void KeyInit(void)
{
	key_sl_list = 0;
}

void KeyRegister(sl_list_t * pkey)
{
	if (key_sl_list == 0)
	{
		key_sl_list = pkey;
	}
	else
	{
		sl_list_append(key_sl_list,pkey);
	}
}

void KeyUnRegister(sl_list_t * pkey)
{
	sl_list_remove(key_sl_list,pkey);
}

void KeyDetect(void)
{
	sl_list_t * p = key_sl_list;
	while(p)
	{
		KeyInfo_t * pKeyInfo = (KeyInfo_t*)(p->p);
		eKey status = (eKey)pKeyInfo->pKeyCB->pKeyIoDetect(0);
		if (status == eKeyLow)
		{
			pKeyInfo->usDebounce++;
			if (pKeyInfo->ekey == eKeyLongPress)
			{
				#if LONG_CONTINUE
				pKeyInfo->ekey = eKeyLongContinnue;
				pKeyInfo->usLongContinueDebounce = pKeyInfo->usDebounce + pKeyInfo->usLongPressDebounce;
				#endif
			}
			#if LONG_CONTINUE
			else if ((pKeyInfo->ekey == eKeyLongContinnue) && (pKeyInfo->usLongContinueDebounce == pKeyInfo->usDebounce))
			{
				if (pKeyInfo->pKeyCB->pKeyLongContinueCB != NULL)
				{
					pKeyInfo->pKeyCB->pKeyLongContinueCB(pKeyInfo);
				}
				pKeyInfo->usLongContinueDebounce = pKeyInfo->usDebounce + pKeyInfo->usLongPressDebounce;
			}
			#endif
			else if ((pKeyInfo->ekey != eKeyLongPress)&&(pKeyInfo->usDebounce == pKeyInfo->usLongPressDebounce))
			{
				pKeyInfo->ekey = eKeyLongPress;
				if (pKeyInfo->pKeyCB->pKeyLongPressCB != NULL)
				{
					pKeyInfo->pKeyCB->pKeyLongPressCB(pKeyInfo);
				}
			}
			else if (pKeyInfo->usDebounce == pKeyInfo->usShortPressDebounce)
			{
				pKeyInfo->ekey = eKeyPress;
				if (pKeyInfo->pKeyCB->pKeyPressCB != NULL)
				{
					pKeyInfo->pKeyCB->pKeyPressCB(pKeyInfo);
				}
				#if MULTI_CLICK
				pKeyInfo->ucShortKeyTimes++;
				pKeyInfo->usReleaseDebounce = 0;
				#endif
			}
		}
		else
		{
			if (pKeyInfo->ekey == eKeyPress)
			{
				#if MULTI_CLICK
				pKeyInfo->usReleaseDebounce++;
				if (pKeyInfo->usReleaseDebounce > pKeyInfo->usShortKeyReleaseDeobunce)
				{
					//short key release
					if (pKeyInfo->pKeyCB->pKeyReleaseCB != NULL)
					{
						pKeyInfo->pKeyCB->pKeyReleaseCB(pKeyInfo);
						pKeyInfo->ucShortKeyTimes = 0;
						pKeyInfo->usReleaseDebounce = 0;
						pKeyInfo->ekey = eKeyNull;
					}
				}
				#else
				if (pKeyInfo->pKeyCB->pKeyReleaseCB != NULL)
				{
					pKeyInfo->pKeyCB->pKeyReleaseCB(pKeyInfo);
					pKeyInfo->ekey = eKeyNull;
				}
				#endif
			}
			#if LONG_CONTINUE
			else if ((pKeyInfo->ekey == eKeyLongContinnue)||(pKeyInfo->ekey == eKeyLongPress))
			{
				if (pKeyInfo->pKeyCB->pKeyLongReleaseCB != NULL)
				{
					pKeyInfo->pKeyCB->pKeyLongReleaseCB(pKeyInfo);
					#if MULTI_CLICK
					pKeyInfo->ucShortKeyTimes = 0;
					pKeyInfo->usReleaseDebounce = 0;
					#endif
					pKeyInfo->ekey = eKeyNull;
				}
			}
			#else
			else if (pKeyInfo->ekey == eKeyLongPress)
			{
				if (pKeyInfo->pKeyCB->pKeyLongReleaseCB != NULL)
				{
					pKeyInfo->pKeyCB->pKeyLongReleaseCB(pKeyInfo);
					#if MULTI_CLICK
					pKeyInfo->ucShortKeyTimes = 0;
					pKeyInfo->usReleaseDebounce = 0;
					#endif
					pKeyInfo->ekey = eKeyNull;
				}
			}
			#endif
			pKeyInfo->usDebounce = 0;
		}
		p = p->pnext;
	}
}

void KeyProcess(unsigned int systick)
{
	static uint32_t keytick = 0;
	#ifdef __RT_THREAD_H__
	systick = rt_tick_get();
	#endif
	if ((systick - (keytick + 10)) <= (0xFFFFFFFF / 2)){
		keytick = systick;
		KeyDetect();
	}
}
