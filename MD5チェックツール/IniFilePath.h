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

#ifndef __INIFILEPATH_H__
#define __INIFILEPATH_H__

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <tchar.h>

#ifndef MAX_PATH_SIZE
#ifdef _UNICODE
#define MAX_PATH_SIZE	_MAX_PATH
#else
#define MAX_PATH_SIZE	(_MAX_PATH * 2)
#endif
#endif


#ifdef __cplusplus
extern "C" {
#endif

// このコード モジュールに含まれる関数の宣言を転送します:
VOID GetIniFilePath(TCHAR *inPath, const TCHAR *inINIFileName, const TCHAR *inAppName);

#ifdef __cplusplus
}
#endif
#endif /* __INIFILEPATH_H__ */
