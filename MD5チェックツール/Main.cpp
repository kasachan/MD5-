// Main.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "Main.h"
#include "aboutdlg.h"
#include "OptionDlg.h"
#include "FileListDlg.h"
#include "IniFilePath.h"
#include "hashthread.h"
#include "ToolTip.h"

// グローバル変数の定義します:
INITCOMMONCONTROLSEX InitCtrls;
MainWindow MainWindow1;
DWORD dwAppFrag  = 0;
DWORD dwAppState = (DWORD)-1;

// 外部ファイルの変数定義します。
extern FileListWindow	FileListWindow1;
extern AboutWindow		AboutWindow1;

#if _MSC_VER < 1500
typedef BOOL (WINAPI *PGPI)(HWND, COLORREF, BYTE, DWORD);
PGPI DllSetLayeredWindowAttributes;
#endif /* __MSC_VER < 1500 */

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: ここにコードを挿入してください。
	HACCEL	hAccelTable;
	MSG		msg			= {0};
	TCHAR	*pMsg		= NULL;
	TCHAR	*p;
	DWORD	err			= 0;
	DWORD	lerr		= 0;
	BOOL	MutexFoul	= 0;
	BOOL	IsTempFile	= 0;
	size_t	buflen		= 0;
	time_t	time1;
	struct	tm *ltm1;
	int		i			= 0;

#if _MSC_VER < 1500
	HMODULE hDll1	= NULL;
#endif /* __MSC_VER < 1500 */

#ifdef _DEBUG
	TCHAR szDebugText[1024];
#endif

	_tsetlocale(LC_ALL, _T("Japanese_Japan.932"));

	// オプション解析
	for(i = 1; i < __argc; i++)
	{
		if(*__targv[i] == '/')
		{
			TCHAR *szOption = __targv[i] + 1;

			for(; *szOption != NULL;)
			{
				switch(*szOption)
				{
				// MD5ファイルのみ追記モードで開きます。(ファイルの計算が省略されます
				case 'a':
					dwAppFrag |= APP_MD5FILE_ADDMODE;
					break;
				}
				szOption++;
			}
		}
	}


	buflen = ((
				(1024) +				// szBuf
				(MAX_STRINGTABLE) +		// szStBuf
				MAX_PATH_SIZE + 100 +	// szTempBuf
				(MAX_LOADSTRING) +		// szTitle
				(MAX_LOADSTRING) +		// szWindowClass
				(MAX_LOADSTRING) +		// szVersion
				(MAX_LOADSTRING) +		// szCopyRight
				(MAX_STRINGTABLE) +		// szStatusTitle
				HASH_LOADSTRING +		// szMD5String
				HASH_LOADSTRING +		// szMD5CompString
				NUMBER_LOADSTRING +		// szInputtext
				MAX_PATH_SIZE + 100 +	// szFile
				MAX_PATH_SIZE + 100 +	// szMD5File
				MAX_PATH_SIZE + 100 +	// szINIFile
				_MAX_EXT +				// szFileExtension
				MAX_STRINGTABLE +		// szChackSumFile
				((MAX_PATH_SIZE + 100) * 10) +	// szNotFoundFile
				((MAX_PATH_SIZE + 100) * 10) +	// szEmptyFolders
				MAX_LOADSTRING +		// szTimeText
				NUMBER_LOADSTRING +		// szTimeView1_Tenso
				NUMBER_LOADSTRING +		// szTimeView1_Size1
				NUMBER_LOADSTRING +		// szTimeView1_Size2
				NUMBER_LOADSTRING +		// szTimeView1_Time
				((MAX_PATH_SIZE + 100) * 256) +	// szOfn1Buf
				(MAX_STRINGTABLE * (sizeof(MainWindow1.szWindowTitle) / sizeof(TCHAR*))) +		// szWindowTitle[]
				(NUMBER_LOADSTRING * (sizeof(MainWindow1.szButtonText1) / sizeof(TCHAR*))) +	// szButtonText1[]
				(NUMBER_LOADSTRING * (sizeof(MainWindow1.szButtonText2) / sizeof(TCHAR*))) +	// szButtonText2[]
				(MAX_LOADSTRING) * (sizeof(MainWindow1.szToolTipText) / sizeof(TCHAR*)) +		// szToolTipText[]
				(NUMBER_LOADSTRING * (sizeof(MainWindow1.szStatusText) / sizeof(TCHAR*))) +		// szStatusText[]
				(MAX_LOADSTRING * (sizeof(MainWindow1.szTimeText1) / sizeof(TCHAR*)))) *		// szTimeText
					sizeof(TCHAR)) + ((

				MAX_LOADSTRING) *		// pChackSumFileName;
					sizeof(TCHAR*));

	MainWindow1.szBuf = (TCHAR*)malloc(buflen);
	if(MainWindow1.szBuf == NULL)
	{
		pMsg = _T("WinMain: MainWindow1.szBuf");
		err  = DEBUG_ALLOC_ERROR;
		goto APPEXIT;
	}
#ifdef _DEBUG
	_stprintf(szDebugText, _T("WinMain: %s のメモリー確保できました。(0x%08p, %dbyte)\r\n"), 
		_T("MainWindow1.szBuf"),
		MainWindow1.szBuf,
		buflen);
	szDebugText[1023] = '\0';
	OutputDebugString(szDebugText);
#endif

	*MainWindow1.szBuf				= '\0';
	MainWindow1.szStBuf				= 1024 + MainWindow1.szBuf;
	*MainWindow1.szStBuf			= '\0';
	MainWindow1.szTempBuf			= MAX_STRINGTABLE + MainWindow1.szStBuf;
	*MainWindow1.szTempBuf			= '\0';
	MainWindow1.szTitle				= MAX_PATH_SIZE + 100 + MainWindow1.szTempBuf;
	*MainWindow1.szTitle			= '\0';
	MainWindow1.szWindowClass		= MAX_LOADSTRING + MainWindow1.szTitle;
	*MainWindow1.szWindowClass		= '\0';
	MainWindow1.szVersion			= MAX_LOADSTRING + MainWindow1.szWindowClass;
	*MainWindow1.szVersion			= '\0';
	MainWindow1.szCopyRight			= MAX_LOADSTRING + MainWindow1.szVersion;
	*MainWindow1.szCopyRight		= '\0';
	MainWindow1.szStatusTitle		= MAX_LOADSTRING + MainWindow1.szCopyRight;
	*MainWindow1.szStatusTitle		= '\0';
	MainWindow1.szMD5String			= MAX_STRINGTABLE + MainWindow1.szStatusTitle;
	*MainWindow1.szMD5String		= '\0';
	MainWindow1.szMD5CompString		= HASH_LOADSTRING + MainWindow1.szMD5String;
	*MainWindow1.szMD5CompString	= '\0';
	MainWindow1.szInputtext			= HASH_LOADSTRING + MainWindow1.szMD5CompString;
	*MainWindow1.szInputtext		= '\0';
	MainWindow1.szFile				= NUMBER_LOADSTRING + MainWindow1.szInputtext;
	*MainWindow1.szFile				= '\0';
	MainWindow1.szMD5File			= MAX_PATH_SIZE + 100 + MainWindow1.szFile;
	*MainWindow1.szMD5File			= '\0';
	MainWindow1.szINIFile			= MAX_PATH_SIZE + 100 + MainWindow1.szMD5File;
	*MainWindow1.szINIFile			= '\0';
	MainWindow1.szFileExtension		= MAX_PATH_SIZE + 100 + MainWindow1.szINIFile;
	*MainWindow1.szFileExtension	= '\0';
	MainWindow1.szChackSumFile		= _MAX_EXT + MainWindow1.szFileExtension;
	*MainWindow1.szChackSumFile		= '\0';
	MainWindow1.pChackSumFile		= (TCHAR**)(MAX_STRINGTABLE + MainWindow1.szChackSumFile);
	memset(MainWindow1.pChackSumFile, NULL, MAX_LOADSTRING * sizeof(TCHAR*));
	MainWindow1.szNotFoundFile		= (TCHAR*)(MAX_LOADSTRING + MainWindow1.pChackSumFile);
	*MainWindow1.szNotFoundFile		= '\0';
	MainWindow1.szEmptyFolder		= ((MAX_PATH_SIZE + 100) * 10) + MainWindow1.szNotFoundFile;
	*MainWindow1.szEmptyFolder		= '\0';
	MainWindow1.szTimeText			= ((MAX_PATH_SIZE + 100) * 10) + MainWindow1.szEmptyFolder;
	*MainWindow1.szTimeText			= '\0';
	MainWindow1.szTimeView1_Tenso	= MAX_LOADSTRING + MainWindow1.szTimeText;
	*MainWindow1.szTimeView1_Tenso	= '\0';
	MainWindow1.szTimeView1_Size1	= NUMBER_LOADSTRING + MainWindow1.szTimeView1_Tenso;
	*MainWindow1.szTimeView1_Size1	= '\0';
	MainWindow1.szTimeView1_Size2	= NUMBER_LOADSTRING + MainWindow1.szTimeView1_Size1;
	*MainWindow1.szTimeView1_Size2	= '\0';
	MainWindow1.szTimeView1_Time	= NUMBER_LOADSTRING + MainWindow1.szTimeView1_Size2;
	*MainWindow1.szTimeView1_Time	= '\0';
	MainWindow1.szOfn1Buf			= NUMBER_LOADSTRING + MainWindow1.szTimeView1_Time;
	*MainWindow1.szOfn1Buf			= '\0';

	MainWindow1.szWindowTitle[0] = ((MAX_PATH_SIZE + 100) * 256) + MainWindow1.szOfn1Buf;
	LoadString(hInstance, IDS_WINDOW_TITLE1, MainWindow1.szWindowTitle[0], MAX_STRINGTABLE);
	for(i = 1; i < sizeof(MainWindow1.szWindowTitle) / sizeof(TCHAR*); i++)
	{
		MainWindow1.szWindowTitle[i] = MAX_STRINGTABLE + MainWindow1.szWindowTitle[i - 1];
		LoadString(hInstance, IDS_WINDOW_TITLE1 + i, MainWindow1.szWindowTitle[i], MAX_STRINGTABLE);
	}

	MainWindow1.szButtonText1[0] = MAX_STRINGTABLE + MainWindow1.szWindowTitle[sizeof(MainWindow1.szWindowTitle) / sizeof(TCHAR*) - 1];
	LoadString(hInstance, IDS_BUTTON1_TEXT1, MainWindow1.szButtonText1[0], NUMBER_LOADSTRING);
	for(i = 1; i < sizeof(MainWindow1.szButtonText1) / sizeof(TCHAR*); i++)
	{
		MainWindow1.szButtonText1[i] = NUMBER_LOADSTRING + MainWindow1.szButtonText1[i - 1];
		LoadString(hInstance, IDS_BUTTON1_TEXT1 + i, MainWindow1.szButtonText1[i], NUMBER_LOADSTRING);
	}

	MainWindow1.szButtonText2[0] = NUMBER_LOADSTRING + MainWindow1.szButtonText1[sizeof(MainWindow1.szButtonText1) / sizeof(TCHAR*) - 1];
	LoadString(hInstance, IDS_BUTTON2_TEXT1, MainWindow1.szButtonText2[0], NUMBER_LOADSTRING);
	for(i = 1; i < sizeof(MainWindow1.szButtonText2) / sizeof(TCHAR*); i++)
	{
		MainWindow1.szButtonText2[i] = NUMBER_LOADSTRING + MainWindow1.szButtonText2[i - 1];
		LoadString(hInstance, IDS_BUTTON2_TEXT1 + i, MainWindow1.szButtonText2[i], NUMBER_LOADSTRING);
	}

	MainWindow1.szToolTipText[0] = NUMBER_LOADSTRING + MainWindow1.szButtonText2[sizeof(MainWindow1.szButtonText2) / sizeof(TCHAR*) - 1];
	LoadString(hInstance, IDS_TOOLTIP_MSG1, MainWindow1.szToolTipText[0], MAX_LOADSTRING);
	for(i = 1; i < sizeof(MainWindow1.szToolTipText) / sizeof(TCHAR*); i++)
	{
		MainWindow1.szToolTipText[i] = MAX_LOADSTRING + MainWindow1.szToolTipText[i - 1];
		LoadString(hInstance, IDS_TOOLTIP_MSG1 + i, MainWindow1.szToolTipText[i], MAX_LOADSTRING);
	}

	MainWindow1.szStatusText[0] = MAX_LOADSTRING + MainWindow1.szToolTipText[sizeof(MainWindow1.szToolTipText) / sizeof(TCHAR*) - 1];
	LoadString(hInstance, IDS_STATUS1, MainWindow1.szStatusText[0], NUMBER_LOADSTRING);
	for(i = 1; i < sizeof(MainWindow1.szStatusText) / sizeof(TCHAR*); i++)
	{
		MainWindow1.szStatusText[i] = NUMBER_LOADSTRING + MainWindow1.szStatusText[i - 1];
		LoadString(hInstance, IDS_STATUS1 + i, MainWindow1.szStatusText[i], NUMBER_LOADSTRING);
	}

	MainWindow1.szTimeText1[0] = NUMBER_LOADSTRING + MainWindow1.szStatusText[sizeof(MainWindow1.szStatusText) / sizeof(TCHAR*) - 1];
	LoadString(hInstance, IDS_TIME_TEXT1, MainWindow1.szTimeText1[0], MAX_LOADSTRING);
	for(i = 1; i < sizeof(MainWindow1.szTimeText1) / sizeof(TCHAR*); i++)
	{
		MainWindow1.szTimeText1[i] = MAX_LOADSTRING + MainWindow1.szTimeText1[i - 1];
		LoadString(hInstance, IDS_TIME_TEXT1 + i, MainWindow1.szTimeText1[i], MAX_LOADSTRING);
	}

#if _MSC_VER < 1500
	if(GetVerifyVersionInfo(5, 0, 0))
	{
		UINT  nRet = 0, nLen;
		TCHAR *szPath;
		const TCHAR *szFile = _T("USER32.DLL");
		const char  *szFunc = "SetLayeredWindowAttributes";

		nLen = GetSystemDirectory(0, 0);

		if(nLen < 1)
			goto DLLFOUL;

		nLen  += (UINT)_tcslen(szFile) + 1;
		szPath = (TCHAR*)malloc(nLen * sizeof(TCHAR));
		if(szPath == NULL)
			goto DLLFOUL;

		nRet = GetSystemDirectory(szPath, nLen);

		if(nRet > 0)
		{
			TCHAR *p = szPath + nRet;

			*(p++) = _T('\\');
			_tcscpy(p, szFile);

			hDll1 = LoadLibrary(szPath);
			if(hDll1 != NULL)
			{
				DllSetLayeredWindowAttributes = reinterpret_cast<BOOL(WINAPI*)(HWND, COLORREF, BYTE, DWORD)>(GetProcAddress(hDll1, szFunc));
			}
		}
		free(szPath);
	}
