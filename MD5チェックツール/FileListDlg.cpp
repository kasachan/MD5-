// FileListDlg.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "FileListDlg.h"

// グローバル変数の定義します:
FileListWindow FileListWindow1;

// 外部ファイルの変数定義します。
extern MainWindow	MainWindow1;
extern AboutWindow	AboutWindow1;
extern DWORD		dwAppFrag;
extern DWORD		dwAppState;

#define FILELISTDLG_X 378
#define FILELISTDLG_Y 193


// ファイルリストのメッセージ ハンドラです。
INT_PTR CALLBACK FileList_WndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	int wmId, wmEvent;
	int i, cx, cy;
	static DWORD dwbaseX;
	static DWORD dwbaseY;
	static SHELLEXECUTEINFO	ShellInfo;
	static HFONT hFont;
	static HIMAGELIST hImgS;
	static HIMAGELIST hDragImage;
	static RECT AearoRect;
	static TCHAR *p;
	static TCHAR szBuf[1024 +							// szFldBuf[1024]
			MAX_STRINGTABLE +							// szFldStBuf[255]
			MAX_PATH_SIZE +								// szFileChangeMD5[255]
			(NUMBER_LOADSTRING * (sizeof(FileListWindow1.szButtonText1) / sizeof(TCHAR*))) +	// szButtonText1[][NUMBER_LOADSTRING]
			(NUMBER_LOADSTRING * (sizeof(FileListWindow1.szButtonText2) / sizeof(TCHAR*)))		// szButtonText2[][NUMBER_LOADSTRING]];
			];
	RECT dlgRect;
	RECT subRect;
	DWORD dwCount;
	DWORD dwRet;
	DWORD dwStyle;
	LV_DISPINFO *lvInfo;
	LVHITTESTINFO lvHitInfo;
	POINT pt;
	LPMINMAXINFO lpmm;
	LV_COLUMN lvColumn;
	LPNMLVCUSTOMDRAW lpCustomDraw;
	LV_ITEM lvItem1;
	NMLISTVIEW *pNMLV;

#ifdef _DEBUG
	DWORD dwTime1, dwTime2;
	TCHAR szDebugText[1024];
#endif


