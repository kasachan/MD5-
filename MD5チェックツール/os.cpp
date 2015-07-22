// os.cpp

#include "os.h"

TCHAR *os_qtcscpy(TCHAR *dst, const TCHAR *src);


VOID SetEnableThemeDialogTexture(HWND hWnd)
{
#if _MSC_VER < 1600 || defined(_NODLL)
	if(GetVerifyVersionInfo(5, 1, 0))
	{
		UINT  nRet, nLen;
		TCHAR *szPath;
		const TCHAR *szFile  = _T("UXTHEME.DLL");
		const char  *szFunc1 = "IsAppThemed";
		const char  *szFunc2 = "EnableThemeDialogTexture";

		nLen = GetSystemDirectory(0, 0);

		if(nLen < 1)
			return;

		nLen  += (UINT)_tcslen(szFile) + 1;
		szPath = (TCHAR*)malloc(nLen * sizeof(TCHAR));
		if(szPath == NULL)
			return;

		nRet = GetSystemDirectory(szPath, nLen);

		if(nRet > 0)
		{
			TCHAR *p = szPath + nRet;

			*(p++) = _T('\\');
			_tcscpy(p, szFile);

			HMODULE hDll = LoadLibrary(szPath);
			if(hDll != NULL)
			{
				BOOL (WINAPI *IsAppThemed)();
				IsAppThemed = reinterpret_cast<BOOL(WINAPI*)()>(GetProcAddress(hDll, szFunc1));

				BOOL (WINAPI *EnableThemeDialogTexture)(HWND hWnd, DWORD dwFlags);
				EnableThemeDialogTexture = reinterpret_cast<BOOL(WINAPI*)(HWND, DWORD)>(GetProcAddress(hDll, szFunc2));

				if(IsAppThemed != NULL && EnableThemeDialogTexture != NULL)
				{
#endif /* _MSC_VER < 1600 || defined(_NODLL) */
					if(IsAppThemed())
						EnableThemeDialogTexture(hWnd, ETDT_ENABLETAB);
#if _MSC_VER < 1600 || defined(_NODLL)
				}
				FreeLibrary(hDll);
			}
		}
		free(szPath);

	}
#endif /* _MSC_VER < 1600 || defined(_NODLL) */
}

BOOL GetVerifyVersionInfo(const DWORD dwMajor, const DWORD dwMinor, const DWORD dwSPMajor)
{
	UINT  nRet;

#if _MSC_VER < 1500
	UINT  nLen;
	TCHAR *szPath;
	const TCHAR *szFile  = _T("KERNEL32.DLL");
#ifdef _UNICODE
	const char  *szFunc1 = "VerifyVersionInfoW";
#else
	const char  *szFunc1 = "VerifyVersionInfoA";
#endif
	const char  *szFunc2 = "VerSetConditionMask";


	nLen = GetSystemDirectory(0, 0);

	if(nLen < 1)
		return FALSE;

	nLen  += (UINT)_tcslen(szFile) + 1;
	szPath = (TCHAR*)malloc(nLen * sizeof(TCHAR));
	if(szPath == NULL)
		return FALSE;

	nRet = GetSystemDirectory(szPath, nLen);

	if(nRet > 0)
	{
		TCHAR *p = szPath + nRet;

		*(p++) = _T('\\');
		_tcscpy(p, szFile);

		HMODULE hDll = LoadLibrary(szPath);
		if(hDll != NULL)
		{
			BOOL (WINAPI *VerifyVersionInfo)(LPOSVERSIONINFOEX, DWORD, DWORDLONG);
			VerifyVersionInfo = reinterpret_cast<BOOL(WINAPI*)(LPOSVERSIONINFOEX, DWORD, DWORDLONG)>(GetProcAddress(hDll, szFunc1));

			ULONGLONG (WINAPI *VerSetConditionMask)(ULONGLONG, DWORD, BYTE);
			VerSetConditionMask = reinterpret_cast<ULONGLONG(WINAPI*)(ULONGLONG, DWORD, BYTE)>(GetProcAddress(hDll, szFunc2));

			if(VerifyVersionInfo != NULL && VerSetConditionMask != NULL)
			{
#endif /* _MSC_VER < 1500 */
				OSVERSIONINFOEX osVerInfo = { sizeof(osVerInfo), 0, 0, 0, 0,{ 0 }, 0, 0 };
				const DWORDLONG	dwlConditionMask = VerSetConditionMask(
									VerSetConditionMask(
									VerSetConditionMask(
									0, VER_MAJORVERSION, VER_GREATER_EQUAL),
									VER_MINORVERSION, VER_GREATER_EQUAL),
									VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

				osVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
				osVerInfo.dwMajorVersion      = dwMajor;
				osVerInfo.dwMinorVersion      = dwMinor;
				osVerInfo.wServicePackMajor   = (WORD)dwSPMajor;

				nRet = VerifyVersionInfo(&osVerInfo, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask);
#if _MSC_VER < 1500
			}
			else
				nRet = FALSE;

			FreeLibrary(hDll);
		}
	}
	free(szPath);
#endif /* _MSC_VER < 1500 */
	return (BOOL)nRet;
}

UINT GetOSVersion(OSVERSIONINFOEX *osVerInfo)
{
#if _MSC_VER > 1500

	TCHAR szBuf[33];
	HKEY hKey;
	LONG lRet;
	DWORD dwType = REG_SZ;
	DWORD dwByte = 33;
	DWORD dwSP = 0;
	TCHAR *p;

	lRet = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"),
		0,
		KEY_QUERY_VALUE,
		&hKey);

	if(lRet != ERROR_SUCCESS)
		return FALSE;

	lRet = RegQueryValueEx(
		hKey,
		_T("CurrentVersion"),
		0,
		&dwType,
		(LPBYTE)szBuf,
		&dwByte);
	RegCloseKey(hKey);

	if(lRet != ERROR_SUCCESS)
		return FALSE;

	// サービスパックの取得
	dwType = REG_DWORD;
	dwByte = sizeof(DWORD);

	lRet = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		_T("SYSTEM\\CurrentControlSet\\Control\\Windows"),
		0,
		KEY_QUERY_VALUE,
		&hKey);

	if(lRet == ERROR_SUCCESS)
	{
		lRet = RegQueryValueEx(
			hKey,
			_T("CSDVersion"),
			0,
			&dwType,
			(LPBYTE)&dwSP,
			&dwByte);
		if(lRet == ERROR_SUCCESS)
		{
			dwSP /= 256;
		}
		RegCloseKey(hKey);
	}


	if(szBuf[2] == '.')
	{
		szBuf[2] = '\0';
		p = szBuf + 3;
	}
	else if(szBuf[1] == '.')
	{
		szBuf[1] = '\0';
		p = szBuf + 2;
	}
	else
	{
		return FALSE;
	}

	memset(osVerInfo, 0, sizeof(OSVERSIONINFOEX));
	osVerInfo->dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osVerInfo->dwPlatformId = GetVerifyVersionInfo(5, 0, 0) ? VER_PLATFORM_WIN32_NT : VER_PLATFORM_WIN32_WINDOWS;
	osVerInfo->dwMajorVersion = _ttoi(szBuf);
	osVerInfo->dwMinorVersion = _ttoi(p);
	osVerInfo->wServicePackMajor = (WORD)dwSP;

	return TRUE;

