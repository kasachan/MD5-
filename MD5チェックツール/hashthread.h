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

#ifndef __HASHTHREAD_H__
#define __HASHTHREAD_H__

#include "charenc.h"
#include <windows.h>
#include <shlwapi.h>
#include <tchar.h>
#include <errno.h>


#ifndef MAX_PATH_SIZE
#ifdef _UNICODE
#define MAX_PATH_SIZE	_MAX_PATH
#else
#define MAX_PATH_SIZE	(_MAX_PATH * 2)
#endif
#endif

#define MAX_STRINGTABLE					255
#define MAX_LOADSTRING					100
#define NUMBER_LOADSTRING				33

#define MD5FILE_CHARCODE_ANSI			0
#define MD5FILE_CHARCODE_SHIFT_JIS		0
#define MD5FILE_CHARCODE_UNICODE		1
#define MD5FILE_CHARCODE_UTF16			1
#define MD5FILE_CHARCODE_UTF8			2
#define MD5FILE_CHARCODE_BUFFERSIZE		4096

#define HASH_ALG_SID					CALG_MD5
#define HASH_LEN						16
#define HASH_LOADSTRING					HASH_LEN * 2 + 1

#define FILE_BUFFER_SIZE				0x00010000	// 65536
#define FILE_DEF_COUNTSIZE				0x00000010	// 16
#define FILE_MAX_COUNTSIZE				0x01000000	// 16777216
#define MD5FILE_NOLINE					0xFFFFFFFF
#define APP_PROGRESS_SLEEP_TIME			100
#define DEF_FILE_ATTRIBUTE				~(FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)

#define APP_MESSAGE_INIT				0x00000001
#define APP_MESSAGE_START				0x00000002
#define APP_MESSAGE_PROGRESS			0x00000003
#define APP_MESSAGE_END					0x00000004
#define APP_MESSAGE_ERROR				0x00000005
#define APP_MESSAGE_PAUSE				0x00000006
#define APP_MESSAGE_FILEOPEN			0x00000007
#define APP_MESSAGE_FILEEND				0x00000008
#define APP_MESSAGE_FILECHANGE			0x00000009
#define APP_MESSAGE_FILECLOSE			0x0000000A
#define APP_MESSAGE_MULTIFILE			0x0000000B
#define APP_MESSAGE_FOLDER_INIT			0x0000000C
#define APP_MESSAGE_FOLDER_INIT_EXIT	0x0000000D
#define APP_MESSAGE_SUBFOLDER_OPEN		0x0000000E
#define APP_MESSAGE_MD5FILE_INIT		0x0000000F
#define APP_MESSAGE_MD5FILE_INIT_EXIT	0x00000010
#define APP_MESSAGE_LASTERROR			0x00000100

#define APP_PRGRESSTHREAD_START			0x00010000
#define APP_PRGRESSTHREAD_EXIT			0x00020000
#define APP_PRGRESSTHREAD_CANCEL		0x00040000

#define APP_PRGRESS_START				0x00000001
#define APP_PRGRESS_EXIT				0x00000002
#define APP_PRGRESS_PAUSE				0x00000004
#define APP_PRGRESS_CANCEL				0x00000008


struct tagMD5Thread_FileRecode {
	TCHAR		FileName[MAX_PATH_SIZE];
	LONGLONG	FileSize;
	DWORD		FileAttributes;
	FILETIME	FileCreationTime;
	FILETIME	FileLastAccessTime;
	FILETIME	FileLastWriteTime;
	BYTE		FileHashByte[HASH_LEN];
	DWORD		FileLastError; 
	DWORD		MD5FileID;
	DWORD		MD5FileHashLine;
	BYTE		MD5FileHash[HASH_LEN];
	BOOL		MD5FileHashCmp;//MD5ファイルの比較が記録されます。ただし memcmp の戻り値が代入されることに注意してください。
	BYTE		BlankSpace[1024 -
							((sizeof(TCHAR) * MAX_PATH_SIZE) + 
							sizeof(LONGLONG) +
							sizeof(DWORD) + 
							sizeof(FILETIME) + 
							sizeof(FILETIME) + 
							sizeof(FILETIME) + 
							(sizeof(BYTE) * HASH_LEN) +
							sizeof(DWORD) + 
							sizeof(DWORD) + 
							sizeof(DWORD) + 
							(sizeof(BYTE) * HASH_LEN) +
							sizeof(BOOL) +
							sizeof(UINT))];
	UINT		NextFile;//次のファイル番号が記録されます。
};

struct tagMD5Thread_MultiFile {
	DWORD		dwStructSize;
	DWORD		MaxBuffer;
	DWORD		FileCount;			// ファイルの数が記録されます。
	DWORD		FileCurrentCount;	// 計算中のファイルNo.が記録されます。
	TCHAR		FileCurrentFolder[MAX_PATH_SIZE];
	LONGLONG	FileAllSize;
	LONGLONG	FileAllCurrentSize;
	LONGLONG	FileAllSizeBackup;
	DWORD		MD5FileCharCode;	// MD5ファイルの文字コードが記録される。
	DWORD		MD5FileBom;			// Unicode UTF-8 の BOM が存在していたら、BOM のサイズが記録されます。
	struct		tagMD5Thread_FileRecode *FileRecode;
};