#define	MDIVX(X)	MulDiv(X, dwbaseX, 4)				// ダイアログ単位からピクセル数への変換用
#define	MDIVY(Y)	MulDiv(Y, dwbaseY, 8)

	if(message == FileListWindow1.WindowMessage)
	{
		switch(wParam)
		{
		case APP_MESSAGE_MD5FILE_OPEN:
			dwRet = FileList_AddMD5File();
			SetWindowLong(hDlg, DWLP_MSGRESULT, dwRet);
			return TRUE;
		case APP_MESSAGE_FOLDER_INIT:
			FileListWindow1.dwFileCount = lParam;
			DragAcceptFiles(hDlg, FALSE);
			return TRUE;
		case APP_MESSAGE_SUBFOLDER_OPEN:
			return TRUE;
		case APP_MESSAGE_FOLDER_INIT_EXIT:
			if((DWORD)lParam == -1)
			{
				DestroyWindow(hDlg);
				return FALSE;
			}
			dwAppFrag |= APP_FILELISTBOX_EDIT;
			dwAppFrag &= ~APP_FOLDERINIT;
			FileListWindow1.dwNewFile++;
			MainWindow1.MD5Thread1.IsFileNoCheck = 1;
			FileList_ListView(FileListWindow1.dwFileCount, FALSE);
			EnableWindow(FileListWindow1.hButton1[0], TRUE);
			EnableWindow(FileListWindow1.hButton1[1], TRUE);
			EnableWindow(FileListWindow1.hButton1[2], TRUE);
			EnableWindow(FileListWindow1.hRadio1[0],  TRUE);
			EnableWindow(FileListWindow1.hRadio1[1],  TRUE);
			EnableWindow(FileListWindow1.hRadio1[2],  TRUE);
			EnableWindow(FileListWindow1.hButton1[3], FALSE);
			FileListWindow1.dwNewFile++;
			DragAcceptFiles(hDlg, TRUE);
			return TRUE;
		case APP_MESSAGE_MD5FILE_INIT:
			if(lParam > 0)
			{
				FileList_ListView(FileListWindow1.dwFileCount, FALSE);
			}
			FileListWindow1.dwFileCount = lParam;
			DragAcceptFiles(hDlg, FALSE);
			return TRUE;
		case APP_MESSAGE_MD5FILE_INIT_EXIT:
			if(lParam > 0)
			{
				FileList_ListView(FileListWindow1.dwFileCount, FALSE);
			}
			FileListWindow1.dwFileCount = lParam;
			DragAcceptFiles(hDlg, TRUE);
			return TRUE;
		case APP_MESSAGE_INIT:
			DragAcceptFiles(hDlg, FALSE);
			FileListWindow1.dwFileCount = 0;
			FileListWindow1.IsMD5FileCompMassege = 0;
			SetWindowText(FileListWindow1.hButton1[0], FileListWindow1.szButtonText1[1]);
			EnableWindow(FileListWindow1.hButton1[0], TRUE);
			SetWindowText(FileListWindow1.hButton1[1], FileListWindow1.szButtonText2[1]);
			EnableWindow(FileListWindow1.hButton1[1], TRUE);
			EnableWindow(FileListWindow1.hButton1[2], FALSE);
			EnableWindow(FileListWindow1.hRadio1[0],  FALSE);
			EnableWindow(FileListWindow1.hRadio1[1],  FALSE);
			EnableWindow(FileListWindow1.hRadio1[2],  FALSE);
			EnableWindow(FileListWindow1.hButton1[3], FALSE);
			return TRUE;
		case APP_MESSAGE_MULTIFILE:
			return TRUE;
		case APP_MESSAGE_FILECHANGE:
			if(lParam >= 0)
			{
				ListView_SetItemText(FileListWindow1.hList1, lParam, 2, _T("計算中..."));
			}
			return TRUE;
		case APP_MESSAGE_FILEEND:
			if(lParam >= 0)
			{
				FileList_SetListView(lParam, FALSE);
			}
			return TRUE;
		case APP_MESSAGE_FILECLEAR:
			if(dwAppFrag & APP_FILE_CLEAR)
			{
				FileList_Clear(FALSE);
				return TRUE;
			}
			break;
		case APP_MESSAGE_END:
		case APP_MESSAGE_ERROR:
			if(dwAppFrag & APP_MD5FILE_ADDMODE)
			{
				return FALSE;
			}
			if(!FileListWindow1.dwNewFile)
			{
				FileListWindow1.dwNewFile++;
			}

			memset(&FileListWindow1.dwMD5CheckCount, 0, sizeof(FileListWindow1.dwMD5CheckCount));
			for(dwCount = 0; dwCount < MainWindow1.MD5Thread1.MultiFile->FileCount; dwCount++)
			{
				if((MainWindow1.MD5Thread1.MultiFile->FileRecode + dwCount)->MD5FileHashLine != MD5FILE_NOLINE &&
					(MainWindow1.MD5Thread1.MultiFile->FileRecode + dwCount)->FileLastError == 0)
				{
					FileListWindow1.dwMD5CheckCount[0]++;
					if((MainWindow1.MD5Thread1.MultiFile->FileRecode + dwCount)->MD5FileHashCmp == 0)
					{
						FileListWindow1.dwMD5CheckCount[1]++;
					}
					else
					{
						FileListWindow1.dwMD5CheckCount[2]++;
					}
				}
			}

			FileListWindow1.dwFileCount = MainWindow1.MD5Thread1.MultiFile->FileCount;
			dwRet = FileListWindow1.dwFileCount > 0;
			InvalidateRect(FileListWindow1.hList1, NULL, TRUE);
			SetWindowText(FileListWindow1.hButton1[0], FileListWindow1.szButtonText1[0]);
			SetWindowText(FileListWindow1.hButton1[1], FileListWindow1.szButtonText2[0]);
			if(MainWindow1.dwAddFileModeFileCount != 0)
			{
				EnableWindow(FileListWindow1.hButton1[0], TRUE);
				EnableWindow(FileListWindow1.hButton1[1], TRUE);
			}
			else
			{
				EnableWindow(FileListWindow1.hButton1[0], FALSE);
				EnableWindow(FileListWindow1.hButton1[1], dwRet ? TRUE : FALSE);
			}
			EnableWindow(FileListWindow1.hButton1[2], TRUE);
			EnableWindow(FileListWindow1.hButton1[3], dwRet && wParam == APP_MESSAGE_END ? TRUE : FALSE);
			EnableWindow(FileListWindow1.hRadio1[0],  TRUE);
			EnableWindow(FileListWindow1.hRadio1[1],  TRUE);
			EnableWindow(FileListWindow1.hRadio1[2],  TRUE);

			if(FileListWindow1.IsMD5FileCompMassege == 0 &&
				FileListWindow1.dwMD5CheckCount[2] > 0)
			{

				LoadString(MainWindow1.hInst, IDS_MD5FILE_COMP1, FileListWindow1.szStBuf, MAX_STRINGTABLE);
				MessageFormat(FileListWindow1.szBuf, MAX_STRINGTABLE, FileListWindow1.szStBuf, FileListWindow1.dwMD5CheckCount[2]);
				FileList_MessageBox(hDlg, FileListWindow1.szBuf, MainWindow1.szTitle, MB_ICONHAND);
				FileListWindow1.IsMD5FileCompMassege++;
			}
			DragAcceptFiles(hDlg, TRUE);
			SetFocus(FileListWindow1.hButton1[3]);
			return TRUE;
		case APP_MESSAGE_PAUSE:
			SetWindowText(FileListWindow1.hButton1[0], lParam ? FileListWindow1.szButtonText1[2] : FileListWindow1.szButtonText1[1]);
			return TRUE;
		}
		return FALSE;
	}

	switch(message)
	{
	case WM_INITDIALOG:
		dwAppFrag |= APP_WINDOW_NOALPHA;

		FileListWindow1.szBuf = szBuf;

		*FileListWindow1.szBuf				= '\0';
		FileListWindow1.szStBuf				= 1024 + FileListWindow1.szBuf;
		*FileListWindow1.szStBuf			= '\0';
		FileListWindow1.szFileChangeMD5		= MAX_STRINGTABLE + FileListWindow1.szStBuf;
		*FileListWindow1.szFileChangeMD5	= '\0';

		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)MainWindow1.hIcon);
		SetEnableThemeDialogTexture(hDlg);

		FileListWindow1.szButtonText1[0] = MAX_PATH_SIZE + FileListWindow1.szFileChangeMD5;
		LoadString(MainWindow1.hInst, IDS_FILELIST_BUTTON1_TEXT1, FileListWindow1.szButtonText1[0], NUMBER_LOADSTRING);
		for(i = 1; i < sizeof(FileListWindow1.szButtonText1) / sizeof(TCHAR*); i++)
		{
			FileListWindow1.szButtonText1[i] = NUMBER_LOADSTRING + FileListWindow1.szButtonText1[i - 1];
			LoadString(MainWindow1.hInst, IDS_FILELIST_BUTTON1_TEXT1 + i, FileListWindow1.szButtonText1[i], NUMBER_LOADSTRING);
		}

		FileListWindow1.szButtonText2[0] = NUMBER_LOADSTRING + FileListWindow1.szButtonText1[sizeof(FileListWindow1.szButtonText1) / sizeof(TCHAR*) - 1];
		LoadString(MainWindow1.hInst, IDS_FILELIST_BUTTON2_TEXT1, FileListWindow1.szButtonText2[0], NUMBER_LOADSTRING);
		for(i = 1; i < sizeof(FileListWindow1.szButtonText2) / sizeof(TCHAR*); i++)
		{
			FileListWindow1.szButtonText2[i] = NUMBER_LOADSTRING + FileListWindow1.szButtonText2[i - 1];
			LoadString(MainWindow1.hInst, IDS_FILELIST_BUTTON2_TEXT1 + i, FileListWindow1.szButtonText2[i], NUMBER_LOADSTRING);
		}


		FileListWindow1.WindowMessage = RegisterWindowMessage(_T("FileList"));

		FileListWindow1.hList1 = GetDlgItem(hDlg, IDC_FILELIST_LIST1);
		SendMessage(FileListWindow1.hList1, WM_SETFONT, (WPARAM)MainWindow1.hFont[0], 0);
		dwStyle = ListView_GetExtendedListViewStyle(FileListWindow1.hList1);
		dwStyle |= LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
		ListView_SetExtendedListViewStyle(FileListWindow1.hList1, dwStyle);

		hImgS = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 3, 0);
		ListView_SetImageList(FileListWindow1.hList1, hImgS, LVSIL_SMALL);

		ImageList_AddIcon(hImgS, LoadIcon(MainWindow1.hInst, MAKEINTRESOURCE(IDI_ICON1)));
		ImageList_AddIcon(hImgS, LoadIcon(MainWindow1.hInst, MAKEINTRESOURCE(IDI_ICON2)));
		ImageList_AddIcon(hImgS, LoadIcon(MainWindow1.hInst, MAKEINTRESOURCE(IDI_ICON3)));
		ImageList_AddIcon(hImgS, LoadIcon(MainWindow1.hInst, MAKEINTRESOURCE(IDI_ICON4)));

		FileListWindow1.hGroup1[0] = GetDlgItem(hDlg, IDC_FILELIST_GROUP1);
		SendMessage(FileListWindow1.hGroup1[0], WM_SETFONT, (WPARAM)MainWindow1.hFont[0], 0);

		for(i = 0; i < sizeof(FileListWindow1.hButton1) / sizeof(HWND); i++)
		{
			FileListWindow1.hButton1[i] = GetDlgItem(hDlg, IDC_FILELIST_BUTTON1 + i);
			SendMessage(FileListWindow1.hButton1[i], WM_SETFONT, (WPARAM)MainWindow1.hFont[0], 0);
		}

		for(i = 0; i < sizeof(FileListWindow1.hRadio1) / sizeof(HWND); i++)
		{
			FileListWindow1.hRadio1[i] = GetDlgItem(hDlg, IDC_FILELIST_RADIO1 + i);
			SendMessage(FileListWindow1.hRadio1[i], WM_SETFONT, (WPARAM)MainWindow1.hFont[0], 0);
		}

		FileListWindow1.hMenu1 = LoadMenu(MainWindow1.hInst, MAKEINTRESOURCE(IDR_FILELIST_LISTMENU));
		FileListWindow1.hPupMenu1[0] = GetSubMenu(FileListWindow1.hMenu1, 0);
		FileListWindow1.hPupMenu1[1] = GetSubMenu(FileListWindow1.hMenu1, 1);

		ZeroMemory(&dlgRect, sizeof(RECT));
		dlgRect.left	= 4;
		dlgRect.top		= 8;
		MapDialogRect(hDlg, &dlgRect);
		dwbaseX = dlgRect.left;
		dwbaseY = dlgRect.top;

		GetWindowRect(hDlg, &dlgRect);
		GetAearoWindowRect(hDlg, &AearoRect);
		AearoRect.left		= dlgRect.left - AearoRect.left;
		AearoRect.top		= dlgRect.top - AearoRect.top;
		AearoRect.right		= dlgRect.right - AearoRect.right;
		AearoRect.bottom	= dlgRect.bottom - AearoRect.bottom;

		lvColumn.mask		= LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvColumn.fmt		= LVCFMT_LEFT;
		lvColumn.cx			= 150;
		lvColumn.pszText	= _T("ファイル名");
		lvColumn.iSubItem	= 0;
		ListView_InsertColumn(FileListWindow1.hList1, 0, &lvColumn);

		lvColumn.cx			= 80;
		lvColumn.fmt		= LVCFMT_RIGHT;
		lvColumn.pszText	= _T("サイズ");
		lvColumn.iSubItem	= 1;
		ListView_InsertColumn(FileListWindow1.hList1, 1, &lvColumn);

		lvColumn.cx			= 250;
		lvColumn.fmt		= LVCFMT_LEFT;
		lvColumn.pszText	= _T("MD5");
		lvColumn.iSubItem	= 2;
		ListView_InsertColumn(FileListWindow1.hList1, 2, &lvColumn);

		FileListWindow1.hStatus1 = CreateWindowEx(
			0,
			STATUSCLASSNAME,
			NULL,
			WS_CHILD | SBARS_SIZEGRIP | CCS_BOTTOM | WS_VISIBLE,
			0,
			0,
			0,
			0,
			hDlg,
			(HMENU)IDC_STATUS1,
			MainWindow1.hInst,
			NULL);
		SendMessage(FileListWindow1.hStatus1, SB_SIMPLE, TRUE, 0);

		GetWindowRect(FileListWindow1.hStatus1, &subRect);
		FileListWindow1.Status1Height = subRect.bottom - subRect.top;

		ZeroMemory(&ShellInfo, sizeof(ShellInfo));
		ShellInfo.cbSize	= sizeof(ShellInfo);
		ShellInfo.fMask		= SEE_MASK_NOCLOSEPROCESS | SEE_MASK_INVOKEIDLIST | SEE_MASK_FLAG_NO_UI;
		ShellInfo.hwnd		= hDlg;
		ShellInfo.lpVerb	= _T("Properties");//プロパティダイアログを開くオプション。
		ShellInfo.lpFile	= NULL;

		GetPrivateProfileString(_T("FileList"), _T("OpenOption"), _T("0"), FileListWindow1.szBuf, 2, MainWindow1.szINIFile);
		FileListWindow1.dwOpenOption = _ttoi(FileListWindow1.szBuf);
		// 開くオプションの改変チェック
		if(FileListWindow1.dwOpenOption > sizeof(FileListWindow1.hRadio1) / sizeof(HWND))
		{
			FileListWindow1.dwOpenOption = 0;
		}
		SendMessage(FileListWindow1.hRadio1[FileListWindow1.dwOpenOption], BM_SETCHECK, BST_CHECKED, 0);
		RestoreWindow_LoadState(hDlg, _T("FileList"), MainWindow1.szINIFile);
		FileListWindow1.IsMD5FileCompMassege		= 1;
		FileListWindow1.FileListItem1.dwListBufSize	= 0;
		FileList_ListView(0, FALSE);
		if(FileListWindow1.FileListItem1.dwListSelectItem != 0)
		{
			*FileListWindow1.FileListItem1.dwListSelectItem = (DWORD)-1;
		}
		else
		{
			DestroyWindow(hDlg);
		}

		dwAppFrag |= APP_FILELISTBOX;

		// MD5計算中は自分のウインドウに APP_MESSAGE_INIT を送る。
		if(dwAppFrag & APP_MD5THREAD)
		{
			SendMessage(hDlg, FileListWindow1.WindowMessage, APP_MESSAGE_INIT, 0);
			SendMessage(FileListWindow1.hWnd, FileListWindow1.WindowMessage, APP_MESSAGE_FILECHANGE, MainWindow1.MD5Thread1.MultiFile->FileCurrentCount);
		}
		else
		{
			SendMessage(hDlg, FileListWindow1.WindowMessage, dwAppState != 0 ? APP_MESSAGE_ERROR : APP_MESSAGE_END, 0);
		}

		if(dwAppFrag & APP_PAUSE)
		{
			SendMessage(hDlg, FileListWindow1.WindowMessage, APP_MESSAGE_PAUSE, dwAppFrag & APP_PAUSE ? TRUE : FALSE);
		}
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 選択されたメニューの解析:
		switch(wmId)
		{
		case IDC_FILELIST_RADIO1:
			FileListWindow1.dwOpenOption = 0;
			break;
		case IDC_FILELIST_RADIO2:
			FileListWindow1.dwOpenOption = 1;
			break;
		case IDC_FILELIST_RADIO3:
			FileListWindow1.dwOpenOption = 2;
			break;
		case IDC_FILELIST_BUTTON1:
			SendMessage(MainWindow1.MD5Thread1.hWnd, WM_COMMAND, dwAppFrag & APP_MD5THREAD ? IDM_PAUSE : IDM_REOPEN, 0);
			break;
		case IDC_FILELIST_BUTTON2:
			if(dwAppFrag & APP_MD5THREAD)
			{
				SendMessage(MainWindow1.MD5Thread1.hWnd, WM_COMMAND, IDM_CANCEL, 0);
				return 0;
			}
			FileList_Clear(TRUE);
			SendMessage(MainWindow1.MD5Thread1.hWnd, WM_COMMAND, IDM_CLEAR, 0);
			break;
		case IDC_FILELIST_BUTTON3:
			switch(FileListWindow1.dwOpenOption)
			{
			case 1:
				FileList_FolderOpen();
				break;
			case 2:
				FileList_MD5FileOpen();
				break;
			default:
				FileList_FileOpen();
			}
			break;
		case IDC_FILELIST_BUTTON4:
			SaveFile();
			break;
		case ID_FILELIST_LISTMENU1_1:
			if(*FileListWindow1.FileListItem1.dwListSelectItem != -1)
			{
				SendMessage(MainWindow1.MD5Thread1.hWnd, MainWindow1.MD5Thread1.MessageID, APP_MESSAGE_CHANGE_FILEVIEW, *FileListWindow1.FileListItem1.dwListSelectItem);
			}
			break;
		case ID_FILELIST_LISTMENU1_2:
			if(*FileListWindow1.FileListItem1.dwListSelectItem != -1)
			{
				for(DWORD dwI = FileListWindow1.FileListItem1.dwListSelectCount; dwI > 0; dwI--)
				{
					_tcscpy(FileListWindow1.szBuf, _T("/select,"));
					_tcscpy(FileListWindow1.szBuf + 8, (MainWindow1.MD5Thread1.MultiFile->FileRecode + *(FileListWindow1.FileListItem1.dwListSelectItem + dwI - 1))->FileName);
					ShellExecute(hDlg, _T("open"), _T("EXPLORER.EXE"), FileListWindow1.szBuf, NULL, SW_SHOWNORMAL);
				}
			}
			break;
		case ID_FILELIST_LISTMENU1_3:
			if(*FileListWindow1.FileListItem1.dwListSelectItem != -1 &&
				(MainWindow1.MD5Thread1.MultiFile->FileRecode + *FileListWindow1.FileListItem1.dwListSelectItem)->FileLastError == 0)
			{
				MD5_Thread_MD5ToString(FileListWindow1.szBuf, (MainWindow1.MD5Thread1.MultiFile->FileRecode + *FileListWindow1.FileListItem1.dwListSelectItem)->FileHashByte, ~dwAppFrag & APP_MD5OUT_LOWER);
				SetClipboardText(hDlg, FileListWindow1.szBuf);
			}
			break;
		case ID_FILELIST_LISTMENU1_4:
			if(*FileListWindow1.FileListItem1.dwListSelectItem != -1)
			{
				Copy((MainWindow1.MD5Thread1.MultiFile->FileRecode + *FileListWindow1.FileListItem1.dwListSelectItem)->FileName);
			}
			break;
		case ID_FILELIST_LISTMENU1_5:
			FileList_FileDelete();
			break;
		case ID_FILELIST_LISTMENU1_6:
			if(*FileListWindow1.FileListItem1.dwListSelectItem != -1)
			{
				for(DWORD dwI = FileListWindow1.FileListItem1.dwListSelectCount; dwI > 0; dwI--)
				{
					ShellInfo.lpFile = (MainWindow1.MD5Thread1.MultiFile->FileRecode + *(FileListWindow1.FileListItem1.dwListSelectItem + dwI - 1))->FileName;
					ShellExecuteEx(&ShellInfo);
				}
			}
			break;
		case ID_FILELIST_LISTMENU1_1_1:
			if(*FileListWindow1.FileListItem1.dwListSelectItem != -1)
			{
				Copy(PathFindFileName((MainWindow1.MD5Thread1.MultiFile->FileRecode + *FileListWindow1.FileListItem1.dwListSelectItem)->FileName));
			}
			break;
		case ID_FILELIST_LISTMENU1_1_2:
			if(*FileListWindow1.FileListItem1.dwListSelectItem != -1)
			{
				_tcscpy(FileListWindow1.szBuf, (MainWindow1.MD5Thread1.MultiFile->FileRecode + *FileListWindow1.FileListItem1.dwListSelectItem)->FileName);
				PathRemoveFileSpec(FileListWindow1.szBuf);
				Copy(FileListWindow1.szBuf);
			}
			break;
		case ID_FILELIST_LISTMENU1_1_3:
			if(*FileListWindow1.FileListItem1.dwListSelectItem != -1)
			{
				GetSizeText(FileListWindow1.szBuf, (MainWindow1.MD5Thread1.MultiFile->FileRecode + *FileListWindow1.FileListItem1.dwListSelectItem)->FileSize);
				Copy(FileListWindow1.szBuf);
			}
			break;
		case ID_FILELIST_LISTMENU2_1:
			FileList_FileOpen();
			break;
		case ID_FILELIST_LISTMENU2_2:
			FileList_FolderOpen();
			break;
		case ID_FILELIST_LISTMENU2_3:
			FileList_MD5FileOpen();
			break;
		case ID_FILELIST_LISTMENU2_4:
			if(OpenClipboard(hDlg))
			{
				HANDLE hMem;
				HDROP  hdropFile;

				hMem = GetClipboardData(CF_HDROP);
				if(hMem == NULL)
				{
					return FALSE;
				}

				hdropFile = (HDROP)GlobalLock(hMem);
				dwRet = FileList_DropFile(hdropFile);

				GlobalUnlock(hMem);
				CloseClipboard();

				if(dwRet == -1)
				{
					FileRecodeFoul(hDlg);
					DestroyWindow(MainWindow1.MD5Thread1.hWnd);
					return FALSE;
				}
			}
			break;
		case ID_FILELIST_LISTMENU2_5:
			FileList_Clear(TRUE);
			break;
		}
		return TRUE;
	case WM_DROPFILES:
		dwRet = FileList_DropFile((HDROP)wParam);
		DragFinish((HDROP)wParam);

		if(dwRet == -1)
		{
			FileRecodeFoul(hDlg);
			DestroyWindow(MainWindow1.MD5Thread1.hWnd);
			return FALSE;
		}
		return TRUE;
	case WM_GETMINMAXINFO:
		lpmm = (MINMAXINFO*)lParam;
		lpmm->ptMinTrackSize.x = GetSystemMetrics(SM_CXSIZEFRAME) * 2 + MDIVX(FILELISTDLG_X) - AearoRect.left;
		lpmm->ptMinTrackSize.y = GetSystemMetrics(SM_CYSIZEFRAME) * 2 + MDIVY(FILELISTDLG_Y) + GetSystemMetrics(SM_CYCAPTION) - AearoRect.top;
		return TRUE;
	case WM_SIZE:
		cx = LOWORD(lParam);
		cy = HIWORD(lParam);

		MoveWindow(FileListWindow1.hList1, 0, 0, cx - MDIVX(FILELISTDLG_X - 288), cy - FileListWindow1.Status1Height, FALSE);
		MoveWindow(FileListWindow1.hButton1[0], cx - MDIVX(FILELISTDLG_X - 294), MDIVY(12), MDIVX(78), MDIVY(18), FALSE);
		MoveWindow(FileListWindow1.hButton1[1], cx - MDIVX(FILELISTDLG_X - 294), MDIVY(36), MDIVX(78), MDIVY(18), FALSE);
		MoveWindow(FileListWindow1.hButton1[2], cx - MDIVX(FILELISTDLG_X - 294), MDIVY(60), MDIVX(78), MDIVY(18), FALSE);
		MoveWindow(FileListWindow1.hGroup1[0], cx - MDIVX(FILELISTDLG_X - 294), MDIVY(84), MDIVX(78), MDIVY(54), FALSE);
		MoveWindow(FileListWindow1.hRadio1[0], cx - MDIVX(FILELISTDLG_X - 300), MDIVY(96), MDIVX(71), MDIVY(10), FALSE);
		MoveWindow(FileListWindow1.hRadio1[1], cx - MDIVX(FILELISTDLG_X - 300), MDIVY(108), MDIVX(71), MDIVY(10), FALSE);
		MoveWindow(FileListWindow1.hRadio1[2], cx - MDIVX(FILELISTDLG_X - 300), MDIVY(120), MDIVX(71), MDIVY(10), FALSE);
		MoveWindow(FileListWindow1.hButton1[3], cx - MDIVX(FILELISTDLG_X - 294), MDIVY(144), MDIVX(78), MDIVY(18), FALSE);
		SendMessage(FileListWindow1.hStatus1, WM_SIZE, wParam, lParam);

		InvalidateRect(hDlg, NULL, TRUE);
		return TRUE;
	case WM_NOTIFY:
		lvInfo = (LV_DISPINFO*)lParam;

		switch(wParam)
		{
		case IDC_FILELIST_LIST1:
			switch(lvInfo->hdr.code)
			{
			case LVN_ITEMCHANGED:
				if(FileListWindow1.IsListLock == FALSE)
				{
					DWORD dwListSelect = (DWORD)-1;

#ifdef _DEBUG
					OutputDebugString(_T("FileList: FileList_WndProc(): LVN_ITEMCHANGED\r\n"));
#endif

					FileListWindow1.FileListItem1.dwListSelectCount = 0;
					for(;;)
					{
						dwListSelect = ListView_GetNextItem(FileListWindow1.hList1, dwListSelect, LVNI_ALL | LVNI_SELECTED);
						*(FileListWindow1.FileListItem1.dwListSelectItem + FileListWindow1.FileListItem1.dwListSelectCount) = dwListSelect;
						if(dwListSelect == -1)
						{
							break;
						}
						FileListWindow1.FileListItem1.dwListSelectCount++;
					}
					*(FileListWindow1.FileListItem1.dwListSelectItem + FileListWindow1.FileListItem1.dwListSelectCount) = (DWORD)-1;


					if(*FileListWindow1.FileListItem1.dwListSelectItem != -1)
					{
#ifdef _DEBUG
						TCHAR szNum[NUMBER_LOADSTRING];

						p = qtcscpy(FileListWindow1.szStBuf, _T("選択された項目: "));
						for(DWORD dwI = 0; dwI < FileListWindow1.FileListItem1.dwListSelectCount; dwI++)
						{
							_itot(*(FileListWindow1.FileListItem1.dwListSelectItem + dwI), szNum, 10);
							p = qtcscpy(qtcscpy(p, szNum), _T(", "));
						}
						p -= 2;
						*p = '\0';

						p = qtcscpy(p, _T("\r\n"));
						OutputDebugString(FileListWindow1.szStBuf);
#endif
						LoadString(MainWindow1.hInst, IDS_FILELIST_STATUS_TEXT2, FileListWindow1.szStBuf, MAX_STRINGTABLE);
						MessageFormat(FileListWindow1.szBuf, 1024, FileListWindow1.szStBuf, FileListWindow1.FileListItem1.dwListSelectCount, (MainWindow1.MD5Thread1.MultiFile->FileRecode + *FileListWindow1.FileListItem1.dwListSelectItem)->FileName);
						SendMessage(FileListWindow1.hStatus1, SB_SETTEXT, SB_SIMPLEID, (WPARAM)FileListWindow1.szBuf);
					}
				}
				break;
			case NM_CLICK:
			case NM_DBLCLK:
			case NM_RCLICK:
				GetCursorPos(&lvHitInfo.pt);
				ScreenToClient(FileListWindow1.hList1, &lvHitInfo.pt);
				ListView_SubItemHitTest(FileListWindow1.hList1, &lvHitInfo);

				if(lvHitInfo.iItem != -1 &&
					MainWindow1.MD5Thread1.MultiFile->FileCount > 0)
				{
					DWORD dwListSelect = (DWORD)-1;

					dwCount = 0;
					*FileListWindow1.FileListItem1.dwListSelectItem = lvHitInfo.iItem;

					for(;;)
					{
						dwListSelect = ListView_GetNextItem(FileListWindow1.hList1, dwListSelect, LVNI_ALL | LVNI_SELECTED);
						if(dwListSelect == -1)
						{
							break;
						}
						dwCount++;
					}

					switch(lvInfo->hdr.code)
					{
					case NM_DBLCLK:
						if(dwAppFrag & APP_MD5THREAD)
						{
							MessageBeep(MB_ICONERROR);
						}
						else
						{
							SendMessage(MainWindow1.MD5Thread1.hWnd, MainWindow1.MD5Thread1.MessageID, APP_MESSAGE_CHANGE_FILEVIEW, *FileListWindow1.FileListItem1.dwListSelectItem);
						}
						break;
					case NM_CLICK:
						break;
					case NM_RCLICK:
						GetCursorPos(&pt);
						FileList_FileMenu(hDlg, *FileListWindow1.FileListItem1.dwListSelectItem, &pt);
						break;
					}
				}
				else
				{
					LoadString(MainWindow1.hInst, IDS_FILELIST_STATUS_TEXT1, FileListWindow1.szStBuf, MAX_STRINGTABLE);
					MessageFormat(FileListWindow1.szBuf, 1024, FileListWindow1.szStBuf, MainWindow1.MD5Thread1.MultiFile->FileCount);
					SendMessage(FileListWindow1.hStatus1, SB_SETTEXT, SB_SIMPLEID, (WPARAM)FileListWindow1.szBuf); 

					switch(lvInfo->hdr.code)
					{
					case NM_RCLICK:
						GetCursorPos(&pt);
						FileList_NoFileMenu(hDlg, &pt);
						break;
					}
				}
				return TRUE;
			case LVN_KEYDOWN:
				switch(((LPNMLVKEYDOWN)lParam)->wVKey)
				{
				case VK_RETURN:
					if(*FileListWindow1.FileListItem1.dwListSelectItem != -1)
					{
						SendMessage(MainWindow1.MD5Thread1.hWnd, MainWindow1.MD5Thread1.MessageID, APP_MESSAGE_CHANGE_FILEVIEW, *FileListWindow1.FileListItem1.dwListSelectItem);
					}
					else
					{
						MessageBeep(MB_ICONERROR);
					}
					break;
				case VK_APPS:
					if(*FileListWindow1.FileListItem1.dwListSelectItem != -1)
					{
						static DWORD dwPos = (DWORD)-1;

						if(dwPos == -1)
						{
							ListView_GetItemRect(FileListWindow1.hList1, *FileListWindow1.FileListItem1.dwListSelectItem, &subRect, LVIR_ICON);
							dwPos = (subRect.bottom - subRect.top) / 2;
						}
						ListView_GetItemPosition(FileListWindow1.hList1, *FileListWindow1.FileListItem1.dwListSelectItem, &pt);
						pt.x += dwPos;
						pt.y += dwPos;
						ClientToScreen(FileListWindow1.hList1, &pt);
						FileList_FileMenu(hDlg, *FileListWindow1.FileListItem1.dwListSelectItem, &pt);
					}
					else
					{
						pt.x = 0;
						pt.y = 0;
						ClientToScreen(FileListWindow1.hList1, &pt);
						FileList_NoFileMenu(hDlg, &pt);
					}
					break;
				case VK_DELETE:
					if(MainWindow1.dwAddFileModeFileCount != 0 &&
						dwAppFrag & APP_MD5THREAD)
					{
						MessageBeep(MB_ICONERROR);
					}
					else
					{
						FileList_FileDelete();
					}
					break;
				case VK_TAB:
					return FALSE;
				}
				return TRUE;
			case LVN_COLUMNCLICK:
				if(MainWindow1.dwAddFileModeFileCount == 0 &&
					~dwAppFrag & APP_MD5THREAD)
				{
					pNMLV = (NM_LISTVIEW*)lParam;

					// コラム別にソートの状態を保存します。
					FileListWindow1.FileSortSubNo[pNMLV->iSubItem] = FileListWindow1.FileSortSubNo[pNMLV->iSubItem] ? 0 : 1;

#ifdef _DEBUG
					OutputDebugString(_T("FileList: FileList_WndProc(): ソートの処理開始\r\n"));
#endif

					dwRet = MD5_Thread_CreateBuffer(&MainWindow1.MD5Thread1, MainWindow1.MD5Thread1.MultiFile->FileCount * 2, TRUE);
					if(dwRet == -1)
					{
						FileRecodeFoul(hDlg);
						return FALSE;
					}

#ifdef _DEBUG
					OutputDebugString(_T("FileList: FileList_WndProc(): ソート前のファイルリスト\r\n"));

					for(DWORD dwI = 0; dwI < MainWindow1.MD5Thread1.MultiFile->FileCount; dwI++)
					{
						_stprintf(szDebugText, _T("FileList: FileList_WndProc(): No.%d,%s\r\n"),
							dwI, PathFindFileName((MainWindow1.MD5Thread1.MultiFile->FileRecode + dwI)->FileName));
						szDebugText[1023] = '\0';
						OutputDebugString(szDebugText);
					}

					OutputDebugString(_T("FileList: FileList_WndProc(): ソート開始\r\n"));
#endif

					dwRet = ListView_SortItems(FileListWindow1.hList1, FileList_SortProc, pNMLV);

					if(dwRet)
					{
#ifdef _DEBUG
						OutputDebugString(_T("FileList: FileList_WndProc(): ソート完了\r\n"));
#endif
						DWORD dwFileCount	= MainWindow1.MD5Thread1.MultiFile->FileCount;
						DWORD dwNextFile	= dwFileCount;

						lvItem1.mask		= LVIF_PARAM;
						lvItem1.iSubItem	= 0;

#ifdef _DEBUG
						OutputDebugString(_T("FileList: FileList_WndProc(): ソートの結果を MD5Threadバッファに反映します。\r\n"));
						dwTime1 = timeGetTime();
#endif

						for(DWORD dwI = 0; dwI < dwFileCount; dwI++, dwNextFile++)
						{
							lvItem1.iItem  = dwI;
							dwRet = ListView_GetItem(FileListWindow1.hList1, &lvItem1);

							if(dwRet > 0)
							{
#ifdef _DEBUG
								_stprintf(szDebugText, _T("FileList: FileList_WndProc(): No.%d(%d),%s\r\n"),
									dwI, lvItem1.lParam, PathFindFileName((MainWindow1.MD5Thread1.MultiFile->FileRecode + lvItem1.lParam)->FileName));
								szDebugText[1023] = '\0';
								OutputDebugString(szDebugText);
#endif
								memcpy(MainWindow1.MD5Thread1.MultiFile->FileRecode + dwNextFile,
									MainWindow1.MD5Thread1.MultiFile->FileRecode + lvItem1.lParam,
									sizeof(tagMD5Thread_FileRecode));

								lvItem1.lParam = dwI;
								ListView_SetItem(FileListWindow1.hList1, &lvItem1);
							}
						}

						memmove(MainWindow1.MD5Thread1.MultiFile->FileRecode,
							MainWindow1.MD5Thread1.MultiFile->FileRecode + dwFileCount,
							sizeof(tagMD5Thread_FileRecode) * dwFileCount);

#ifdef _DEBUG
						dwTime2 = timeGetTime();
						_stprintf(szDebugText, _T("FileList: FileList_WndProc(): ソート処理完了  %dミリ秒\r\n"), 
							dwTime2 - dwTime1);
						szDebugText[1023] = '\0';
						OutputDebugString(szDebugText);
#endif
					}
#ifdef _DEBUG
					else
					{
						OutputDebugString(_T("FileList: FileList_WndProc(): ソート失敗\r\n"));
					}
#endif
					return TRUE;
				}
				else
				{
					LoadString(MainWindow1.hInst, MainWindow1.dwAddFileModeFileCount == 0 ? IDS_FILELIST_SORT1 : IDS_FILELIST_SORT2, FileListWindow1.szStBuf, MAX_STRINGTABLE);
					FileList_MessageBox(hDlg, FileListWindow1.szStBuf, MainWindow1.szTitle, MB_ICONHAND);
				}
				break;
			case LVN_BEGINDRAG:
				if(MainWindow1.dwAddFileModeFileCount == 0 &&
					~dwAppFrag & APP_MD5THREAD)
				{
					DWORD dwListSelect = (DWORD)-1;
					pNMLV = (NM_LISTVIEW*)lParam;

					GetCursorPos(&lvHitInfo.pt);
					ScreenToClient(FileListWindow1.hList1, &lvHitInfo.pt);
					ListView_SubItemHitTest(FileListWindow1.hList1, &lvHitInfo);

					if(lvHitInfo.iItem == -1)
					{
						return FALSE;
					}

					FileListWindow1.dwDragListNo = lvHitInfo.iItem;
					FileListWindow1.dwMoveListNo = (UINT)-1;
					dwCount = 0;
					for(;;)
					{
						dwListSelect = ListView_GetNextItem(FileListWindow1.hList1, dwListSelect, LVNI_ALL | LVNI_SELECTED);
						if(dwListSelect == -1)
						{
							break;
						}
						dwCount++;

						if(FileListWindow1.dwDragListNo != dwListSelect)
							ListView_SetItemState(FileListWindow1.hList1, dwListSelect, 0, LVIS_SELECTED | LVIS_FOCUSED);
					}

					ListView_SetItemState(FileListWindow1.hList1, FileListWindow1.dwDragListNo, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVNI_SELECTED);
					hDragImage = ListView_CreateDragImage(FileListWindow1.hList1, pNMLV->iItem, &pt);
					ScreenToClient(hDlg, &pt);
					ImageList_BeginDrag(hDragImage, 0, 0, 0);
					ImageList_DragEnter(hDlg, pt.x, pt.y);
					SetCapture(hDlg);
#ifdef _DEBUG
					_stprintf(szDebugText, _T("FileList: FileList_WndProc(): ドラッグ処理開始[%d]\r\n"),
						FileListWindow1.dwDragListNo);
					szDebugText[1023] = '\0';
					OutputDebugString(szDebugText);
#endif
					return TRUE;
				}
				break;
			case NM_CUSTOMDRAW:
				lpCustomDraw = (LPNMLVCUSTOMDRAW)lParam;

				switch(lpCustomDraw->nmcd.dwDrawStage)
				{
				case CDDS_PREPAINT:
					SetWindowLong(hDlg, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW);
					return TRUE;
				case CDDS_ITEMPREPAINT:
					if(lpCustomDraw->nmcd.dwItemSpec <= MainWindow1.MD5Thread1.MultiFile->FileCurrentCount)
					{
						lvItem1.mask     = LVIF_IMAGE;
						lvItem1.iItem    = (int)lpCustomDraw->nmcd.dwItemSpec;
						lvItem1.iSubItem = 0;
						lvItem1.iImage   = 0;
						
						if((MainWindow1.MD5Thread1.MultiFile->FileRecode + lpCustomDraw->nmcd.dwItemSpec)->MD5FileHashLine != MD5FILE_NOLINE)
						{
							if((MainWindow1.MD5Thread1.MultiFile->FileRecode + lpCustomDraw->nmcd.dwItemSpec)->FileLastError == 0)
							{
								if((MainWindow1.MD5Thread1.MultiFile->FileRecode + lpCustomDraw->nmcd.dwItemSpec)->MD5FileHashCmp == 0)
								{
									lpCustomDraw->clrText = (MainWindow1.dwAddFileModeFileCount != 0 && lpCustomDraw->nmcd.dwItemSpec < MainWindow1.dwAddFileModeFileCount) ?
										MainWindow1.GrayTextColor : FileListWindow1.ListTextColor[0];
									lvItem1.iImage = 1;
								}
								else
								{
									lpCustomDraw->clrText = FileListWindow1.ListTextColor[1];
									lvItem1.iImage = 2;
								}
							}
							else
							{
								if((MainWindow1.MD5Thread1.MultiFile->FileRecode + lpCustomDraw->nmcd.dwItemSpec)->FileLastError != -1)
								{
									lpCustomDraw->clrText = FileListWindow1.ListTextColor[2];
									lvItem1.iImage = 3;
								}
							}
							ListView_SetItem(FileListWindow1.hList1, &lvItem1);
							SetWindowLong(hDlg, DWLP_MSGRESULT, CDRF_NEWFONT);
						}
						else
						{
							if((MainWindow1.MD5Thread1.MultiFile->FileRecode + lpCustomDraw->nmcd.dwItemSpec)->FileLastError != 0 &&
								(MainWindow1.MD5Thread1.MultiFile->FileRecode + lpCustomDraw->nmcd.dwItemSpec)->FileLastError != -1)
							{
								lpCustomDraw->clrText = FileListWindow1.ListTextColor[2];//[MainWindow1.dwAddFileModeFileCount < lpCustomDraw->nmcd.dwItemSpec ? 2 : 5];
								lvItem1.iImage = 3;
							}
							else
							{
								lvItem1.iImage = 0;
							}
							ListView_SetItem(FileListWindow1.hList1, &lvItem1);
						}
						return TRUE;
					}
					break;
				}
				break;
			}
			break;
		}
		break;
	case WM_MOUSEMOVE:
		if(~dwAppFrag & APP_MD5THREAD &&
			GetCapture() == hDlg)
		{
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);

			ClientToScreen(hDlg, &pt);
			GetWindowRect(hDlg, &dlgRect);
			ImageList_DragMove(pt.x - dlgRect.left, pt.y - dlgRect.top);

			lvHitInfo.pt.x = pt.x;
			lvHitInfo.pt.y = pt.y;
			ScreenToClient(FileListWindow1.hList1, &lvHitInfo.pt);
			ListView_SubItemHitTest(FileListWindow1.hList1, &lvHitInfo);

			if(lvHitInfo.iItem != -1 &&
				FileListWindow1.dwDragListNo != (DWORD)lvHitInfo.iItem)
			{
				if(FileListWindow1.dwMoveListNo != -1)
				{
					ListView_SetItemState(FileListWindow1.hList1, FileListWindow1.dwMoveListNo, 0, LVIS_SELECTED | LVIS_FOCUSED | LVIS_DROPHILITED);
				}
				FileListWindow1.dwMoveListNo = lvHitInfo.iItem;
				ListView_SetItemState(FileListWindow1.hList1, FileListWindow1.dwMoveListNo, LVIS_DROPHILITED, LVIS_DROPHILITED);
			}
		}
		return TRUE;
	case WM_LBUTTONUP:
		if(~dwAppFrag & APP_MD5THREAD &&
			GetCapture() == hDlg)
		{
			ImageList_DragLeave(hDlg);
			ImageList_EndDrag();
			ImageList_Destroy(hDragImage);
			ReleaseCapture();

			GetCursorPos(&lvHitInfo.pt);
			ScreenToClient(FileListWindow1.hList1, &lvHitInfo.pt);
			ListView_SubItemHitTest(FileListWindow1.hList1, &lvHitInfo);
			ListView_SetItemState(FileListWindow1.hList1, -1, 0, LVIS_SELECTED | LVIS_DROPHILITED);

#ifdef _DEBUG
			_stprintf(szDebugText, _T("FileList: FileList_WndProc(): ドラッグ処理しています...[%d]\r\n"),
				lvHitInfo.iItem);
			szDebugText[1023] = '\0';
			OutputDebugString(szDebugText);
#endif

			if(lvHitInfo.iItem != -1 &&
				FileListWindow1.dwDragListNo != (DWORD)lvHitInfo.iItem)
			{
				FileListWindow1.dwMoveListNo = lvHitInfo.iItem;
				MD5_Thread_SwapItemBuffer(&MainWindow1.MD5Thread1, FileListWindow1.dwDragListNo, FileListWindow1.dwMoveListNo);

				FileList_SetListView(FileListWindow1.dwDragListNo, FALSE);
				FileList_SetListView(FileListWindow1.dwMoveListNo, FALSE);
				ListView_SetItemState(FileListWindow1.hList1, FileListWindow1.dwMoveListNo, LVIS_FOCUSED | LVIS_SELECTED , LVIS_FOCUSED | LVNI_SELECTED);

#ifdef _DEBUG
				OutputDebugString(_T("FileList: FileList_WndProc(): ドラッグ処理終了\r\n"));
#endif
				return TRUE;
			}
		}
		break;
	case WM_SYSCOLORCHANGE:
		SendMessage(FileListWindow1.hList1, WM_SYSCOLORCHANGE, 0, 0);
	case WM_CLOSE:
		if(dwAppFrag & APP_FOLDERINIT)
		{
			LoadString(MainWindow1.hInst, IDS_FILELIST_CLOSE, FileListWindow1.szStBuf, MAX_STRINGTABLE);
			FileList_MessageBox(hDlg, FileListWindow1.szStBuf, MainWindow1.szTitle, MB_ICONHAND);
			return FALSE;
		}
		DestroyWindow(hDlg);
		return TRUE;
	case WM_DESTROY:
		dwAppFrag &= ~(APP_FILELISTBOX | APP_WINDOW_NOALPHA);
		ImageList_Destroy(hImgS);
		_itot(FileListWindow1.dwOpenOption, FileListWindow1.szBuf, 10);
		WritePrivateProfileString(_T("FileList"), _T("OpenOption"), FileListWindow1.szBuf, MainWindow1.szINIFile);
		RestoreWindow_SaveState(hDlg, _T("FileList"), MainWindow1.szINIFile);
		if(FileListWindow1.FileListItem1.dwListSelectItem)
		{
			free(FileListWindow1.FileListItem1.dwListSelectItem);
			FileListWindow1.FileListItem1.dwListSelectItem = NULL;
		}
		FileListWindow1.FileListItem1.dwListBufSize = 0;
		FileListWindow1.szBuf	= NULL;
		DestroyMenu(FileListWindow1.hMenu1);
		FileListWindow1.hMenu1	= NULL;
		FileListWindow1.hWnd	= NULL;
		FileListWindow1.WindowMessage = (UINT)-1;
		return TRUE;
	}
	return (INT_PTR)FALSE;
}