#else

	if(osVerInfo == NULL)
		return 0;

	osVerInfo->dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if(!(GetVersionEx((OSVERSIONINFO*)osVerInfo)))
	{
		osVerInfo->dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		return (GetVersionEx((OSVERSIONINFO*)osVerInfo)) ? 1 : 0;
	}
	return 2;

#endif
}

BOOL GetGokanMode(OSVERSIONINFOEX *osVerInfo)
{
	UINT  nRet = FALSE;
#if _MSC_VER < 1500
	UINT  nLen;
	TCHAR *szPath;
	const TCHAR *szFile  = _T("KERNEL32.DLL");
#ifdef _UNICODE
	const char  *szFunc1 = "VerifyVersionInfoW";
#else
	const char  *szFunc1 = "VerifyVersionInfoA";
#endif
	const char  *szFunc2 = "VerSetConditionMask";


	nLen = GetSystemDirectory(0, 0);

	if(nLen < 1)
		return FALSE;

	nLen  += (UINT)_tcslen(szFile) + 1;
	szPath = (TCHAR*)malloc(nLen * sizeof(TCHAR));
	if(szPath == NULL)
		return FALSE;

	nRet = GetSystemDirectory(szPath, nLen);

	if(nRet > 0)
	{
		TCHAR *p = szPath + nRet;

		*(p++) = _T('\\');
		_tcscpy(p, szFile);

		HMODULE hDll = LoadLibrary(szPath);
		if(hDll != NULL)
		{
			BOOL (WINAPI *VerifyVersionInfo)(LPOSVERSIONINFOEX, DWORD, DWORDLONG);
			VerifyVersionInfo = reinterpret_cast<BOOL(WINAPI*)(LPOSVERSIONINFOEX, DWORD, DWORDLONG)>(GetProcAddress(hDll, szFunc1));

			ULONGLONG (WINAPI *VerSetConditionMask)(ULONGLONG, DWORD, BYTE);
			VerSetConditionMask = reinterpret_cast<ULONGLONG(WINAPI*)(ULONGLONG, DWORD, BYTE)>(GetProcAddress(hDll, szFunc2));

			if(VerifyVersionInfo != NULL && VerSetConditionMask != NULL)
			{
#endif /* _MSC_VER < 1500 */
				OSVERSIONINFOEX	osFindVerInfo;
				const DWORDLONG	dwlConditionMask = VerSetConditionMask(
						VerSetConditionMask(
						VerSetConditionMask(
						0, VER_MAJORVERSION, VER_GREATER_EQUAL),
						VER_MINORVERSION, VER_GREATER_EQUAL),
						VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
				const DWORD			dwData[][3] = {
					{10,2,0},
					{10,1,0},								// Windows 10.1
					{10,0,0},								// Windows 10
					{6,4,0},								// Windows 10 Preview
					{6,3,0},								// Windows 8.1
					{6,2,0},								// Windows 8
					{6,1,2},{6,1,1},{6,1,0},				// Windows 7					
					{6,0,2},{6,0,1},{6,0,0},				// Windows Vista
					{5,2,2},{5,2,1},{5,2,0},				// Windows XP 64bit / Server 2003
					{5,1,3},{5,1,2},{5,1,1},{5,1,0},		// Windows XP
					{5,0,4},{5,0,3},{5,0,2},{5,0,1},{5,0,0}	// Windows 2000
				};

				ZeroMemory(&osFindVerInfo, sizeof(OSVERSIONINFOEX));
				osFindVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

				for(int i = 0; i < sizeof(dwData) / sizeof(dwData[0]); i++)
				{
					osFindVerInfo.dwMajorVersion    = dwData[i][0];
					osFindVerInfo.dwMinorVersion    = dwData[i][1];
					osFindVerInfo.wServicePackMajor = (WORD)dwData[i][2];

					if((nRet = VerifyVersionInfo(&osFindVerInfo, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask)) != FALSE)
					{
						OSVERSIONINFOEX osVerifyVerInfo;

						if(GetOSVersion(&osVerifyVerInfo))
						{
							if(osFindVerInfo.dwMajorVersion == osVerifyVerInfo.dwMajorVersion &&
								osFindVerInfo.dwMinorVersion == osVerifyVerInfo.dwMinorVersion &&
								osFindVerInfo.wServicePackMajor == osVerifyVerInfo.wServicePackMajor)
								nRet = FALSE;

							if(&osVerInfo != NULL)
							{
								osVerInfo->dwMajorVersion    = dwData[i][0];
								osVerInfo->dwMinorVersion    = dwData[i][1];
								osVerInfo->wServicePackMajor = (WORD)dwData[i][2];
							}
						}
						else
							nRet = FALSE;
						break;
					}
				}
#if _MSC_VER < 1500
			}
			else
				nRet = FALSE;

			FreeLibrary(hDll);
		}
	}
	free(szPath);
#endif /* _MSC_VER < 1500 */
	return nRet;
}

BOOL isWin9X()
{
#if _MSC_VER > 1700
	return FALSE;
#else
	return !GetVerifyVersionInfo(5, 0, 0);
#endif
}

UINT GetOSName(TCHAR *lpText, size_t size, BOOL FullName, BOOL VerifyVersion)
{
	UNREFERENCED_PARAMETER(size);
	UNREFERENCED_PARAMETER(FullName);
	UNREFERENCED_PARAMETER(VerifyVersion);

	TCHAR szBuf[100];
	TCHAR *p1;
	HKEY hKey;
	LONG lRet;
	DWORD dwType = REG_SZ;
	DWORD dwByte = 100;

	lRet = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"),
		0,
		KEY_QUERY_VALUE,
		&hKey);

	if(lRet != ERROR_SUCCESS)
		return FALSE;


	lRet = RegQueryValueEx(
		hKey,
		_T("ProductName"),
		0,
		&dwType,
		(LPBYTE)szBuf,
		&dwByte);

	if(lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
#if _MSC_VER < 1500
		lRet = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,
			_T("Software\\Microsoft\\Windows\\CurrentVersion"),
			0,
			KEY_QUERY_VALUE,
			&hKey);

		if(lRet != ERROR_SUCCESS)
			return FALSE;


		lRet = RegQueryValueEx(
			hKey,
			_T("Version"),
			0,
			&dwType,
			(LPBYTE)szBuf,
			&dwByte);
		RegCloseKey(hKey);

		if(lRet != ERROR_SUCCESS)
			return FALSE;

		os_qtcscpy(lpText, szBuf);
		return TRUE;
#else
		return FALSE;
#endif
	}

	p1 = os_qtcscpy(lpText, szBuf);


	// サービスパックの取得
	lRet = RegQueryValueEx(
		hKey,
		_T("CSDVersion"),
		0,
		&dwType,
		(LPBYTE)&szBuf,
		&dwByte);
	RegCloseKey(hKey);

	if(lRet == ERROR_SUCCESS)
	{
		*p1++ = ' ';
		os_qtcscpy(p1, szBuf);
	}

	return TRUE;

