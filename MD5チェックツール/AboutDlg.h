#pragma once
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

#ifndef __ABOUTDLG_H__
#define __ABOUTDLG_H__

#include "Main.h"
#include "resource.h"


typedef struct tagAboutWindow {
	HWND hWnd;

	HWND hStatic[6];
} AboutWindow, *lpAboutWindow;

// このコード モジュールに含まれる関数の宣言を転送します:
INT_PTR CALLBACK	AboutDlg_WndProc(HWND, UINT, WPARAM, LPARAM);

VOID	AboutDlg_AboutBox(HWND hWnd);
VOID	AboutDlg_SetOSText(HWND hWnd, BOOL VerifyVersion);
#endif /* __ABOUTDLG_H__ */