int FileList_MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	int ret;

	// ウィンドウロックします。
	FILELIST_WINDOW_LOCK;

	ret = MessageBox(hWnd, lpText, lpCaption, uType);

	// ウィンドウロックの解除します。
	FILELIST_WINDOW_UNLOCK;

	return ret;
}

DWORD FileList_DropFile(HDROP hdropFile)
{
	DWORD dwRet = (DWORD)-1;
	DWORD dwCount;

	dwCount = DragQueryFile(hdropFile, (DWORD)-1, NULL, NULL);

	if(dwCount > 0)
	{
		BOOL IsMD5FileRet;

		MainWindow1.IsSubFolder		= -1;
		MainWindow1.IsEmptyFolder	= 0;


		for(UINT i = 0; i < dwCount; i++)
		{
			DragQueryFile(hdropFile, i, MainWindow1.szFile, MAX_PATH_SIZE);
			if(MainWindow1.MD5Thread1.MultiFile->FileCount >= FILE_MAX_COUNTSIZE)
			{
				LoadString(MainWindow1.hInst, IDS_OVERFILE1, FileListWindow1.szStBuf, MAX_STRINGTABLE);
				MessageFormat(FileListWindow1.szBuf, 1024, FileListWindow1.szStBuf, MainWindow1.szFile);
				FileList_MessageBox(FileListWindow1.hWnd, FileListWindow1.szBuf, MainWindow1.szTitle, MB_ICONHAND);
				dwRet = FILE_MAX_COUNTSIZE;

				break;
			}
			MainWindow1.MD5Thread1.IsFileNoCheck = 0;
			IsMD5FileRet = GetMD5FilePath();

			if(dwAppFrag & APP_FOLDEROPEN)
				dwRet = FileList_AddFolder();
			else if(IsMD5FileRet != FALSE)
				dwRet = FileList_AddMD5File();
			else
				dwRet = FileList_AddFile();

			if(dwRet == -1)
			{
				break;
			}
		}

		if(MainWindow1.MD5Thread1.MultiFile->FileCount > 0)
		{
			_tcscpy(MainWindow1.szOfn1Buf, (MainWindow1.MD5Thread1.MultiFile->FileRecode + MainWindow1.MD5Thread1.MultiFile->FileCount - 1)->FileName);
		}
	}

	return dwRet;
}


