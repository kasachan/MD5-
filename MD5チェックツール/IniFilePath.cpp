// IniFilePath.cpp
//

#include "IniFilePath.h"
#if _MSC_VER < 1500
#include "../../lib/os.h"
#endif

// このコード モジュールに含まれる関数の宣言を転送します:
TCHAR *_qtcscpy(TCHAR *dst, const TCHAR *src);


VOID GetIniFilePath(TCHAR *inPath, const TCHAR *inINIFileName, const TCHAR *inAppName)
{
	TCHAR szFilePath[MAX_PATH_SIZE + 100];
	UINT nRet = 0;

	*szFilePath = '\0';

#if _MSC_VER < 1500
	if(GetVerifyVersionInfo(5, 0, 0))
	{
		HMODULE hDll;
		UINT  nLen;
		TCHAR *szPath;
		const TCHAR *szFile = _T("SHELL32.DLL");
#ifdef _UNICODE
		const char  *szFunc = "SHGetFolderPathW";
#else
		const char  *szFunc = "SHGetFolderPathA";
#endif

		nLen = GetSystemDirectory(0, 0);

		if(nLen < 1)
			goto FOUL;

		nLen  += (UINT)_tcslen(szFile) + 1;
		szPath = (TCHAR*)malloc(nLen * sizeof(TCHAR));

		if(szPath == NULL)
			goto FOUL;

		nRet = GetSystemDirectory(szPath, nLen);

		if(nRet < 0)
			goto FOUL;

		TCHAR *p = szPath + nRet;
		*(p++) = _T('\\');
		_tcscpy(p, szFile);

		hDll = LoadLibrary(szPath);
		if(hDll != NULL)
		{
			HRESULT (WINAPI *SHGetFolderPath)(HWND, int, HANDLE, DWORD, LPTSTR);
			SHGetFolderPath = reinterpret_cast<HRESULT(WINAPI*)(HWND, int, HANDLE, DWORD, LPTSTR)>(GetProcAddress(hDll, szFunc));

			if(SHGetFolderPath != NULL)
			{
#endif /* _MSC_VER < 1500 */
				HRESULT hRet;

				if(GetModuleFileName(NULL, inPath, MAX_PATH_SIZE + 100) != NULL)
				{
					hRet = SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 0, szFilePath);

					if (SUCCEEDED(hRet))
					{
						TCHAR *p;

						//  Program File から実行されているか確認。
						p = _tcsstr(inPath, szFilePath);

						//  実行されていたら Appdata のパスにする。
						if(p != NULL)
						{
							hRet = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szFilePath);

							if(SUCCEEDED(hRet))
							{
								// inPath に Appdata のパスをコピーする。
								p = _qtcscpy(inPath, szFilePath);
								*(p++) = _T('\\');

								// inPath に inAppName のパスを追記する。
								p = _qtcscpy(p, inAppName);
								*(p++) = _T('\\');
								*p = _T('\0');

								// inAppName のフォルダーがない場合は作成する。
								if(!PathIsDirectory(inPath))
								{
									CreateDirectory(inPath, NULL);
								}

								// 最後に inINIFileName を追記してINIファイルのパスが完成。
								_tcscpy(p, inINIFileName);

								nRet = 1;
							}
						}
#if _MSC_VER < 1500
						else
						{
							nRet = 0;
						}
#endif /* _MSC_VER < 1500 */
					}
				}

#if _MSC_VER < 1500
			}
			FreeLibrary(hDll);
		}
FOUL:
		if(szPath)
			free(szPath);

	}
#endif /* _MSC_VER < 1500 */

	if(!nRet)
	{
		TCHAR *p;

		if(GetModuleFileName(NULL, szFilePath, MAX_PATH_SIZE + 100) != NULL)
		{
			PathRemoveFileSpec(szFilePath);
			p = _qtcscpy(inPath, szFilePath);
			*(p++) = _T('\\');
			_tcscpy(p, inINIFileName);
		}
	}

	return;
}

TCHAR *_qtcscpy(TCHAR *dst, const TCHAR *src)
{
	while(*src)
		*dst++ = *src++;

	*dst = '\0';
	return dst;
}