DLLFOUL:
#endif /* __MSC_VER < 1500 */

	// グローバル文字列を初期化しています。
	MainWindow1.hInst = hInstance;// グローバル変数にインスタンス処理を格納します。

	LoadString(hInstance, IDS_APP_TITLE,			MainWindow1.szTitle,			MAX_LOADSTRING);
	LoadString(hInstance, IDC_MD5,					MainWindow1.szWindowClass,		MAX_LOADSTRING);
	LoadString(hInstance, IDS_STATUS_TITLE1,		MainWindow1.szStatusTitle,		MAX_STRINGTABLE);
	LoadString(hInstance, IDS_INPUT_TEXT,			MainWindow1.szInputtext,		NUMBER_LOADSTRING);
	LoadString(hInstance, IDS_FILE_EXTENSION,		MainWindow1.szFileExtension,	MAX_LOADSTRING);
	LoadString(hInstance, IDS_CHACKSUM_FILENAME,	MainWindow1.szChackSumFile,		MAX_STRINGTABLE);

	ZeroMemory(&MainWindow1.MD5Thread1, sizeof(MainWindow1.MD5Thread1));
	ZeroMemory(&MainWindow1.ofn1, sizeof(MainWindow1.ofn1));
	ZeroMemory(&MainWindow1.ofn2, sizeof(MainWindow1.ofn2));
	CoInitialize(NULL);

	MainWindow1.hMutex = CreateMutex(FALSE, 0, MainWindow1.szTitle);
	if(MainWindow1.hMutex == NULL ||
		GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MutexFoul++;
	}
	GetModuleFileName(NULL, MainWindow1.szTempBuf, MAX_PATH_SIZE + 100);

	// バージョンの更新
	LoadString(MainWindow1.hInst, IDS_VERSION, MainWindow1.szStBuf, MAX_STRINGTABLE);
	GetFileVersion(MainWindow1.szBuf, MainWindow1.szTempBuf, _T("\\StringFileInfo\\041104b0\\ProductVersion"));
	MessageFormat(MainWindow1.szVersion, 1024, MainWindow1.szStBuf, MainWindow1.szBuf);

	// Copyrightの更新
	GetFileVersion(MainWindow1.szCopyRight, MainWindow1.szTempBuf, _T("\\StringFileInfo\\041104b0\\LegalCopyright"));
	p = MainWindow1.szCopyRight + _tcslen(MainWindow1.szCopyRight) - 4;
	time(&time1);
	ltm1 = localtime(&time1);
	_itot(ltm1->tm_year + 1900, MainWindow1.szBuf, 10);
	_tcscpy(p, MainWindow1.szBuf);

	SetChackSumFile();
	LoadString(MainWindow1.hInst, IDS_INIFILE_NAME, MainWindow1.szStBuf, MAX_STRINGTABLE);
	GetIniFilePath(MainWindow1.szINIFile, MainWindow1.szStBuf, MainWindow1.szTitle);
	GetIniFileSetting(MainWindow1.szINIFile);
	GetIniFilePath(MainWindow1.MD5Thread1.FileMapping, _T("FileMapping.bin"), MainWindow1.szTitle);

	if(dwAppFrag & APP_FILE_MAPPING)
	{
		HANDLE hFile;

		MainWindow1.MD5Thread1.IsFileMapping = TRUE;
		hFile = CreateFile(MainWindow1.MD5Thread1.FileMapping, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			IsTempFile++;
			GetTempPath(MAX_PATH_SIZE, MainWindow1.szBuf);
			GetTempFileName(MainWindow1.szBuf, _T("MD5"), NULL, MainWindow1.MD5Thread1.FileMapping);
		}
		else
		{
			CloseHandle(hFile);
		}
	}
	if(dwAppFrag & APP_OLDMD5FILE)
	{
		MainWindow1.ofn2.nFilterIndex = 2;
	}
	MainWindow1.MD5Thread1.dwStructSize = sizeof(MD5Thread);
	if(MD5_Thread_Startup(&MainWindow1.MD5Thread1) == FALSE)
	{
		FileRecodeFoul(NULL);
		goto APPEXIT;
	}

	// ウィンドウ クラスを登録します。
	if(!MyRegisterClass(hInstance))
	{
		pMsg = _T("WinMain: ウィンドウクラスの登録");
		err  = DEBUG_MISSING;
		lerr = GetLastError();
		goto APPEXIT;
	}

	// アプリケーションの初期化を実行します:
	if(!InitInstance(nCmdShow))
	{
		if(MainWindow1.IsWindowCancel == FALSE)
		{
			pMsg = _T("WinMain: アプリケーションの初期化");
			err  = DEBUG_MISSING;
			lerr = GetLastError();
		}
		goto APPEXIT;
	}
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MD5));

	// メイン メッセージ ループ:
	while(GetMessage(&msg, NULL, 0, 0))
	{
		if(!TranslateAccelerator(MainWindow1.hWnd, hAccelTable, &msg) &&
			!IsDialogMessage(MainWindow1.hWnd, &msg))
		{
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		}
	}


	dwAppFrag |= APP_CALCEL | APP_EXIT;
	MainWindow1.MD5Thread1.SuspendMessage = APP_PRGRESS_EXIT;

	if(dwAppFrag & APP_MD5THREAD)
	{
		if(dwAppFrag & APP_PAUSE)
		{
			ResumeThread(MainWindow1.MD5Thread1.ProgressHandle);
			ResumeThread(MainWindow1.MD5Thread1.MD5ThreadHandle);
		}
		WaitForSingleObject(MainWindow1.MD5Thread1.MD5ThreadHandle, INFINITE);
		CloseHandle(MainWindow1.MD5Thread1.MD5ThreadHandle);
	}

APPEXIT:
	CoUninitialize();

#if _MSC_VER < 1500
	if(DllSetLayeredWindowAttributes != NULL)
	{
		FreeLibrary(hDll1);
	}
#endif /* __MSC_VER < 1500 */


	MD5_Thread_Shutdown(&MainWindow1.MD5Thread1);
	if(pMsg)
	{
		DebugText(err, pMsg, lerr);
	}

	if(MainWindow1.hMutex != NULL)
	{
		ReleaseMutex(MainWindow1.hMutex);
		CloseHandle(MainWindow1.hMutex);
	}
	if(IsTempFile != NULL)
	{
		DeleteFile(MainWindow1.MD5Thread1.FileMapping);
	}

	if(MainWindow1.szBuf)
	{
		free(MainWindow1.szBuf);
	}

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
	return err ? lerr : (int)msg.wParam;
}

//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
//  コメント:
//
//    この関数および使い方は、'RegisterClassEx' 関数が追加された
//    Windows 95 より前の Win32 システムと互換させる場合にのみ必要です。
//    アプリケーションが、関連付けられた
//    正しい形式の小さいアイコンを取得できるようにするには、
//    この関数を呼び出してください。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= DLGWINDOWEXTRA;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(MainWindow1.hInst, MAKEINTRESOURCE(IDI_MD5));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MD5);
	wcex.lpszClassName	= MainWindow1.szWindowClass;
	wcex.hIconSm		= LoadIcon(MainWindow1.hInst, MAKEINTRESOURCE(IDI_MD5));

	return RegisterClassEx(&wcex);
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します。
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(int nCmdShow)
{
	// アプリケーション マニフェストが visual スタイルを有効にするために、
	// ComCtl32.dll Version 6 以降の使用を指定する場合は、
	// Windows XP に InitCommonControlsEx() が必要です。さもなければ、ウィンドウ作成はすべて失敗します。

	InitCtrls.dwSize = sizeof(InitCtrls);
	// アプリケーションで使用するすべてのコモン コントロール クラスを含めるには、
	// これを設定します。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	MainWindow1.hWnd = CreateDialog(MainWindow1.hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)WndProc);

	if(!MainWindow1.hWnd)
	{
		return FALSE;
	}

	ShowWindow(MainWindow1.hWnd, nCmdShow);
	UpdateWindow(MainWindow1.hWnd);

	return TRUE;
}

//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:  メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND	- アプリケーション メニューの処理
//  WM_PAINT	- メイン ウィンドウの描画
//  WM_DESTROY	- 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent, i, dpi;
	DWORD dwRet, dwCount;
	DWORD dwStyle;
	HDC hDC;
	POINT pt;
	NONCLIENTMETRICS NCMetrics;
	LOGFONT lFont;
	TCHAR *p;
	static UINT wmTBC = (UINT)-1;
	static HRESULT hr3;
	static HBRUSH hbr1;
	static HWND h_NextWnd;
	static DWORD dwTime;

#ifdef _DEBUG
	TCHAR szDebugText[1024];
#endif

	if(message == MainWindow1.MD5Thread1.MessageID)
	{
		switch(wParam)
		{
		case APP_PRGRESSTHREAD_START:
#ifdef _DEBUG
			OutputDebugString(_T("MainWindow: WndProc(): プログレスのスレッド開始\r\n"));
#endif
			if(lParam != 0)
				dwAppFrag |= APP_PROGRESS_THREAD;
			else
				dwAppFrag &= ~APP_PROGRESS_THREAD;

			break;
		case APP_PRGRESSTHREAD_EXIT:
#ifdef _DEBUG
			OutputDebugString(_T("MainWindow: WndProc(): プログレスのスレッド終了\r\n"));
#endif
			break;
		case APP_MESSAGE_INIT:
#ifdef _DEBUG
			OutputDebugString(_T("MainWindow: WndProc(): MD5スレッドの準備中...\r\n"));
#endif
			dwAppFrag &= ~(APP_MD5_INPUT_EDIT | APP_LASTERROR | APP_CALCEL | APP_MD5FILE_ADDMODE);


			DragAcceptFiles(hWnd, FALSE);
			SendMessage(MainWindow1.hProgress[0], PBM_SETPOS, 0, 0);
			SendMessage(MainWindow1.hBmp[0], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)MainWindow1.hBitmap[3]);
			SetWindowText(hWnd, MainWindow1.szTitle);
			StatusOut(MainWindow1.szStatusText[0]);
			SetWindowText(MainWindow1.hStatic[1], _T(""));
			SetWindowText(MainWindow1.hButton[0], MainWindow1.szButtonText1[1]);
			SetWindowText(MainWindow1.hButton[1], MainWindow1.szButtonText2[1]);
			SetToolTipText(MainWindow1.hTool[0], MainWindow1.hButton[0], MainWindow1.szToolTipText[6]);
			SetToolTipText(MainWindow1.hTool[0], MainWindow1.hButton[1], MainWindow1.szToolTipText[7]);
			SetWindowText(MainWindow1.hEdit[0], _T(""));
			SetWindowText(MainWindow1.hEdit[1], _T(""));
			EnableWindow(MainWindow1.hEdit[0], FALSE);
			EnableWindow(MainWindow1.hEdit[1], FALSE);
			EnableWindow(MainWindow1.hButton[1], TRUE);
			EnableWindow(MainWindow1.hButton[2], FALSE);
			EnableWindow(MainWindow1.hButton[3], FALSE);
			EnableMenuItem(MainWindow1.hPupMenu1, 0, MF_BYPOSITION | MF_GRAYED);
			EnableMenuItem(MainWindow1.hPupMenu1, 1, MF_BYPOSITION | MF_ENABLED);
			EnableMenuItem(MainWindow1.hPupMenu1, 3, MF_BYPOSITION | MF_GRAYED);
			EnableMenuItem(MainWindow1.hPupMenu1, 4, MF_BYPOSITION | MF_GRAYED);
			EnableMenuItem(MainWindow1.hPupMenu1, 6, MF_BYPOSITION | MF_GRAYED);
			EnableMenuItem(MainWindow1.hPupMenu1, 7, MF_BYPOSITION | MF_GRAYED);
			EnableMenuItem(MainWindow1.hPupMenu1, 9, MF_BYPOSITION | MF_GRAYED);
			EnableMenuItem(MainWindow1.hPupMenu1, 14, MF_BYPOSITION | MF_GRAYED);

			*MainWindow1.szTimeText = '\0';
			SetFileInfo(0);
			MainWindow1.dwTimeViewFileCount = 0;
			SetTimer(hWnd, TIMER, 500, NULL);
			MainWindow1.dwStartTime = timeGetTime();
			dwTime = MainWindow1.dwStartTime;

			if(MainWindow1.pTaskbarList3 != NULL)
			{
				MainWindow1.pTaskbarList3->SetProgressState(hWnd, TBPF_INDETERMINATE);
			}
			SetFocus(MainWindow1.hButton[0]);
			break;
		case APP_MESSAGE_MULTIFILE:
			if((~dwAppFrag & APP_FILELISTBOX && MainWindow1.MD5Thread1.MultiFile->FileCount > 1) ||
				(dwAppFrag & APP_FILELISTBOX && FileListWindow1.dwNewFile == 0))
			{
				FileListBox();
			}
			break;
		case APP_MESSAGE_FILECHANGE:
			MainWindow1.dwOpenItem = lParam;
			if(~dwAppFrag & APP_DISABLE_MULTIFILE_UPDATE_DRAW)
			{
				SetFileInfo(lParam);
			}
			break;
		case APP_MESSAGE_FILEEND:
			break;
		case APP_MESSAGE_FILEOPEN:
			break;
		case APP_MESSAGE_FILECLOSE:
#ifdef _DEBUG
			OutputDebugString(_T("MainWindow: WndProc(): MD5スレッドのファイルのクローズが完了しました。\r\n"));
#endif
			break;
		case APP_MESSAGE_FOLDER_INIT:
#ifdef _DEBUG
			OutputDebugString(_T("MainWindow: WndProc(): MD5スレッドのフォルダーを開いています。\r\n"));
#endif
			dwAppFrag |= APP_FOLDERINIT;

			dwStyle = GetWindowLong(MainWindow1.hProgress[0], GWL_STYLE);
			dwStyle |= PBS_MARQUEE;
			SetWindowLong(MainWindow1.hProgress[0], GWL_STYLE, dwStyle);
			SendMessage(MainWindow1.hProgress[0], PBM_SETMARQUEE, TRUE, 0);

			EnableWindow(MainWindow1.hButton[0], FALSE);
			EnableWindow(MainWindow1.hButton[1], FALSE);
			if(MainWindow1.pTaskbarList3 != NULL)
			{
				MainWindow1.pTaskbarList3->SetProgressState(hWnd, TBPF_INDETERMINATE);
			}
			break;
		case APP_MESSAGE_SUBFOLDER_OPEN:
#ifdef _DEBUG
			_stprintf(MainWindow1.szBuf, _T("MainWindow: WndProc(): MD5スレッドのサブフォルダーを展開しています。 (%d)\r\n"), lParam);
			MainWindow1.szBuf[1023] = '\0';
			OutputDebugString(MainWindow1.szBuf);
#endif
			break;
		case APP_MESSAGE_FOLDER_INIT_EXIT:
#ifdef _DEBUG
			OutputDebugString(_T("MainWindow: WndProc(): MD5スレッドのフォルダー展開が完了しました。\r\n"));
#endif
			if((DWORD)lParam == -1)
			{
				FileRecodeFoul(hWnd);
				return FALSE;
			}

			dwAppFrag &= ~APP_FOLDERINIT;

			dwStyle = GetWindowLong(MainWindow1.hProgress[0], GWL_STYLE);
			dwStyle &= ~PBS_MARQUEE;
			SetWindowLong(MainWindow1.hProgress[0], GWL_STYLE, dwStyle);

			EnableWindow(MainWindow1.hButton[0], TRUE);
			if (MainWindow1.pTaskbarList3 != NULL)
			{
				MainWindow1.pTaskbarList3->SetProgressState(hWnd, TBPF_NOPROGRESS);
			}
			break;
		case APP_MESSAGE_MD5FILE_INIT:
#ifdef _DEBUG
			OutputDebugString(_T("MainWindow: WndProc(): MD5ファイルを開いています...\r\n"));
#endif
			break;
		case APP_MESSAGE_MD5FILE_INIT_EXIT:
#ifdef _DEBUG
			OutputDebugString(_T("MainWindow: WndProc(): MD5ファイルが開き終わりました。\r\n"));
#endif
			break;
		case APP_MESSAGE_START:
			//処理開始
#ifdef _DEBUG
			OutputDebugString(_T("MainWindow: WndProc(): MD5ハッシュスレッドが開始しました。\r\n"));
#endif
			dwAppFrag &= ~(APP_LASTERROR | APP_CALCEL);
			MainWindow1.dwStartTime = timeGetTime();

			if(MainWindow1.pTaskbarList3 != NULL)
			{
				MainWindow1.pTaskbarList3->SetProgressState(hWnd, TBPF_NORMAL);
			}
			break;
		case APP_MESSAGE_PROGRESS:
			//処理中の表示
			if(lParam < 100)
			{
				SendMessage(MainWindow1.hProgress[0], PBM_SETPOS, lParam, 0);
				MainWindow1.dwPersent = lParam;

				MessageFormat(MainWindow1.szBuf, 1024, MainWindow1.szWindowTitle[1], (int)lParam, MainWindow1.szTitle);
				SetWindowText(hWnd, MainWindow1.szBuf);
				if(MainWindow1.pTaskbarList3 != NULL)
				{
					MainWindow1.pTaskbarList3->SetProgressValue(hWnd, lParam, 100);
				}
			}
			break;
		case APP_MESSAGE_END:
			//終了の表示、リセット
#ifdef _DEBUG
			_stprintf(szDebugText, _T("MainWindow: WndProc(): MD5ハッシュスレッドが正常に終了しました。(%d)\r\n"), lParam);
			szDebugText[1023] = '\0';
			OutputDebugString(szDebugText);
#endif
			dwAppState = 0;
			SendMessage(MainWindow1.hBmp[0], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)MainWindow1.hBitmap[1]);
			SetWindowText(hWnd, MainWindow1.szTitle);
			SendMessage(MainWindow1.hProgress[0], PBM_SETPOS, 100, 0);
			MainWindow1.dwOpenItem = lParam;
			MD5_FileView(MainWindow1.dwOpenItem);
			EnableMenuItem(MainWindow1.hPupMenu1, 4, MF_BYPOSITION | MF_ENABLED);
			EnableMenuItem(MainWindow1.hPupMenu1, 9, MF_BYPOSITION | MF_ENABLED);
			EnableMenuItem(MainWindow1.hPupMenu1, 14, MF_BYPOSITION | MF_ENABLED);

			if(dwAppFrag & APP_MD5FILE_ADDMODE)
			{
				EnableMenuItem(MainWindow1.hPupMenu1, 0, MF_BYPOSITION | MF_GRAYED);
				StatusOut(MainWindow1.szStatusText[8]);
				LoadString(MainWindow1.hInst, IDS_STATIC_TEXT4, MainWindow1.szStBuf, MAX_STRINGTABLE);
				SetWindowText(MainWindow1.hStatic[1], MainWindow1.szStBuf);
				EnableWindow(MainWindow1.hButton[1], FALSE);

				SendMessage(MainWindow1.hEdit[1], WM_SETFONT, (WPARAM)MainWindow1.hFont[1], 0);
				SetWindowText(MainWindow1.hEdit[1], MainWindow1.szInputtext);
				MessageBeep(MB_ICONASTERISK);
				SetFocus(MainWindow1.hButton[5]);
			}
			else
			{
				StatusOut(MainWindow1.szStatusText[1]);

				if((MainWindow1.MD5Thread1.MultiFile->FileRecode + MainWindow1.dwOpenItem)->MD5FileHashLine != MD5FILE_NOLINE)
				{
					MD5_HashComp();
				}
				else
				{
					SendMessage(MainWindow1.hEdit[1], WM_SETFONT, (WPARAM)MainWindow1.hFont[1], 0);
					SetWindowText(MainWindow1.hEdit[1], MainWindow1.szInputtext);
				}

				KillTimer(hWnd, TIMER);
				MD5_TimeView(TRUE);
				if((MainWindow1.MD5Thread1.MultiFile->FileRecode + MainWindow1.dwOpenItem)->MD5FileHashLine == MD5FILE_NOLINE)
				{
					MessageBeep(MB_ICONASTERISK);
				}
				SetWindowText(MainWindow1.hButton[0], MainWindow1.szButtonText1[0]);
				SetWindowText(MainWindow1.hButton[1], MainWindow1.szButtonText2[0]);
				SetToolTipText(MainWindow1.hTool[0], MainWindow1.hButton[0], MainWindow1.szToolTipText[0]);
				SetToolTipText(MainWindow1.hTool[0], MainWindow1.hButton[1], MainWindow1.szToolTipText[1]);

				if(MainWindow1.pTaskbarList3 != NULL)
				{
					MainWindow1.pTaskbarList3->SetProgressState(hWnd, TBPF_NOPROGRESS);
				}

				WaitForSingleObject(MainWindow1.MD5Thread1.MD5ThreadHandle, INFINITE);
				CloseHandle(MainWindow1.MD5Thread1.MD5ThreadHandle);
				MainWindow1.MD5Thread1.MD5ThreadHandle = NULL;

				if(MainWindow1.dwAddFileModeFileCount == 0)
				{
					DragAcceptFiles(hWnd, TRUE);
				}
				SetFocus(MainWindow1.hButton[0]);
			}

			dwAppFrag &= ~(APP_MD5THREAD | APP_PROGRESS_THREAD);
			SetForegroundWindow(dwAppFrag & APP_FILELISTBOX ? FileListWindow1.hWnd : hWnd);
			break;
		case APP_MESSAGE_ERROR:
			//中断の表示、リセット