VOID FileList_FileMenu(HWND hWnd, DWORD dwItem, POINT *pt)
{
	DWORD dwMenuState;

	dwMenuState = (MainWindow1.MD5Thread1.MultiFile->FileRecode + dwItem)->FileLastError == 0 &&
		dwItem <= MainWindow1.MD5Thread1.MultiFile->FileCurrentCount ? (MF_BYPOSITION | MF_ENABLED) : (MF_BYPOSITION | MF_GRAYED);
	EnableMenuItem(FileListWindow1.hPupMenu1[0], 0, dwMenuState);
	EnableMenuItem(FileListWindow1.hPupMenu1[0], 3, dwMenuState);
	dwMenuState = dwAppFrag & APP_MD5THREAD ||
		(MainWindow1.dwAddFileModeFileCount != 0 && dwItem < MainWindow1.dwAddFileModeFileCount) ? (MF_BYPOSITION | MF_GRAYED) : (MF_BYPOSITION | MF_ENABLED);
	EnableMenuItem(FileListWindow1.hPupMenu1[0], 8, dwMenuState);						

	switch((MainWindow1.MD5Thread1.MultiFile->FileRecode + dwItem)->FileLastError)
	{
	case ERROR_FILE_NOT_FOUND:
	case ERROR_PATH_NOT_FOUND:
	case ERROR_TOO_MANY_OPEN_FILES:
		dwMenuState = (MF_BYPOSITION | MF_GRAYED);
		break;
	default:
		dwMenuState = (MF_BYPOSITION | MF_ENABLED);
	}

	EnableMenuItem(FileListWindow1.hPupMenu1[0], 10, dwMenuState);
	FileList_MenuShow(hWnd, FileListWindow1.hPupMenu1[0], pt);
}

