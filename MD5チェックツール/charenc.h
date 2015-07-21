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

#ifndef __CHARENC_H__
#define __CHARENCD_H__

#include <windows.h>


int ShiftJisToUnicode(wchar_t *dst, char *src);
int UTF8ToUnicode(wchar_t *dst, char *src);

int UnicodeToShiftJis(char *dst, wchar_t *src);
int UnicodeToUTF8(char *dst, wchar_t *src);

int ShiftJisToUTF8(char *dst, char *src);
int UTF8ToShiftJis(char *dst, char *src);
#endif
