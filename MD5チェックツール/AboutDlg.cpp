// AboutDlg.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "AboutDlg.h"

// グローバル変数の定義します:
AboutWindow AboutWindow1;

// 外部ファイルの変数定義します。
extern MainWindow	MainWindow1;
extern DWORD		dwAppFrag;

// バージョン情報ボックスのメッセージ ハンドラです。
INT_PTR CALLBACK AboutDlg_WndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	int wmId, wmEvent, i;
	
	switch(message)
	{
	case WM_INITDIALOG:
		dwAppFrag |= APP_ABOUTBOX | APP_WINDOW_NOALPHA;

		SetEnableThemeDialogTexture(hDlg);
		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)MainWindow1.hIcon);

		for(i = 0; i < sizeof(AboutWindow1.hStatic) / sizeof(HWND); i++)
		{
			AboutWindow1.hStatic[i] = GetDlgItem(hDlg, IDC_ABOUT_STATIC1 + i);
			SendMessage(AboutWindow1.hStatic[i], WM_SETFONT, (WPARAM)MainWindow1.hFont[0], 0);
		}
		SendMessage(GetDlgItem(hDlg, IDOK), WM_SETFONT, (WPARAM)MainWindow1.hFont[0], 0);

		// バージョンの更新
		SetWindowText(AboutWindow1.hStatic[1], MainWindow1.szVersion);

		// Copyrightの更新
		SetWindowText(AboutWindow1.hStatic[2], MainWindow1.szCopyRight);

		// OSバージョンの更新
		AboutDlg_SetOSText(AboutWindow1.hStatic[4], FALSE);
		AboutDlg_SetOSText(AboutWindow1.hStatic[5], TRUE);

		MessageBeep(MB_ICONASTERISK);
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 選択されたメニューの解析:
		switch(wmId)
		{
		case IDOK:
		case IDCANCEL:
			dwAppFrag &= ~(APP_ABOUTBOX | APP_WINDOW_NOALPHA);
			EndDialog(hDlg, LOWORD(wParam));
			AboutWindow1.hWnd = NULL;
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

VOID AboutDlg_AboutBox(HWND hWnd)
{
	SetFocus(MainWindow1.hButton[0]);
	if(~dwAppFrag & APP_ABOUTBOX)
	{
		AboutWindow1.hWnd = CreateDialog(MainWindow1.hInst, MAKEINTRESOURCE(IDD_ABOUT), hWnd, (DLGPROC)AboutDlg_WndProc);

		if(AboutWindow1.hWnd != NULL)
		{
			ShowWindow(AboutWindow1.hWnd, SW_SHOW);
			UpdateWindow(AboutWindow1.hWnd);
		}
	}
	else
	{
		SetForegroundWindow(AboutWindow1.hWnd);
	}
}

VOID AboutDlg_SetOSText(HWND hWnd, BOOL VerifyVersion)
{
	TCHAR szText1[MAX_LOADSTRING] = _T("取得に失敗しました。");
	size_t nLen1 = sizeof(szText1) / sizeof(TCHAR);
	size_t nLen2 = 0;

	if(VerifyVersion != FALSE)
	{
		OSVERSIONINFOEX osInfo;
		const TCHAR *szName = _T("[実]");

		if(GetOSVersion(&osInfo))
		{
			if(!GetGokanMode(&osInfo))
				return;
		}
		dwAppFrag |= APP_GOKANMODE;
		_tcscpy(szText1, szName);
		nLen2 = _tcslen(szName);
	}

	if(GetOSName(szText1 + nLen2, nLen1 - nLen2, TRUE, VerifyVersion))
		SetWindowText(hWnd, szText1);
}