VOID FileList_NoFileMenu(HWND hWnd, POINT *pt)
{
	DWORD dwClipCopy = 0;
	DWORD dwMenuState;


	if(dwAppFrag & APP_MD5THREAD)
	{
		MessageBeep(MB_ICONERROR);
		return;
	}

	dwMenuState = MF_BYPOSITION | MF_ENABLED;
	EnableMenuItem(FileListWindow1.hPupMenu1[1], 0, dwMenuState);
	EnableMenuItem(FileListWindow1.hPupMenu1[1], 1, dwMenuState);
	EnableMenuItem(FileListWindow1.hPupMenu1[1], 2, dwMenuState);

	dwMenuState = MainWindow1.MD5Thread1.MultiFile->FileCount == 0 ? (MF_BYPOSITION | MF_GRAYED) : (MF_BYPOSITION | MF_ENABLED);
	EnableMenuItem(FileListWindow1.hPupMenu1[1], 6, dwMenuState);

	if(OpenClipboard(FileListWindow1.hWnd))
	{
		HDROP hdropFile;

		hdropFile = (HDROP)GetClipboardData(CF_HDROP);
		if(hdropFile != NULL)
		{
			dwClipCopy = 1;
		}
		CloseClipboard();
	}
	dwMenuState = dwClipCopy == 0 ? (MF_BYPOSITION | MF_GRAYED) : (MF_BYPOSITION | MF_ENABLED);
	EnableMenuItem(FileListWindow1.hPupMenu1[1], 4, dwMenuState);
	FileList_MenuShow(hWnd, FileListWindow1.hPupMenu1[1], pt);
}

