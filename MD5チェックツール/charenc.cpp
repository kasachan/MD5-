// charenc.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "charenc.h"



int ShiftJisToUnicode(wchar_t *dst, char *src)
{
	int nsize = MultiByteToWideChar(932, 0, src, -1, NULL, 0);
	return MultiByteToWideChar(932, 0, src, -1, dst, nsize + 1);
}

int UTF8ToUnicode(wchar_t *dst, char *src)
{
	int nsize = MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);
	return MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, nsize + 1);
}

int UnicodeToShiftJis(char *dst, wchar_t *src)
{
	int nsize = WideCharToMultiByte(932, WC_NO_BEST_FIT_CHARS, src, -1, NULL, 0, NULL, NULL);
	return WideCharToMultiByte(932, WC_NO_BEST_FIT_CHARS, src, -1, dst, nsize + 1, NULL, NULL);
}

int UnicodeToUTF8(char *dst, wchar_t *src)
{
	int nsize = WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0, NULL, NULL);
	return WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, nsize + 1, NULL, NULL);
}

int ShiftJisToUTF8(char *dst, char *src)
{
	int nRet = ShiftJisToUnicode((wchar_t*)dst, src);
	if(nRet > 0)
	{
		wcscpy((wchar_t*)src, (wchar_t*)dst);
		nRet = UnicodeToUTF8(dst, (wchar_t*)src);
	}
	return nRet;
}

int UTF8ToShiftJis(char *dst, char *src)
{
	int nRet = UTF8ToUnicode((wchar_t*)dst, src);
	if(nRet > 0)
	{
		wcscpy((wchar_t*)src, (wchar_t*)dst);
		nRet = UnicodeToShiftJis(dst, (wchar_t*)src);
	}
	return nRet;
}
