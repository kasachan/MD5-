// os.h

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __OS_H__
#define __OS_H__

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
#include <dwmapi.h>
#include <stdio.h>
#include <tchar.h>
#if _MSC_VER >= 1600
#include <Uxtheme.h>
#endif

// <Uxtheme.h> 1139çs
#ifndef ETDT_ENABLETAB
#define ETDT_DISABLE                    0x00000001
#define ETDT_ENABLE                     0x00000002
#define ETDT_USETABTEXTURE              0x00000004
#define ETDT_ENABLETAB                  (ETDT_ENABLE | ETDT_USETABTEXTURE)
#endif

#define _OS_FULL_SIZE 85

#ifdef  __cplusplus
extern "C" {
#endif

VOID SetEnableThemeDialogTexture(HWND hWnd);
BOOL GetVerifyVersionInfo(const DWORD dwMajor, const DWORD dwMinor, const DWORD dwSPMajor);
BOOL GetGokanMode(OSVERSIONINFOEX *osVerInfo);
BOOL GetUserAgentName(TCHAR* lpText, size_t size);
BOOL isWin9X();
BOOL isUxTheme();
UINT GetOSVersion(OSVERSIONINFOEX *osVerInfo);
UINT GetOSName(TCHAR *lpText, size_t size, BOOL FullName, BOOL VerifyVersion);
BOOL GetAearoWindowRect(HWND hWnd, RECT *lpRect);

#ifdef  __cplusplus
}
#endif
#endif
