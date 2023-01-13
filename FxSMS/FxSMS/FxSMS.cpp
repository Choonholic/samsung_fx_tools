//
// FxSMS.cpp
// Version 1.0
//
// Copyright (c) 2006 InSoftware House.
// December 7, 2006
//

#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>
#include "nled.h"

#define	KEY_SETTINGS		_T("Software\\ProjectFx\\FxSMS")
#define	KEY_SMS_UNREAD		_T("System\\State\\Messages\\SMS\\Unread")
#define	KEY_MMS_UNREAD		_T("System\\State\\Messages\\MMS\\Unread")
#define	KEY_SMS_TIMESTAMP	_T("Software\\Samsung\\RILDrv\\SMSCash\\TimStamp")

#define	VAL_SOUNDFILE		_T("SoundFile")
#define	VAL_NOTIFYTYPE		_T("NotifyType")
#define	VAL_INTERVAL		_T("Interval")
#define	VAL_MAXNOTIFY		_T("MaxNotify")
#define	VAL_EXIT			_T("Exit")

#define	VAL_COUNT			_T("Count")
#define	VAL_YEAR			_T("Year")
#define	VAL_MONTH			_T("Month")
#define	VAL_DAY				_T("Day")
#define	VAL_HOUR			_T("Hour")
#define VAL_MINUTE			_T("Minute")
#define	VAL_SECOND			_T("Second")

#define	ITV_DETECT			5000							// 5 seconds
#define	ITV_BASE			(ULONGLONG)600000000UL			// 1 minute
#define	ITV_NOTIFY			2								// 2 minutes
#define	MAX_NOTIFY			5								// 5 times

#define	NTP_SOUND			0
#define	NTP_VIBRATION		1
#define	NTP_LED				2

UINT GetLEDCount(void)
{
	NLED_COUNT_INFO nci;
	UINT			uiCount	= 0;

	if (NLedGetDeviceInfo(NLED_COUNT_INFO_ID, (PVOID)&nci))
	{
		uiCount	= (UINT)nci.cLeds;
	}

	return uiCount;
}

void SetLEDStatus(UINT uiLed, INT iStatus)
{
	NLED_SETTINGS_INFO	nsi;
       
	nsi.LedNum		= uiLed;
	nsi.OffOnBlink	= iStatus;

	NLedSetDevice(NLED_SETTINGS_INFO_ID, &nsi);
}

void LEDDance(void)
{
	for (int i = 0; i < 13; i++)
	{
		SetLEDStatus(i, 1);
		Sleep(120);
		SetLEDStatus(i, 0);
	}
}

void Vibrate(void)
{
	SetLEDStatus(13, 1);
	Sleep(2500);
	SetLEDStatus(13, 0);
}