/*
	OSVERSIONINFOEX os;
	UINT nRet;

	if((nRet = GetOSVersion(&os)) == FALSE)
		return 0;

	if(VerifyVersion != FALSE)
		GetGokanMode(&os);


	if(nRet > 0)
	{
		TCHAR *szOS			= _T("");
		TCHAR *szEdition	= _T("");
		TCHAR *szSub		= _T("");
		TCHAR *dst			= NULL;
		TCHAR *cp			= NULL;
		TCHAR *szText1		= NULL;
		TCHAR *p1			= NULL;
		BOOL NT40EX			= FALSE;
		BOOL R2				= FALSE;
		BOOL GokanMode		= FALSE;


		cp = (TCHAR*)malloc(_OS_FULL_SIZE * 2 * sizeof(TCHAR));
		if(cp == NULL)
		{
			return 0;
		}

		dst = cp;
		szText1 = cp + _OS_FULL_SIZE;

		switch(os.dwPlatformId)
		{
		case VER_PLATFORM_WIN32_WINDOWS:
#if _MSC_VER >= 1500
			// VC++2008 から Windows 98 / Me のコード省略
			// NT系の互換モードを使わない限りここに来られません（＾＾；
			GokanMode = TRUE;
			szOS = (os.dwMinorVersion == 0) ? _T("Windows 95") : (FullName != FALSE) ? _T("Windows 98 / Me") : _T("Windows 98");

			if(FullName != FALSE)
				szEdition = _T("互換モード");

#else
			switch(os.dwMinorVersion)
			{
			case 0:
				szOS = _T("Windows 95");
#if _MSC_VER >= 1400
				// VC++2005 から Windows 95 のコード省略
				GokanMode = TRUE;
				if(FullName != FALSE)
					szEdition = _T("互換モード");
#else
				// Windows 95 の互換モードにするとバグなのか「950」が返ります。通常は「67109814」が返ります。
				GokanMode = GetVerifyVersionInfo(5, 0, 0);

				if(GokanMode != FALSE)// || os.dwBuildNumber < 0xFFFF
				{
					if(FullName != FALSE)
						szEdition = _T("互換モード");
				}
				else if(_tcslen(os.szCSDVersion) > 0)
				{
					if((WORD)os.dwBuildNumber == 950)//      67109814
						szEdition = _T("OSR1");
					else if((WORD)os.dwBuildNumber > 1111)// 67109975
					{
						if((WORD)os.dwBuildNumber > 1212)//  67110076
						{
							switch((WORD)os.dwBuildNumber)
							{
							case 1212:
							case 1213:
								szEdition = _T("OSR2.1");
								break;
							default:
								szEdition = _T("OSR2.5");
								break;
							}
						}
						else
							szEdition = _T("OSR2");
					}
				}
#endif
				break;
			case 10:
				szOS = _T("Windows 98");
				if(FullName != FALSE)
				{
					GokanMode = GetVerifyVersionInfo(5, 0, 0);

					if(GokanMode != FALSE)
						szEdition = _T("互換モード");
					else if((WORD)os.dwBuildNumber == 2222)
						szEdition = _T("Second Edition");
				}
				break;
			case 90:
				szOS = _T("Windows Me");

				if(FullName != FALSE)
				{
					GokanMode = GetVerifyVersionInfo(5, 0, 0);

					if(GokanMode != FALSE)
						szEdition = _T("互換モード");
				}
				break;
			}
#endif
			break;
		case VER_PLATFORM_WIN32_NT:
			switch(os.dwMajorVersion)
			{
			case 3:
			case 4:
#if _MSC_VER >= 1400
				// VC++2005 から Windows NT 4.0 のコード省略
				NT40EX = TRUE;
				szOS   = _T("Windows NT 4.0");

				if(FullName != FALSE)
					szEdition = _T("互換モード");

#else
				szOS = (os.dwMajorVersion == 4) ? _T("Windows NT 4.0") : _T("Windows NT 3.51");

				if(FullName != FALSE)
				{
					if(nRet == 1)
					{
						// NT 4.0 SP6 だけのはず
						NT40EX = TRUE;

						if(os.wProductType == VER_NT_WORKSTATION)
							szEdition = _T("Workstation");
						else
						{
							if(os.wSuiteMask & VER_SUITE_ENTERPRISE)
								szEdition = _T("Server Enterprise");
							else if(os.wSuiteMask & VER_SUITE_TERMINAL)
								szEdition = _T("Server Terminal Server");
							else if(os.wSuiteMask & (VER_SUITE_EMBEDDEDNT | VER_SUITE_EMBEDDED_RESTRICTED))
								szEdition = _T("Server Embedded");
							else
								szEdition = _T("Server");
						}
					}
					else
					{
						// NT 4.0 SP5 以前はレジストリから取得
						HKEY  hKey;
						DWORD dwLen	= 65;

						Ret	= RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\ProductOptions"), 0, KEY_QUERY_VALUE, &hKey);

						if(Ret == ERROR_SUCCESS)
						{
							Ret = RegQueryValueEx(hKey, _T("ProductType"), NULL, NULL, (LPBYTE)szText1, &dwLen);
							RegCloseKey(hKey);
						}
						if(_tcsicmp(szText1, _T("LANMANNT")) == 0)
							szEdition = _T("Server");
						if(_tcsicmp(szText1, _T("SERVERNT")) == 0)
							szEdition = _T("Enterprise");
						if(_tcsicmp(szText1, _T("WINNT")) == 0)
							szEdition = _T("Workstation");
					}
				}
#endif
				break;
			case 5:
				switch(os.dwMinorVersion)
				{
				case 0:
					// Windows 2000
					szOS = _T("Windows 2000");

					if(FullName != FALSE)
					{

#if _MSC_VER < 1700
						GokanMode = GetVerifyVersionInfo(5, 1, 0);

						if(GokanMode != FALSE)
							szEdition = _T("互換モード");
						else if(os.wProductType == VER_NT_WORKSTATION)
							szEdition = _T("Professional");
						else
						{
							if(os.wSuiteMask & VER_SUITE_DATACENTER)
								szEdition = _T("Datacenter Server");
							else if(os.wSuiteMask & VER_SUITE_ENTERPRISE)
								szEdition = _T("Advanced Server");
							else
								szEdition = _T("Server");
						}
#else
						szEdition = _T("互換モード");
#endif // _MSC_VER < 1700

					}
					break;
				case 1:
					// Windows XP
					szOS = _T("Windows XP");

					if(FullName != FALSE)
					{
#if _MSC_VER <= 1700
						GokanMode = GetVerifyVersionInfo(5, 2, 0);

						if(GokanMode != FALSE)
							szEdition = _T("互換モード");
						else 
#endif						
						if(GetSystemMetrics(SM_TABLETPC))
							szEdition = _T("Tablet PC Edition");
						else if(GetSystemMetrics(SM_MEDIACENTER))
							szEdition = _T("Media Center Edition");
						else if(GetSystemMetrics(SM_STARTER))
							szEdition = _T("Starter Edition");
						else if(os.wSuiteMask & VER_SUITE_PERSONAL)
							szEdition = _T("Home Edition");
						else if(os.wSuiteMask & (VER_SUITE_EMBEDDEDNT | VER_SUITE_EMBEDDED_RESTRICTED))
							szEdition = _T("Embedded");
						else
							szEdition = _T("Professional");
					}
					break;
				case 2:
					// Windows Server 2003 / XP x64 Edition
					if(os.wProductType == VER_NT_WORKSTATION)
					{
						szOS = _T("Windows XP");

						if(FullName != FALSE)
						{
#if _MSC_VER <= 1700
							GokanMode = GetVerifyVersionInfo(6, 0, 0);

							if(GokanMode != FALSE)
								szEdition = _T("x64 互換モード");
							else if(os.wSuiteMask & VER_SUITE_PERSONAL)
#endif
								szEdition = _T("Professional x64 Edition");
						}
					}
					else
					{
						// Windows Server 2003 ファミリー
						szOS = _T("Windows Server 2003");

						if(FullName != FALSE)
						{
#if _MSC_VER <= 1700
							GokanMode = GetVerifyVersionInfo(6, 0, 0);

							if(GokanMode != FALSE)
								szEdition = _T("互換モード");
							// Compute Cluster Server には R2 も x64 もつかない
							else 
#endif
							if(os.wSuiteMask & VER_SUITE_COMPUTE_SERVER)
								szOS = _T("Compute Cluster Server 2003");
							// Windows Home Server にも R2 はつかない (x64 は無い?)
							else if(os.wSuiteMask & VER_SUITE_WH_SERVER)
								szOS = _T("Windows Home Server");
							else
							{
								// Small Business か Storage か無印か
								if(os.wSuiteMask & (VER_SUITE_SMALLBUSINESS | VER_SUITE_SMALLBUSINESS_RESTRICTED))
									szOS = _T("Small Business Server 2003");
								else if(os.wSuiteMask & VER_SUITE_STORAGE_SERVER)
									szOS = _T("Small Storage Server 2003");

								// R2 か
								if(GetSystemMetrics(SM_SERVERR2))
									R2 = TRUE;

								// Edition
								if(os.wSuiteMask & VER_SUITE_DATACENTER)
									szEdition = _T("Datacenter Edition");
								else if(os.wSuiteMask & VER_SUITE_ENTERPRISE)
									szEdition = _T("Enterprise Edition");
								else if(os.wSuiteMask & VER_SUITE_BLADE)
									szEdition = _T("Web Edition");
								else
									szEdition = _T("Standard Edition");
							}
						}
					}
					break;
				}
				break;
			case 6:
				// Vista / 2008 / 7 / 2008 R2 / 8 / 2012 / 8.1
				if(os.wProductType == VER_NT_WORKSTATION)
				{
					switch(os.dwMinorVersion)
					{
					case 0:
						szOS = _T("Windows Vista");
						break;
					case 1:
						szOS = _T("Windows 7");
						break;
					case 2:
						szOS = _T("Windows 8");
						break;
					case 3:
						szOS = _T("Windows 8.1");
						break;
					case 4:
						szOS = _T("Windows 10 Technical Preview");
						break;
					}
				}
				// Server 2008 / R2
				else
				{
					switch(os.dwMinorVersion)
					{
					case 0:
					case 1:
						szOS = _T("Windows Server 2008");

						if(os.dwMinorVersion == 1)
							R2 = TRUE;
						break;
					case 2:
					case 3:
						szOS = _T("Windows Server 2012");

						if(os.dwMinorVersion == 3)
							R2 = TRUE;
						break;
					}
				}

				if(FullName != FALSE)
				{
#if _MSC_VER <= 1700
					switch(os.dwMinorVersion)
					{
					case 0:
						GokanMode = GetVerifyVersionInfo(6, 1, 0);
						break;
					case 1:
						GokanMode = GetVerifyVersionInfo(6, 2, 0);
						break;
					case 2:
						GokanMode = GetVerifyVersionInfo(6, 3, 0);
						break;
					case 3:
						GokanMode = GetVerifyVersionInfo(6, 4, 0);
						break;
					case 4:
						GokanMode = GetVerifyVersionInfo(10, 0, 0);
						break;
					}

					if(GokanMode != FALSE)
						szEdition = _T("互換モード");
					else
#endif
					{
						DWORD dwType = 0;

#if _MSC_VER < 1800 || defined(_NODLL)
						UINT  nLen;
						TCHAR *szPath;
						const TCHAR *szFile = _T("KERNEL32.DLL");

						nLen = GetSystemDirectory(0, 0);
						if(nLen < 1)
						{
							free(cp);
							return FALSE;
						}

						nLen  += (UINT)_tcslen(szFile) + 1;
						szPath = (TCHAR*)malloc(nLen * sizeof(TCHAR));

						if(szPath == NULL)
						{
							free(cp);
							return FALSE;
						}

						nRet = GetSystemDirectory(szPath, nLen);
						if(nRet > 0)
						{
							TCHAR *p = szPath + nRet;

							*(p++) = _T('\\');
							_tcscpy(p, szFile);

							HMODULE hDll = LoadLibrary(szPath);
							if(hDll != NULL)
							{
								BOOL (WINAPI *GetProductInfo)(DWORD, DWORD, DWORD, DWORD, PDWORD);
								GetProductInfo = reinterpret_cast<BOOL(WINAPI*)(DWORD, DWORD, DWORD, DWORD, PDWORD)>(GetProcAddress(hDll, "GetProductInfo"));

								if(GetProductInfo != NULL)
								{
#endif // _MSC_VER < 1800 || defined(_NODLL)

									nRet = GetProductInfo(os.dwMajorVersion, os.dwMinorVersion, 0, 0, &dwType);

#if _MSC_VER < 1800 || defined(_NODLL)
								}
								FreeLibrary(hDll);
							}
						}
						free(szPath);
#endif // _MSC_VER < 1800 || defined(_NODLL)

						if(nRet != FALSE)
						{
							switch(dwType)
							{
							// Windows Vista / 7 / 8
							case PRODUCT_CORE_ARM:
							case PRODUCT_CORE_N:
							case PRODUCT_CORE:
								break;
							case PRODUCT_CORE_COUNTRYSPECIFIC:
								szEdition = _T("China");
								break;
							case PRODUCT_CORE_SINGLELANGUAGE:
								szEdition = _T("Single Language");
								break;
							case PRODUCT_ULTIMATE:
							case PRODUCT_ULTIMATE_N:
								szEdition = _T("Ultimate");
								break;
							case PRODUCT_HOME_BASIC:
							case PRODUCT_HOME_BASIC_N:
								szEdition = _T("Home Basic");
								break;
							case PRODUCT_HOME_PREMIUM:
							case PRODUCT_HOME_PREMIUM_N:
								szEdition = _T("Home Premium");
								break;
							case PRODUCT_ENTERPRISE:
							case PRODUCT_ENTERPRISE_N:
							case PRODUCT_ENTERPRISE_EVALUATION:
							case PRODUCT_ENTERPRISE_N_EVALUATION:
								szEdition = _T("Enterprise");
								break;
							case PRODUCT_BUSINESS:
							case PRODUCT_BUSINESS_N:
								szEdition = _T("Business");
								break;
							case PRODUCT_STARTER:
							case PRODUCT_STARTER_N:
								szEdition = _T("Starter");
								break;
							case PRODUCT_PROFESSIONAL:
							case PRODUCT_PROFESSIONAL_N:
								switch(os.dwMinorVersion)
								{
								case 0:
								case 1:
									szEdition = _T("Professional");
									break;
								default:
									szEdition = _T("Pro");
								}
								break;
								// Windows Server 2008 / R2
							case PRODUCT_STANDARD_SERVER:
							case PRODUCT_STANDARD_SERVER_CORE:
							case PRODUCT_STANDARD_SERVER_V:
							case PRODUCT_STANDARD_SERVER_CORE_V:
							case PRODUCT_STANDARD_EVALUATION_SERVER:
								szEdition = _T("Standard Edition");
								break;
							case PRODUCT_DATACENTER_SERVER:
							case PRODUCT_DATACENTER_SERVER_CORE:
							case PRODUCT_DATACENTER_SERVER_V:
							case PRODUCT_DATACENTER_SERVER_CORE_V:
							case PRODUCT_DATACENTER_EVALUATION_SERVER:
								szEdition = _T("Datacenter Edition");
								break;
							case PRODUCT_ENTERPRISE_SERVER:
							case PRODUCT_ENTERPRISE_SERVER_CORE:
							case PRODUCT_ENTERPRISE_SERVER_V:
							case PRODUCT_ENTERPRISE_SERVER_CORE_V:
							case PRODUCT_ENTERPRISE_SERVER_IA64:
								szEdition = (dwType == PRODUCT_ENTERPRISE_SERVER_IA64) ? _T("Enterprise Edition for Itanium-based Systems") : _T("Enterprise Edition");
								break;
							case PRODUCT_WEB_SERVER:
							case PRODUCT_WEB_SERVER_CORE:
								switch(os.dwMinorVersion)
								{
								case 0:
								case 1:
									szOS = _T("Windows Web Server 2008");
									break;
								case 2:
									szOS = _T("Windows Web Server 2012");
									break;
								}
								break;
							case PRODUCT_CLUSTER_SERVER: // Cluster Server Edition?
								switch(os.dwMinorVersion)
								{
								case 0:
								case 1:
									szOS = _T("Windows HPC Server 2008");
									break;
								case 2:
									szOS = _T("Windows HPC Server 2012");
									break;
								}
								break;
							case PRODUCT_HOME_SERVER:
							case PRODUCT_HOME_PREMIUM_SERVER: // Premium Edition?
								switch(os.dwMinorVersion)
								{
								case 0:
								case 1:
									szOS = _T("Windows Home Server 2008");
									break;
								case 2:
									szOS = _T("Windows Home Server 2012");
									break;
								}

								if(dwType == PRODUCT_HOME_PREMIUM_SERVER)
									szEdition = _T("Premium Edition");
								break;
							case PRODUCT_SMALLBUSINESS_SERVER:
							case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
								switch(os.dwMinorVersion)
								{
								case 0:
								case 1:
									szOS = _T("Windows Small Business Server 2008");
									break;
								case 2:
									szOS = _T("Windows Small Business Server 2012");
									break;
								}

								if(dwType == PRODUCT_SMALLBUSINESS_SERVER_PREMIUM)
									szEdition = _T("Premium Edition");

								break;
							case PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT:
							case PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY:
							case PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING:
								switch(os.dwMinorVersion)
								{
								case 0:
								case 1:
									szOS = _T("Windows Essential Business Server 2008");
									break;
								case 2:
									szOS = _T("Windows Essential Business Server 2012");
									break;
								}

								switch(dwType)
								{
								case PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT:
									szEdition = _T("Management Server");
									break;
								case PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY:
									szEdition = _T("Security Server");
									break;
								case PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING:
									szEdition = _T("Messaging Server");
									break;
								}
								break;
							case PRODUCT_SERVER_FOUNDATION:
								szEdition = _T("Foundation");
								break;
							case PRODUCT_SERVER_FOR_SMALLBUSINESS:
								szEdition = _T("for Small Business Edition");
								break;
							case PRODUCT_SERVER_FOR_SMALLBUSINESS_V:
								szEdition = _T("for Windows Essential Server Solutions");
								break;
							case PRODUCT_HYPERV:
								switch(os.dwMinorVersion)
								{
								case 0:
								case 1:
									szOS = _T("Hyper-V Server 2008");
									break;
								case 2:
									szOS = _T("Hyper-V Server 2012");
									break;
								}
								break;
							case PRODUCT_STORAGE_EXPRESS_SERVER:
							case PRODUCT_STORAGE_EXPRESS_SERVER_CORE:
							case PRODUCT_STORAGE_STANDARD_SERVER:
							case PRODUCT_STORAGE_STANDARD_SERVER_CORE:
							case PRODUCT_STORAGE_STANDARD_EVALUATION_SERVER:
							case PRODUCT_STORAGE_WORKGROUP_SERVER:
							case PRODUCT_STORAGE_WORKGROUP_SERVER_CORE:
							case PRODUCT_STORAGE_WORKGROUP_EVALUATION_SERVER:
							case PRODUCT_STORAGE_ENTERPRISE_SERVER:
							case PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE:
								switch(os.dwMinorVersion)
								{
								case 0:
								case 1:
									szOS = _T("Windows Storage Server 2008");
									break;
								case 2:
									szOS = _T("Windows Storage Server 2012");
									break;
								}

								switch(dwType)
								{
								case PRODUCT_STORAGE_EXPRESS_SERVER:
								case PRODUCT_STORAGE_EXPRESS_SERVER_CORE:
									szEdition = _T("Express Edition");
									break;
								case PRODUCT_STORAGE_STANDARD_SERVER:
								case PRODUCT_STORAGE_STANDARD_SERVER_CORE:
									szEdition = _T("Standard Edition");
									break;
								case PRODUCT_STORAGE_WORKGROUP_SERVER:
								case PRODUCT_STORAGE_WORKGROUP_SERVER_CORE:
									szEdition = _T("Workgroup Edition");
									break;
								case PRODUCT_STORAGE_ENTERPRISE_SERVER:
								case PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE:
									szEdition = _T("Enterprise Edition");
									break;
								}
								break;
							case PRODUCT_MULTIPOINT_STANDARD_SERVER:
							case PRODUCT_MULTIPOINT_PREMIUM_SERVER:
								szOS = _T("Windows MultiPoint Server");

								switch(dwType)
								{
								case PRODUCT_MULTIPOINT_STANDARD_SERVER:
									szEdition = _T("Standard");
									break;
								case PRODUCT_MULTIPOINT_PREMIUM_SERVER:
									szEdition = _T("Premium");
									break;
								}
								break;
							case PRODUCT_PROFESSIONAL_WMC:
								szEdition = _T("Pro with Media Center");
								break;
							default:
								szEdition = _T("Unknown Edition");
								_stprintf(szText1, _T("Unknown Edition [0x%2X (%d)]"), dwType, dwType);
							}
							// without Evaluation
							switch(dwType)
							{
							case PRODUCT_ENTERPRISE_EVALUATION:
							case PRODUCT_ENTERPRISE_N_EVALUATION:
							case PRODUCT_STANDARD_EVALUATION_SERVER:
							case PRODUCT_DATACENTER_EVALUATION_SERVER:
								szSub = _T("評価版");
								break;
							}
							// without Hyper-V
							switch(dwType)
							{
							case PRODUCT_STANDARD_SERVER_V:
							case PRODUCT_DATACENTER_SERVER_V:
							case PRODUCT_ENTERPRISE_SERVER_V:
							case PRODUCT_STANDARD_SERVER_CORE_V:
							case PRODUCT_DATACENTER_SERVER_CORE_V:
							case PRODUCT_ENTERPRISE_SERVER_CORE_V:
							case PRODUCT_SERVER_FOR_SMALLBUSINESS_V:
								szSub = _T("without Hyper-V");
							}
							// core installation
							switch(dwType)
							{
							case PRODUCT_STANDARD_SERVER_CORE:
							case PRODUCT_DATACENTER_SERVER_CORE:
							case PRODUCT_ENTERPRISE_SERVER_CORE:
							case PRODUCT_WEB_SERVER_CORE:
							case PRODUCT_STANDARD_SERVER_CORE_V:
							case PRODUCT_DATACENTER_SERVER_CORE_V:
							case PRODUCT_ENTERPRISE_SERVER_CORE_V:
							case PRODUCT_STORAGE_EXPRESS_SERVER_CORE:
							case PRODUCT_STORAGE_STANDARD_SERVER_CORE:
							case PRODUCT_STORAGE_WORKGROUP_SERVER_CORE:
							case PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE:
								szSub = _T("(core installation)");
							}
						}
					}
				}
				break;
			case 10:
				// 10
				if(os.wProductType == VER_NT_WORKSTATION)
				{
					switch(os.dwMinorVersion)
					{
					case 0:
						szOS = _T("Windows 10");
						break;
					case 1:
						szOS = _T("Windows 10.1");
						break;
					default:
						szOS = _T("Windows 10.x 系");
					}
				}
				// Server 2015
				else
				{
					switch(os.dwMinorVersion)
					{
					case 0:
					case 1:
						szOS = _T("Windows Server 2015");

						if(os.dwMinorVersion == 1)
							R2 = TRUE;
						break;
					default:
						szOS = _T("Windows Server 系");
					}
				}

				if(FullName != FALSE)
				{
#if _MSC_VER <= 1700
					switch(os.dwMinorVersion)
					{
					case 0:
						GokanMode = GetVerifyVersionInfo(10, 1, 0);
						break;
					case 1:
						GokanMode = GetVerifyVersionInfo(10, 2, 0);
						break;
					}

					if(GokanMode != FALSE)
						szEdition = _T("互換モード");
					else
#endif
					{
						DWORD dwType = 0;

#if _MSC_VER < 1800 || defined(_NODLL)
						UINT  nLen;
						TCHAR *szPath;
						const TCHAR *szFile = _T("KERNEL32.DLL");

						nLen = GetSystemDirectory(0, 0);
						if(nLen < 1)
						{
							free(cp);
							return FALSE;
						}

						nLen += (UINT)_tcslen(szFile) + 1;
						szPath = (TCHAR*)malloc(nLen * sizeof(TCHAR));

						if(szPath == NULL)
						{
							free(cp);
							return FALSE;
						}

						nRet = GetSystemDirectory(szPath, nLen);
						if(nRet > 0)
						{
							TCHAR *p = szPath + nRet;

							*(p++) = _T('\\');
							_tcscpy(p, szFile);

							HMODULE hDll = LoadLibrary(szPath);
							if(hDll != NULL)
							{
								BOOL(WINAPI *GetProductInfo)(DWORD, DWORD, DWORD, DWORD, PDWORD);
								GetProductInfo = reinterpret_cast<BOOL(WINAPI*)(DWORD, DWORD, DWORD, DWORD, PDWORD)>(GetProcAddress(hDll, "GetProductInfo"));

								if(GetProductInfo != NULL)
								{
#endif // _MSC_VER < 1800 || defined(_NODLL)

									nRet = GetProductInfo(os.dwMajorVersion, os.dwMinorVersion, 0, 0, &dwType);

#if _MSC_VER < 1800 || defined(_NODLL)
								}
								FreeLibrary(hDll);
							}
						}
						free(szPath);
#endif // _MSC_VER < 1800 || defined(_NODLL)

						if(nRet != FALSE)
						{
							switch(dwType)
							{
							// Windows 10
							case PRODUCT_CORE_ARM:
							case PRODUCT_CORE_N:
							case PRODUCT_CORE:
								break;
							case PRODUCT_CORE_COUNTRYSPECIFIC:
								szEdition = _T("China");
								break;
							case PRODUCT_CORE_SINGLELANGUAGE:
								szEdition = _T("Single Language");
								break;
							case PRODUCT_ULTIMATE:
							case PRODUCT_ULTIMATE_N:
								szEdition = _T("Ultimate");
								break;
							case PRODUCT_HOME_BASIC:
							case PRODUCT_HOME_BASIC_N:
								szEdition = _T("Home Basic");
								break;
							case PRODUCT_HOME_PREMIUM:
							case PRODUCT_HOME_PREMIUM_N:
								szEdition = _T("Home Premium");
								break;
							case PRODUCT_ENTERPRISE:
							case PRODUCT_ENTERPRISE_N:
							case PRODUCT_ENTERPRISE_EVALUATION:
							case PRODUCT_ENTERPRISE_N_EVALUATION:
								szEdition = _T("Enterprise");
								break;
							case PRODUCT_BUSINESS:
							case PRODUCT_BUSINESS_N:
								szEdition = _T("Business");
								break;
							case PRODUCT_STARTER:
							case PRODUCT_STARTER_N:
								szEdition = _T("Starter");
								break;
							case PRODUCT_PROFESSIONAL:
							case PRODUCT_PROFESSIONAL_N:
								szEdition = _T("Pro");
								break;
								// Windows Server 2015
							case PRODUCT_STANDARD_SERVER:
							case PRODUCT_STANDARD_SERVER_CORE:
							case PRODUCT_STANDARD_SERVER_V:
							case PRODUCT_STANDARD_SERVER_CORE_V:
							case PRODUCT_STANDARD_EVALUATION_SERVER:
								szEdition = _T("Standard Edition");
								break;
							case PRODUCT_DATACENTER_SERVER:
							case PRODUCT_DATACENTER_SERVER_CORE:
							case PRODUCT_DATACENTER_SERVER_V:
							case PRODUCT_DATACENTER_SERVER_CORE_V:
							case PRODUCT_DATACENTER_EVALUATION_SERVER:
								szEdition = _T("Datacenter Edition");
								break;
							case PRODUCT_ENTERPRISE_SERVER:
							case PRODUCT_ENTERPRISE_SERVER_CORE:
							case PRODUCT_ENTERPRISE_SERVER_V:
							case PRODUCT_ENTERPRISE_SERVER_CORE_V:
							case PRODUCT_ENTERPRISE_SERVER_IA64:
								szEdition = (dwType == PRODUCT_ENTERPRISE_SERVER_IA64) ? _T("Enterprise Edition for Itanium-based Systems") : _T("Enterprise Edition");
								break;
							case PRODUCT_WEB_SERVER:
							case PRODUCT_WEB_SERVER_CORE:
								szOS = _T("Windows Web Server 2015");
								break;
							case PRODUCT_CLUSTER_SERVER: // Cluster Server Edition?
								szOS = _T("Windows HPC Server 2015");
								break;
							case PRODUCT_HOME_SERVER:
							case PRODUCT_HOME_PREMIUM_SERVER: // Premium Edition?
								szOS = _T("Windows Home Server 2015");

								if(dwType == PRODUCT_HOME_PREMIUM_SERVER)
									szEdition = _T("Premium Edition");
								break;
							case PRODUCT_SMALLBUSINESS_SERVER:
							case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
								szOS = _T("Windows Small Business Server 2015");

								if(dwType == PRODUCT_SMALLBUSINESS_SERVER_PREMIUM)
									szEdition = _T("Premium Edition");
								break;
							case PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT:
							case PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY:
							case PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING:
								szOS = _T("Windows Essential Business Server 2015");

								switch(dwType)
								{
								case PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT:
									szEdition = _T("Management Server");
									break;
								case PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY:
									szEdition = _T("Security Server");
									break;
								case PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING:
									szEdition = _T("Messaging Server");
									break;
								}
								break;
							case PRODUCT_SERVER_FOUNDATION:
								szEdition = _T("Foundation");
								break;
							case PRODUCT_SERVER_FOR_SMALLBUSINESS:
								szEdition = _T("for Small Business Edition");
								break;
							case PRODUCT_SERVER_FOR_SMALLBUSINESS_V:
								szEdition = _T("for Windows Essential Server Solutions");
								break;
							case PRODUCT_HYPERV:
								szOS = _T("Hyper-V Server 2015");
								break;
							case PRODUCT_STORAGE_EXPRESS_SERVER:
							case PRODUCT_STORAGE_EXPRESS_SERVER_CORE:
							case PRODUCT_STORAGE_STANDARD_SERVER:
							case PRODUCT_STORAGE_STANDARD_SERVER_CORE:
							case PRODUCT_STORAGE_STANDARD_EVALUATION_SERVER:
							case PRODUCT_STORAGE_WORKGROUP_SERVER:
							case PRODUCT_STORAGE_WORKGROUP_SERVER_CORE:
							case PRODUCT_STORAGE_WORKGROUP_EVALUATION_SERVER:
							case PRODUCT_STORAGE_ENTERPRISE_SERVER:
							case PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE:
								szOS = _T("Windows Storage Server 2015");

								switch(dwType)
								{
								case PRODUCT_STORAGE_EXPRESS_SERVER:
								case PRODUCT_STORAGE_EXPRESS_SERVER_CORE:
									szEdition = _T("Express Edition");
									break;
								case PRODUCT_STORAGE_STANDARD_SERVER:
								case PRODUCT_STORAGE_STANDARD_SERVER_CORE:
									szEdition = _T("Standard Edition");
									break;
								case PRODUCT_STORAGE_WORKGROUP_SERVER:
								case PRODUCT_STORAGE_WORKGROUP_SERVER_CORE:
									szEdition = _T("Workgroup Edition");
									break;
								case PRODUCT_STORAGE_ENTERPRISE_SERVER:
								case PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE:
									szEdition = _T("Enterprise Edition");
									break;
								}
								break;
							case PRODUCT_MULTIPOINT_STANDARD_SERVER:
							case PRODUCT_MULTIPOINT_PREMIUM_SERVER:
								szOS = _T("Windows MultiPoint Server");

								switch(dwType)
								{
								case PRODUCT_MULTIPOINT_STANDARD_SERVER:
									szEdition = _T("Standard");
									break;
								case PRODUCT_MULTIPOINT_PREMIUM_SERVER:
									szEdition = _T("Premium");
									break;
								}
								break;
							case PRODUCT_PROFESSIONAL_WMC:
								szEdition = _T("Pro with Media Center");
								break;
							default:
								szEdition = _T("Unknown Edition");
								_stprintf(szText1, _T("Unknown Edition [0x%2X (%d)]"), dwType, dwType);
							}
							// without Evaluation
							switch(dwType)
							{
							case PRODUCT_ENTERPRISE_EVALUATION:
							case PRODUCT_ENTERPRISE_N_EVALUATION:
							case PRODUCT_STANDARD_EVALUATION_SERVER:
							case PRODUCT_DATACENTER_EVALUATION_SERVER:
								szSub = _T("評価版");
								break;
							}
							// without Hyper-V
							switch(dwType)
							{
							case PRODUCT_STANDARD_SERVER_V:
							case PRODUCT_DATACENTER_SERVER_V:
							case PRODUCT_ENTERPRISE_SERVER_V:
							case PRODUCT_STANDARD_SERVER_CORE_V:
							case PRODUCT_DATACENTER_SERVER_CORE_V:
							case PRODUCT_ENTERPRISE_SERVER_CORE_V:
							case PRODUCT_SERVER_FOR_SMALLBUSINESS_V:
								szSub = _T("without Hyper-V");
							}
							// core installation
							switch(dwType)
							{
							case PRODUCT_STANDARD_SERVER_CORE:
							case PRODUCT_DATACENTER_SERVER_CORE:
							case PRODUCT_ENTERPRISE_SERVER_CORE:
							case PRODUCT_WEB_SERVER_CORE:
							case PRODUCT_STANDARD_SERVER_CORE_V:
							case PRODUCT_DATACENTER_SERVER_CORE_V:
							case PRODUCT_ENTERPRISE_SERVER_CORE_V:
							case PRODUCT_STORAGE_EXPRESS_SERVER_CORE:
							case PRODUCT_STORAGE_STANDARD_SERVER_CORE:
							case PRODUCT_STORAGE_WORKGROUP_SERVER_CORE:
							case PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE:
								szSub = _T("(core installation)");
							}
						}
					}
				}
				break;
			}
			break;
		}

		if(*szOS == '\0')
		{
			p1 = os_qtcscpy(cp, _T("Windows NT"));

			if(os.dwMajorVersion >= 3)
			{
				TCHAR szVal[33];

				*(p1++) = ' ';
				_itot(os.dwMajorVersion, szVal, 10);
				p1 = os_qtcscpy(p1, szVal);
				*(p1++) = '.';
				_itot(os.dwMinorVersion, szVal, 10);
				p1 = os_qtcscpy(p1, szVal);
			}
		}
		else
			p1 = os_qtcscpy(cp, szOS);


		if(FullName != FALSE)
		{
			if(R2 != FALSE)
			{
				*(p1++) = ' ';
				p1 = os_qtcscpy(p1, _T("R2"));
			}

			if(*szEdition != '\0')
			{
				*(p1++) = ' ';

				if(_tcscmp(szEdition, _T("Unknown Edition")))
					p1 = os_qtcscpy(p1, szEdition);
				else
					p1 = os_qtcscpy(p1, szText1);
			}

			if(*szSub != '\0')
			{
				*(p1++) = ' ';
				p1 = os_qtcscpy(p1, szSub);
			}


			if(!NT40EX && !GokanMode && os.wServicePackMajor > 0)
			{
				TCHAR szVal[33];

				*(p1++) = ' ';
				*(p1++) = 'S';
				*(p1++) = 'P';

				_itot((int)os.wServicePackMajor, szVal, 10);
				p1 = os_qtcscpy(p1, szVal);
			}
#if _MSC_VER < 1500
			// VC++2008 から Windows NT 4.0 のコード省略
			else if(NT40EX)
			{
				*(p1++) = ' ';
				p1 = os_qtcscpy(p1, _T("SP6"));
			}
#endif
		}

		nRet = (UINT)_tcslen(dst) + 1;
		cp   = dst;
		if(lpText != 0 && size >= nRet)
			_tcsncpy(lpText, cp, size - 1);

		free(cp);

		return nRet;
	}

	return 0;
	*/
}

