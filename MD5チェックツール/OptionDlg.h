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

#ifndef __OPTIONDLG_H__
#define __OPTIONDLG_H__

#include "FileListDlg.h"
#include "AboutDlg.h"
#include "hashthread.h"
#include "Main.h"
#include "resource.h"

typedef struct tagOptionWindow {
	DWORD		dwStructSize; // バージョンアップ用に予約
	HWND		hWnd;
	HWND		hEdit[1];
	HWND		hButton[1];
	HWND		hCheckbox[7];
	HWND		hGroup[1];
	TCHAR		*szBuf;
	TCHAR		*szChackSumFile;
} OptionWindow, *lpOptionWindow;

// このコード モジュールに含まれる関数の宣言を転送します:
INT_PTR CALLBACK OptionDlg_WndProc(HWND, UINT, WPARAM, LPARAM);
VOID OptionBox(HWND hWnd);
VOID GetIniFileSetting(const TCHAR *inFilePath);
VOID SetIniFileSetting(const TCHAR *inFilePath);
VOID GetChackSumFile(TCHAR *inBuf);
VOID SetChackSumFile(VOID);
#endif /* __OPTIONDLG_H__ */