#ifdef _DEBUG
			_stprintf(szDebugText, _T("MainWindow: WndProc(): MD5ハッシュスレッドが異常終了しました。(%d)\r\n"), lParam);
			szDebugText[1023] = '\0';
			OutputDebugString(szDebugText);
#endif
			dwAppFrag &= ~(APP_MD5THREAD | APP_PROGRESS_THREAD);
			dwAppState = lParam;

			KillTimer(hWnd, TIMER);
			WaitForSingleObject(MainWindow1.MD5Thread1.MD5ThreadHandle, INFINITE);
			CloseHandle(MainWindow1.MD5Thread1.MD5ThreadHandle);
			MainWindow1.MD5Thread1.MD5ThreadHandle = NULL;

			if(MainWindow1.dwAddFileModeFileCount == 0)
			{
				DragAcceptFiles(hWnd, TRUE);
			}

			MainWindow1.dwBackTime = 0;
			SetWindowText(MainWindow1.hButton[0], MainWindow1.szButtonText1[0]);
			SetWindowText(MainWindow1.hButton[1], MainWindow1.szButtonText2[0]);
			SetToolTipText(MainWindow1.hTool[0], MainWindow1.hButton[0], MainWindow1.szToolTipText[0]);
			SetToolTipText(MainWindow1.hTool[0], MainWindow1.hButton[1], MainWindow1.szToolTipText[1]);
			EnableMenuItem(MainWindow1.hPupMenu1, 14, MF_BYPOSITION | MF_ENABLED);

			if(dwAppFrag & APP_FILE_CLEAR)
			{
				Clear();
			}
			else
			{
				SendMessage(MainWindow1.hBmp[0], STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)MainWindow1.hBitmap[2]);
				MainWindow1.dwOpenItem = MD5_Thread_GetCurrentItem(&MainWindow1.MD5Thread1);
				MD5_FileView(MainWindow1.dwOpenItem);

				if(MainWindow1.pTaskbarList3 != NULL)
				{
					MainWindow1.pTaskbarList3->SetProgressState(hWnd, TBPF_ERROR);
				}
			}

			SetForegroundWindow(dwAppFrag & APP_FILELISTBOX ? FileListWindow1.hWnd : hWnd);
			SetFocus(MainWindow1.hButton[0]);
			break;
		case APP_MESSAGE_PAUSE:
			if(dwAppFrag & APP_MD5THREAD)
			{
				if(dwAppFrag & APP_PAUSE)
				{
					dwAppFrag &= ~APP_PAUSE;
					MainWindow1.MD5Thread1.SuspendMessage &= ~APP_PRGRESS_PAUSE;
					ResumeThread(MainWindow1.MD5Thread1.MD5ThreadHandle);
					MainWindow1.dwStartTime = timeGetTime();
					MainWindow1.dwEndTime   = 0;
					StatusOut(MainWindow1.szStatusText[0]);
					SetWindowText(MainWindow1.hButton[0], MainWindow1.szButtonText1[1]);
					SetToolTipText(MainWindow1.hTool[0], MainWindow1.hButton[0], MainWindow1.szToolTipText[6]);
					SetToolTipText(MainWindow1.hTool[0], MainWindow1.hButton[1], MainWindow1.szToolTipText[7]);
					SendMessage(MainWindow1.hBmp[0], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)MainWindow1.hBitmap[3]);
					if(MainWindow1.pTaskbarList3 != NULL)
					{
						MainWindow1.pTaskbarList3->SetProgressState(hWnd, TBPF_NORMAL);
					}
				}
				else
				{
					dwAppFrag |= APP_PAUSE;
					MainWindow1.MD5Thread1.SuspendMessage |= APP_PRGRESS_PAUSE;
					SuspendThread(MainWindow1.MD5Thread1.MD5ThreadHandle);
					StatusOut(MainWindow1.szStatusText[2]);
					SetWindowText(MainWindow1.hButton[0], MainWindow1.szButtonText1[2]);
					SetToolTipText(MainWindow1.hTool[0], MainWindow1.hButton[0], MainWindow1.szToolTipText[8]);
					SetToolTipText(MainWindow1.hTool[0], MainWindow1.hButton[1], MainWindow1.szToolTipText[7]);
					SendMessage(MainWindow1.hBmp[0], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)MainWindow1.hBitmap[6]);
					if(MainWindow1.pTaskbarList3 != NULL)
					{
						MainWindow1.pTaskbarList3->SetProgressState(hWnd, TBPF_PAUSED);
					}
				}
			}
			break;
		case APP_MESSAGE_CHANGE_FILEVIEW:
			if(~dwAppFrag & APP_MD5THREAD)
			{
				MainWindow1.dwOpenItem = lParam;
				MD5_FileView(lParam);
			}
			break;
		case APP_MESSAGE_LASTERROR:
			dwAppFrag |= APP_LASTERROR;
			MainWindow1.dwLasterror = (DWORD)lParam;
			break;
		}
		if(dwAppFrag & APP_FILELISTBOX)
		{
			SendMessage(FileListWindow1.hWnd, FileListWindow1.WindowMessage, wParam, lParam);
		}
		return TRUE;
	}
	else if(message == wmTBC)
	{
#ifdef _DEBUG
		OutputDebugString(_T("MainWindow: WndProc(): TaskbarButtonCreated のメッセージを受信しました。\r\n"));
#endif /* _DEBUG */
		hr3 = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&MainWindow1.pTaskbarList3));
		if(SUCCEEDED(hr3))
			return TRUE;
		else
			return FALSE;
	}

	switch(message)
	{
	case WM_INITDIALOG:
		/* ダイアログの初期化 ここから */
		MainWindow1.hWnd = hWnd;

		SetEnableThemeDialogTexture(hWnd);
		SetWindowText(hWnd, MainWindow1.szTitle);
		MainWindow1.hIcon = LoadIcon(MainWindow1.hInst, MAKEINTRESOURCE(IDI_MD5));
		hDC = GetDC(hWnd);

		if(hDC != NULL)
		{
			HDC		hMemDC, hTmpDC;
			HBITMAP	hTmpBMP;
			int		IconSize;

			dpi = GetDeviceCaps(hDC, LOGPIXELSY);
			// dpi の取得
			//
			// 小 - 100% ...  96dpi
			// 中 - 125% ... 120dpi
			// 大 - 150% ... 144dpi

			hTmpDC		= CreateCompatibleDC(hDC);
			hMemDC		= CreateCompatibleDC(hDC);
			IconSize	= (int)(24. * (dpi / 96.));

			for(i = 0; i < sizeof(MainWindow1.hBitmap) / sizeof(HBITMAP); i++)
			{
				hTmpBMP = LoadBitmap(MainWindow1.hInst, MAKEINTRESOURCE(IDB_BITMAP1 + i));
				SelectObject(hTmpDC, hTmpBMP);

				MainWindow1.hBitmap[i] = CreateCompatibleBitmap(hDC, IconSize, IconSize);
				SelectObject(hMemDC, MainWindow1.hBitmap[i]);
				SetStretchBltMode(hMemDC, COLORONCOLOR);
				StretchBlt(
					hMemDC, 0, 0, IconSize, IconSize,
					hTmpDC, 0, 0, 24, 24, SRCCOPY
				);
				DeleteObject(hTmpBMP);
			}
			DeleteDC(hMemDC);
			DeleteDC(hTmpDC);
			ReleaseDC(hWnd, hDC);
		}
		else
		{
			dpi = 96;
		}


		NCMetrics.cbSize = sizeof(NONCLIENTMETRICS);
		if(SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &NCMetrics, 0) != 0)
		{
			lFont = NCMetrics.lfMenuFont;
			MainWindow1.hFont[0] = CreateFontIndirect(&lFont);
		}

		for(i = 1; i < sizeof(MainWindow1.hFont) / sizeof(HFONT); i++)
		{
			LoadString(MainWindow1.hInst, IDS_FONT1 + i - 1, MainWindow1.szStBuf, MAX_STRINGTABLE);
			if(*MainWindow1.szStBuf == '\0')
			{
				_tcscpy(MainWindow1.szStBuf, NCMetrics.lfMenuFont.lfFaceName);
			}
			MainWindow1.hFont[i] = CreateFont(-MulDiv(12, dpi, 72), 0, 0, 0, FW_BOLD, 0, 0, 0, 128, 0, 0, 0, 0, MainWindow1.szStBuf);
		}

		MainWindow1.hTool[0] = CreateWindowEx(0, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP , CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWnd, NULL, MainWindow1.hInst, NULL);
		SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)MainWindow1.hIcon);

		MainWindow1.hMenu1			= LoadMenu(MainWindow1.hInst, MAKEINTRESOURCE(IDR_MAINSUBMENU));
		MainWindow1.hPupMenu1		= GetSubMenu(MainWindow1.hMenu1, 0);
		MainWindow1.hBmp[0]			= GetDlgItem(hWnd, IDC_BITMAP1);
		MainWindow1.hProgress[0]	= GetDlgItem(hWnd, IDC_PROGRESS1);
		SendMessage(MainWindow1.hProgress[0], PBM_SETRANGE32, 0, 100);

		MainWindow1.hGroup[0]		= GetDlgItem(hWnd, IDC_GROUP1);
		SendMessage(MainWindow1.hGroup[0], WM_SETFONT, (WPARAM)MainWindow1.hFont[0], 0);

		MainWindow1.hEdit[0] = GetDlgItem(hWnd, IDC_EDIT1);
		for(i = 1; i < sizeof(MainWindow1.hEdit) / sizeof(HWND); i++)
		{
			MainWindow1.hEdit[i] = GetDlgItem(hWnd, IDC_EDIT1 + i);
			SendMessage(MainWindow1.hEdit[i], WM_SETFONT, (WPARAM)MainWindow1.hFont[0], 0);
		}
		SendMessage(MainWindow1.hEdit[0], WM_SETFONT, (WPARAM)MainWindow1.hFont[2], 0);
		SendMessage(MainWindow1.hEdit[1], WM_SETFONT, (WPARAM)MainWindow1.hFont[1], 0);

		for(i = 0; i < sizeof(MainWindow1.hButton) / sizeof(HWND); i++)
		{
			MainWindow1.hButton[i] = GetDlgItem(hWnd, IDC_BUTTON1 + i);
			SendMessage(MainWindow1.hButton[i], WM_SETFONT, (WPARAM)MainWindow1.hFont[0], 0);
		}

		for(i = 0; i < sizeof(MainWindow1.hStatic) / sizeof(HWND); i++)
		{
			MainWindow1.hStatic[i] = GetDlgItem(hWnd, IDC_STATIC1 + i);
			SendMessage(MainWindow1.hStatic[i], WM_SETFONT, (WPARAM)MainWindow1.hFont[i > 0 ? 0 : 1], 0);
		}

		if(GetVerifyVersionInfo(6, 1, 0))
		{
			wmTBC = RegisterWindowMessage(_T("TaskbarButtonCreated"));
		}

		MainWindow1.GrayTextColor	= GetSysColor(COLOR_GRAYTEXT);
		hbr1						= CreateSolidBrush(GetSysColor(COLOR_WINDOW));
		dwAppFrag					&= ~APP_WINDOW_NOALPHA;
		h_NextWnd					= SetClipboardViewer(hWnd);

		SendMessage(MainWindow1.hProgress[0], PBM_SETPOS, 0, 0);
		SendMessage(MainWindow1.hEdit[1], EM_LIMITTEXT, 32, 0);
		EnableWindow(MainWindow1.hButton[0], dwAppFrag & APP_MD5FILE_ADDMODE ? FALSE : TRUE);
		EnableMenuItem(MainWindow1.hPupMenu1, 7, MF_BYPOSITION | MF_GRAYED);
		EnableMenuItem(MainWindow1.hPupMenu1, 9, MF_BYPOSITION | MF_GRAYED);

		for(i = 0; i < sizeof(MainWindow1.hButton) / sizeof(HWND); i++)
		{
			SetToolTip(MainWindow1.hTool[0], MainWindow1.hButton[i], MainWindow1.szToolTipText[i]);
		}


		// Windows98 対策 (VC2005コード)
		// Windows95, NT4.0 の処理は省略しています。> OPENFILENAME_SIZE_VERSION_400