int _tmain(int argc, _TCHAR* argv[])
{
	BOOL	bLoop		= TRUE;
	BOOL	bPlayed		= FALSE;
	BOOL	bNeedToPlay	= FALSE;

	HKEY	hKey;
	DWORD	dwDisposition;
	DWORD	dwType;
	DWORD	dwData;
	DWORD	dwcbData;
	DWORD	dwTotal;
	DWORD	dwPlayed;

	SYSTEMTIME		stCurrent;
	FILETIME		ftCurrent;
	ULARGE_INTEGER	uiCurrent;
	ULARGE_INTEGER	uiLastPlayed;
	FILETIME		ftLastReceived;

	TCHAR		szSoundFile[MAX_PATH + 1];
	DWORD		dwNotifyType				= NTP_SOUND;
	ULONGLONG	ullInterval					= (ITV_NOTIFY * ITV_BASE);
	DWORD		dwMaxNotify					= MAX_NOTIFY;

	while (bLoop == TRUE)
	{
		bNeedToPlay	= FALSE;
		dwTotal		= 0;

		Sleep(ITV_DETECT);

		if (RegCreateKeyEx(HKEY_CURRENT_USER, KEY_SETTINGS, 0, NULL, 0, 0, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS)
		{
			dwType		= REG_SZ;
			dwcbData	= (sizeof(TCHAR) * (MAX_PATH + 1));

			RegQueryValueEx(hKey, VAL_SOUNDFILE, NULL, &dwType, (LPBYTE)szSoundFile, &dwcbData);

			dwType		= REG_DWORD;
			dwData		= 0;
			dwcbData	= sizeof(DWORD);

			RegQueryValueEx(hKey, VAL_NOTIFYTYPE, NULL, &dwType, (LPBYTE)&dwNotifyType, &dwcbData);

			if ((dwNotifyType < 0) || (dwNotifyType > 2))
			{
				dwNotifyType	= NTP_SOUND;
			}

			RegQueryValueEx(hKey, VAL_INTERVAL, NULL, &dwType, (LPBYTE)&dwData, &dwcbData);

			if ((dwData < 1) || (dwData > 5))
			{
				dwData	= ITV_NOTIFY;
			}

			ullInterval	= (dwData * ITV_BASE);

			RegQueryValueEx(hKey, VAL_MAXNOTIFY, NULL, &dwType, (LPBYTE)&dwMaxNotify, &dwcbData);

			if ((dwMaxNotify < 1) || (dwMaxNotify > 10))
			{
				dwMaxNotify	= MAX_NOTIFY;
			}

			dwData		= 0;

			RegQueryValueEx(hKey, VAL_EXIT, NULL, &dwType, (LPBYTE)&dwData, &dwcbData);

			if (dwData != 0)
			{
				dwData	= 0;
				bLoop	= FALSE;

				RegDeleteValue(hKey, VAL_EXIT);
			}

			RegCloseKey(hKey);
		}

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEY_SMS_UNREAD, 0, 0, &hKey) == ERROR_SUCCESS)
		{
			RegQueryValueEx(hKey, VAL_COUNT, NULL, &dwType, (LPBYTE)&dwData, &dwcbData);
			RegCloseKey(hKey);

			dwTotal	+= dwData;
		}

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEY_MMS_UNREAD, 0, 0, &hKey) == ERROR_SUCCESS)
		{
			RegQueryValueEx(hKey, VAL_COUNT, NULL, &dwType, (LPBYTE)&dwData, &dwcbData);
			RegCloseKey(hKey);

			dwTotal	+= dwData;
		}

		if (dwTotal > 0)
		{
			SystemIdleTimerReset();

			if (bPlayed == TRUE)
			{
				if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEY_SMS_TIMESTAMP, 0, 0, &hKey) == ERROR_SUCCESS)
				{
					memset(&stCurrent, 0, sizeof(SYSTEMTIME));
					RegQueryValueEx(hKey, VAL_YEAR, NULL, &dwType, (LPBYTE)&stCurrent.wYear, &dwcbData);
					RegQueryValueEx(hKey, VAL_MONTH, NULL, &dwType, (LPBYTE)&stCurrent.wMonth, &dwcbData);
					RegQueryValueEx(hKey, VAL_DAY, NULL, &dwType, (LPBYTE)&stCurrent.wDay, &dwcbData);
					RegQueryValueEx(hKey, VAL_HOUR, NULL, &dwType, (LPBYTE)&stCurrent.wHour, &dwcbData);
					RegQueryValueEx(hKey, VAL_MINUTE, NULL, &dwType, (LPBYTE)&stCurrent.wMinute, &dwcbData);
					RegQueryValueEx(hKey, VAL_SECOND, NULL, &dwType, (LPBYTE)&stCurrent.wSecond, &dwcbData);
					RegCloseKey(hKey);

					if (CompareFileTime(&ftLastReceived, &ftCurrent) < 0)
					{
						memcpy_s(&ftLastReceived, sizeof(FILETIME), &ftCurrent, sizeof(FILETIME));
						
						dwPlayed				= 0;
						uiLastPlayed.QuadPart	= 0;
					}
				}
				
				GetLocalTime(&stCurrent);
				SystemTimeToFileTime(&stCurrent, &ftCurrent);

				uiCurrent.LowPart	= ftCurrent.dwLowDateTime;
				uiCurrent.HighPart	= ftCurrent.dwHighDateTime;

				if ((uiCurrent.QuadPart - uiLastPlayed.QuadPart) > ullInterval)
				{
					++dwPlayed;

					if (dwPlayed <= dwMaxNotify)
					{
						uiLastPlayed.QuadPart	= uiCurrent.QuadPart;
						bNeedToPlay				= TRUE;
					}
				}
			}
			else
			{
				++dwPlayed;

				bPlayed		= TRUE;

				GetLocalTime(&stCurrent);
				SystemTimeToFileTime(&stCurrent, &ftCurrent);

				uiLastPlayed.LowPart	= ftCurrent.dwLowDateTime;
				uiLastPlayed.HighPart	= ftCurrent.dwHighDateTime;
				bNeedToPlay				= TRUE;
			}
		}
		else
		{
			dwPlayed				= 0;
			bPlayed					= FALSE;
			uiLastPlayed.QuadPart	= 0;
		}

		if (bNeedToPlay == TRUE)
		{
			switch (dwNotifyType)
			{
			case NTP_SOUND:
				PlaySound(szSoundFile, NULL, SND_FILENAME | SND_ASYNC);
				break;
			case NTP_VIBRATION:
				Vibrate();
				break;
			case NTP_LED:
				LEDDance();
				break;
			}
		}
	}

	return 0;
}