BOOL GetUserAgentName(TCHAR* lpText, size_t size)
{
	UNREFERENCED_PARAMETER(size);

	TCHAR szBuf[33];
	TCHAR *p1;
	HKEY hKey;
	LONG lRet;
	DWORD dwType = REG_SZ;
	DWORD dwByte = 33;

#if _MSC_VER < 1600
	if(GetVerifyVersionInfo(5, 0, 0))
	{
#endif

		lRet = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,
			_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"),
			0,
			KEY_QUERY_VALUE,
			&hKey);

		if(lRet != ERROR_SUCCESS)
			return FALSE;

		lRet = RegQueryValueEx(
			hKey,
			_T("CurrentVersion"),
			0,
			&dwType,
			(LPBYTE)szBuf,
			&dwByte);
		RegCloseKey(hKey);

		if(lRet != ERROR_SUCCESS)
			return FALSE;

		p1 = os_qtcscpy(lpText, _T("Windows NT"));
		*p1++ = _T(' ');
		os_qtcscpy(p1, szBuf);

#if _MSC_VER < 1600
	}
	else
	{
		lRet = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,
			_T("Software\\Microsoft\\Windows\\CurrentVersion"),
			0,
			KEY_QUERY_VALUE,
			&hKey);

		if(lRet != ERROR_SUCCESS)
			return FALSE;


		lRet = RegQueryValueEx(
			hKey,
			_T("Version"),
			0,
			&dwType,
			(LPBYTE)szBuf,
			&dwByte);
		RegCloseKey(hKey);

		if(lRet != ERROR_SUCCESS)
			return FALSE;

		os_qtcscpy(lpText, szBuf);
	}
