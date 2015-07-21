// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

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

#define _CRTDBG_MAP_ALLOC

// 下で指定された定義の前に対象プラットフォームを指定しなければならない場合、以下の定義を変更してください。
// 異なるプラットフォームに対応する値に関する最新情報については、MSDN を参照してください。
#ifndef WINVER				// Windows XP 以降のバージョンに固有の機能の使用を許可します。
#define WINVER 0x0500		// これを Windows の他のバージョン向けに適切な値に変更してください。
#endif

#ifndef _WIN32_WINNT		// Windows XP 以降のバージョンに固有の機能の使用を許可します。                   
#define _WIN32_WINNT 0x0500	// これを Windows の他のバージョン向けに適切な値に変更してください。
#endif						

#ifndef _WIN32_WINDOWS		// Windows 98 以降のバージョンに固有の機能の使用を許可します。
#define _WIN32_WINDOWS 0x0410 // これを Windows Me またはそれ以降のバージョン向けに適切な値に変更してください。
#endif

#ifndef _WIN32_IE			// IE 6.0 以降のバージョンに固有の機能の使用を許可します。
#define _WIN32_IE 0x0500	// これを IE. の他のバージョン向けに適切な値に変更してください。
#endif

#define WIN32_LEAN_AND_MEAN		// Windows ヘッダーから使用されていない部分を除外します。
// Windows ヘッダー ファイル:
#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <dwmapi.h>
#include <shobjidl.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <shlobj.h>
#include <commdlg.h>
#include <wincrypt.h>
#include <mmsystem.h>

// C ランタイム ヘッダー ファイル
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <stdio.h>
#include <locale.h>
#include <process.h>
#include <crtdbg.h>
#include <errno.h>

// 追加の ヘッダー ファイル
#include "../../lib/os.h"
#include "../../lib/clipboard.h"

// 追加の LIB ファイル
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "version.lib")
#if _MSC_VER >= 1600
#pragma comment(lib, "uxtheme.lib")
#endif

// _WIN32_WINNT > 0x0500 の定義
//
// <winuser.h>
// #if(_WIN32_WINNT >= 0x0501)
#define SM_TABLETPC             86
#define SM_MEDIACENTER          87
#define SM_STARTER              88
#define SM_SERVERR2             89
// #endif /* _WIN32_WINNT >= 0x0501 */

// <CommCtrl.h>
// #if(_WIN32_WINNT >= 0x0501)
#define PBS_MARQUEE             0x08
#define PBM_SETMARQUEE          (WM_USER+10)
//#endif      // _WIN32_WINNT >= 0x0501


// TODO: プログラムに必要な追加ヘッダーをここで参照してください。

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