VOID FileList_MenuShow(HWND hWnd, HMENU hMenu, POINT *pt)
{
	DrawMenuBar(hWnd);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, pt->x, pt->y, 0, hWnd, NULL);
}

VOID FileList_FileDelete(VOID)
{
	DWORD dwCount;
	DWORD dwSelectItem;
	DWORD dwAddModeFileCount = 0;


	for(dwCount = FileListWindow1.FileListItem1.dwListSelectCount; dwCount > 0; dwCount--)
	{
		dwSelectItem = *(FileListWindow1.FileListItem1.dwListSelectItem + dwCount - 1);

		if(MainWindow1.dwAddFileModeFileCount != 0 &&
			dwSelectItem < MainWindow1.dwAddFileModeFileCount)
		{
			dwAddModeFileCount++;
		}
		else
		{
			ListView_DeleteItem(FileListWindow1.hList1, dwSelectItem);
			MD5_Thread_DeleteItemBuffer(&MainWindow1.MD5Thread1, dwSelectItem);
		}
	}

	if(dwAddModeFileCount > 0)
	{
		LoadString(MainWindow1.hInst, IDS_FILELIST_DELETE1, FileListWindow1.szStBuf, MAX_STRINGTABLE);
		FileList_MessageBox(FileListWindow1.hWnd, FileListWindow1.szStBuf, MainWindow1.szTitle, MB_ICONHAND);
	}

	if(MainWindow1.MD5Thread1.MultiFile->FileCount == 0)
	{
		FileList_Clear(TRUE);
	}
	else
	{
		dwAppFrag |= APP_FILELISTBOX_EDIT;
	}
	if(MainWindow1.dwOpenItem >= MainWindow1.MD5Thread1.MultiFile->FileCount != 0)
	{
		MainWindow1.dwOpenItem = MainWindow1.MD5Thread1.MultiFile->FileCount - 1;
	}
}

int CALLBACK FileList_BrowseCallbackProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);

	DWORD dwRet;

	switch(message)
	{
	case BFFM_INITIALIZED:
		SendMessage(hWnd, BFFM_SETSELECTION, TRUE, lParam);
		break;
	case BFFM_SELCHANGED:
		dwRet = SHGetPathFromIDList((LPITEMIDLIST)wParam, MainWindow1.szTempBuf);
		if(dwRet != NULL)
		{
			SendMessage(hWnd, BFFM_ENABLEOK, 0, *MainWindow1.szTempBuf != '\0' ? TRUE : FALSE);
			SendMessage(hWnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)MainWindow1.szTempBuf);
		}
		else
		{
			SendMessage(hWnd, BFFM_ENABLEOK, 0, FALSE);
			SendMessage(hWnd, BFFM_SETSTATUSTEXT, 0, NULL);
		}
		break;
	case BFFM_VALIDATEFAILED:
		return 1;
	}
    return 0;
}

int CALLBACK FileList_SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParam3)
{
	NMLISTVIEW *pNMLV = (NMLISTVIEW*)lParam3;
	int SubItem = pNMLV->iSubItem;
	int nItemScore1 = 0;
	int nItemScore2 = 0;
	LONGLONG llRet  = 0;
	LONGLONG llItem1;
	LONGLONG llItem2;
	static LV_FINDINFO lvf;
	static TCHAR szText1[MAX_LOADSTRING];
	static TCHAR szText2[MAX_LOADSTRING];
	static int nItem1;
	static int nItem2;
	__int64 nRet = 0;
#ifdef _DEBUG
	TCHAR *szSubItem[] = {_T("ファイル名"), _T("サイズ"), _T("MD5")};
	TCHAR szDebugText[1024];
#endif



	lvf.flags  = LVFI_PARAM;
	lvf.lParam = lParam1;
	nItem1 = ListView_FindItem(FileListWindow1.hList1, -1, &lvf);

	lvf.lParam = lParam2;
	nItem2 = ListView_FindItem(FileListWindow1.hList1, -1, &lvf);

#ifdef _DEBUG
	_stprintf(szDebugText, _T("FileList: File_SortProc(): ソートの項目[%s]: Item1[%d(%d)], Item2[%d(%d)]\r\n"),
		szSubItem[SubItem], lParam1, nItem1, lParam2, nItem2);
	szDebugText[1023] = '\0';
	OutputDebugString(szDebugText);
#endif


	if((MainWindow1.MD5Thread1.MultiFile->FileRecode + nItem1)->MD5FileHashLine != MD5FILE_NOLINE)
	{
		nItemScore1 += (MainWindow1.MD5Thread1.MultiFile->FileRecode + nItem1)->MD5FileHashCmp == 0 ? 10000 : 1000;
	}
	if((MainWindow1.MD5Thread1.MultiFile->FileRecode + nItem2)->MD5FileHashLine != MD5FILE_NOLINE)
	{
		nItemScore2 += (MainWindow1.MD5Thread1.MultiFile->FileRecode + nItem2)->MD5FileHashCmp == 0 ? 10000 : 1000;
	}

	if((MainWindow1.MD5Thread1.MultiFile->FileRecode + nItem1)->FileLastError == 0)
	{
		nItemScore1 += 100;
	}
	else
	{
		if((MainWindow1.MD5Thread1.MultiFile->FileRecode + nItem2)->FileLastError == -1 ||
			(MainWindow1.MD5Thread1.MultiFile->FileRecode + nItem1)->FileLastError > (MainWindow1.MD5Thread1.MultiFile->FileRecode + nItem2)->FileLastError)
			nItemScore1 += 10;
	}
	if((MainWindow1.MD5Thread1.MultiFile->FileRecode + nItem2)->FileLastError == 0)
	{
		nItemScore2 += 100;
	}
	else
	{
		if((MainWindow1.MD5Thread1.MultiFile->FileRecode + nItem1)->FileLastError == -1 ||
			(MainWindow1.MD5Thread1.MultiFile->FileRecode + nItem2)->FileLastError > (MainWindow1.MD5Thread1.MultiFile->FileRecode + nItem1)->FileLastError)
			nItemScore2 += 10;
	}

	switch(SubItem)
	{
	case 0:
	case 2:
	case 3:
		ListView_GetItemText(FileListWindow1.hList1, nItem1, SubItem, szText1, sizeof(szText1));
		szText1[MAX_LOADSTRING - 1] = '\0';
		ListView_GetItemText(FileListWindow1.hList1, nItem2, SubItem, szText2, sizeof(szText2));
		szText2[MAX_LOADSTRING - 1] = '\0';

#ifdef _DEBUG
		_stprintf(szDebugText, _T("FileList: File_SortProc(): 比較文字列: \"%10s\", \"%10s\"\r\n"),
			szText1, szText2);
		szDebugText[1023] = '\0';
		OutputDebugString(szDebugText);
#endif

		nRet = _tcscmp(szText1, szText2);

		if(nRet > 0)
		{
			nItemScore1--;
			nItemScore2++;
		}
		else if(nRet < 0)
		{
			nItemScore1++;
			nItemScore2--;
		}
		break;
	case 1:
		llItem1 = (MainWindow1.MD5Thread1.MultiFile->FileRecode + nItem1)->FileSize;
		llItem2 = (MainWindow1.MD5Thread1.MultiFile->FileRecode + nItem2)->FileSize;

		if(llItem1 != llItem2)
			llRet = llItem1 - llItem2;

#ifdef _DEBUG
		_stprintf(szDebugText, _T("FileList: File_SortProc(): 比較数値: %lld B, %lld B\r\n"),
			llItem1, llItem2);
		szDebugText[1023] = '\0';
		OutputDebugString(szDebugText);
#endif

		if(llRet > 0)
		{
			nItemScore1--;
			nItemScore2++;
		}
		else if(llRet < 0)
		{
			nItemScore1++;
			nItemScore2--;
		}
		break;
	}

	if(nItemScore1 != nItemScore2)
	{
		nRet = nItemScore1 - nItemScore2;
	}


	if(FileListWindow1.FileSortSubNo[SubItem])
	{
		nRet *= -1;
	}


#ifdef _DEBUG
	_stprintf(szDebugText, _T("FileList: FileList_SortProc(): ソート結果[%2s]: %I64d\r\n"),
		FileListWindow1.FileSortSubNo[SubItem] ? _T("昇順") : _T("降順"), nRet);
	szDebugText[1023] = '\0';
	OutputDebugString(szDebugText);
#endif

	return (int)nRet;
}

VOID FileListBoxInit(VOID)
{
	if(FileListWindow1.hWnd == NULL)
	{
		FileListWindow1.hWnd = CreateDialog(MainWindow1.hInst, MAKEINTRESOURCE(IDD_FILELIST), MainWindow1.hWnd, (DLGPROC)FileList_WndProc);
	}
}

VOID FileListBox(VOID)
{
	if(~dwAppFrag & APP_FILELISTBOX)
	{
		FileListBoxInit();

		if(FileListWindow1.hWnd != NULL)
		{
#ifdef _DEBUG
			OutputDebugString(_T("FileList: FileListBox()\r\n"));
#endif
			ShowWindow(FileListWindow1.hWnd, SW_SHOW);
			UpdateWindow(FileListWindow1.hWnd);
		}
	}
	else
	{
		if(FileListWindow1.hWnd != NULL)
		{
			SetForegroundWindow(FileListWindow1.hWnd);
		}
	}
}

