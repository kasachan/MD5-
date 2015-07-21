// OptionDlg.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "OptionDlg.h"

// グローバル変数の定義します:
OptionWindow OptionWindow1;

// 外部ファイルの変数定義します。
extern MainWindow		MainWindow1;
extern FileListWindow	FileListWindow1;
extern AboutWindow		AboutWindow1;
extern DWORD			dwAppFrag;


// オプション画面のメッセージ ハンドラです。
INT_PTR CALLBACK OptionDlg_WndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	size_t sSize;
	int wmId, wmEvent, i;
	static TCHAR *p;
	static TCHAR szBuf[MAX_STRINGTABLE * 2];


	switch(message)
	{
	case WM_INITDIALOG:
		dwAppFrag |= APP_WINDOW_NOALPHA;
		OptionWindow1.hWnd = hDlg;

		OptionWindow1.szBuf = szBuf;
		OptionWindow1.szChackSumFile = OptionWindow1.szBuf + MAX_STRINGTABLE;

		if(dwAppFrag & APP_FILELISTBOX)
		{
			EnableWindow(FileListWindow1.hWnd, FALSE);
		}
		if(dwAppFrag & APP_ABOUTBOX)
		{
			EnableWindow(AboutWindow1.hWnd, FALSE);
		}

		SetEnableThemeDialogTexture(hDlg);
		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)MainWindow1.hIcon);

		OptionWindow1.hGroup[0] = GetDlgItem(hDlg, IDC_OPTION_GROUP1);
		SendMessage(OptionWindow1.hGroup[0], WM_SETFONT, (WPARAM)MainWindow1.hFont[0], 0);

		for(i = 0; i < sizeof(OptionWindow1.hEdit) / sizeof(HWND); i++)
		{
			OptionWindow1.hEdit[i] = GetDlgItem(hDlg, IDC_OPTION_EDIT1 + i);
			SendMessage(OptionWindow1.hEdit[i], WM_SETFONT, (WPARAM)MainWindow1.hFont[0], 0);
		}

		OptionWindow1.hButton[0] = GetDlgItem(hDlg, IDC_OPTION_BUTTON1);
		SendMessage(OptionWindow1.hButton[0], WM_SETFONT, (WPARAM)MainWindow1.hFont[0], 0);

		SendMessage(GetDlgItem(hDlg, IDOK),		WM_SETFONT, (WPARAM)MainWindow1.hFont[0], 0);
		SendMessage(GetDlgItem(hDlg, IDCANCEL),	WM_SETFONT, (WPARAM)MainWindow1.hFont[0], 0);

		for(i = 0; i < sizeof(OptionWindow1.hCheckbox) / sizeof(HWND); i++)
		{
			OptionWindow1.hCheckbox[i] = GetDlgItem(hDlg, IDC_OPTION_CHECK1 + i);
			SendMessage(OptionWindow1.hCheckbox[i], WM_SETFONT, (WPARAM)MainWindow1.hFont[0], 0);
		}

		SendMessage(OptionWindow1.hCheckbox[0], BM_SETCHECK, dwAppFrag & APP_MD5OUT_LOWER ? BST_CHECKED : BST_UNCHECKED, 0);
		SendMessage(OptionWindow1.hCheckbox[1], BM_SETCHECK, dwAppFrag & APP_WINDOWALPHA ? BST_CHECKED : BST_UNCHECKED, 0);
		SendMessage(OptionWindow1.hCheckbox[2], BM_SETCHECK, dwAppFrag & APP_MD5FILE_ENABLE_HASH ? BST_CHECKED : BST_UNCHECKED, 0);
		SendMessage(OptionWindow1.hCheckbox[3], BM_SETCHECK, dwAppFrag & APP_ENABLE_FILELIST ? BST_CHECKED : BST_UNCHECKED, 0);
		SendMessage(OptionWindow1.hCheckbox[4], BM_SETCHECK, dwAppFrag & APP_FILE_NOCACHE ? BST_CHECKED : BST_UNCHECKED, 0);
		SendMessage(OptionWindow1.hCheckbox[5], BM_SETCHECK, dwAppFrag & APP_DISABLE_MULTIFILE_UPDATE_DRAW ? BST_CHECKED : BST_UNCHECKED, 0);
		SendMessage(OptionWindow1.hCheckbox[6], BM_SETCHECK, dwAppFrag & APP_ENABLE_HIDDENFILE ? BST_CHECKED : BST_UNCHECKED, 0);
		GetChackSumFile(OptionWindow1.szChackSumFile);
		SetWindowText(OptionWindow1.hEdit[0], OptionWindow1.szChackSumFile);
		SendMessage(OptionWindow1.hEdit[0], EM_LIMITTEXT, 255, 0);

		return (INT_PTR)TRUE;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 選択されたメニューの解析:
		switch(wmId)
		{
		case IDC_OPTION_BUTTON1:
			if(LoadString(MainWindow1.hInst, IDS_CHACKSUM_FILENAME, OptionWindow1.szBuf, MAX_STRINGTABLE) != 0)
			{
				sSize = _tcslen(OptionWindow1.szBuf);
				SetWindowText(OptionWindow1.hEdit[0], OptionWindow1.szBuf);
				SetFocus(OptionWindow1.hEdit[0]);
				SendMessage(OptionWindow1.hEdit[0], EM_SETSEL, (WPARAM)sSize, (LPARAM)sSize);
				dwAppFrag &= ~APP_EDIT_CHACKSUMFILE;
			}
			break;
		case IDOK:
			if(SendMessage(OptionWindow1.hCheckbox[0], BM_GETCHECK, 0, 0) == BST_CHECKED)
			{
				dwAppFrag |= APP_MD5OUT_LOWER;
				_tcslwr(MainWindow1.szMD5String);
			}
			else
			{
				dwAppFrag &= ~APP_MD5OUT_LOWER;
				_tcsupr(MainWindow1.szMD5String);
			}
			SetWindowText(MainWindow1.hEdit[0], MainWindow1.szMD5String);

			if(SendMessage(OptionWindow1.hCheckbox[1], BM_GETCHECK, 0, 0) == BST_CHECKED)
			{
				dwAppFrag |= APP_WINDOWALPHA;
				SetLayeredWindow(MainWindow1.hWnd);
			}
			else
			{
				dwAppFrag &= ~APP_WINDOWALPHA;
				SetAlphaWindow(MainWindow1.hWnd, 0, ACTIVE_ALPHA, LWA_ALPHA);
			}

			if(SendMessage(OptionWindow1.hCheckbox[2], BM_GETCHECK, 0, 0) == BST_CHECKED)
				dwAppFrag |= APP_MD5FILE_ENABLE_HASH;
			else
				dwAppFrag &= ~APP_MD5FILE_ENABLE_HASH;

			if(SendMessage(OptionWindow1.hCheckbox[3], BM_GETCHECK, 0, 0) == BST_CHECKED)
			{
				dwAppFrag |= APP_ENABLE_FILELIST;
				FileListBox();
			}
			else
				dwAppFrag &= ~APP_ENABLE_FILELIST;

			if(SendMessage(OptionWindow1.hCheckbox[4], BM_GETCHECK, 0, 0) == BST_CHECKED)
			{
				dwAppFrag |= APP_FILE_NOCACHE;
				MainWindow1.MD5Thread1.FileNoCache = TRUE;
			}
			else
			{
				dwAppFrag &= ~APP_FILE_NOCACHE;
				MainWindow1.MD5Thread1.FileNoCache = FALSE;
			}

			if(SendMessage(OptionWindow1.hCheckbox[5], BM_GETCHECK, 0, 0) == BST_CHECKED)
				dwAppFrag |= APP_DISABLE_MULTIFILE_UPDATE_DRAW;
			else
				dwAppFrag &= ~APP_DISABLE_MULTIFILE_UPDATE_DRAW;

			if(SendMessage(OptionWindow1.hCheckbox[6], BM_GETCHECK, 0, 0) == BST_CHECKED)
			{
				dwAppFrag |= APP_ENABLE_HIDDENFILE;
				MainWindow1.MD5Thread1.FileAttributeMask = 0xFFFFFFFF;
			}
			else
			{
				dwAppFrag &= ~APP_ENABLE_HIDDENFILE;
				MainWindow1.MD5Thread1.FileAttributeMask = (DWORD)DEF_FILE_ATTRIBUTE;
			}

			GetWindowText(OptionWindow1.hEdit[0], OptionWindow1.szBuf, MAX_STRINGTABLE);
			if(_tcsicmp(OptionWindow1.szBuf, OptionWindow1.szChackSumFile))
			{
				LoadString(MainWindow1.hInst, IDS_CHACKSUM_FILENAME, MainWindow1.szStBuf, MAX_STRINGTABLE);
				if(_tcsicmp(OptionWindow1.szBuf, MainWindow1.szStBuf))
				{
					_tcscpy(MainWindow1.szChackSumFile, OptionWindow1.szBuf);
					dwAppFrag |= APP_EDIT_CHACKSUMFILE;
				}
				else
				{
					_tcscpy(MainWindow1.szChackSumFile, MainWindow1.szStBuf);
					dwAppFrag &= ~APP_EDIT_CHACKSUMFILE;
				}
				SetChackSumFile();
			}
			SetIniFileSetting(MainWindow1.szINIFile);
			OptionWindow1.hWnd = NULL;

		case IDCANCEL:
			if(dwAppFrag & APP_FILELISTBOX)
			{
				EnableWindow(FileListWindow1.hWnd, TRUE);
			}
			if(dwAppFrag & APP_ABOUTBOX)
			{
				EnableWindow(AboutWindow1.hWnd, TRUE);
			}
			dwAppFrag &= ~APP_WINDOW_NOALPHA;
			OptionWindow1.szBuf = NULL;
			OptionWindow1.hWnd = NULL;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

VOID OptionBox(HWND hWnd)
{
	DialogBox(MainWindow1.hInst, MAKEINTRESOURCE(IDD_OPTION), hWnd, OptionDlg_WndProc);
}

VOID GetChackSumFile(TCHAR *inBuf)
{
	TCHAR *p1 = MainWindow1.szChackSumFile;
	TCHAR *p2 = inBuf;


#ifdef _UNICODE
	while(*(DWORD*)p1 != NULL)
#else
	while(*(WORD*)p1 != NULL)
#endif
	{
		if(*p1 == '\0')
		{
			p1++;
			*p2++ = ';';
		}
		else
		{
			*p2++ = *p1++;
		}
	}
	*p2 = '\0';
}

VOID SetChackSumFile(VOID)
{
	TCHAR **p1	= MainWindow1.pChackSumFile;
	TCHAR *p2	= MainWindow1.szChackSumFile;


	*p1++ = p2;
	while(*p2 != '\0')
	{
		if(*p2 == ';')
		{
			*p2 = '\0';
			*p1++ = p2 + 1;
		}
		p2++;
	}
	*p1 = '\0';
	*(p2 + 1) = '\0';
}

VOID GetIniFileSetting(const TCHAR *inFilePath)
{
	TCHAR szBuf[NUMBER_LOADSTRING];

	GetPrivateProfileString(_T("Option"), _T("EnableMD5Lower"), _T("0"), szBuf, 2, inFilePath);
	if(*szBuf != '0')
	{
		dwAppFrag |= APP_MD5OUT_LOWER;
	}
	else
	{
		dwAppFrag &= ~APP_MD5OUT_LOWER;
	}

	GetPrivateProfileString(_T("Option"), _T("EnableWindowAlpha"), _T("0"), szBuf, 2, inFilePath);
	if(*szBuf != '0')
	{
		dwAppFrag |= APP_WINDOWALPHA;
	}
	else
	{
		dwAppFrag &= ~APP_WINDOWALPHA;
	}

	GetPrivateProfileString(_T("Option"), _T("EnableMD5FileHash"), _T("0"), szBuf, 2, inFilePath);
	if(*szBuf != '0')
	{
		dwAppFrag |= APP_MD5FILE_ENABLE_HASH;
	}
	else
	{
		dwAppFrag &= ~APP_MD5FILE_ENABLE_HASH;
	}

	GetPrivateProfileString(_T("Option"), _T("EnableFileList"), _T("0"), szBuf, 2, inFilePath);
	if(*szBuf != '0')
	{
		dwAppFrag |= APP_ENABLE_FILELIST;
	}
	else
	{
		dwAppFrag &= ~APP_ENABLE_FILELIST;
	}

	GetPrivateProfileString(_T("Option"), _T("EnableFileNoCache"), _T("0"), szBuf, 2, inFilePath);
	if(*szBuf != '0')
	{
		MainWindow1.MD5Thread1.FileNoCache = TRUE;
		dwAppFrag |= APP_FILE_NOCACHE;
	}
	else
	{
		MainWindow1.MD5Thread1.FileNoCache = FALSE;
		dwAppFrag &= ~APP_FILE_NOCACHE;
	}

	GetPrivateProfileString(_T("Option"), _T("EnableOldMD5File"), _T("0"), szBuf, 2, inFilePath);
	if(*szBuf != '0')
	{
		dwAppFrag |= APP_OLDMD5FILE;
	}
	else
	{
		dwAppFrag &= ~APP_OLDMD5FILE;
	}

	GetPrivateProfileString(_T("Option"), _T("SaveMD5FileCharCode"), _T("0"), szBuf, 2, inFilePath);
	MainWindow1.dwSaveMD5FileCharCode = _ttoi(szBuf);
	if(MainWindow1.dwSaveMD5FileCharCode > 2)
	{
		MainWindow1.dwSaveMD5FileCharCode = 0;
	}

	GetPrivateProfileString(_T("Option"), _T("DisableMultiFileUpDateDraw"), _T("0"), szBuf, 2, inFilePath);
	if(*szBuf != '0')
	{
		dwAppFrag &= ~APP_DISABLE_MULTIFILE_UPDATE_DRAW;
	}
	else
	{
		dwAppFrag |= APP_DISABLE_MULTIFILE_UPDATE_DRAW;
	}

	GetPrivateProfileString(_T("Option"), _T("EnableHiddenFile"), _T("0"), szBuf, 2, inFilePath);
	if(*szBuf != '0')
	{
		dwAppFrag |= APP_ENABLE_HIDDENFILE;
		MainWindow1.MD5Thread1.FileAttributeMask = 0xFFFFFFFF;
	}
	else
	{
		dwAppFrag &= ~APP_ENABLE_HIDDENFILE;
		MainWindow1.MD5Thread1.FileAttributeMask = (DWORD)DEF_FILE_ATTRIBUTE;
	}

	GetPrivateProfileString(_T("Option"), _T("EnableFileMapping"), _T("0"), szBuf, 2, inFilePath);
	if(*szBuf != '0')
	{
		dwAppFrag |= APP_FILE_MAPPING;
	}
	else
	{
		dwAppFrag &= ~APP_FILE_MAPPING;
	}

	GetPrivateProfileString(_T("Option"), _T("ChackSumFile"), _T(""), MainWindow1.szBuf, MAX_STRINGTABLE, inFilePath);
	if(*MainWindow1.szBuf != '\0')
	{
		_tcscpy(MainWindow1.szChackSumFile, MainWindow1.szBuf);
		SetChackSumFile();
		dwAppFrag |= APP_EDIT_CHACKSUMFILE;
	}
	else
	{
		dwAppFrag &= ~APP_EDIT_CHACKSUMFILE;
	}

	GetPrivateProfileString(_T("FileList"), _T("ListViewTextColorMD5Comp"), _T(""), szBuf, 11, inFilePath);
	if(
#ifdef _UNICODE
		*(DWORD*)szBuf == 0x00780030
#else
		*(WORD*)szBuf == 0x7830
#endif
		)
	{
		_stscanf(szBuf + 2, _T("%x"), &FileListWindow1.ListTextColor[0]);
	}
	else
	{
		FileListWindow1.ListTextColor[0] = 0x00FF0000;
	}

	GetPrivateProfileString(_T("FileList"), _T("ListViewTextColorMD5CompFoul"), _T(""), szBuf, 11, inFilePath);
	if(
#ifdef _UNICODE
		*(DWORD*)szBuf == 0x00780030
#else
		*(WORD*)szBuf == 0x7830
#endif
		)
	{
		_stscanf(szBuf + 2, _T("%x"), &FileListWindow1.ListTextColor[1]);
	}
	else
	{
		FileListWindow1.ListTextColor[1] = 0x000000FF;
	}


	GetPrivateProfileString(_T("FileList"), _T("ListViewTextColorFileError"), _T(""), szBuf, 11, inFilePath);
	if(
#ifdef _UNICODE
		*(DWORD*)szBuf == 0x00780030
#else
		*(WORD*)szBuf == 0x7830
#endif
		)
	{
		_stscanf(szBuf + 2, _T("%x"), &FileListWindow1.ListTextColor[2]);
	}
	else
	{
		FileListWindow1.ListTextColor[2] = 0x000040FF;
	}
}

VOID SetIniFileSetting(const TCHAR *inFilePath)
{
	TCHAR szBuf[MAX_STRINGTABLE];

	WritePrivateProfileString(_T("Option"), _T("EnableMD5Lower"), dwAppFrag & APP_MD5OUT_LOWER ? _T("1") : _T("0"), inFilePath);
	WritePrivateProfileString(_T("Option"), _T("EnableWindowAlpha"), dwAppFrag & APP_WINDOWALPHA ? _T("1") : _T("0"), inFilePath);
	WritePrivateProfileString(_T("Option"), _T("EnableMD5FileHash"), dwAppFrag & APP_MD5FILE_ENABLE_HASH ? _T("1") : _T("0"), inFilePath);
	WritePrivateProfileString(_T("Option"), _T("EnableFileList"), dwAppFrag & APP_ENABLE_FILELIST ? _T("1") : _T("0"), inFilePath);
	WritePrivateProfileString(_T("Option"), _T("EnableFileNoCache"), dwAppFrag & APP_FILE_NOCACHE ? _T("1") : _T("0"), inFilePath);
	WritePrivateProfileString(_T("Option"), _T("EnableOldMD5File"), dwAppFrag & APP_OLDMD5FILE ? _T("1") : _T("0"), inFilePath);
	WritePrivateProfileString(_T("Option"), _T("DisableMultiFileUpDateDraw"), dwAppFrag & APP_DISABLE_MULTIFILE_UPDATE_DRAW ? _T("0") : _T("1"), inFilePath);
	_itot(MainWindow1.dwSaveMD5FileCharCode, szBuf, 10);
	szBuf[MAX_STRINGTABLE - 1] = '\0';
	WritePrivateProfileString(_T("Option"), _T("SaveMD5FileCharCode"), szBuf, inFilePath);
	WritePrivateProfileString(_T("Option"), _T("EnableHiddenFile"), dwAppFrag & APP_ENABLE_HIDDENFILE ? _T("1") : _T("0"), inFilePath);
	GetChackSumFile(szBuf);
	WritePrivateProfileString(_T("Option"), _T("ChackSumFile"), szBuf, inFilePath);

	_stprintf(szBuf, _T("0x%08X"), FileListWindow1.ListTextColor[0]);
	WritePrivateProfileString(_T("FileList"), _T("ListViewTextColorMD5Comp"), szBuf, inFilePath);
	_stprintf(szBuf, _T("0x%08X"), FileListWindow1.ListTextColor[1]);
	WritePrivateProfileString(_T("FileList"), _T("ListViewTextColorMD5CompFoul"), szBuf, inFilePath);
	_stprintf(szBuf, _T("0x%08X"), FileListWindow1.ListTextColor[2]);
	WritePrivateProfileString(_T("FileList"), _T("ListViewTextColorFileError"), szBuf, inFilePath);

	/*
	_stprintf(szBuf, _T("0x%08x"), dwAppFrag);
	WritePrivateProfileString(_T("Debug"), _T("dwAppFrag"), szBuf, inFilePath);
	*/
}
