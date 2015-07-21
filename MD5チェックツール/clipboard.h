// clipboard.h

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __CLIPBOARD_H__
#define __CLIPBOARD_H__

#if _MSC_VER >= 1400
#define _CRT_SECURE_NO_DEPRECATE
#ifndef _NODLL
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 0
#endif
#endif

#if _MSC_VER >= 1500
#define _BIND_TO_CURRENT_VCLIBS_VERSION 1
#endif

#include <windows.h>

#ifdef  __cplusplus
extern "C" {
#endif
	BOOL  SetClipboardTextA(HWND hWnd, const char* lpText);
	BOOL  SetClipboardTextW(HWND hWnd, const WCHAR* lpText);
	DWORD GetClipboardTextA(HWND hWnd, char* lpText, size_t size);
	DWORD GetClipboardTextW(HWND hWnd, WCHAR* lpText, size_t size);
#ifdef  __cplusplus
}
#endif

#ifdef UNICODE
#define GetClipboardText  GetClipboardTextW
#define SetClipboardText  SetClipboardTextW
#else
#define GetClipboardText  GetClipboardTextA
#define SetClipboardText  SetClipboardTextA
#endif // !UNICODE

#endif