VOID FileList_ListView(DWORD dwStart, BOOL IsDelete)
{
	DWORD dwCount;
	DWORD dwListCount = ListView_GetItemCount(FileListWindow1.hList1);


	if (MainWindow1.MD5Thread1.MultiFile->FileCount == 0 ||
		MainWindow1.MD5Thread1.MultiFile->FileCount > FileListWindow1.FileListItem1.dwListBufSize)
	{
		DWORD ___dwFileCount	= MainWindow1.MD5Thread1.MultiFile->FileCount;
		DWORD ___dwBufSize		= FileListWindow1.FileListItem1.dwListBufSize;
		DWORD *___dwNewItem		= NULL;
		DWORD *___dwOldItem		= FileListWindow1.FileListItem1.dwListSelectItem;


		if(MainWindow1.MD5Thread1.MultiFile->MaxBuffer > FILE_MAX_COUNTSIZE)
		{
			MainWindow1.MD5Thread1.MultiFile->MaxBuffer = dwListCount = FILE_MAX_COUNTSIZE;
			if(dwStart > FILE_MAX_COUNTSIZE)
			{
				dwStart = FILE_MAX_COUNTSIZE - 1;
			}
		}
		else
		{
			if(___dwBufSize == 0)
			{
				___dwBufSize = FILE_DEF_COUNTSIZE;
				FileListWindow1.FileListItem1.dwListSelectItem = ___dwOldItem = NULL;
			}

			// 大きめにバッファのサイズを計算する
			while(___dwBufSize < ___dwFileCount)
			{
				if(___dwBufSize < 4096)
					___dwBufSize *= FILE_DEF_COUNTSIZE;
				else
					___dwBufSize *= 2;
			}

			___dwNewItem = (DWORD*)malloc(___dwBufSize * sizeof(DWORD));

			if(___dwNewItem == NULL)
			{
				LoadString(MainWindow1.hInst, IDS_FILELIST_MALLOC_FAIL, FileListWindow1.szStBuf, MAX_STRINGTABLE);
				MessageFormat(FileListWindow1.szBuf, MAX_STRINGTABLE, FileListWindow1.szStBuf, _T("FileListWindow1.FileListItem1.dwListSelectItem"));
				FileList_MessageBox(FileListWindow1.hWnd, FileListWindow1.szBuf, MainWindow1.szTitle, MB_ICONHAND);

				if(FileListWindow1.hWnd != NULL)
				{
					DestroyWindow(FileListWindow1.hWnd);
				}
				return;
			}

			if(FileListWindow1.FileListItem1.dwListBufSize > 0)
			{
				memcpy(___dwNewItem, ___dwOldItem, MainWindow1.MD5Thread1.MultiFile->FileCount * sizeof(DWORD));
			}
			FileListWindow1.FileListItem1.dwListSelectItem	= ___dwNewItem;
			FileListWindow1.FileListItem1.dwListBufSize		= ___dwBufSize;

			if(___dwOldItem != NULL)
				free(___dwOldItem);
		}
	}


	// リストの削除
	if(IsDelete != 0)
	{
		ListView_DeleteAllItems(FileListWindow1.hList1);
		dwStart = 0;
	}

	for(dwCount = dwStart; dwCount < MainWindow1.MD5Thread1.MultiFile->FileCount; dwCount++)
	{
		FileList_SetListView(dwCount, (dwCount + 1) > dwListCount ? TRUE : FALSE);
	}

	if(MainWindow1.MD5Thread1.MultiFile->FileCurrentCount > 0)
	{
		DWORD dwI = MainWindow1.MD5Thread1.MultiFile->FileCount;

		dwI--;
		ListView_EnsureVisible(FileListWindow1.hList1, dwI, FALSE);
	}
	LoadString(MainWindow1.hInst, IDS_FILELIST_STATUS_TEXT1, FileListWindow1.szStBuf, MAX_STRINGTABLE);
	MessageFormat(FileListWindow1.szBuf, 1024, FileListWindow1.szStBuf, MainWindow1.MD5Thread1.MultiFile->FileCount);
	SendMessage(FileListWindow1.hStatus1, SB_SETTEXT, SB_SIMPLEID, (WPARAM)FileListWindow1.szBuf); 
}

VOID FileList_SetListView(DWORD dwItem, BOOL IsAdd)
{
	LV_ITEM lvItem;


	lvItem.mask     = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	lvItem.pszText  = PathFindFileName((MainWindow1.MD5Thread1.MultiFile->FileRecode + dwItem)->FileName);
	lvItem.iItem    = dwItem;
	lvItem.iSubItem = 0;
	lvItem.iImage   = 0;
	lvItem.lParam   = dwItem;
	if(IsAdd != FALSE)
		ListView_InsertItem(FileListWindow1.hList1, &lvItem);
	else
		ListView_SetItemText(FileListWindow1.hList1, dwItem, lvItem.iSubItem, lvItem.pszText);

	StrFormatKBSize((MainWindow1.MD5Thread1.MultiFile->FileRecode + dwItem)->FileSize, FileListWindow1.szBuf, MAX_LOADSTRING);
	lvItem.mask     = LVIF_TEXT;
	lvItem.pszText  = FileListWindow1.szBuf;
	lvItem.iItem    = dwItem;
	lvItem.iSubItem = 1;
	ListView_SetItem(FileListWindow1.hList1, &lvItem);

	if(dwItem <= MainWindow1.MD5Thread1.MultiFile->FileCurrentCount &&
		(MainWindow1.MD5Thread1.MultiFile->FileRecode + dwItem)->FileLastError != -1)
	{
		if((MainWindow1.MD5Thread1.MultiFile->FileRecode + dwItem)->FileLastError == 0)
		{
			MD5_Thread_MD5ToString(FileListWindow1.szFileChangeMD5, (MainWindow1.MD5Thread1.MultiFile->FileRecode + dwItem)->FileHashByte, ~dwAppFrag & APP_MD5OUT_LOWER);
		}
		else
		{
			GetLastError_String(FileListWindow1.szFileChangeMD5, (MainWindow1.MD5Thread1.MultiFile->FileRecode + dwItem)->FileLastError);
		}
		lvItem.pszText	= FileListWindow1.szFileChangeMD5;
		lvItem.iSubItem	= 2;
		ListView_SetItem(FileListWindow1.hList1, &lvItem);
	}

	if(IsAdd == FALSE)
	{
		ListView_Update(FileListWindow1.hList1, dwItem);
	}
}

VOID FileList_Clear(BOOL IsFilePathClear)
{
	dwAppFrag &= ~APP_FILELISTBOX_EDIT;

	if(MainWindow1.dwAddFileModeFileCount != 0)
	{
		int nRet;

		LoadString(MainWindow1.hInst, IDS_FILELIST_CLEAR1, FileListWindow1.szStBuf, MAX_STRINGTABLE);
		nRet = FileList_MessageBox(FileListWindow1.hWnd, FileListWindow1.szStBuf, MainWindow1.szTitle, MB_YESNO | MB_ICONQUESTION);

		if(nRet != IDYES)
		{
			return;
		}

		dwAppFrag &= ~APP_MD5FILE_ADDMODE;
		MainWindow1.dwAddFileModeFileCount								= 0;
		MainWindow1.dwOpenItem											= 0;
		MainWindow1.MD5Thread1.MultiFile->FileCount						= 0;
		MainWindow1.MD5Thread1.MultiFile->FileRecode->MD5FileHashLine	= MD5FILE_NOLINE;
		MD5_FileView(MainWindow1.dwOpenItem);
		SendMessage(MainWindow1.hProgress[0], PBM_SETPOS, 0, 0);
		EnableWindow(MainWindow1.hButton[0], TRUE);
		DragAcceptFiles(MainWindow1.hWnd, TRUE);
	}
	FileListWindow1.dwNewFile							= 0;
	FileListWindow1.dwFileCount							= 0;
	MainWindow1.MD5Thread1.MultiFile->FileCount			= 0;
	MainWindow1.MD5Thread1.MultiFile->FileCurrentCount	= 0;
	FileListWindow1.IsListLock = TRUE;
	ListView_DeleteAllItems(FileListWindow1.hList1);
	FileListWindow1.IsListLock = FALSE;
	LoadString(MainWindow1.hInst, IDS_FILELIST_STATUS_TEXT1, FileListWindow1.szStBuf, MAX_STRINGTABLE);
	MessageFormat(FileListWindow1.szBuf, 1024, FileListWindow1.szStBuf, 0);
	SendMessage(FileListWindow1.hStatus1, SB_SETTEXT, SB_SIMPLEID, (WPARAM)FileListWindow1.szBuf);
	EnableWindow(FileListWindow1.hButton1[0], FALSE);
	EnableWindow(FileListWindow1.hButton1[1], FALSE);
	EnableWindow(FileListWindow1.hButton1[2], TRUE);
	EnableWindow(FileListWindow1.hRadio1[0],  TRUE);
	EnableWindow(FileListWindow1.hRadio1[1],  TRUE);
	EnableWindow(FileListWindow1.hRadio1[2],  TRUE);
	EnableWindow(FileListWindow1.hButton1[3], FALSE);
	SetFocus(FileListWindow1.hButton1[2]);

	if(IsFilePathClear != 0)
	{
		*MainWindow1.szMD5File = *MainWindow1.szFile = '\0';
	}
}

VOID FileList_FileOpen(VOID)
{
	size_t szLen = _tcslen(MainWindow1.szOfn1Buf);

	// ウィンドウロックします。
	FILELIST_WINDOW_LOCK;

	// 最初のファイル名以降はゼロクリアする。
	memset(MainWindow1.szOfn1Buf + szLen * sizeof(TCHAR), NULL, ((MAX_PATH_SIZE + 100) * 256 - szLen) * sizeof(TCHAR));

	if(GetOpenFileName(&MainWindow1.ofn1))
	{
		TCHAR *pFileTextPos; 
		BOOL  IsMD5FileRet;

		// OpenFileNameの解析
		pFileTextPos = MainWindow1.szOfn1Buf + MainWindow1.ofn1.nFileOffset;

		for(;;)
		{
			MainWindow1.MD5Thread1.IsFileNoCheck = 0;
			if(_tfullpath(MainWindow1.szFile, pFileTextPos, MAX_PATH_SIZE * 2) != NULL)
			{
				*(MainWindow1.szFile + MAX_PATH_SIZE - 1) = '\0';
			}
			IsMD5FileRet = GetMD5FilePath();

			if(IsMD5FileRet != FALSE)
			{
				FileList_AddMD5File();
			}
			else
			{
				FileList_AddFile();
			}

			while(*pFileTextPos++ != NULL)
				;

			if(*pFileTextPos == NULL)
				break;
		}

		// 複数選択時に次回のフォルダー表示がおかしくならないように最後のファイルにしておきます。
		_tcscpy(MainWindow1.szOfn1Buf, (MainWindow1.MD5Thread1.MultiFile->FileRecode + MainWindow1.MD5Thread1.MultiFile->FileCount - 1)->FileName);
		_tcscpy(MainWindow1.szFile, MainWindow1.szOfn1Buf);
	}

	// ウィンドウロックの解除します。
	FILELIST_WINDOW_UNLOCK;
}

