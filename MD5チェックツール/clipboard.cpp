// clipboard.cpp

#include "clipboard.h"

BOOL SetClipboardTextA(HWND hWnd, const char *lpText)
{
	if(OpenClipboard(hWnd))
	{
		size_t  szlen = (strlen(lpText) + 1) * sizeof(char);
		HGLOBAL hg    = GlobalAlloc(GHND, szlen);
		if(hg != NULL)
		{
			char *pText = (char*)GlobalLock(hg);
			if(pText != NULL)
			{
				strcpy(pText, lpText);
				GlobalUnlock(hg);
				EmptyClipboard();
				SetClipboardData(CF_TEXT, hg);
				CloseClipboard();
				return TRUE;
			}
			GlobalUnlock(hg);
		}
		CloseClipboard();
		//MessageBoxA(hWnd, "メモリの確保に失敗しました。", "Clipboardクラス", MB_OK | MB_ICONSTOP | MB_ICONERROR | MB_ICONHAND);
	}
	return FALSE;
}

BOOL SetClipboardTextW(HWND hWnd, const wchar_t *lpText)
{
	if(OpenClipboard(hWnd))
	{
		size_t  szlen = (wcslen(lpText) + 1) * sizeof(wchar_t);
		HGLOBAL hg    = GlobalAlloc(GHND, szlen);
		if(hg != NULL)
		{
			WCHAR *pText = (WCHAR*)GlobalLock(hg);
			if(pText != NULL)
			{
				wcscpy(pText, lpText);
				GlobalUnlock(hg);
				EmptyClipboard();
				SetClipboardData(CF_UNICODETEXT, hg);
				CloseClipboard();
				return TRUE;
			}
			GlobalUnlock(hg);
		}
		CloseClipboard();
		//MessageBoxW(hWnd, L"メモリの確保に失敗しました。", L"Clipboardクラス", MB_OK | MB_ICONSTOP | MB_ICONERROR | MB_ICONHAND);
	}
	return FALSE;
}

DWORD GetClipboardTextA(HWND hWnd, char *lpText, size_t size)
{
	if(OpenClipboard(hWnd))
	{
		if(IsClipboardFormatAvailable(CF_TEXT))
		{
			HANDLE hMem = GetClipboardData(CF_TEXT);
			if(hMem != NULL)
			{
				int ret;
				char *pText = (char*)GlobalLock(hMem);

				if(pText != NULL)
				{
					ret = (int)strlen(pText);
					if(size > 0)
					{
						strncpy(lpText, pText, size);
						lpText[size-1] = '\0';
					}
					GlobalUnlock(hMem);
					CloseClipboard();
					return ret;
				}
				GlobalUnlock(hMem);
			}
		}
		CloseClipboard();
	}
	return FALSE;
}

DWORD GetClipboardTextW(HWND hWnd, wchar_t *lpText, size_t size)
{
	if(OpenClipboard(hWnd))
	{
		if(IsClipboardFormatAvailable(CF_UNICODETEXT))
		{
			HANDLE hMem = GetClipboardData(CF_UNICODETEXT);
			if(hMem != NULL)
			{
				int ret;
				WCHAR *pText = (WCHAR*)GlobalLock(hMem);

				if(pText != NULL)
				{
					ret = (int)wcslen(pText);
					if(size > 0)
					{
						wcsncpy(lpText, pText, size);
						lpText[size-1] = L'\0';
					}
					GlobalUnlock(hMem);
					CloseClipboard();
					return ret;
				}
				GlobalUnlock(hMem);
			}
		}
		CloseClipboard();
	}
	return FALSE;
}