#ifdef _UNICODE
		MainWindow1.ofn1.lStructSize		= sizeof(OPENFILENAME);
#else
		MainWindow1.ofn1.lStructSize		= GetVerifyVersionInfo(5, 0, 0) ? sizeof(OPENFILENAME) : OPENFILENAME_SIZE_VERSION_400;
#endif /* _UNICODE */
		MainWindow1.ofn1.hwndOwner			= hWnd;
		MainWindow1.ofn1.hInstance			= MainWindow1.hInst;
		MainWindow1.ofn1.lpstrFilter		= _T("すべてのファイル\0*.*\0\0");
		MainWindow1.ofn1.nFilterIndex		= 0;
		MainWindow1.ofn1.lpstrFile			= MainWindow1.szOfn1Buf;
		MainWindow1.ofn1.nMaxFile			= (MAX_PATH_SIZE + 100) * 256;
		MainWindow1.ofn1.lpstrDefExt		= _T(""); 
		MainWindow1.ofn1.Flags				= OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_CREATEPROMPT | OFN_HIDEREADONLY | OFN_NODEREFERENCELINKS | OFN_OVERWRITEPROMPT;

		MainWindow1.ofn2.lStructSize		= MainWindow1.ofn1.lStructSize;
		MainWindow1.ofn2.hwndOwner			= hWnd;
		MainWindow1.ofn2.hInstance			= MainWindow1.hInst;
		MainWindow1.ofn2.lpstrFilter		= _T("MD5ファイル (md5sum形式)\0*.md5;*.txt\0")
												_T("MD5ファイル (MD5チェックツール (1x) 互換形式)\0*.md5;*.txt\0\0");
		MainWindow1.ofn2.nFilterIndex		= 0;
		MainWindow1.ofn2.lpstrFile			= MainWindow1.szMD5File;
		MainWindow1.ofn2.nMaxFile			= MAX_PATH_SIZE + 100;
		MainWindow1.ofn2.lpstrDefExt		= _T("");
		MainWindow1.ofn2.lpfnHook			= OFN2HookProc;
		MainWindow1.ofn2.lpTemplateName		= MAKEINTRESOURCE(IDD_OFN2);
		MainWindow1.ofn2.Flags				= OFN_EXPLORER | OFN_CREATEPROMPT | OFN_HIDEREADONLY | OFN_NODEREFERENCELINKS | OFN_OVERWRITEPROMPT;

		MainWindow1.MD5Thread1.hWnd			= hWnd;
		MainWindow1.MD5Thread1.MessageID	= RegisterWindowMessage(MainWindow1.szWindowClass);
		MainWindow1.MD5Thread1.FilePath		= MainWindow1.szFile;
		GetCurrentDirectory(MAX_PATH_SIZE, MainWindow1.MD5Thread1.MultiFile->FileCurrentFolder);
		MainWindow1.dwOpenItem				= 0;
		MainWindow1.IsSubFolder				= -1;
		MainWindow1.dwTimeViewFileCount		= 0;
		MainWindow1.dwAddFileModeFileCount	= 0;
		MD5_FileView(MainWindow1.dwOpenItem);

		if(dwAppFrag & APP_WINDOWALPHA)
		{
			SetLayeredWindow(hWnd);
		}
		SetAlphaWindow(hWnd, 0, ACTIVE_ALPHA, LWA_ALPHA);
		FileListWindow1.WindowMessage = (UINT)-1;
		memset(&FileListWindow1.FileSortSubNo, 1, sizeof(FileListWindow1.FileSortSubNo));
		if(dwAppFrag & APP_ENABLE_FILELIST)
		{
			FileListBox();
		}
		SetFocus(MainWindow1.hButton[0]);

		dwAppFrag |= APP_WINDOW;
		dwRet = 0;

		// コマンドラインにファイル名が指定してあったら開きます。
		if(__argc > 1)
		{
#if _MSC_VER < 1500
			BOOL IsWin9x = isWin9X();
#endif
			MainWindow_AddFile_Start();

			// 複数の引数ファイルを開く
			for(i = 1; i < __argc; i++)
			{
				// 引数のオプションが指定されていたら次へスキップします。
				if(*__targv[i] == '/')
				{
					continue;
				}

#if _MSC_VER < 1500
				dwRet = GetLongPathName(__targv[i], MainWindow1.szFile,  MAX_PATH_SIZE + 100);
				if(dwRet == 0)
				{
					_tcscpy(MainWindow1.szFile, __targv[i]);
				}
#else
				_tcscpy(MainWindow1.szFile, __targv[i]);
#endif

				dwRet = MD5_Thread_FileNameCheck(MainWindow1.szFile);
				if(dwRet == 0)
				{
					continue;
				}

				if(PathIsRelative(MainWindow1.szFile))
				{
					_tcscpy(MainWindow1.szBuf, MainWindow1.szFile);
					PathCombine(MainWindow1.szFile, MainWindow1.MD5Thread1.MultiFile->FileCurrentFolder, MainWindow1.szBuf);
				}

				dwRet = MainWindow_AddFile(i - 1);
				if(dwRet == -1 ||
					dwRet >= FILE_MAX_COUNTSIZE)
				{
					break;
				}
			}

			if(dwRet == -1)
			{
				FileRecodeFoul(hWnd);
				return FALSE;
			}
			MainWindow_AddFile_End();
		}

		if(__argc > 1 &&
			dwRet == 0)
		{
			MainWindow1.IsWindowCancel = TRUE;
			DestroyWindow(hWnd);
		}

		if(MainWindow1.MD5Thread1.MultiFile->FileCount == 0 &&
			MainWindow1.dwAddFileModeFileCount == 0)
		{
			DragAcceptFiles(hWnd, TRUE);
		}
		return FALSE;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 選択されたメニューの解析:
		switch(wmId)
		{
		case IDC_BUTTON1:
			Open(TRUE);
			return TRUE;
		case IDM_OPEN:
		case ID_MAINSUBMENU1_2:
			Open(FALSE);
			return TRUE;
		case IDM_SAVE:
		case IDC_BUTTON2:
		case ID_MAINSUBMENU1_3:
			Save();
			return TRUE;
		case IDM_COPY:
		case IDC_BUTTON3:
		case ID_MAINSUBMENU1_4:
			if(~dwAppFrag & APP_MD5THREAD)
			{
				Copy(MainWindow1.szMD5String);
			}
			return TRUE;
		case IDM_PASTE:
		case IDC_BUTTON4:
		case ID_MAINSUBMENU1_5:
			if(~dwAppFrag & APP_MD5THREAD)
			{
				Paste();
			}
			return TRUE;
		case IDM_CLEAR:
		case ID_MAINSUBMENU1_13:
			if(dwAppFrag & APP_MD5THREAD)
			{
				int nRet;
				LoadString(MainWindow1.hInst, IDS_CLEAR2, MainWindow1.szStBuf, MAX_STRINGTABLE);
				MessageFormat(MainWindow1.szBuf, 1024, MainWindow1.szStBuf, MainWindow1.szFile);
				nRet = MainWindow_MessageBox(MainWindow1.hWnd, MainWindow1.szBuf, MainWindow1.szTitle, MB_YESNO | MB_ICONQUESTION);

				if(nRet == IDYES)
				{
					Cancel(TRUE);
				}
			}
			else
			{
				Clear();
			}
			break;
		case ID_MAINSUBMENU1_12:
			if(~dwAppFrag & APP_MD5THREAD)
			{
				Bench();
			}
			break;
		case ID_MAINSUBMENU1_6:
			return ReadMeOpen();
		case ID_MAINSUBMENU1_7:
			return VersionCheck();
		case ID_MAINSUBMENU1_8:
			FileListBox();
			return TRUE;
		case IDC_BUTTON5:
		case ID_MAINSUBMENU1_9:
			OptionBox(hWnd);
			return TRUE;
		case IDM_ABOUT:
		case IDC_BUTTON6:
		case ID_MAINSUBMENU1_10:
			AboutDlg_AboutBox(hWnd);
			return TRUE;
		case IDC_EDIT2:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				GetWindowText(MainWindow1.hEdit[1], MainWindow1.szMD5CompString, NUMBER_LOADSTRING);
				if(_tcslen(MainWindow1.szMD5CompString) == HASH_LEN * 2)
				{
					MD5_HashComp();
				}
				return TRUE;
			case EN_SETFOCUS:
				if(~dwAppFrag & APP_MD5_INPUT_EDIT)
				{
					dwAppFrag |= APP_MD5_INPUT_EDIT;

					*MainWindow1.szMD5CompString = '\0';
					SendMessage(MainWindow1.hEdit[1], WM_SETFONT, (WPARAM)MainWindow1.hFont[2], 0);
					SetWindowText(MainWindow1.hEdit[1], _T(""));
				}
				return TRUE;
			case EN_KILLFOCUS:
				GetWindowText(MainWindow1.hEdit[1], MainWindow1.szMD5CompString, NUMBER_LOADSTRING);
				if(*MainWindow1.szMD5CompString == '\0')
				{
					dwAppFrag &= ~APP_MD5_INPUT_EDIT;

					SendMessage(MainWindow1.hEdit[1], WM_SETFONT, (WPARAM)MainWindow1.hFont[1], 0);
					SetWindowText(MainWindow1.hEdit[1], MainWindow1.szInputtext);
				}
				return TRUE;
			}
			return FALSE;
		case IDM_CANCEL:
			Cancel(FALSE);
			return TRUE;
		case IDM_VK_F5:
		case IDM_REOPEN:
		case ID_MAINSUBMENU1_1:
			if(dwAppFrag & APP_MD5FILE_ADDMODE &&
				wmId == IDM_VK_F5)
			{
				return FALSE;
			}
			ReOpen();
			return TRUE;
		case IDM_PAUSE:
			Pause();
			return TRUE;
		case ID_MAINSUBMENU1_1_1:
			Copy(PathFindFileName(MainWindow1.szFile));
			return TRUE;
		case ID_MAINSUBMENU1_1_2:
			Copy(MainWindow1.szFile);
			return TRUE;
		case ID_MAINSUBMENU1_1_3:
			GetSizeText(MainWindow1.szBuf, (MainWindow1.MD5Thread1.MultiFile->FileRecode + MainWindow1.dwOpenItem)->FileSize);
			Copy(MainWindow1.szBuf);
			return TRUE;
		case ID_MAINSUBMENU1_1_4:
			p = MainWindow1.szBuf + 512;
			if(PathRelativePathTo(p,
				MainWindow1.MD5Thread1.MultiFile->FileCurrentFolder, FILE_ATTRIBUTE_DIRECTORY,
				MainWindow1.szFile,
				FILE_ATTRIBUTE_ARCHIVE))
			{
				// ファイル名の先頭に『.\』が付いていたら削除する。
				// ↓_tcscmp(p, _T(".\")) の最適化
#ifdef _UNICODE
				if(*(DWORD*)p == 0x005C002E)
#else
				if(*(WORD*)p == 0x5C2E)
#endif
				{
					*(p + 511) = '\0';
					memmove(p, p + 2, 511 * sizeof(TCHAR));
				}
			}
			else
			{
				_tcscpy(p, MainWindow1.szFile);
				PathRemoveFileSpec(p);
			}
			GetSizeText(MainWindow1.szTempBuf, (MainWindow1.MD5Thread1.MultiFile->FileRecode + MainWindow1.dwOpenItem)->FileSize);
			LoadString(MainWindow1.hInst, IDS_HASHPRINT1, MainWindow1.szStBuf, MAX_STRINGTABLE);
			MessageFormat(MainWindow1.szBuf, 512, MainWindow1.szStBuf, MainWindow1.szMD5String, p, MainWindow1.szTempBuf);
			Copy(MainWindow1.szBuf);
			return TRUE;
		case ID_MAINSUBMENU1_11:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			return TRUE;
		}
		return FALSE;
	case WM_RBUTTONDOWN:
		GetCursorPos(&pt);
		DrawMenuBar(hWnd);
		TrackPopupMenu(MainWindow1.hPupMenu1, TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
		return TRUE;
	case WM_SYSCOLORCHANGE:
		DeleteObject(hbr1);
		MainWindow1.GrayTextColor = GetSysColor(COLOR_GRAYTEXT);
		hbr1 = CreateSolidBrush(GetSysColor(COLOR_WINDOW));

		for(i = 0; i < sizeof(MainWindow1.hStatic) / sizeof(HWND); i++)
			SendMessage(MainWindow1.hStatic[i], WM_SYSCOLORCHANGE, 0, 0);

		SendMessage(MainWindow1.hBmp[0], WM_SYSCOLORCHANGE, 0, 0);
		SendMessage(MainWindow1.hProgress[0], WM_SYSCOLORCHANGE, 0, 0);

		for(i = 0; i < sizeof(MainWindow1.hEdit) / sizeof(HWND); i++)
			SendMessage(MainWindow1.hEdit[i], WM_SYSCOLORCHANGE, 0, 0);

		for(i = 0; i < sizeof(MainWindow1.hButton) / sizeof(HWND); i++)
			SendMessage(MainWindow1.hButton[i], WM_SYSCOLORCHANGE, 0, 0);
		return TRUE;
	case WM_CTLCOLOREDIT:
		if((HWND)lParam == MainWindow1.hEdit[1] &&
			~dwAppFrag & APP_MD5_INPUT_EDIT)
		{
			hDC = (HDC)wParam;
			SetBkMode(hDC, TRANSPARENT);
			SetTextColor(hDC, MainWindow1.GrayTextColor);
			return (LRESULT)hbr1;
		}
		return FALSE;
	case WM_DROPFILES:
		if(dwAppFrag & APP_MD5FILE_ADDMODE &&
			~dwAppFrag & APP_FILELISTBOX)
		{
			FileListBox();
		}
		
		if(dwAppFrag & APP_FILELISTBOX)
		{	
			return SendMessage(FileListWindow1.hWnd, message, wParam, lParam);
		}

		if(dwAppFrag & APP_WINDOWALPHA)
		{
			SetAlphaWindow(hWnd, 0, ACTIVE_ALPHA, LWA_ALPHA);
		}
		dwCount = DragQueryFile((HDROP)wParam, (DWORD)-1, NULL, NULL);

		if(dwCount >= 0)
		{
			dwRet = 0;
			MainWindow_AddFile_Start();

			for(UINT ui = 0; ui < dwCount; ui++)
			{
				DragQueryFile((HDROP)wParam, ui, MainWindow1.szFile, MAX_PATH_SIZE);
				dwRet = MainWindow_AddFile(ui);

				if(dwRet == -1 ||
					dwRet >= FILE_MAX_COUNTSIZE)
				{
					break;
				}
			}

			if(dwRet == -1)
			{
				FileRecodeFoul(hWnd);
				DragFinish((HDROP)wParam);
				DestroyWindow(hWnd);
				return FALSE;
			}
			MainWindow_AddFile_End();
		}
		DragFinish((HDROP)wParam);
		return TRUE;
	case WM_TIMER:
		if(wParam == TIMER &&
			dwAppFrag & APP_MD5THREAD)
		{
			MD5_TimeView(0);
		}
		return TRUE;
	case WM_ACTIVATE:
		if(dwAppFrag & APP_WINDOWALPHA)
		{
			SetAlphaWindow(hWnd, 0, ~dwAppFrag & APP_WINDOW_NOALPHA &&
					wParam == 0 ?
					INACTIVE_ALPHA : ACTIVE_ALPHA, LWA_ALPHA);
		}
		return FALSE;
	case WM_DRAWCLIPBOARD:
		if(h_NextWnd)
		{
			SendMessage(h_NextWnd, message, wParam, lParam);
		}

		if(~dwAppFrag & APP_MD5THREAD)
		{
			EnableWindow(MainWindow1.hButton[3], ClipboardCheck(MainWindow1.dwOpenItem));
		}
		return TRUE;
	case WM_CHANGECBCHAIN:
		if((HWND)wParam == h_NextWnd)
			h_NextWnd = (HWND)lParam;
		else if(h_NextWnd)
			return SendMessage(h_NextWnd, message, wParam, lParam);
		return TRUE;
	case WM_CLOSE:
		if(dwAppFrag & APP_FOLDERINIT)
		{
			LoadString(MainWindow1.hInst, IDS_FILELIST_CLOSE, MainWindow1.szStBuf, MAX_STRINGTABLE);
			MainWindow_MessageBox(hWnd, MainWindow1.szStBuf, MainWindow1.szTitle, MB_ICONHAND);

			return FALSE;
		}
		DestroyWindow(hWnd);
		return TRUE;
	case WM_DESTROY:
		dwAppFrag &= ~APP_WINDOW;

		if(dwAppFrag & APP_FILELISTBOX)
		{
			DestroyWindow(FileListWindow1.hWnd);
		}
		ChangeClipboardChain(hWnd, h_NextWnd);
		for(i = 1; i < sizeof(MainWindow1.hFont) / sizeof(HFONT); i++)
			DeleteObject(MainWindow1.hFont[i]);
		for(i = 0; i < sizeof(MainWindow1.hBitmap) / sizeof(HBITMAP); i++)
			DeleteObject(MainWindow1.hBitmap[i]);
		DestroyMenu(MainWindow1.hMenu1);
		DeleteObject(hbr1);
		EndDialog(hWnd, LOWORD(wParam));
		SetIniFileSetting(MainWindow1.szINIFile);
		PostQuitMessage(0);
		return TRUE;
	}
	return FALSE;
}

