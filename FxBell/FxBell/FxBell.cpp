//
// FxBell.cpp
// Version 1.0
//
// Copyright (c) 2006 InSoftware House.
// October 29, 2006
//

#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>

LPCTSTR TakeFileName(LPCTSTR lpszFullPath, LPTSTR lpszName)
{
	LPTSTR	lpszFound	= _tcsrchr(lpszFullPath, _T('\\'));
	int		nPos		= -1;

	if (lpszFound != NULL)
	{
		nPos	= lpszFound - lpszFullPath;
	}

	if (nPos >= 0)
	{
		_tcscpy(lpszName, lpszFound + 1);
	}

	return (LPCTSTR)lpszName;
}

LPCTSTR TakeTitle(LPCTSTR lpszFullPath, LPTSTR lpszTitle)
{
	TCHAR	szName[MAX_PATH + 1];

	if (TakeFileName(lpszFullPath, szName))
	{
		LPTSTR	lpszFound	= _tcsrchr(szName, _T('.'));
		int		nPos		= -1;

		if (lpszFound != NULL)
		{
			nPos	= lpszFound - szName;
		}

		if (nPos > 0)
		{
			_tcsncpy(lpszTitle, szName, nPos);

			lpszTitle[nPos]	= NULL;
		}
	}

	return (LPCTSTR)lpszTitle;
}

int CombineFileName(LPTSTR lpszFullPath, LPCTSTR lpszFolder, LPCTSTR lpszFileName)
{
	if (!_tcslen(lpszFolder))
	{
		_tcscpy(lpszFullPath, _T("\\"));
	}
	else if (lpszFolder[0] != '\\')
	{
		_tcscpy(lpszFullPath, _T("\\"));
		_tcscat(lpszFullPath, lpszFolder);
	}
	else
	{
		_tcscpy(lpszFullPath, lpszFolder);
	}

	if (lpszFullPath[_tcslen(lpszFullPath) - 1] != _T('\\'))
	{
		_tcscat(lpszFullPath, _T("\\"));
	}

	_tcscat(lpszFullPath, lpszFileName);

	return (_tcslen(lpszFullPath));
}

INT GetSpecialDirectoryEx(INT nFolderID, LPTSTR lpszDir)
{
	int				rc;
	LPITEMIDLIST	pidl;
	BOOL			fUseIMalloc	= TRUE;
	LPMALLOC		lpMalloc	= NULL;

	rc = SHGetMalloc(&lpMalloc);

	if (rc == E_NOTIMPL)
	{
		fUseIMalloc = FALSE;
	}
	else if (rc != NOERROR)
	{
		return rc;
	}

	rc = SHGetSpecialFolderLocation(NULL, nFolderID, &pidl);

	if (rc == NOERROR)
	{
		if (SHGetPathFromIDList(pidl, lpszDir))
		{
			rc = E_FAIL;
		}
		if (fUseIMalloc)
		{
			lpMalloc->Free(pidl);
		}
	}

	if (fUseIMalloc)
	{
		lpMalloc->Release();
	}

	return rc;
}

#define	MMF_EXT			_T(".mmf")
#define	MMF_FIND		_T("*.mmf")
#define	PRESOUND_MMF	_T("PreSoundP11.mmf")
#define	RINGS_FOLDER	_T("\\Rings")
#define	RINGTONES_KEY	_T("\\ControlPanel\\Sounds\\RingTone0")
#define	SOUND_VALUE		_T("Sound")
#define BELLSET_TEMP	_T("'%s' 이(가) 벨소리로 지정되었습니다.")
#define MESSAGE_TITLE	_T("알림")

int _tmain(int argc, _TCHAR* argv[])
{
	TCHAR			szBellName[MAX_PATH];
	TCHAR			szWindows[MAX_PATH];
	TCHAR			szRings[MAX_PATH];
	TCHAR			szDummyName[MAX_PATH];
	TCHAR			szMessage[256];
	HANDLE			hFile;
	HKEY			hKey;
	DWORD			dwDisposition;
	WIN32_FIND_DATA	fd;

	if (argc == 2)
	{
		// Build Dummy File
		TakeTitle(argv[1], szBellName);
		GetSpecialDirectoryEx(CSIDL_WINDOWS, szWindows);
		CombineFileName(szRings, szWindows, RINGS_FOLDER);
		CombineFileName(szDummyName, szRings, MMF_FIND);

		hFile	= FindFirstFile(szDummyName, &fd);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			do
			{
				if ((fd.nFileSizeLow == 0) && (fd.nFileSizeHigh == 0))
				{
					CombineFileName(szDummyName, szRings, fd.cFileName);
					SetFileAttributes(szDummyName, FILE_ATTRIBUTE_NORMAL);
					DeleteFile(szDummyName);
				}
			}
			while (FindNextFile(hFile, &fd));
			
			FindClose(hFile);
		}

		CombineFileName(szDummyName, szRings, szBellName);
		_tcscat_s(szDummyName, MAX_PATH, MMF_EXT);

		hFile	= CreateFile(szDummyName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		CloseHandle(hFile);

		// Build Real Bell File

		CombineFileName(szRings, szWindows, PRESOUND_MMF);
		CopyFile(argv[1], szRings, FALSE);

		RegCreateKeyEx(HKEY_CURRENT_USER, RINGTONES_KEY, 0, NULL, 0, NULL, NULL, &hKey, &dwDisposition);
		RegSetValueEx(hKey, SOUND_VALUE, 0, REG_SZ, (LPBYTE)szDummyName, sizeof(TCHAR) * (_tcslen(szDummyName) + 1));
		RegCloseKey(hKey);

		wsprintf(szMessage, BELLSET_TEMP, szBellName);
		MessageBox(NULL, szMessage, MESSAGE_TITLE, MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
	}

	return 0;
}