#endif
	return TRUE;
}

BOOL isUxTheme()
{
#if _MSC_VER < 1600 || defined(_NODLL)
	if(GetVerifyVersionInfo(5, 1, 0))
	{
#endif /* _MSC_VER < 1600 || defined(_NODLL) */
		UINT  nRet = 0;
#if _MSC_VER < 1600 || defined(_NODLL)
		UINT  nLen;
		TCHAR *szPath;
		const TCHAR *szFile = _T("UXTHEME.DLL");
		const char  *szFunc = "IsAppThemed";

		nLen = GetSystemDirectory(0, 0);

		if(nLen < 1)
			goto FOUL;

		nLen  += (UINT)_tcslen(szFile) + 1;
		szPath = (TCHAR*)malloc(nLen * sizeof(TCHAR));
		if(szPath == NULL)
			goto FOUL;

		nRet = GetSystemDirectory(szPath, nLen);

		if(nRet > 0)
		{
			HMODULE hDll;
			TCHAR *p = szPath + nRet;

			*(p++) = _T('\\');
			_tcscpy(p, szFile);

			hDll = LoadLibrary(szPath);
			if(hDll != NULL)
			{
				BOOL (WINAPI *IsAppThemed)();
				IsAppThemed = reinterpret_cast<BOOL(WINAPI*)()>(GetProcAddress(hDll, szFunc));

				if(IsAppThemed != NULL)
				{
#endif /* _MSC_VER < 1600 || defined(_NODLL) */
					if(IsAppThemed())
						nRet = 1;
#if _MSC_VER < 1600 || defined(_NODLL)
				}
				FreeLibrary(hDll);
			}
		}
		free(szPath);
#endif /* _MSC_VER < 1600 || defined(_NODLL) */
		return (BOOL)nRet;
#if _MSC_VER < 1600 || defined(_NODLL)
	}
FOUL:
	return FALSE;
#endif /* _MSC_VER < 1600 || defined(_NODLL) */
}