UINT CALLBACK OFN2HookProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	static HWND hCombo;
	int wmId, wmEvent;
	DWORD dwItem;

	switch(message)
	{
	case WM_INITDIALOG:
		hCombo = GetDlgItem(hWnd, IDC_OFN2_COMBO1);
		SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("Shift_JIS　(初期設定)"));
		SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("Unicode (UTF-16)"));
		SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("UTF-8"));
		SendMessage(hCombo, CB_SETCURSEL, MainWindow1.dwSaveMD5FileCharCode, 0);
		return TRUE;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 選択されたメニューの解析:
		switch(wmId)
		{
		case IDC_OFN2_COMBO1:
			switch(wmEvent)
			{
			case CBN_SELCHANGE:
				dwItem = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
				MainWindow1.dwSaveMD5FileCharCode = dwItem;
				break;
			}
		}
	}
	return FALSE;
}

int MainWindow_MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	int ret;

	// ウィンドウロックします。
	WINDOW_LOCK;

	ret = MessageBox(hWnd, lpText, lpCaption, uType);

	// ウィンドウロックの解除します。
	WINDOW_UNLOCK;

	return ret;
}

VOID Clear(VOID)
{
	dwAppFrag |= APP_FILE_CLEAR;

	*MainWindow1.szFile = '\0';
	MainWindow1.MD5Thread1.MultiFile->FileCount = 0;
	MainWindow1.MD5Thread1.MultiFile->FileCurrentCount = 0;
	MainWindow1.dwOpenItem = 0;
	SendMessage(MainWindow1.hProgress[0], PBM_SETPOS, 0, 0);
	MD5_FileView(MainWindow1.dwOpenItem);

	if(dwAppFrag & APP_FILELISTBOX)
	{
		SendMessage(FileListWindow1.hWnd, FileListWindow1.WindowMessage, APP_MESSAGE_FILECLEAR, TRUE);
	}
	if(MainWindow1.pTaskbarList3 != NULL)
	{
		MainWindow1.pTaskbarList3->SetProgressState(MainWindow1.hWnd, TBPF_NOPROGRESS);
	}

	dwAppFrag &= ~APP_FILE_CLEAR;
}

VOID Bench(VOID)
{
	TCHAR szHashBuf[HASH_LOADSTRING];
	TCHAR szHashComp[HASH_LOADSTRING];
	DWORD dwHashByte[HASH_LEN * 2 / sizeof(DWORD)];
	DWORD dwTimeStart, dwTime[8];
	DWORD dwRet, dwCount;
	double dScore[8] = {
		156.176,
		0.882,
		114.706,
		230.,
		14.72,
		133.24,
		64.12,
		0
	};
	static unsigned __int64 llClock;
	BYTE *bBuf;

	DWORD dwWrite;
	TCHAR *szTempFile;
	BYTE  *FileBuf;
	tagMD5Thread_MultiFile *MD5Thread_MultiFile1;
	tagMD5Thread_FileRecode *MD5Thread_FileRecode1;
	HANDLE hFile;
#ifdef _DEBUG
	TCHAR szDebugText[1024];
#endif

	bBuf = (BYTE*)malloc(
		(MAX_PATH_SIZE * sizeof(TCHAR)) + 
		sizeof(tagMD5Thread_MultiFile) + 
		sizeof(tagMD5Thread_FileRecode) + 
		BENCH_FILESIZE);

	if(bBuf == NULL)
	{
		return;
	}

	szTempFile				= (TCHAR*) bBuf;
	*szTempFile				= '\0';
	MD5Thread_MultiFile1	= (tagMD5Thread_MultiFile*)(szTempFile + MAX_PATH_SIZE);
	MD5Thread_FileRecode1	= (tagMD5Thread_FileRecode*)(MD5Thread_MultiFile1 + 1);
	FileBuf					= (BYTE*)(MD5Thread_FileRecode1 + 1);

	GetTempPath(MAX_PATH_SIZE, MainWindow1.szBuf);
	GetTempFileName(MainWindow1.szBuf, _T("MD5"), NULL, szTempFile);
	memcpy(MD5Thread_MultiFile1, MainWindow1.MD5Thread1.MultiFile, sizeof(tagMD5Thread_MultiFile));
	memcpy(MD5Thread_FileRecode1, MainWindow1.MD5Thread1.MultiFile->FileRecode, sizeof(tagMD5Thread_FileRecode));
	memset(FileBuf, 0, BENCH_FILESIZE);


	WINDOW_LOCK;
	EnableWindow(MainWindow1.hWnd, FALSE);

	if(!llClock)
	{
		dwTime[0] = GetTickCount() + 1000;
		while(dwTime[0] > GetTickCount());

		llClock = __rdtsc();

		dwTime[0] = GetTickCount() + 1000;
		while(dwTime[0] > GetTickCount());

		llClock = __rdtsc() - llClock;

#ifdef _DEBUG
		_stprintf(szDebugText, _T("MainWindow: WndProc(): クロック数が取得できました。%.2fMHz\r\n"), (double)llClock / 1000000);
		szDebugText[1023] = '\0';
		OutputDebugString(szDebugText);
#endif
	}

	hFile = CreateFile(
		szTempFile, GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
	);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		if(DeviceIoControl(hFile, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &dwRet, NULL))
		{
			LARGE_INTEGER liSize;

#if _MSC_VER < 1500
			UINT  nRet;
			UINT  nLen;
			TCHAR *szPath;
			const TCHAR *szFile = _T("KERNEL32.DLL");
			const char  *szFunc = "SetFilePointerEx";

			nLen = GetSystemDirectory(0, 0);

			if(nLen < 1)
				goto BENCH_DLLFOUL;

			nLen  += (UINT)_tcslen(szFile) + 1;
			szPath = (TCHAR*)malloc(nLen * sizeof(TCHAR));
			if(szPath == NULL)
				goto BENCH_DLLFOUL;

			nRet = GetSystemDirectory(szPath, nLen);

			if(nRet > 0)
			{
				HMODULE hDll;
				TCHAR *p = szPath + nRet;

				*(p++) = _T('\\');
				_tcscpy(p, szFile);

				hDll = LoadLibrary(szPath);
				if(hDll != NULL)
				{
					BOOL (WINAPI *SetFilePointerEx)(HANDLE, LARGE_INTEGER, PLARGE_INTEGER, DWORD);
					SetFilePointerEx = reinterpret_cast<BOOL(WINAPI*)(HANDLE, LARGE_INTEGER, PLARGE_INTEGER, DWORD)>(GetProcAddress(hDll, szFunc));

					if(SetFilePointerEx != NULL)
					{
#endif /* _MSC_VER < 1500 */

#ifdef _DEBUG
						OutputDebugString(_T("MainWindow: WndProc(): スパースファイルの作成します。\r\n"));
#endif

						liSize.QuadPart = BENCH_FILESIZE;
						SetFilePointerEx(hFile, liSize, NULL, FILE_BEGIN);
						SetEndOfFile(hFile);

#if _MSC_VER < 1500
					}
					else
					{
						goto BENCH_DLLFOUL;
					}
					FreeLibrary(hDll);
				}
			}
			free(szPath);
#endif /* _MSC_VER < 1500 */

		}
		else
		{
#if _MSC_VER < 1500
BENCH_DLLFOUL:
#endif /* _MSC_VER < 1500 */

#ifdef _DEBUG
			OutputDebugString(_T("MainWindow: WndProc(): スパースファイルの作成に失敗しました。\r\n"));
#endif

			WriteFile(hFile, FileBuf, BENCH_FILESIZE, &dwWrite, NULL);
		}
		CloseHandle(hFile);
	}
	else
	{
		goto BENCH_EXIT;
	}

	MD5_Thread_MultiFile_Init(&MainWindow1.MD5Thread1, szTempFile, FALSE);


	// ランダムなハッシュデータを作ります。
	srand((unsigned)time(NULL));
	for(dwCount = 0; dwCount < HASH_LEN / sizeof(DWORD); dwCount++)
	{
		dwHashByte[dwCount] = (rand() << 16) | rand();
	}
	memcpy(dwHashByte + HASH_LEN / sizeof(DWORD), dwHashByte, HASH_LEN);
	MD5_Thread_MD5ToString_Old(szHashComp, (BYTE*)&dwHashByte, FALSE);


	memset(szHashBuf, 0, HASH_LEN * sizeof(TCHAR));
	dwTimeStart = timeGetTime();
	for(dwCount = 0; dwCount < BENCH_LOOP; dwCount++)
	{
		MD5_Thread_MD5ToString_Old(szHashBuf, (BYTE*)&dwHashByte, FALSE);
	}
	dwTime[0] = timeGetTime() - dwTimeStart;

	dwTimeStart = timeGetTime();
	for(dwCount = 0; dwCount < BENCH_LOOP; dwCount++)
	{
		MD5_Thread_MD5ToString_no_sprintf(szHashBuf, (BYTE*)&dwHashByte, FALSE);
	}
	dwTime[1] = timeGetTime() - dwTimeStart;
	if(dwTime[1] == 0)
		dwTime[1] = 1;

	dwTimeStart = timeGetTime();
	for(dwCount = 0; dwCount < BENCH_LOOP; dwCount++)
	{
		MD5_Thread_MD5ToString_Default(szHashBuf, (BYTE*)&dwHashByte, FALSE);
	}
	dwTime[2] = timeGetTime() - dwTimeStart;


	dwTimeStart = timeGetTime();
	for(dwCount = 0; dwCount < BENCH_LOOP; dwCount++)
	{
		MD5_Thread_MD5ToHash_Old((BYTE*)&dwHashByte, szHashBuf);
	}
	dwTime[3] = timeGetTime() - dwTimeStart;

	dwTimeStart = timeGetTime();
	for(dwCount = 0; dwCount < BENCH_LOOP; dwCount++)
	{
		MD5_Thread_MD5ToHash_no_scanf((BYTE*)&dwHashByte, szHashBuf, FALSE);
	}
	dwTime[4] = timeGetTime() - dwTimeStart;

	dwTimeStart = timeGetTime();
	for(dwCount = 0; dwCount < BENCH_LOOP; dwCount++)
	{
		MD5_Thread_MD5ToHash_Default((BYTE*)&dwHashByte, szHashBuf, FALSE);
	}
	dwTime[5] = timeGetTime() - dwTimeStart;

	MainWindow1.MD5Thread1.hWnd = NULL;
	dwTimeStart = timeGetTime();
	MD5_MultiFile_Thread(&MainWindow1.MD5Thread1);
	dwTime[6] = timeGetTime() - dwTimeStart;
	MainWindow1.MD5Thread1.hWnd = MainWindow1.hWnd;


	dwRet = MessageFormat(MainWindow1.szBuf,
		MAX_STRINGTABLE * 2,
		_T("ベンチマークが完了しました。%n%n") \
		_T("MD5ToString_Old():%t%1!u!点 (%2!u!ms)%n") \
		_T("MD5ToString_no_sprintf():%t%3!u!点 (%4!u!ms)%n") \
		_T("MD5ToString():%t%t%5!u!点 (%6!u!ms)%n") \
		_T("MD5ToHash_Old():%t%t%7!u!点 (%8!u!ms)%n") \
		_T("MD5ToHash_no_scanf():%t%9!u!点 (%10!u!ms)%n") \
		_T("MD5ToHash():%t%t%11!u!点 (%12!u!ms)%n") \
		_T("MD5_MultiFile_Thread():%t%13!u!点 (%14!u!ms)%n"),
		(DWORD)(dScore[0] / (dwTime[0] / ((double)llClock / 1000000.))), dwTime[0],
		(DWORD)(dScore[1] / (dwTime[1] / ((double)llClock / 1000000.))), dwTime[1],
		(DWORD)(dScore[2] / (dwTime[2] / ((double)llClock / 1000000.))), dwTime[2],
		(DWORD)(dScore[3] / (dwTime[3] / ((double)llClock / 1000000.))), dwTime[3],
		(DWORD)(dScore[4] / (dwTime[4] / ((double)llClock / 1000000.))), dwTime[4],
		(DWORD)(dScore[5] / (dwTime[5] / ((double)llClock / 1000000.))), dwTime[5],
		(DWORD)(dScore[6] / (dwTime[6] / ((double)llClock / 1000000.))), dwTime[6]
	);
	if(dwRet != 0)
		MessageBox(MainWindow1.hWnd, MainWindow1.szBuf, MainWindow1.szTitle, MB_ICONINFORMATION);
	else
		GetLastErrorMsg(dwRet);

	memcpy(MainWindow1.MD5Thread1.MultiFile, MD5Thread_MultiFile1, sizeof(tagMD5Thread_MultiFile));
	memcpy(MainWindow1.MD5Thread1.MultiFile->FileRecode, MD5Thread_FileRecode1, sizeof(tagMD5Thread_FileRecode));
	DeleteFile(szTempFile);

BENCH_EXIT:
	free(bBuf);
	EnableWindow(MainWindow1.hWnd, TRUE);
	WINDOW_UNLOCK;
}

DWORD GetSubFolderCount(const TCHAR *inFile)
{
	HANDLE hFind = NULL;
	WIN32_FIND_DATA FindData;
	TCHAR szFile[MAX_PATH_SIZE + 100];
	DWORD dwRet = 0;


	ExpandEnvironmentStrings(inFile, szFile, MAX_PATH_SIZE + 100);
	if(dwRet == 0)
	{
		_tcscpy(szFile, inFile);
	}
	_tcscat(szFile, _T("\\*"));

	hFind = FindFirstFile(szFile, &FindData);
	if(hFind == INVALID_HANDLE_VALUE)
	{
		return dwRet;
	}

	do
	{
		if(~FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ||
			FindData.dwFileAttributes & ~MainWindow1.MD5Thread1.FileAttributeMask ||
			_tcscmp(FindData.cFileName, _T(".")) == 0 ||
			_tcscmp(FindData.cFileName, _T("..")) == 0)
		{
			continue;
		}
		dwRet++;
	}
	while(FindNextFile(hFind, &FindData));
	FindClose(hFind);

	return dwRet;
}

VOID MainWindow_AddFile_Start(VOID)
{
	MainWindow1.MD5Thread1.MultiFile->FileCount			= 0;
	MainWindow1.MD5Thread1.MultiFile->FileCurrentCount	= 0;
	MainWindow1.IsSubFolder		= -1;
	MainWindow1.IsNotFoundFile	= 0;
	*MainWindow1.szNotFoundFile	= '\0';
	MainWindow1.IsEmptyFolder	= 0;
	*MainWindow1.szEmptyFolder	= '\0';
	*MainWindow1.szMD5File		= '\0';
}

