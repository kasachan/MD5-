// ToolTip.c : �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "ToolTip.h"


VOID SetToolTip(const HWND hTool, const HWND hWnd, const TCHAR *lpString)
{
	TOOLINFO ti;

	ZeroMemory(&ti, sizeof(TOOLINFO));
	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd   = GetParent(hWnd);
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	ti.uId    = (UINT_PTR)hWnd;

	ti.lpszText = (LPTSTR)lpString;
	SendMessage(hTool, TTM_ADDTOOL, 0, (LPARAM)&ti);
}

VOID SetToolTipText(const HWND hTool, const HWND hWnd, const TCHAR *lpString)
{
	TOOLINFO ti;

	ZeroMemory(&ti, sizeof(TOOLINFO));
	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd   = GetParent(hWnd);
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	ti.uId    = (UINT_PTR)hWnd;

	ti.lpszText = (LPTSTR)lpString;
	SendMessage(hTool, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
}
