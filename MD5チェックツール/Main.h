#pragma once

#ifndef __Main_H__
#define __Main_H__

#include <intrin.h>
#include <winioctl.h>
#include <math.h>
#include <time.h>
#include "hashthread.h"
#include "resource.h"

#ifndef MAX_PATH_SIZE
#ifdef _UNICODE
#define MAX_PATH_SIZE	_MAX_PATH
#else
#define MAX_PATH_SIZE	(_MAX_PATH * 2)
#endif
#endif

#define MAX_STRINGTABLE						255
#define MAX_LOADSTRING						100
#define NUMBER_LOADSTRING					33

#define ACTIVE_ALPHA						255
#define INACTIVE_ALPHA						192

#define BENCH_FILESIZE						104857600

// アプリケーションフラグの設定します。
#define APP_WINDOW							0x00000001
#define APP_WINDOW_NOALPHA					0x00000002
#define APP_MD5THREAD						0x00000004
#define APP_MD5_INPUT_EDIT					0x00000008
#define APP_PAUSE							0x00000010
#define APP_CALCEL							0x00000020
#define APP_FOLDERINIT						0x00000040
#define APP_FOLDEROPEN						0x00000080
#define APP_OLDMD5FILE						0x00000100
#define APP_ABOUTBOX						0x00000200
#define APP_GOKANMODE						0x00000400
#define APP_MD5FILE_ADDMODE					0x00000800
#define APP_FILE_CLEAR						0x00001000
#define APP_MD5OUT_LOWER					0x00010000
#define APP_WINDOWALPHA						0x00020000
#define APP_MD5FILE_ENABLE_HASH				0x00040000
#define APP_ENABLE_FILELIST					0x00080000
#define APP_FILE_NOCACHE					0x00100000
#define APP_FILE_MAPPING					0x00200000
#define APP_DISABLE_MULTIFILE_UPDATE_DRAW	0x00400000
#define APP_ENABLE_HIDDENFILE				0x00800000
#define APP_EDIT_CHACKSUMFILE				0x01000000
#define APP_PROGRESS_THREAD					0x02000000
#define APP_FILELISTBOX						0x04000000
#define APP_FILELISTBOX_EDIT				0x08000000
#define APP_EXIT							0x40000000
#define APP_LASTERROR						0x80000000

// ウインドウメッセージ
#define APP_MESSAGE_CHANGE_FILEVIEW			0x00100000
#define APP_MESSAGE_MD5FILE_OPEN			0x00200000
#define APP_MESSAGE_FILECLEAR				0x00010000

#define DEBUG_PLAIN_TEXT    				0				// そのまま文字列を出力する。
#define DEBUG_ALLOC_ERROR 					1				// 動的メモリの確保に失敗しましたの文字列を出力する。
#define DEBUG_MISSING     					2				// ○○に失敗しましたの文字列を出力する。
#define DEBUG_FALSE_RETURN					3				// 関数が FALSE 返しました。


typedef struct tagMainWindow {
	DWORD			dwStructSize;			// バージョンアップ用に予約
	HWND			hWnd;
	HINSTANCE		hInst;					// 現在のインターフェイス
	HANDLE			hMutex;
	MD5Thread		MD5Thread1;
	HWND			hStatic[6];
	HWND			hBmp[1];
	HWND			hProgress[1];
	HWND			hEdit[3];
	HWND			hButton[6];
	HWND			hGroup[1];
	HWND			hTool[1];
	HMENU			hMenu1;
	HMENU			hPupMenu1;
	HICON			hIcon;
	HFONT			hFont[3];
	HBITMAP			hBitmap[7];
	OPENFILENAME	ofn1;
	OPENFILENAME	ofn2;
	ITaskbarList3	*pTaskbarList3;
	TCHAR			*szBuf;
	TCHAR			*szStBuf;
	TCHAR			*szTempBuf;
	TCHAR			*szTitle;				// タイトル バーのテキスト
	TCHAR			*szWindowClass;			// メイン ウィンドウ クラス名
	TCHAR			*szVersion;
	TCHAR			*szCopyRight;
	TCHAR			*szStatusTitle;
	TCHAR			*szMD5String;
	TCHAR			*szMD5CompString;
	TCHAR			*szInputtext;
	TCHAR			*szFile;
	TCHAR			*szMD5File;
	TCHAR			*szINIFile;
	TCHAR			*szFileExtension;
	TCHAR			**pChackSumFile;
	TCHAR			*szChackSumFile;
	TCHAR			*szNotFoundFile;
	TCHAR			*szEmptyFolder;
	TCHAR			*szTimeText;
	TCHAR			*szTimeView1_Tenso;
	TCHAR			*szTimeView1_Size1;
	TCHAR			*szTimeView1_Size2;
	TCHAR			*szTimeView1_Time;
	TCHAR			*szOfn1Buf;
	TCHAR			*szWindowTitle[3];
	TCHAR			*szStatusText[9];
	TCHAR			*szButtonText1[3];
	TCHAR			*szButtonText2[2];
	TCHAR			*szToolTipText[9];
	TCHAR			*szTimeText1[2];
	LONGLONG		szTimeView1_nTenso;
	DWORD			szTimeView1_dwTime;
	DWORD			dwOpenItem;
	DWORD			dwStartTime;
	DWORD			dwCurrentTime;
	DWORD			dwEndTime;
	DWORD			dwBackTime;
	DWORD			dwPersent;
	DWORD			dwTimeViewFileCount;
	DWORD			dwLasterror;
	DWORD			dwSaveMD5FileCharCode;	// 0.Shift_JIS　1.Unicode　2.UTF-8
	BOOL			IsSubFolder;
	BOOL			IsNotFoundFile;
	BOOL			IsEmptyFolder;
	BOOL			IsWindowCancel;
	DWORD			dwAddFileModeFileCount;
	COLORREF		GrayTextColor;
} MainWindow, *lpMainWindow;