DWORD MainWindow_AddFile(UINT nCount)
{
	BOOL IsMD5FileRet;
	DWORD dwFileCount = MainWindow1.MD5Thread1.MultiFile->FileCount;
	DWORD dwRet = 0;


	if(MainWindow1.MD5Thread1.MultiFile->FileCount >= FILE_MAX_COUNTSIZE)
	{
		LoadString(MainWindow1.hInst, IDS_OVERFILE1, MainWindow1.szStBuf, MAX_STRINGTABLE);
		MessageFormat(MainWindow1.szBuf, 1024, MainWindow1.szStBuf, MainWindow1.szFile);
		MainWindow_MessageBox(MainWindow1.hWnd, MainWindow1.szBuf, MainWindow1.szTitle, MB_ICONHAND);

		return FILE_MAX_COUNTSIZE;
	}

	MainWindow1.MD5Thread1.IsFileNoCheck = 0;
	IsMD5FileRet = GetMD5FilePath();

	if(dwAppFrag & APP_FOLDEROPEN)
	{
		if(MainWindow1.IsSubFolder == -1 &&
			GetSubFolderCount(MainWindow1.szFile) > 0)
		{
			LoadString(MainWindow1.hInst, IDS_FOLDER_OPEN2, MainWindow1.szStBuf, MAX_STRINGTABLE);
			MessageFormat(MainWindow1.szBuf, 1024, MainWindow1.szStBuf, MainWindow1.szFile);
			MainWindow1.IsSubFolder = MainWindow_MessageBox(MainWindow1.hWnd, MainWindow1.szBuf, MainWindow1.szTitle, MB_YESNOCANCEL | MB_ICONQUESTION);
			
			if(MainWindow1.IsSubFolder == IDCANCEL)
			{
				MainWindow1.IsSubFolder = FALSE;

#ifdef _DEBUG
				OutputDebugString(_T("MainWindow: MainWindow_AddFile(): サブフォルダの展開をキャンセルしました。\r\n"));
#endif
				return 0;
			}
		}
		dwRet = MD5_Thread_Folder_Init(&MainWindow1.MD5Thread1, MainWindow1.szFile, nCount != 0 ? TRUE : FALSE, MainWindow1.IsSubFolder == IDYES ? TRUE : FALSE);

		if(dwFileCount == dwRet)
		{
			if(MainWindow1.IsEmptyFolder < 10)
			{
				if(MainWindow1.IsEmptyFolder < 9)
				{
					_stprintf(MainWindow1.szBuf, _T("%s\r\n"), MainWindow1.szFile);
					_tcscat(MainWindow1.szEmptyFolder, MainWindow1.szBuf);
				}
				else
				{
					_tcscat(MainWindow1.szEmptyFolder, _T("..."));
				}
			}
			MainWindow1.IsEmptyFolder++;
		}
	}
	else if(IsMD5FileRet != FALSE)
		dwRet = MD5File_FileOpen(nCount != 0 ? TRUE : FALSE);
	else
	{
		dwRet = MD5_Thread_MultiFile_Init(&MainWindow1.MD5Thread1, MainWindow1.szFile, nCount != 0 ? TRUE : FALSE);

		if(dwFileCount == dwRet)
		{
			if(MainWindow1.IsNotFoundFile < 10)
			{
				if(MainWindow1.IsNotFoundFile < 9)
				{
					_stprintf(MainWindow1.szBuf, _T("%s\r\n"), MainWindow1.szFile);
					_tcscat(MainWindow1.szNotFoundFile, MainWindow1.szBuf);
				}
				else
				{
					_tcscat(MainWindow1.szNotFoundFile, _T("..."));
				}
			}
			MainWindow1.IsNotFoundFile++;
		}
		else if(dwAppFrag & APP_FILELISTBOX)
		{
			FileList_ListView(dwRet - 1, FALSE);
		}
	}

	return dwRet;
}

VOID MainWindow_AddFile_End(VOID)
{
	if(MainWindow1.IsNotFoundFile > 0 ||
		MainWindow1.IsEmptyFolder > 0)
	{
		TCHAR *szLargeBuf;

		szLargeBuf = (TCHAR*)malloc(1024 * 40 * sizeof(TCHAR));
		if(szLargeBuf != NULL)
		{
			TCHAR *p1 = szLargeBuf;

			if(MainWindow1.IsNotFoundFile > 0)
			{
				LoadString(MainWindow1.hInst, IDS_FILE_OPEN2, MainWindow1.szStBuf, MAX_STRINGTABLE);
				MessageFormat(szLargeBuf, 1024 * 20, MainWindow1.szStBuf, MainWindow1.szNotFoundFile);
			}

			if(MainWindow1.IsEmptyFolder > 0)
			{
				if(MainWindow1.IsNotFoundFile > 0)
				{
					p1 = _tcslen(szLargeBuf) + szLargeBuf;
					p1 = qtcscpy(p1, _T("\r\n\r\n"));
				}
				LoadString(MainWindow1.hInst, IDS_FOLDER_OPEN3, MainWindow1.szStBuf, MAX_STRINGTABLE);
				MessageFormat(p1, 1024 * 20, MainWindow1.szStBuf, MainWindow1.szEmptyFolder);
			}

			MainWindow_MessageBox(MainWindow1.hWnd, szLargeBuf, MainWindow1.szTitle, MB_ICONHAND);
			free(szLargeBuf);
		}
	}

	if(MainWindow1.MD5Thread1.MultiFile->FileCount > 0)
	{
		if(MainWindow1.MD5Thread1.MultiFile->FileCount > 1)
		{
			dwAppFrag |= APP_FILELISTBOX_EDIT;
		}
		else
		{
			dwAppFrag &= ~APP_FILELISTBOX_EDIT;
		}

		_tcscpy(MainWindow1.szOfn1Buf, (MainWindow1.MD5Thread1.MultiFile->FileRecode + MainWindow1.MD5Thread1.MultiFile->FileCount - 1)->FileName);
		if(dwAppFrag & APP_MD5FILE_ADDMODE)
		{
			static int init = 0;

			if(init == 0)
			{
				MainWindow1.dwAddFileModeFileCount = MainWindow1.MD5Thread1.MultiFile->FileCount;
				MainWindow1.dwOpenItem = MainWindow1.dwAddFileModeFileCount - 1;
				init++;
			}
			SendMessage(MainWindow1.hWnd, MainWindow1.MD5Thread1.MessageID, APP_MESSAGE_END, MD5_Thread_GetCurrentItem(&MainWindow1.MD5Thread1));
			return;
		}
		GetMD5();
	}
}

DWORD ReadMeOpen(VOID)
{
	HINSTANCE hInst;
	UINT nRet = FALSE;
	TCHAR *p;
	const TCHAR *szFile = _T("ReadMe.txt");


	if(GetModuleFileName(NULL, MainWindow1.szTempBuf, MAX_PATH_SIZE + 100) == NULL)
	{
		return 0;
	}
	p = _tcsrchr(MainWindow1.szTempBuf, _T('\\'));

	if(p == NULL)
	{
		return 0;
	}
	p++;
	_tcscpy(p, szFile);

	hInst = ShellExecute(MainWindow1.hWnd, _T("open"), MainWindow1.szTempBuf, 0, 0, SW_SHOWNORMAL);
	if((UINT)hInst < 32)
	{
		switch((UINT)hInst)
		{
		case ERROR_FILE_NOT_FOUND:
		case ERROR_PATH_NOT_FOUND:
			LoadString(MainWindow1.hInst, IDS_READMEOPEN1, MainWindow1.szStBuf, MAX_STRINGTABLE);
			MainWindow_MessageBox(MainWindow1.hWnd, MainWindow1.szStBuf, MainWindow1.szTitle, MB_OK | MB_ICONERROR);

			break;
		default:
			nRet = TRUE;
		}
	}

	return nRet;
}

DWORD VersionCheck(VOID)
{
	LoadString(MainWindow1.hInst, IDS_APP_URL, MainWindow1.szStBuf, MAX_STRINGTABLE);
	if(*MainWindow1.szStBuf != NULL)
	{
		HINSTANCE hInst;

		hInst = ShellExecute(MainWindow1.hWnd, _T("open"), MainWindow1.szStBuf, 0, 0, SW_SHOWNORMAL);
	}
	return TRUE;
}

VOID SetFileInfo(DWORD dwItem)
{
	DWORD dwRet = 0;

	if(MainWindow1.MD5Thread1.MultiFile->FileCount == 0 ||
		dwItem > MainWindow1.MD5Thread1.MultiFile->FileCount)
	{
		SetWindowText(MainWindow1.hEdit[2], _T(""));
		EnableWindow(MainWindow1.hEdit[2], FALSE);
		SetWindowText(MainWindow1.hStatic[3], _T(""));
		SetToolTip(MainWindow1.hTool[0], MainWindow1.hStatic[3], _T(""));
		SetWindowText(MainWindow1.hStatic[5], _T(""));

		return;
	}

	EnableWindow(MainWindow1.hEdit[2], TRUE);
	SetWindowText(MainWindow1.hEdit[2], PathFindFileName((MainWindow1.MD5Thread1.MultiFile->FileRecode + dwItem)->FileName));
	_tcscpy(MainWindow1.szBuf, (MainWindow1.MD5Thread1.MultiFile->FileRecode + dwItem)->FileName);
	PathRemoveFileSpec(MainWindow1.szBuf);
	SetWindowText(MainWindow1.hStatic[3], MainWindow1.szBuf);

	dwRet = ExpandEnvironmentStrings(MainWindow1.szBuf, MainWindow1.szTempBuf, MAX_PATH_SIZE + 100);
	if(dwRet == 0)
	{
		_tcscpy(MainWindow1.szTempBuf, MainWindow1.szBuf);
	}
	SetToolTip(MainWindow1.hTool[0], MainWindow1.hStatic[3], MainWindow1.szTempBuf);

	switch((MainWindow1.MD5Thread1.MultiFile->FileRecode + dwItem)->FileLastError)
	{
	case ERROR_FILE_NOT_FOUND:
	case ERROR_PATH_NOT_FOUND:
	case ERROR_TOO_MANY_OPEN_FILES:
		LoadString(MainWindow1.hInst, IDS_FILESIZE_TEXT2, MainWindow1.szBuf, MAX_STRINGTABLE);
		break;
	default:
		GetSizeText(MainWindow1.szBuf, (MainWindow1.MD5Thread1.MultiFile->FileRecode + dwItem)->FileSize);
	}
	SetWindowText(MainWindow1.hStatic[5], MainWindow1.szBuf);
}

DWORD MD5File_FileOpen(BOOL IsAdd)
{
	DWORD dwRet;

	if(IsAdd == FALSE)
	{
		MainWindow1.MD5Thread1.MultiFile->FileCount = 0;
	}

	FileListBox();
	dwRet = SendMessage(FileListWindow1.hWnd, FileListWindow1.WindowMessage, APP_MESSAGE_MD5FILE_OPEN, 0);

	return dwRet;
}

BOOL ClipboardCheck(DWORD dwItem)
{
	BOOL Ret = FALSE;

	if((MainWindow1.MD5Thread1.MultiFile->FileRecode + dwItem)->FileLastError == 0)
	{
		if(OpenClipboard(MainWindow1.hWnd))
		{
			Ret = IsClipboardFormatAvailable(CF_TEXT);
			CloseClipboard();
		}
	}
	return Ret;
}

VOID Open(BOOL IsPause)
{
	if(dwAppFrag & APP_FOLDERINIT)
	{
		return;
	}

	if(dwAppFrag & APP_MD5THREAD)
	{
		if(IsPause)
			Pause();
	}
	else
	{
		OpenFile();
	}
}

BOOL OpenFile(VOID)
{
	DWORD	dwRet = 0;
	size_t	szLen = _tcslen(MainWindow1.szOfn1Buf);

	// ウィンドウロックします。
	WINDOW_LOCK;

	// 最初のファイル名以降はゼロクリアする。
	memset(MainWindow1.szOfn1Buf + szLen * sizeof(TCHAR), NULL, ((MAX_PATH_SIZE + 100) * 256 - szLen) * sizeof(TCHAR));
	*MainWindow1.szMD5File = '\0';

	if(GetOpenFileName(&MainWindow1.ofn1))
	{
		TCHAR *pFileTextPos; 
		DWORD dwCount = 0;

		dwAppFrag &= ~(APP_FOLDEROPEN | APP_FILELISTBOX_EDIT);

		// OpenFileNameの解析
		MainWindow_AddFile_Start();
		pFileTextPos = MainWindow1.szOfn1Buf + MainWindow1.ofn1.nFileOffset;

		for(;; dwCount++)
		{
			MainWindow1.MD5Thread1.IsFileNoCheck = 0;
			if(_tfullpath(MainWindow1.szFile, pFileTextPos, MAX_PATH_SIZE * 2) != NULL)
			{
				*(MainWindow1.szFile + MAX_PATH_SIZE - 1) = '\0';
			}

			dwRet = MainWindow_AddFile(dwCount);
			if(dwRet == -1)
			{
				break;
			}

			while(*pFileTextPos++ != NULL)
				;

			if(*pFileTextPos == NULL)
				break;
		}
		if(dwRet == -1)
		{
			FileRecodeFoul(MainWindow1.hWnd);
			DestroyWindow(MainWindow1.hWnd);
			return FALSE;
		}

		// 複数選択時に次回のフォルダー表示がおかしくならないように最後のファイルをしておきます。
		_tcscpy(MainWindow1.szOfn1Buf, (MainWindow1.MD5Thread1.MultiFile->FileRecode + MainWindow1.MD5Thread1.MultiFile->FileCount - 1)->FileName);
		_tcscpy(MainWindow1.szFile, MainWindow1.szOfn1Buf);

		if(MainWindow1.MD5Thread1.MultiFile->FileCount > 0)
		{
			if(MainWindow1.MD5Thread1.MultiFile->FileCount > 1)
			{
				dwAppFrag |= APP_FILELISTBOX_EDIT;
			}
			else
			{
				dwAppFrag &= ~APP_FILELISTBOX_EDIT;
			}

			GetMD5();
			dwRet = TRUE;
		}
	}

	// ウィンドウロックの解除します。
	WINDOW_UNLOCK;

	return dwRet;
}

BOOL GetMD5FilePath(VOID)
{
	DWORD dwRet = 0;


	if(MainWindow1.MD5Thread1.IsFileNoCheck == 1)
	{
		return FALSE;
	}

	dwRet = ExpandEnvironmentStrings(MainWindow1.szFile, MainWindow1.szBuf, MAX_PATH_SIZE + 100);
	if(dwRet == 0)
	{
		_tcscpy(MainWindow1.szBuf, MainWindow1.szFile);
	}

	if(PathIsDirectory(MainWindow1.szBuf))
	{
		dwAppFrag |= APP_FOLDEROPEN;

		if(*MainWindow1.szMD5File == '\0')
		{
			TCHAR *p = qtcscpy(MainWindow1.szMD5File, MainWindow1.szFile);

			if(_tcslen(MainWindow1.szFile) > 0 &&
				*(p - 1) == '\\')
			{
				*(p - 1) = '\0';
			}
			goto CREATE_FILENAME;
		}
	}
	else
	{
		BOOL IsMD5File = FALSE;

		dwAppFrag &= ~APP_FOLDEROPEN;


		if(_tcsicmp(PathFindExtension(MainWindow1.szBuf), MainWindow1.szFileExtension) == 0)
		{
			IsMD5File = TRUE;
		}
		else
		{
			TCHAR **p = MainWindow1.pChackSumFile;

			_tcscpy(MainWindow1.szTempBuf, PathFindFileName(MainWindow1.szBuf));
			while(*p != NULL)
			{
				if(_tcsicmp(MainWindow1.szTempBuf, *p) == 0)
				{
					IsMD5File = TRUE;
				}
				p++;
			}
		}


		if(IsMD5File != FALSE)
		{
			if(~dwAppFrag & APP_MD5FILE_ENABLE_HASH)
			{
				if(*MainWindow1.szMD5File == '\0')
				{
					_tcscpy(MainWindow1.szMD5File, MainWindow1.szFile);
				}
				return TRUE;
			}
			_tcscpy(MainWindow1.szMD5File, MainWindow1.szFile);
		}
		else if(*MainWindow1.szMD5File == '\0')
		{
			_tcscpy(MainWindow1.szMD5File, MainWindow1.szFile);
			PathRemoveFileSpec(MainWindow1.szMD5File);

			goto CREATE_FILENAME;
		}
	}
	return FALSE;

CREATE_FILENAME:
	if(PathIsRoot(MainWindow1.szMD5File) &&
// _tcsncmp(MainWindow1.szMD5File + 1, _T(":\\"), 2) の最適化
#ifdef _UNICODE		
		*(DWORD*)(MainWindow1.szMD5File + 1) == 0x005C003A
#else
		*(WORD*)(MainWindow1.szMD5File + 1) == 0x5C3A
#endif
	)
	{
		*(MainWindow1.szMD5File + 2) = '\0';
	}

	LoadString(MainWindow1.hInst, IDS_FILENAME, MainWindow1.szStBuf, MAX_STRINGTABLE);
	_tcscat(MainWindow1.szMD5File, MainWindow1.szStBuf);

	return FALSE;
}