DWORD FileList_AddFile(VOID)
{
	DWORD dwFileCount = MainWindow1.MD5Thread1.MultiFile->FileCount;
	DWORD dwRet;

	dwRet = MD5_Thread_MultiFile_Init(&MainWindow1.MD5Thread1, MainWindow1.szFile, FileListWindow1.dwNewFile != 0 ? TRUE : FALSE);
	if(dwRet == -1)
	{
		FileRecodeFoul(FileListWindow1.hWnd);
		return (DWORD)-1;
	}
	if(dwFileCount == dwRet)
	{
		LoadString(MainWindow1.hInst, IDS_FILE_OPEN2, FileListWindow1.szStBuf, MAX_STRINGTABLE);
		MessageFormat(FileListWindow1.szBuf, 1024, FileListWindow1.szStBuf, MainWindow1.szFile);
		MainWindow1.IsSubFolder = FileList_MessageBox(FileListWindow1.hWnd, FileListWindow1.szBuf, MainWindow1.szTitle, MB_ICONHAND);
	}
	else
	{
		dwAppFrag |= APP_FILELISTBOX_EDIT;
		FileListWindow1.dwNewFile++;
	}

	FileList_ListView(dwRet - 1, FALSE);
	EnableWindow(FileListWindow1.hButton1[0], TRUE);
	EnableWindow(FileListWindow1.hButton1[1], TRUE);
	EnableWindow(FileListWindow1.hRadio1[0],  TRUE);
	EnableWindow(FileListWindow1.hRadio1[1],  TRUE);
	EnableWindow(FileListWindow1.hRadio1[2],  TRUE);
	EnableWindow(FileListWindow1.hButton1[3], FALSE);

	return dwRet;
}

VOID FileList_FolderOpen(VOID)
{
	BROWSEINFO		binfo;
	LPITEMIDLIST	idlist;

	LoadString(MainWindow1.hInst, IDS_FOLDER_OPEN1, FileListWindow1.szStBuf, MAX_STRINGTABLE);
	binfo.hwndOwner			= FileListWindow1.hWnd;
	binfo.pidlRoot			= NULL;
	binfo.pszDisplayName	= MainWindow1.MD5Thread1.MultiFile->FileCurrentFolder;
	binfo.lpszTitle			= FileListWindow1.szStBuf;
	binfo.ulFlags			= BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT | BIF_VALIDATE;
	binfo.lpfn				= (BFFCALLBACK)&FileList_BrowseCallbackProc;
	binfo.lParam			= (LPARAM)MainWindow1.MD5Thread1.MultiFile->FileCurrentFolder;
	binfo.iImage			= NULL;

	// ウィンドウロックします。
	FILELIST_WINDOW_LOCK;

	idlist = SHBrowseForFolder(&binfo);

	// ウィンドウロックの解除します。
	FILELIST_WINDOW_UNLOCK;

	if(idlist != NULL)
	{
#ifdef _DEBUG
		TCHAR szDebugText[1024];
#endif

		SHGetPathFromIDList(idlist, MainWindow1.szFile);

#ifdef _DEBUG
		_stprintf(szDebugText, _T("FileList: FileList_FolderOpen(): %s のフォルダーが選択されました。\r\n"), MainWindow1.szFile);
		szDebugText[1023] = '\0';
		OutputDebugString(szDebugText);
#endif

		MainWindow1.MD5Thread1.IsFileNoCheck = 0;
		GetMD5FilePath();
		FileList_AddFolder();
	}
	CoTaskMemFree(idlist);
}

DWORD FileList_AddFolder(VOID)
{
	DWORD dwFileCount = MainWindow1.MD5Thread1.MultiFile->FileCount;
	DWORD dwRet;

	dwAppFrag |= APP_FOLDEROPEN | APP_FOLDERINIT | APP_FILELISTBOX_EDIT;


	if(FileListWindow1.dwNewFile == 0)
	{
		MainWindow1.IsSubFolder = -1;
	}

	EnableWindow(FileListWindow1.hButton1[0], FALSE);
	EnableWindow(FileListWindow1.hButton1[1], FALSE);
	EnableWindow(FileListWindow1.hButton1[2], FALSE);
	EnableWindow(FileListWindow1.hButton1[3], FALSE);
	EnableWindow(FileListWindow1.hRadio1[0],  FALSE);
	EnableWindow(FileListWindow1.hRadio1[1],  FALSE);
	EnableWindow(FileListWindow1.hRadio1[2],  FALSE);

	if(MainWindow1.IsSubFolder == -1 &&
		GetSubFolderCount(MainWindow1.szFile) > 0)
	{
		LoadString(MainWindow1.hInst, IDS_FOLDER_OPEN2, FileListWindow1.szStBuf, MAX_STRINGTABLE);
		MessageFormat(FileListWindow1.szBuf, 1024, FileListWindow1.szStBuf, MainWindow1.szFile);
		MainWindow1.IsSubFolder = FileList_MessageBox(FileListWindow1.hWnd, FileListWindow1.szBuf, MainWindow1.szTitle, MB_YESNO | MB_ICONQUESTION);
	}
	dwRet = MD5_Thread_Folder_Init(&MainWindow1.MD5Thread1, MainWindow1.szFile, FileListWindow1.dwNewFile != 0, MainWindow1.IsSubFolder == IDYES);

	if(dwRet == -1)
	{
		FileRecodeFoul(FileListWindow1.hWnd);
		return (DWORD)-1;
	}
	if(dwFileCount == dwRet)
	{
		LoadString(MainWindow1.hInst, IDS_FOLDER_OPEN3, FileListWindow1.szStBuf, MAX_STRINGTABLE);
		MessageFormat(FileListWindow1.szBuf, 1024, FileListWindow1.szStBuf, MainWindow1.szFile);
		MainWindow1.IsSubFolder = FileList_MessageBox(FileListWindow1.hWnd, FileListWindow1.szBuf, MainWindow1.szTitle, MB_ICONHAND);
	}
	return dwRet;
}

VOID FileList_MD5FileOpen(VOID)
{
	// ウィンドウロックします。
	FILELIST_WINDOW_LOCK;

	MainWindow1.ofn2.Flags &= ~(OFN_ENABLEHOOK | OFN_ENABLETEMPLATE);

	if(GetOpenFileName(&MainWindow1.ofn2))
	{
		MainWindow1.MD5Thread1.IsFileNoCheck = 0;
		_tcscpy(MainWindow1.szFile, MainWindow1.szMD5File);
		GetMD5FilePath();
		FileList_AddMD5File();
	}
	else
	{
		// ウィンドウロックの解除します。
		FILELIST_WINDOW_UNLOCK;
	}

	SetFocus(FileListWindow1.hButton1[0]);
}

DWORD FileList_AddMD5File(VOID)
{
	TCHAR szOption[10];
	TCHAR *p = szOption;
	DWORD dwFileCount;
	DWORD dwRet;


	dwAppFrag |= APP_FILELISTBOX_EDIT;
	dwAppFrag &= ~APP_FOLDEROPEN;

	if(dwAppFrag & APP_MD5FILE_ADDMODE)
	{
		*p++ = 'a';
	}
	*p = '\0';

	dwFileCount	= MainWindow1.MD5Thread1.MultiFile->FileCount;
	dwRet		= MD5_Thread_MD5File_Init(&MainWindow1.MD5Thread1, MainWindow1.szFile, FileListWindow1.dwNewFile, szOption);

	// ウィンドウロックの解除します。
	FILELIST_WINDOW_UNLOCK;

	if(dwRet == -1)
	{
		FileRecodeFoul(FileListWindow1.hWnd);
		return (DWORD)-1;
	}
	else if(dwRet > 0)
	{
		FileListWindow1.dwNewFile++;
		MainWindow1.MD5Thread1.IsFileNoCheck = 1;

		if(dwAppFrag & APP_FILELISTBOX)
		{
			if(~dwAppFrag & APP_MD5FILE_ADDMODE)
			{
				EnableWindow(FileListWindow1.hButton1[0], TRUE);
			}
			EnableWindow(FileListWindow1.hButton1[1], TRUE);
			EnableWindow(FileListWindow1.hRadio1[0],  TRUE);
			EnableWindow(FileListWindow1.hRadio1[1],  TRUE);
			EnableWindow(FileListWindow1.hRadio1[2],  TRUE);
			EnableWindow(FileListWindow1.hButton1[3], FALSE);
		}
	}
	else if(dwAppFrag & APP_FILELISTBOX)
	{
		DWORD dwError = GetLastError();

		if(dwError == 0)
		{
			LoadString(MainWindow1.hInst, GetFileSize(MainWindow1.szFile) > 0 ? IDS_MD5FILE_OPEN1 : IDS_MD5FILE_OPEN2, FileListWindow1.szStBuf, MAX_STRINGTABLE);
			MessageFormat(FileListWindow1.szBuf, 1024, FileListWindow1.szStBuf, MainWindow1.szMD5File);
		}
		else
		{
			LoadString(MainWindow1.hInst, IDS_MD5FILE_OPEN3, FileListWindow1.szStBuf, MAX_STRINGTABLE);
			GetLastError_String(MainWindow1.szTempBuf, dwError);
			MessageFormat(FileListWindow1.szBuf, 1024, FileListWindow1.szStBuf, MainWindow1.szMD5File, dwError, MainWindow1.szTempBuf);
		}

		FileList_MessageBox(FileListWindow1.hWnd, FileListWindow1.szBuf, MainWindow1.szTitle, MB_OK | MB_ICONERROR);
	}
	return dwRet;
}

VOID FileRecodeFoul(HWND hWnd)
{
	LoadString(MainWindow1.hInst, IDS_FILELIST_MALLOC_FAIL, FileListWindow1.szStBuf, MAX_STRINGTABLE);
	MessageFormat(FileListWindow1.szBuf, MAX_STRINGTABLE, FileListWindow1.szStBuf, _T("pMD5Thread->MultiFile->FileRecode"));
	MessageBox(hWnd, FileListWindow1.szBuf, MainWindow1.szTitle, MB_ICONHAND);
	if(hWnd != NULL)
	{
		DestroyWindow(hWnd);
	}
}