BOOL GetAearoWindowRect(HWND hWnd, RECT *lpRect)
{
	if(GetVerifyVersionInfo(6, 0, 0))
	{
		UINT  nRet;
		UINT  nLen;
		TCHAR *szPath;
		const TCHAR *szFile = _T("DWMAPI.DLL");
		const char  *szFunc = "DwmGetWindowAttribute";

		nLen = GetSystemDirectory(0, 0);
		if(nLen < 1)
		{
			return FALSE;
		}

		nLen  += (UINT)_tcslen(szFile) + 1;
		szPath = (TCHAR*)malloc(nLen * sizeof(TCHAR));

		if(szPath == NULL)
		{
			goto FOUL;
		}

		nRet = GetSystemDirectory(szPath, nLen);
		if(nRet > 0)
		{
			HMODULE hDll;
			TCHAR *p;

			p = szPath + nRet;
			*(p++) = _T('\\');
			_tcscpy(p, szFile);

			hDll = LoadLibrary(szPath);
			if(hDll != NULL)
			{
				HRESULT (WINAPI *DwmGetWindowAttribute)(HWND, DWORD, PVOID, DWORD);
				DwmGetWindowAttribute = reinterpret_cast<HRESULT(WINAPI*)(HWND, DWORD, PVOID, DWORD)>(GetProcAddress(hDll, szFunc));

				if(DwmGetWindowAttribute != NULL)
				{
					nRet = DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, lpRect, sizeof(RECT));
					nRet = SUCCEEDED(nRet) ? TRUE : FALSE;
				}
				FreeLibrary(hDll);
			}
			else
			{
				nRet = FALSE;
			}
		}
		free(szPath);
		return nRet;
	}
	GetWindowRect(hWnd, lpRect);

FOUL:
	return FALSE;
}

TCHAR *os_qtcscpy(TCHAR *dst, const TCHAR *src)
{
	while(*src)
		*dst++ = *src++;

	*dst = '\0';
	return dst;
}