VOID Save(VOID)
{
	if(dwAppFrag & APP_FOLDERINIT)
	{
		return;
	}

	if(dwAppFrag & APP_MD5THREAD)
	{
		Cancel(FALSE);
	}
	else
	{
		SaveFile();
	}

	SetFocus(MainWindow1.hButton[0]);
}

BOOL SaveFile(VOID)
{
	BOOL nRet = FALSE;

	// ウィンドウロックします。
	WINDOW_LOCK;

	MainWindow1.ofn2.Flags |= OFN_ENABLEHOOK | OFN_ENABLETEMPLATE;

	if(MainWindow1.dwAddFileModeFileCount > 0 ||
		GetSaveFileName(&MainWindow1.ofn2))
	{
		BOOL IsMD5Comp = (MainWindow1.MD5Thread1.MultiFile->FileRecode + MainWindow1.dwOpenItem)->MD5FileHashLine != MD5FILE_NOLINE ? TRUE : FALSE;

		if(MainWindow1.ofn2.nFilterIndex == 2)
		{
			dwAppFrag |= APP_OLDMD5FILE;
		}
		else
		{
			dwAppFrag &= ~APP_OLDMD5FILE;
		}
		MD5_ListSave();

		if(IsMD5Comp != FALSE)
		{
			MD5_FileView(MainWindow1.dwOpenItem);
		}
		nRet++;
	}

	// ウィンドウロックの解除します。
	WINDOW_UNLOCK;

	SetFocus(MainWindow1.hButton[0]);
	return nRet;
}

VOID MD5_ListSave(VOID)
{
	TCHAR szOption[10];
	TCHAR *p = szOption;
	DWORD dwRet;

	if(dwAppFrag & APP_FOLDERINIT)
	{
		return;
	}

	*p++ = 'c';
	*p++ = (TCHAR)(MainWindow1.dwSaveMD5FileCharCode & 0xFF) + '0';
	*p++ = dwAppFrag & APP_MD5OUT_LOWER ? 'l' : 'u';
	if(dwAppFrag & APP_OLDMD5FILE)
	{
		*p++ = 'o';
	}
	*p = '\0';

	dwRet = MD5_Thread_CreateMD5File(&MainWindow1.MD5Thread1, MainWindow1.szMD5File, szOption);

	if(!dwRet)
	{
		if(dwAppFrag & APP_FILELISTBOX)
		{
			FileList_ListView(0, FALSE);
		}
	}
	else
	{
		dwRet = GetLastError();
		LoadString(MainWindow1.hInst, IDS_MD5FILE_SAVE1, MainWindow1.szStBuf, MAX_STRINGTABLE);
		GetLastError_String(MainWindow1.szTempBuf, dwRet);
		MessageFormat(MainWindow1.szBuf, 1024, MainWindow1.szStBuf, MainWindow1.szMD5File, dwRet, MainWindow1.szTempBuf);
		MainWindow_MessageBox(MainWindow1.hWnd, MainWindow1.szBuf, MainWindow1.szTitle, MB_OK | MB_ICONERROR);
	}
}

VOID Cancel(BOOL IsClear)
{
	if(dwAppFrag & APP_FOLDERINIT)
	{
		return;
	}
	if(IsClear != FALSE)
	{
		dwAppFrag |= APP_FILE_CLEAR;
	}

	if(dwAppFrag & APP_MD5THREAD)
	{
		dwAppFrag |= APP_CALCEL;
		MainWindow1.MD5Thread1.SuspendMessage = APP_PRGRESS_CANCEL;

		if(dwAppFrag & APP_PAUSE)
		{
			dwAppFrag &= ~APP_PAUSE;
			ResumeThread(MainWindow1.MD5Thread1.MD5ThreadHandle);
		}
	}
}

VOID Pause(VOID)
{
	if(dwAppFrag & APP_FOLDERINIT)
	{
		return;
	}
	SendMessage(MainWindow1.hWnd, MainWindow1.MD5Thread1.MessageID, APP_MESSAGE_PAUSE, dwAppFrag & APP_PAUSE ? FALSE : TRUE);
}

VOID ReOpen(VOID)
{
	if(dwAppFrag & APP_FOLDERINIT)
	{
		return;
	}

	if(dwAppFrag & APP_MD5THREAD)
	{
		Cancel(FALSE);
	}
	else
	{
		if(*MainWindow1.szFile != '\0')
			GetMD5();
		else
			OpenFile();
	}
}

VOID Copy(const TCHAR *inString)
{
	dwAppFrag |= APP_WINDOW_NOALPHA;
	SetClipboardText(MainWindow1.hWnd, inString);
	dwAppFrag &= ~APP_WINDOW_NOALPHA;
}

VOID Paste(VOID)
{
	if(~dwAppFrag & APP_MD5THREAD)
	{
		dwAppFrag |= APP_WINDOW_NOALPHA;
		SetFocus(MainWindow1.hEdit[1]);

		if(GetClipboardText(MainWindow1.hWnd, MainWindow1.szBuf, HASH_LEN * 8))
		{
			FixString(MainWindow1.szBuf);
			_tcscpy(MainWindow1.szMD5CompString, MainWindow1.szBuf);
			SetWindowText(MainWindow1.hEdit[1], MainWindow1.szMD5CompString);
		}
		dwAppFrag &= ~APP_WINDOW_NOALPHA;
	}
}

VOID MD5_TimeView(BOOL bView)
{
	static LONGLONG llReadSize[2];
	static BOOL IsView;
	DWORD dwTime;


	if(dwAppFrag & (APP_PAUSE | APP_LASTERROR))
	{
		return;
	}

	if(dwAppFrag & APP_DISABLE_MULTIFILE_UPDATE_DRAW &&
		MainWindow1.dwOpenItem > MainWindow1.dwTimeViewFileCount)
	{
		MainWindow1.dwTimeViewFileCount = MainWindow1.dwOpenItem;
		SetFileInfo(MainWindow1.dwTimeViewFileCount);
	}

	dwTime = timeGetTime();
	MainWindow1.dwCurrentTime = dwTime < MainWindow1.dwStartTime ? (DWORD)((ULONGLONG)dwTime + 0x100000000 - MainWindow1.dwStartTime) : dwTime - MainWindow1.dwStartTime;

#ifndef _DEBUG
	if(MainWindow1.dwCurrentTime < 500)
	{
		IsView = 0;
		return;
	}
#endif
	IsView = IsView ? FALSE : TRUE;


	if(bView != FALSE ||
		IsView != FALSE)
	{
		llReadSize[0] = MainWindow1.MD5Thread1.MultiFile->FileAllCurrentSize + MainWindow1.MD5Thread1.FileReadSize;
		MainWindow1.dwEndTime = MainWindow1.dwCurrentTime;

		if(llReadSize[0] == 0)
		{
			llReadSize[1] = 0;
			return;
		}
		if(llReadSize[1] > llReadSize[0])
		{
			llReadSize[1] = 0;
		}

		if(bView != FALSE)
		{
			if(MainWindow1.dwEndTime > 0)
				MainWindow1.szTimeView1_nTenso = (LONGLONG)ceil((double)llReadSize[0] / MainWindow1.dwCurrentTime * 1000);
			else
				MainWindow1.szTimeView1_nTenso = llReadSize[0];

#ifdef _UNICODE
			StrFormatByteSize(MainWindow1.szTimeView1_nTenso, MainWindow1.szTimeView1_Tenso, NUMBER_LOADSTRING);
			StrFormatByteSize(MainWindow1.MD5Thread1.MultiFile->FileAllSize, MainWindow1.szTimeView1_Size1, NUMBER_LOADSTRING);
#else
			StrFormatByteSize64(MainWindow1.szTimeView1_nTenso, MainWindow1.szTimeView1_Tenso, NUMBER_LOADSTRING);
			StrFormatByteSize64(MainWindow1.MD5Thread1.MultiFile->FileAllSize, MainWindow1.szTimeView1_Size1, NUMBER_LOADSTRING);
#endif

			StrFromTimeInterval(MainWindow1.szTimeView1_Time, NUMBER_LOADSTRING, MainWindow1.dwCurrentTime, 3);
			MessageFormat(MainWindow1.szTimeText, MAX_LOADSTRING, MainWindow1.szTimeText1[1], MainWindow1.szTimeView1_Time, MainWindow1.szTimeView1_Size1, MainWindow1.szTimeView1_Tenso);
			SetWindowText(MainWindow1.hStatic[1], MainWindow1.szTimeText);
			MainWindow1.dwBackTime	= 0;
			llReadSize[1]			= 0;
		}
		else
		{
			if(MainWindow1.dwBackTime == 0)
				MainWindow1.dwBackTime = MainWindow1.dwStartTime;

			MainWindow1.szTimeView1_nTenso = (LONGLONG)((double)(llReadSize[0] - llReadSize[1]) / ((dwTime - MainWindow1.dwBackTime) / 1000.));

#ifdef _UNICODE
			StrFormatByteSize(MainWindow1.szTimeView1_nTenso, MainWindow1.szTimeView1_Tenso, NUMBER_LOADSTRING);
			StrFormatByteSize(MainWindow1.MD5Thread1.MultiFile->FileAllSize, MainWindow1.szTimeView1_Size1, NUMBER_LOADSTRING);
			StrFormatByteSize(MainWindow1.MD5Thread1.MultiFile->FileAllCurrentSize + MainWindow1.MD5Thread1.FileReadSize, MainWindow1.szTimeView1_Size2, NUMBER_LOADSTRING);
#else
			StrFormatByteSize64(MainWindow1.szTimeView1_nTenso, MainWindow1.szTimeView1_Tenso, NUMBER_LOADSTRING);
			StrFormatByteSize64(MainWindow1.MD5Thread1.MultiFile->FileAllSize, MainWindow1.szTimeView1_Size1, NUMBER_LOADSTRING);
			StrFormatByteSize64(MainWindow1.MD5Thread1.MultiFile->FileAllCurrentSize + MainWindow1.MD5Thread1.FileReadSize, MainWindow1.szTimeView1_Size2, NUMBER_LOADSTRING);
#endif

			MainWindow1.szTimeView1_dwTime = MainWindow1.szTimeView1_nTenso != 0 ? (DWORD)ceil((MainWindow1.MD5Thread1.MultiFile->FileAllSize - (MainWindow1.MD5Thread1.MultiFile->FileAllCurrentSize + MainWindow1.MD5Thread1.FileReadSize)) / (double)MainWindow1.szTimeView1_nTenso) : 0;

			StrFromTimeInterval(MainWindow1.szTimeView1_Time, NUMBER_LOADSTRING, MainWindow1.szTimeView1_dwTime * 1000, 3);
			MessageFormat(MainWindow1.szTimeText, MAX_LOADSTRING, MainWindow1.szTimeText1[0], MainWindow1.szTimeView1_Time, MainWindow1.szTimeView1_Size1, MainWindow1.szTimeView1_Size2, MainWindow1.szTimeView1_Tenso);
			SetWindowText(MainWindow1.hStatic[1], MainWindow1.szTimeText);
			MainWindow1.dwBackTime	= dwTime;
			llReadSize[1]			= llReadSize[0];
		}
	}
}

VOID MD5_HashComp(VOID)
{
	if(*MainWindow1.szMD5String == '\0')
	{
		return;
	}

	dwAppFrag |= APP_WINDOW_NOALPHA;
	FixString(MainWindow1.szMD5CompString);

	if(_tcsicmp(MainWindow1.szMD5CompString, MainWindow1.szMD5String))
	{
		SendMessage(MainWindow1.hBmp[0], STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)MainWindow1.hBitmap[5]);
		StatusOut(MainWindow1.szStatusText[6]);
		MessageFormat(MainWindow1.szBuf, 1024, MainWindow1.szWindowTitle[0], MainWindow1.szStatusText[6], MainWindow1.szTitle);
		SetWindowText(MainWindow1.hWnd, MainWindow1.szBuf);
		MessageBeep(MB_ICONERROR);
	}
	else
	{
		SendMessage(MainWindow1.hBmp[0], STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)MainWindow1.hBitmap[4]);
		StatusOut(MainWindow1.szStatusText[5]);
		MessageFormat(MainWindow1.szBuf, 1024, MainWindow1.szWindowTitle[0], MainWindow1.szStatusText[5], MainWindow1.szTitle);
		SetWindowText(MainWindow1.hWnd, MainWindow1.szBuf);
		MessageBeep(MB_ICONASTERISK);
	}
	dwAppFrag &= ~APP_WINDOW_NOALPHA;
}

VOID MD5_FileView(DWORD dwItem)
{
	if(dwAppFrag & APP_PAUSE ||
		(MainWindow1.MD5Thread1.MultiFile->FileCount > 0 &&
		dwItem > MainWindow1.MD5Thread1.MultiFile->FileCount))
	{
		MessageBeep(MB_ICONERROR);
		return;
	}


	if(MainWindow1.MD5Thread1.MultiFile->FileCount == 0)
	{
		SetWindowText(MainWindow1.hWnd, MainWindow1.szTitle);
		MainWindow1.dwOpenItem = 0;
		MainWindow1.MD5Thread1.MultiFile->FileRecode->FileLastError = (DWORD)-1;
		SendMessage(MainWindow1.hBmp[0], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)MainWindow1.hBitmap[2]);
		StatusOut(MainWindow1.szStatusText[7]);
		LoadString(MainWindow1.hInst, IDS_STATIC_TEXT2, MainWindow1.szStBuf, MAX_STRINGTABLE);
		SetWindowText(MainWindow1.hStatic[1], MainWindow1.szStBuf);
		SetWindowText(MainWindow1.hEdit[1], _T(""));

		goto NO_FILE_STEP;
	}

	if((MainWindow1.MD5Thread1.MultiFile->FileRecode + dwItem)->FileLastError != 0)
	{
		DWORD dwLastError = (MainWindow1.MD5Thread1.MultiFile->FileRecode + dwItem)->FileLastError;

		switch(dwLastError)
		{
		case 0:
			break;
		case -1:
			if(dwAppFrag & APP_CALCEL)
				StatusOut(MainWindow1.szStatusText[3]);
			else
			{
				StatusOut(MainWindow1.szStatusText[8]);
			}
			break;
		default:
			SendMessage(MainWindow1.hBmp[0], STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)MainWindow1.hBitmap[2]);
			MessageFormat(MainWindow1.szBuf, 1024, MainWindow1.szStatusText[4], (DWORD)dwLastError);
			StatusOut(MainWindow1.szBuf);
			MessageFormat(MainWindow1.szBuf, 1024, MainWindow1.szWindowTitle[2], (DWORD)dwLastError, MainWindow1.szTitle);
			SetWindowText(MainWindow1.hWnd, MainWindow1.szBuf);
			GetLastErrorMsg((DWORD)dwLastError);
		}