typedef struct tagMD5Thread {
	DWORD		dwStructSize; // バージョンアップ用に予約
	HWND		hWnd;
	UINT		MessageID;
	BOOL		SuspendMessage;
	HANDLE		MD5ThreadHandle;
	HANDLE		ProgressHandle;
	BYTE		HashByte[HASH_LEN];
	DWORD		IsFileNoCheck;
	TCHAR		*FilePath;
	LONGLONG	FileSize;
	LONGLONG	FileReadSize;
	DWORD		FileAttributeMask;	// 追加できるファイルの属性を指定します。
	BOOL		FileNoCache;// MD5計算時にファイルキャッシュを無効にします。
	//
	// ここからマルチファイル用の定義
	//
	BOOL		IsFileMapping;
	TCHAR		FileMapping[MAX_PATH_SIZE]; // ファイルマッピングに使うファイルを指定してください。Malloc に失敗したときに実行されます。
	HANDLE		hFile;
	HANDLE		hFileMapping;
	struct		tagMD5Thread_MultiFile *MultiFile;
} MD5Thread, *lpMD5Thread;



unsigned __stdcall MD5_MultiFile_Thread(void *lpThread);
unsigned __stdcall Progress_MultiFile_Thread(void *lpThread);

BOOL	MD5_Thread_Startup(MD5Thread *lpMD5Thread);
BOOL	MD5_Thread_Shutdown(MD5Thread *lpMD5Thread);
DWORD	MD5_Thread_GetCurrentItem(MD5Thread *pMD5Thread);
BOOL	MD5_Thread_AddFileRecode(MD5Thread *pMD5Thread, const TCHAR *inFilePath, WIN32_FIND_DATA *hFind, DWORD dwHashLine);
DWORD	MD5_Thread_Folder_Init(MD5Thread *pMD5Thread, TCHAR *inFilePath, BOOL IsAddFile, BOOL IsSubFolder);
DWORD	MD5_Thread_Folder_Open(MD5Thread *pMD5Thread, TCHAR *inFilePath, BOOL IsAddFile, BOOL IsSubFolder);
DWORD	MD5_Thread_MultiFile_Init(MD5Thread *pMD5Thread, const TCHAR *inFilePath, BOOL IsAddFile);
DWORD	MD5_Thread_MD5File_Init(MD5Thread *pMD5Thread, const TCHAR *inFilePath, BOOL IsAddFile, const TCHAR *inOption);
DWORD	MD5_Thread_GetCharCode(MD5Thread *lpMD5Thread, const TCHAR *inFilePath);
DWORD	MD5_Thread_CreateBuffer(MD5Thread *lpMD5Thread, DWORD dwFileCount, BOOL IsBufferCopy);
tagMD5Thread_MultiFile	*MD5_Thread_CreateMultiFileBuffer(MD5Thread *lpMD5Thread, DWORD dwFileCount);
tagMD5Thread_FileRecode	*MD5_Thread_CreateFileRecodeBuffer(MD5Thread *lpMD5Thread, DWORD dwFileCount);
BOOL	MD5_Thread_ClearBuffer(MD5Thread *lpMD5Thread);
BOOL	MD5_Thread_ClearBuffer_Core(MD5Thread *lpMD5Thread, BOOL IsMultiFileClear);
BOOL	MD5_Thread_DeleteItemBuffer(MD5Thread *lpMD5Thread, DWORD dwItem);
BOOL	MD5_Thread_SwapItemBuffer(MD5Thread *lpMD5Thread, DWORD dwItem1, DWORD dwItem2);
BOOL	MD5_Thread_CreateMD5File(MD5Thread *lpMD5Thread, const TCHAR *inFilePath, const TCHAR *inOption);

#define MD5_Thread_MD5ToString MD5_Thread_MD5ToString_no_sprintf
#define MD5_Thread_MD5ToHash MD5_Thread_MD5ToHash_no_scanf

TCHAR	*MD5_Thread_MD5ToString_no_sprintf(TCHAR *inString, const BYTE *inHashByte, BOOL IsMD5Upper);
BYTE	*MD5_Thread_MD5ToHash_no_scanf(BYTE *inHashByte, const TCHAR *inString, BOOL DigitCheck = TRUE);
TCHAR	*MD5_Thread_MD5ToString_Default(TCHAR *inString, const BYTE *inHashByte, BOOL IsMD5Upper);
BYTE	*MD5_Thread_MD5ToHash_Default(BYTE *inHashByte, const TCHAR *inString, BOOL DigitCheck = TRUE);
TCHAR	*MD5_Thread_MD5ToString_Old(TCHAR *inString, const BYTE *inHashByte, BOOL IsMD5Upper);
BYTE	*MD5_Thread_MD5ToHash_Old(BYTE *inHashByte, const TCHAR *inString);
BOOL	MD5_Thread_FileNameCheck(TCHAR *inFile);
DWORD	MD5_Thread_GetOldMD5File(MD5Thread *lpMD5Thread, const TCHAR *inFilePath);
TCHAR	*MD5_Thread_tcscpy(TCHAR *dst, const TCHAR *src);
#endif /* __HASHTHREAD_H__ */
