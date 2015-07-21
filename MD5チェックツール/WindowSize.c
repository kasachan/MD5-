// WindowSize.c : アプリケーションのエントリ ポイントを定義します。
//

#include "WindowSize.h"


VOID RestoreWindow_SaveState(HWND hWnd, LPCTSTR inWindowName, LPCTSTR inIniFile)
{
	WINDOWPLACEMENT wndPlace;
	RECT rc;
	int nFlag;
	TCHAR szNum[33];
	MONITORINFO mInfo;
	HMONITOR hMonitor1;
	HMONITOR hMonitor2;
	APPBARDATA ad;

	wndPlace.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(hWnd, &wndPlace);


	//ウィンドウ状態
	switch(wndPlace.showCmd)
	{
	case SW_SHOWMAXIMIZED:
		//最大化
		rc = wndPlace.rcNormalPosition;
		nFlag = 1;
		break;
	case SW_SHOWMINIMIZED:
		//最小化
		rc = wndPlace.rcNormalPosition;
		nFlag = 2;
		break;
	default:
		GetWindowRect(hWnd, &rc);

		//ウィンドウのあるモニタハンドルを取得
		hMonitor1 = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
		mInfo.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hMonitor1, &mInfo);

		//タスクバーのあるモニタハンドルを取得
		ad.cbSize = sizeof(APPBARDATA);
		ad.hWnd = NULL;
		SHAppBarMessage(ABM_GETTASKBARPOS, &ad);
		hMonitor2 = MonitorFromRect(&(ad.rc), MONITOR_DEFAULTTONEAREST);

		if(hMonitor1 == hMonitor2)
		{
			//ウィンドウとタスクバーが同じモニタにある

			if((HWND)SHAppBarMessage(ABM_GETAUTOHIDEBAR, &ad) == NULL)
			{
				//常に表示

				SHAppBarMessage(ABM_GETTASKBARPOS, &ad);
				//座標補正
				switch(ad.uEdge)
				{
				case ABE_TOP:
					rc.top    = rc.top    - (ad.rc.bottom - ad.rc.top);
					rc.bottom = rc.bottom - (ad.rc.bottom - ad.rc.top);
					break;
				case ABE_LEFT:
					rc.left  = rc.left  - (ad.rc.right - ad.rc.left);
					rc.right = rc.right - (ad.rc.right - ad.rc.left);
					break;
				}
			}
		}

		if(rc.bottom == wndPlace.rcNormalPosition.bottom
			&& rc.left == wndPlace.rcNormalPosition.left
			&& rc.right == wndPlace.rcNormalPosition.right
			&& rc.top == wndPlace.rcNormalPosition.top)
		{
				//通常
				rc = wndPlace.rcNormalPosition;
				nFlag = 0;
		}
		else
		{
			//Aero snap
			nFlag = 3;
		}

		break;
	}


	wsprintf(szNum, TEXT("%d"), rc.left);
	WritePrivateProfileString(inWindowName, TEXT("WindowLeft"), szNum, inIniFile);

	wsprintf(szNum, TEXT("%d"), rc.top);
	WritePrivateProfileString(inWindowName, TEXT("WindowTop"), szNum, inIniFile);

	wsprintf(szNum, TEXT("%d"), rc.right);
	WritePrivateProfileString(inWindowName, TEXT("WindowRight"), szNum, inIniFile);

	wsprintf(szNum, TEXT("%d"), rc.bottom);
	WritePrivateProfileString(inWindowName, TEXT("WindowBottom"), szNum, inIniFile);

	wsprintf(szNum, TEXT("%d"), nFlag);
	WritePrivateProfileString(inWindowName, TEXT("WindowState"), szNum, inIniFile);
}

VOID RestoreWindow_LoadState(HWND hWnd, LPCTSTR inWindowName, LPCTSTR inIniFile)
{
	WINDOWPLACEMENT wndPlace;
	RECT rc;
	int nState;

	wndPlace.length = sizeof(WINDOWPLACEMENT);
	wndPlace.flags = WPF_SETMINPOSITION | WPF_RESTORETOMAXIMIZED;

	//デフォルトのウィンドウ位置取得
	GetClientRect(hWnd, &rc);

	//ウィンドウ位置読み込み
	//無ければデフォルトを返す
	rc.left   = GetPrivateProfileInt(inWindowName, TEXT("WindowLeft"),   rc.left,   inIniFile);
	rc.top    = GetPrivateProfileInt(inWindowName, TEXT("WindowTop"),    rc.top,    inIniFile);
	rc.right  = GetPrivateProfileInt(inWindowName, TEXT("WindowRight"),  rc.right,  inIniFile);
	rc.bottom = GetPrivateProfileInt(inWindowName, TEXT("WindowBottom"), rc.bottom, inIniFile);


	//ウィンドウ位置コピー
	wndPlace.rcNormalPosition = rc;

	//ウィンドウ状態
	nState = GetPrivateProfileInt(inWindowName, TEXT("WindowState"), 0, inIniFile);
	switch(nState)
	{
	case 0:
		//通常
	case 2:
		//最小化
	case 3:
		//Aero snap
		wndPlace.showCmd = SW_SHOW;
		SetWindowPlacement(hWnd, &wndPlace);
		break;
	case 1:
		//最大化
		wndPlace.showCmd = SW_SHOWMAXIMIZED;
		SetWindowPlacement(hWnd, &wndPlace);
		break;
	}
	InvalidateRect(hWnd, NULL, TRUE);
}