NO_FILE_STEP:
		EnableWindow(MainWindow1.hEdit[0], FALSE);
		EnableWindow(MainWindow1.hEdit[1], FALSE);
		EnableWindow(MainWindow1.hButton[1],  dwAppState ? FALSE : TRUE);
		EnableWindow(MainWindow1.hButton[2],  FALSE);
		EnableWindow(MainWindow1.hButton[3],  FALSE);
		EnableMenuItem(MainWindow1.hPupMenu1, 0, MF_BYPOSITION | MF_GRAYED);
		EnableMenuItem(MainWindow1.hPupMenu1, 1, MF_BYPOSITION | MF_GRAYED);
		EnableMenuItem(MainWindow1.hPupMenu1, 3, MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(MainWindow1.hPupMenu1, 4, MF_BYPOSITION | MF_GRAYED);
		EnableMenuItem(MainWindow1.hPupMenu1, 6, MF_BYPOSITION | MF_GRAYED);
		EnableMenuItem(MainWindow1.hPupMenu1, 7, MF_BYPOSITION | MF_GRAYED);
	}
	else
	{
		SendMessage(MainWindow1.hBmp[0], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)MainWindow1.hBitmap[1]);
		SetWindowText(MainWindow1.hWnd, MainWindow1.szTitle);
		StatusOut(MainWindow1.szStatusText[1]);
		SetWindowText(MainWindow1.hStatic[1], MainWindow1.szTimeText);
		EnableWindow(MainWindow1.hEdit[0], TRUE);
		EnableWindow(MainWindow1.hEdit[1], TRUE);
		if(~dwAppFrag & APP_MD5FILE_ADDMODE)
		{
			EnableWindow(MainWindow1.hButton[1], TRUE);
			EnableMenuItem(MainWindow1.hPupMenu1, 3, MF_BYPOSITION | MF_ENABLED);
		}
		EnableWindow(MainWindow1.hButton[2], TRUE);
		EnableWindow(MainWindow1.hButton[3], ClipboardCheck(dwItem));
		EnableMenuItem(MainWindow1.hPupMenu1, 0, MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(MainWindow1.hPupMenu1, 3, MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(MainWindow1.hPupMenu1, 6, MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(MainWindow1.hPupMenu1, 7, MF_BYPOSITION | MF_ENABLED);
	}
	SetFileInfo(dwItem);

	if((MainWindow1.MD5Thread1.MultiFile->FileRecode + dwItem)->FileLastError == 0)
	{
		MD5_Thread_MD5ToString(MainWindow1.szMD5String, (MainWindow1.MD5Thread1.MultiFile->FileRecode + dwItem)->FileHashByte, ~dwAppFrag & APP_MD5OUT_LOWER);
	}
	else
	{
		*MainWindow1.szMD5String = '\0';
	}
	SetWindowText(MainWindow1.hEdit[0], MainWindow1.szMD5String);

	if(~dwAppFrag & APP_MD5FILE_ADDMODE &&
		(MainWindow1.MD5Thread1.MultiFile->FileRecode + dwItem)->MD5FileHashLine != MD5FILE_NOLINE)
	{
		dwAppFrag |= APP_MD5_INPUT_EDIT;

		MD5_Thread_MD5ToString(MainWindow1.szMD5CompString, (MainWindow1.MD5Thread1.MultiFile->FileRecode + dwItem)->MD5FileHash, ~dwAppFrag & APP_MD5OUT_LOWER);
		SendMessage(MainWindow1.hEdit[1], WM_SETFONT, (WPARAM)MainWindow1.hFont[2], 0);
		SetWindowText(MainWindow1.hEdit[1], MainWindow1.szMD5CompString);
	}
	return;
}

VOID StatusOut(const TCHAR *inString)
{
	MessageFormat(MainWindow1.szStBuf, 1024, MainWindow1.szStatusTitle, inString);
	SetWindowText(MainWindow1.hStatic[0], MainWindow1.szStBuf);
	return;
}

BOOL OpenFileCancel(VOID)
{
	SendMessage(MainWindow1.hBmp[0], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)MainWindow1.hBitmap[2]);
	EnableWindow(MainWindow1.hEdit[1], FALSE);
	EnableWindow(MainWindow1.hButton[1], FALSE);
	EnableWindow(MainWindow1.hButton[2], FALSE);
	EnableWindow(MainWindow1.hButton[3], FALSE);
	DragAcceptFiles(MainWindow1.hWnd, TRUE);
	SetFocus(MainWindow1.hButton[0]);

	return TRUE;
}

BOOL GetMD5(VOID)
{
	if(MainWindow1.MD5Thread1.MultiFile->FileCount == 0)
	{
		LoadString(MainWindow1.hInst, IDS_FILE_OPEN1, MainWindow1.szStBuf, MAX_STRINGTABLE);
		MessageFormat(MainWindow1.szBuf, 1024, MainWindow1.szStBuf, MainWindow1.szMD5File);
		MainWindow_MessageBox(MainWindow1.hWnd, MainWindow1.szBuf, MainWindow1.szTitle, MB_OK | MB_ICONERROR);

		return FALSE;
	}

	if((MainWindow1.MD5Thread1.MultiFile->FileCurrentCount + 1) >= MainWindow1.MD5Thread1.MultiFile->FileCount)
	{
		MainWindow1.MD5Thread1.MultiFile->FileCurrentCount = MainWindow1.dwAddFileModeFileCount != 0 ?
			MainWindow1.dwAddFileModeFileCount : 0;
	}

	dwAppFrag |= APP_MD5THREAD;

	MainWindow1.dwEndTime = 0;
	MainWindow1.MD5Thread1.MD5ThreadHandle = (HANDLE)_beginthreadex(NULL, 0, MD5_MultiFile_Thread, &MainWindow1.MD5Thread1, 0, 0);

	if(MainWindow1.MD5Thread1.MD5ThreadHandle == NULL)
	{
		dwAppFrag &= ~APP_MD5THREAD;
		DebugText(DEBUG_MISSING, _T("MD5スレッドの作成"), GetLastError());
	}

	return TRUE;
}

BOOL GetSizeText(TCHAR *OutText, const LONGLONG Number)
{
	TCHAR szText1[NUMBER_LOADSTRING];
	TCHAR szText2[NUMBER_LOADSTRING];
	TCHAR szVal  [NUMBER_LOADSTRING];
	NUMBERFMT nfmt;

	nfmt.NumDigits		= 0;
	nfmt.LeadingZero	= 0;
	nfmt.Grouping		= 3;
	nfmt.lpDecimalSep	= _T(".");
	nfmt.lpThousandSep	= _T(",");
	nfmt.NegativeOrder	= 1;


#ifdef _UNICODE
	StrFormatByteSize(Number, szText1, NUMBER_LOADSTRING-1);
#else
	StrFormatByteSize64(Number, szText1, NUMBER_LOADSTRING-1);
#endif
	_i64tot(Number, szVal, 10);

	GetNumberFormat(LOCALE_SYSTEM_DEFAULT, 0, szVal, &nfmt, szText2, NUMBER_LOADSTRING);
	LoadString(MainWindow1.hInst, IDS_FILESIZE_TEXT1, MainWindow1.szStBuf, MAX_STRINGTABLE);
	MessageFormat(OutText, MAX_LOADSTRING, MainWindow1.szStBuf, szText1, szText2);

	return TRUE;
}

// デバッグ用にテキストの出力する。
// 引数2の文字列は 100文字以内 にする。
VOID DebugText(DWORD dwType, const TCHAR* inString, DWORD dwLastError)
{
	TCHAR szOutputText[MAX_LOADSTRING];

	switch(dwType)
	{
	case DEBUG_PLAIN_TEXT:
		_tcscpy(szOutputText, inString);
		break;
	case DEBUG_ALLOC_ERROR:
		MessageFormat(szOutputText, MAX_LOADSTRING, _T("メモリの動的確保に失敗しました。 - %1"), inString);
		break;
	case DEBUG_MISSING:
		if(dwLastError != 0 &&
			MainWindow1.szBuf != NULL)
		{
			GetLastError_String(MainWindow1.szBuf, dwLastError);
			MessageFormat(szOutputText, MAX_LOADSTRING, _T("%1に失敗しました。%nエラーの詳細(#%2!u!):%3"), inString, dwLastError, MainWindow1.szBuf);
		}
		else
			MessageFormat(szOutputText, MAX_LOADSTRING, _T("%1に失敗しました。"), inString);
		break;
	case DEBUG_FALSE_RETURN:
		MessageFormat(szOutputText, MAX_LOADSTRING, _T("%1 が FALSE 返しました。"), inString);
		break;
	}

#ifdef _DEBUG
	_tcscat(szOutputText, _T("\r\n"));
	szOutputText[99] = '\0';
	OutputDebugString(szOutputText);
#endif
	dwAppFrag |= APP_WINDOW_NOALPHA;

	MainWindow_MessageBox(MainWindow1.hWnd, szOutputText, MainWindow1.szTitle, MB_OK | MB_ICONERROR);
	dwAppFrag &= ~APP_WINDOW_NOALPHA;
}

DWORD GetLastErrorMsg(DWORD dwMsg)
{
	GetLastError_String(MainWindow1.szBuf, dwMsg);
	SetWindowText(MainWindow1.hStatic[1], MainWindow1.szBuf);

#ifdef _DEBUG
	OutputDebugString(_T("MainWindow: GetLastErrorMsg()\r\n"));
	OutputDebugString(MainWindow1.szBuf);
	OutputDebugString(_T("\r\n"));
#endif

	return dwMsg;
}

DWORD GetLastError_String(TCHAR *inString, DWORD dwMsg)
{
	LPVOID lpMsgBuf = NULL;
	TCHAR *dst = inString;
	TCHAR *cp;


	if(dwMsg == 0)
	{
		dwMsg = GetLastError();
	}

	if(FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwMsg,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL) != NULL)
	{
		cp = (TCHAR*)lpMsgBuf;

		for (; *cp != NULL;)
		{
#ifdef _UNICODE
			if(*(DWORD*)cp == 0x000A000D)
#else
			if (*(WORD*)cp == 0x0A0D)
#endif
			{
				cp += 2;
			}
			else
			{
				*dst++ = *cp++;
			}
		}
		*dst = '\0';
		LocalFree(lpMsgBuf);
	}

	return dwMsg;
}

BOOL SetLayeredWindow(HWND hWnd)
{
	BOOL ret = FALSE;

#if _MSC_VER < 1500
	if(GetVerifyVersionInfo(5, 0, 0))
	{
#endif /* _MSC_VER < 1500 */
		LONG lStyle;

		lStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
		lStyle |= WS_EX_LAYERED;
		ret = (BOOL)SetWindowLong(hWnd, GWL_EXSTYLE, lStyle);
#if _MSC_VER < 1500
	}
#endif /* _MSC_VER < 1500 */
	return ret;
}

#if _MSC_VER < 1500
BOOL SetAlphaWindow(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags)
{
	if(DllSetLayeredWindowAttributes != NULL &&
		DllSetLayeredWindowAttributes(hWnd, crKey, bAlpha, dwFlags))
	{
		return 1;
	}
	return FALSE;
}
#endif /* _MSC_VER < 1500 */

DWORD MessageFormat(TCHAR *lpBuffer, const DWORD nSize, const TCHAR *lpFormat, ...)
{
	DWORD	dwRet;
	va_list	list;

	va_start(list, lpFormat);
	dwRet = FormatMessage(
		FORMAT_MESSAGE_FROM_STRING,
		lpFormat,
		0,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
		lpBuffer,
		nSize,
		&list
		);
	va_end(list);
 
	return dwRet;
}

VOID FixString(TCHAR* lpString)
{
	int len = (int)_tcslen(lpString);

	if(len > 0)
	{
#ifdef _UNICODE
		TCHAR szText1[HASH_LOADSTRING];
		TCHAR *p1, *p2, *p3;
#else
		char szText1[HASH_LOADSTRING];
		char *p1, *p2, *p3;
		unsigned char szBuf[HASH_LEN * 8 + 1];
		unsigned char *s1, *s2;
#endif
		int i = 0;

		if(len < HASH_LEN * 2)
		{
			len = HASH_LEN * 2;
		}


		// 全角英数字があれば半角に変換する
#ifdef _UNICODE
		p1 = lpString;

		for(; *p1 != NULL; p1++)
		{
			if((*p1 >= 0xFF10 && *p1 <= 0xFF19) || (*p1 >= 0xFF21 && *p1 <= 0xFF3A) || (*p1 >= 0xFF41 && *p1 <= 0xFF5A))
				*p1 -= 0xFEE0;
			else
			{
				if((*p1 >= 0x30 && *p1 <= 0x39) || (*p1 >= 0x41 && *p1 <= 0x5A) || (*p1 >= 0x61 && *p1 <= 0x7A))
					;
				else
					*p1 = 0x20;
			}
		}
#else
		s1 = (unsigned char*)lpString;
		s2 = szBuf;
		szBuf[128] = NULL;

		for(; *s1 != NULL; s1++, s2++)
		{
			if(*s1 == 0x82)
			{
				s1++;
				if((*s1 >= 0x4f && *s1 <= 0x58) || (*s1 >= 0x60 && *s1 <= 0x79))
					*s2 = *s1 - 0x1F;
				else if(*s1 >= 0x81 && *s1 <= 0x9A)
					*s2 = *s1 - 0x20;
				else if(*s1 != '\0')
					*s2 = 0x20;
				else
				{
					*s2 = '\0';
					break;
				}
			}
			else
			{
				if((*s1 >= 0x30 && *s1 <= 0x39) || (*s1 >= 0x41 && *s1 <= 0x5A) || (*s1 >= 0x61 && *s1 <= 0x7A))
					*s2 = *s1;
				else
					*s2 = 0x20;
			}
		}
		*s2 = '\0';
		strcpy(szText1, (char*)szBuf);
#endif

		// MD5の文字列があるか探す
		p3 = _tcsstr(lpString, _T("MD5"));
		if(p3 == NULL)
		{
			p3 = _tcsstr(lpString, _T("md5"));
		}

		if(p3 != NULL)
		{
			p1 = p3 + 3;
			i  = (int)(p3 - lpString) + 3;
		}
		else
			p1 = lpString;
		p2 = szText1;

		for(int n = 0; i < len; i++, p1++)
		{
			if(n < HASH_LEN * 2 &&
				(*p1 >= _T('0') && *p1 <= _T('9') ||
				*p1 >= _T('a') && *p1 <= _T('f') ||
				*p1 >= _T('A') && *p1 <= _T('F')))
			{
				*p2++ = *p1;
				n++;
			}
		}
		*p2++ = '\0';

		_tcsncpy(lpString, szText1, len);
		lpString[HASH_LEN * 2] = '\0';
	}
}

LONGLONG GetFileSize(const TCHAR *inFile)
{
	HANDLE hFind = NULL;
	WIN32_FIND_DATA FindData;
	LONGLONG FileSize = 0;
	TCHAR szBuf[MAX_PATH_SIZE + 100];

	if(!ExpandEnvironmentStrings(inFile,
		szBuf,
		MAX_PATH_SIZE + 100))
	{
		_tcscpy(szBuf, inFile);
	}

	*szBuf = '\0';
	hFind = FindFirstFile(szBuf, &FindData);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		FileSize = FindData.nFileSizeLow;
		if(FindData.nFileSizeHigh > 0)
		{
			FileSize += (LONGLONG)FindData.nFileSizeHigh << 32;
		}
		FindClose(hFind);
	}
	return FileSize;
}

TCHAR *GetFileVersion(TCHAR *outVersionString, const TCHAR *inFilePath, const TCHAR *inQueryValue)
{
	TCHAR *pRet			= NULL;
	DWORD dwZero		= 0;
	DWORD dwVerInfoSize	= GetFileVersionInfoSize(inFilePath, &dwZero);


	if(inQueryValue == NULL)
	{
		inQueryValue = _T("\\StringFileInfo\\041104b0\\ProductVersion");
	}

	if(dwVerInfoSize != 0)
	{
		VOID *pVffInfo;
		VOID *pvVersion;
		UINT VersionLen;

		pVffInfo = malloc(dwVerInfoSize);
		if(pVffInfo == NULL)
		{
			return FALSE;
		}
		GetFileVersionInfo(inFilePath, NULL, dwVerInfoSize, pVffInfo);
		VerQueryValue(pVffInfo, inQueryValue, &pvVersion, &VersionLen);
		pRet = _tcscpy(outVersionString, (TCHAR*)pvVersion);

		free(pVffInfo);
	}
	return pRet;
}

TCHAR *qtcscpy(TCHAR *dst, const TCHAR *src)
{
	while(*src)
		*dst++ = *src++;

	*dst = '\0';
	return dst;
}
