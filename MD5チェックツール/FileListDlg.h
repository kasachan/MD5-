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

#ifndef __FILELISTDLG_H__
#define __FILELISTDLG_H__

#include "Main.h"
#include "AboutDlg.h"
#include "hashthread.h"
#include "WindowSize.h"
#include "resource.h"

#define APP_FILEDLG_FOLDER_OPEN		0x00000001
#define APP_FILEDLG_FOLDER_EXIT		0x00000002

typedef struct tagListItem {
	DWORD		dwListSelectCount;
	DWORD		dwListBufSize;
	DWORD		*dwListSelectItem;
} FileListItem, *lpFileListItem;

typedef struct tagFileListWindow {
	DWORD		dwStructSize; // バージョンアップ用に予約
	HWND		hWnd;
	UINT		WindowMessage;
	HWND		hList1;
	HWND		hStatus1;
	HWND		hGroup1[1];
	HWND		hButton1[4];
	HWND		hRadio1[3];
	HMENU		hMenu1;
	HMENU		hPupMenu1[2];
	int			FileSortSubNo[4];
	TCHAR		*szBuf;
	TCHAR		*szStBuf;
	TCHAR		*szFileChangeMD5;
	TCHAR		*szButtonText1[3];
	TCHAR		*szButtonText2[2];
	DWORD		dwNewFile;
	DWORD		dwFileCount;
	DWORD		dwOpenOption;
	DWORD		dwDragListNo;
	DWORD		dwMoveListNo;
	DWORD		dwMD5CheckCount[3];
	BOOL		IsMD5FileCompMassege;
	COLORREF	ListTextColor[3];
	UINT		Status1Height;
	BOOL		IsSubFolder;
	BOOL		IsListLock;
	tagListItem	FileListItem1;
} FileListWindow, *lpFileListWindow;

#define FILELIST_WINDOW_LOCK EnableWindow(MainWindow1.hWnd, FALSE); \
if(dwAppFrag & APP_ABOUTBOX) \
EnableWindow(AboutWindow1.hWnd, FALSE)

#define FILELIST_WINDOW_UNLOCK EnableWindow(MainWindow1.hWnd, TRUE); \
if(dwAppFrag & APP_ABOUTBOX) \
EnableWindow(AboutWindow1.hWnd, TRUE)

// このコード モジュールに含まれる関数の宣言を転送します:
INT_PTR CALLBACK FileList_WndProc(HWND, UINT, WPARAM, LPARAM);
int CALLBACK FileList_BrowseCallbackProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int CALLBACK FileList_SortProc(LPARAM, LPARAM, LPARAM);

int		FileList_MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);
VOID	FileListBox(VOID);
VOID	FileListBoxInit(VOID);
VOID	FileList_ListView(DWORD dwStart, BOOL IsDelete);
VOID	FileList_SetListView(DWORD dwItem, BOOL IsDelete);
VOID	FileList_Clear(BOOL IsFilePathClear);
VOID	FileList_FileOpen(VOID);
VOID	FileList_FolderOpen(VOID);
VOID	FileList_MD5FileOpen(VOID);
VOID	FileList_MenuShow(HWND hWnd, HMENU hMenu, POINT *pt);
VOID	FileList_FileMenu(HWND hWnd, DWORD dwItem, POINT *pt);
VOID	FileList_NoFileMenu(HWND hWnd, POINT *pt);
VOID	FileList_FileDelete(VOID);
DWORD	FileList_AddFile(VOID);
DWORD	FileList_AddFolder(VOID);
DWORD	FileList_AddMD5File(VOID);
DWORD	FileList_DropFile(HDROP hdropFile);
BOOL	MD5File_EditClear(HWND hWnd, UINT MessageID);
VOID	FileRecodeFoul(HWND hWnd);
#endif /* __FILELIST_H__ */