#define WINDOW_LOCK if(dwAppFrag & APP_FILELISTBOX) \
EnableWindow(FileListWindow1.hWnd, FALSE); \
if(dwAppFrag & APP_ABOUTBOX) \
EnableWindow(AboutWindow1.hWnd, FALSE)

#define WINDOW_UNLOCK if(dwAppFrag & APP_FILELISTBOX) \
EnableWindow(FileListWindow1.hWnd, TRUE); \
if(dwAppFrag & APP_ABOUTBOX) \
EnableWindow(AboutWindow1.hWnd, TRUE)

#define BENCH_LOOP 250000

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(int);
LRESULT CALLBACK	WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
UINT    CALLBACK	OFN2HookProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int		MainWindow_MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);
VOID	Bench(VOID);
DWORD	ReadMeOpen(VOID);
DWORD	VersionCheck(VOID);
BOOL	GetMD5(VOID);
BOOL	OpenFile(VOID);
BOOL	OpenFileCancel(VOID);
BOOL	SaveFile(VOID);
BOOL	SetLayeredWindow(HWND hWnd);
BOOL	GetSizeText(TCHAR *OutText, const LONGLONG Number);
BOOL	GetMD5FilePath(VOID);
VOID	MD5_FileView(DWORD dwItem);
VOID	MD5_HashComp(VOID);
VOID	MD5_TimeView(BOOL bView);
VOID	SetOSText(HWND hWnd, BOOL VerifyVersion);
VOID	DebugText(DWORD dwType, const TCHAR *inString, DWORD dwLastError);
VOID	StatusOut(const TCHAR *inString);
VOID	FixString(TCHAR *lpString);
TCHAR	*qtcscpy(TCHAR *dst, const TCHAR *src);
VOID	Copy(const TCHAR *inString), Paste(VOID);
VOID	Open(BOOL IsPause);
VOID	Save(VOID);
VOID	MD5_ListSave(VOID);
VOID	ReOpen(VOID);
VOID	Cancel(BOOL IsClear);
VOID	Clear(VOID);
VOID	Pause(VOID);
DWORD	MD5File_FileOpen(BOOL IsAdd);
BOOL	ClipboardCheck(DWORD dwItem);
VOID	SetFileInfo(DWORD dwItem);
DWORD	GetSubFolderCount(const TCHAR *inFile);
VOID	MainWindow_AddFile_Start(VOID);
DWORD	MainWindow_AddFile(UINT nCount);
VOID	MainWindow_AddFile_End(VOID);
LONGLONG	GetFileSize(const TCHAR *inFile);
DWORD	GetLastErrorMsg(DWORD dwMsg);
DWORD	GetLastError_String(TCHAR *inString, DWORD dwMsg);
DWORD	MessageFormat(TCHAR *lpBuffer, const DWORD nSize, const TCHAR *lpszFormat, ...);
TCHAR	*GetFileVersion(TCHAR *outVersionString, const TCHAR *inFilePath, const TCHAR *inQueryValue);
#if _MSC_VER < 1500
BOOL SetAlphaWindow(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
#else
#define SetAlphaWindow(hWnd, crKey, bAlpha, dwFlags) \
SetLayeredWindowAttributes(hWnd, crKey, bAlpha, dwFlags)
#endif /* __MSC_VER < 1500 */
#endif /* __Main_H__ */
