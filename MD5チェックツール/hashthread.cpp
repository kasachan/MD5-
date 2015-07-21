// hashthread.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "hashthread.h"
#include <stdio.h>
#include <process.h>
#include <crtdbg.h>
#ifdef _DEBUG
#include <mmsystem.h>
#endif


BOOL MD5_Thread_Startup(MD5Thread *lpMD5Thread)
{
	DWORD dwRet = MD5_Thread_CreateBuffer(lpMD5Thread, FILE_DEF_COUNTSIZE, FALSE);

	if(dwRet != -1 &&
		dwRet != 0 &&
		lpMD5Thread->dwStructSize == 0)
	{
		lpMD5Thread->hWnd							= NULL;
		lpMD5Thread->MessageID						= 0;
		lpMD5Thread->SuspendMessage					= 0;
		lpMD5Thread->ProgressHandle					= 0;
		lpMD5Thread->IsFileNoCheck					= 0;
		lpMD5Thread->FileSize						= 0;
		lpMD5Thread->FileReadSize					= 0;
		lpMD5Thread->FileAttributeMask				= (DWORD)DEF_FILE_ATTRIBUTE;
		lpMD5Thread->FileNoCache					= 0;
		if(lpMD5Thread->IsFileMapping == NULL)
		{
			*lpMD5Thread->FileMapping					= NULL;
			lpMD5Thread->hFile							= NULL;
			lpMD5Thread->hFileMapping					= NULL;
		}
		memset(lpMD5Thread->HashByte, NULL, HASH_LEN);
		lpMD5Thread->MultiFile->MD5FileCharCode		= MD5FILE_CHARCODE_SHIFT_JIS;
		lpMD5Thread->MultiFile->MD5FileBom			= 0;
		GetCurrentDirectory(MAX_PATH_SIZE, lpMD5Thread->MultiFile->FileCurrentFolder);
	}

	if(dwRet > 0)
	{
		memset(lpMD5Thread->MultiFile->FileRecode, 0, sizeof(tagMD5Thread_FileRecode));
		lpMD5Thread->MultiFile->FileRecode->MD5FileHashLine = MD5FILE_NOLINE;
	}

	return lpMD5Thread->MultiFile != NULL ? FILE_DEF_COUNTSIZE : -1;
}

BOOL MD5_Thread_Shutdown(MD5Thread *lpMD5Thread)
{
	if(lpMD5Thread->hFileMapping != NULL)
	{
		UnmapViewOfFile(lpMD5Thread->MultiFile->FileRecode);
		lpMD5Thread->MultiFile->FileRecode = NULL;
		CloseHandle(lpMD5Thread->hFileMapping);
	}
	if(lpMD5Thread->hFile != NULL)
		CloseHandle(lpMD5Thread->hFile);
	return MD5_Thread_ClearBuffer_Core(lpMD5Thread, TRUE);
}

unsigned __stdcall MD5_MultiFile_Thread(void *lpThread)
{
	MD5Thread *pMD5Thread	= (MD5Thread*)lpThread;
	HANDLE		hFile		= NULL;
	HCRYPTPROV	hCryptProv	= NULL;
	HCRYPTHASH	hHash		= NULL;
	TCHAR	*szBuf;
	BYTE	*bBuffer;
	BYTE	*pbHash;
	DWORD	dwBufferSize;
	DWORD	dwCreateFileAttributes;
	DWORD	dwHashLen		= HASH_LEN;
	DWORD	dwRead			= 0;
	DWORD	dwLasterror		= 0;
	BOOL	bRet			= FALSE;
	BOOL	IsFileSkip;
#ifdef _DEBUG
	TCHAR szDebugText[1024];
#endif

	if(pMD5Thread->FileNoCache != 0)
	{
		dwBufferSize			= FILE_BUFFER_SIZE * 16;
		dwCreateFileAttributes	= FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING;
	}
	else
	{
		dwBufferSize			= FILE_BUFFER_SIZE * 16;
		dwCreateFileAttributes	= FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN;
	}

	if(pMD5Thread->MultiFile->FileCount == 0)
	{
		PostMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_MESSAGE_ERROR, 0);
		pMD5Thread->SuspendMessage = APP_PRGRESS_EXIT;
		_endthreadex(0);
		return 0;
	}

	bBuffer = (BYTE*)malloc(dwBufferSize + 
							HASH_LEN +
							((MAX_PATH_SIZE + 100) * sizeof(TCHAR))
							);

	if(bBuffer == NULL)
	{
		dwLasterror = GetLastError();
		SendMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_MESSAGE_LASTERROR, dwLasterror);
		PostMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_MESSAGE_ERROR, dwLasterror);
		pMD5Thread->SuspendMessage = APP_PRGRESS_EXIT;
		_endthreadex(dwLasterror);
		return dwLasterror;
	}
	pbHash	= bBuffer + dwBufferSize;
	szBuf	= (TCHAR*)pbHash + HASH_LEN;


	// 構造体の初期化
	pMD5Thread->SuspendMessage					= 0;
	pMD5Thread->FileSize						= 0;
	pMD5Thread->MultiFile->FileAllCurrentSize	= 0;


	// マルチファイルに関する初期化
	if(pMD5Thread->MultiFile->FileCurrentCount > 0)
	{
		pMD5Thread->MultiFile->FileAllSize = 0;

		for(DWORD dwCount = pMD5Thread->MultiFile->FileCurrentCount; dwCount < pMD5Thread->MultiFile->FileCount; dwCount++)
		{
			pMD5Thread->MultiFile->FileAllSize += (pMD5Thread->MultiFile->FileRecode + dwCount)->FileSize;
		}
	}
	else
	{
		pMD5Thread->MultiFile->FileAllSize = pMD5Thread->MultiFile->FileAllSizeBackup;
	}

	if(pMD5Thread->MultiFile->FileCurrentCount >= pMD5Thread->MultiFile->FileCount)
	{
		if(pMD5Thread->MultiFile->FileCurrentCount != pMD5Thread->MultiFile->FileCount)
		{
			pMD5Thread->MultiFile->FileCurrentCount	= 0;
		}
		else
		{
			pMD5Thread->MultiFile->FileCurrentCount--;
		}
	}

	SendMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_MESSAGE_INIT, pMD5Thread->MultiFile->FileCurrentCount);

	if(pMD5Thread->IsFileNoCheck == 0)
	{
		HANDLE hFind;
		WIN32_FIND_DATA FindData;

		// マルチファイルに関する初期化
		pMD5Thread->MultiFile->FileCount	= 0;
		pMD5Thread->MultiFile->FileAllSize	= 0;

		// ファイルのサイズ取得
		hFind = FindFirstFile(pMD5Thread->FilePath, &FindData);
		if(hFind != INVALID_HANDLE_VALUE)
		{
			FindClose(hFind);

			if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				MD5_Thread_Folder_Init(pMD5Thread, pMD5Thread->FilePath, FALSE, TRUE);
			}
			else
			{
				MD5_Thread_MultiFile_Init(pMD5Thread, pMD5Thread->FilePath, FALSE);
			}
		}
	}

	SendMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_MESSAGE_START, 0);
	SendMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_MESSAGE_MULTIFILE, pMD5Thread->MultiFile->FileCount);

	pMD5Thread->ProgressHandle = (HANDLE)_beginthreadex(NULL, 0, Progress_MultiFile_Thread, pMD5Thread, 0, 0);
	goto FILE_START;

FILE_CHANGE:
	pMD5Thread->MultiFile->FileCurrentCount++;

FILE_START:
	IsFileSkip = FALSE;
	pMD5Thread->FileSize		= (pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileSize;
	pMD5Thread->FileReadSize	= 0;

	SendMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_MESSAGE_FILECHANGE, pMD5Thread->MultiFile->FileCurrentCount);

	if(!ExpandEnvironmentStrings((pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileName,
		szBuf,
		MAX_PATH_SIZE + 100))
	{
		_tcscpy(szBuf, (pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileName);
	}


#ifdef _DEBUG
	_stprintf(szDebugText, _T("MD5Thread: MD5Thread(): [%d] %s\r\n"),
		pMD5Thread->MultiFile->FileCurrentCount,
		szBuf);
	szDebugText[1023] = '\0';
	OutputDebugString(szDebugText);

	if((pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileLastError != -1)
	{
		_stprintf(szDebugText, _T("MD5Thread: MD5Thread(): LastError: %d\r\n"),
			(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileLastError);
		OutputDebugString(szDebugText);
	}
#endif

	// 一度計算済みでファイルのサイズ、更新日時が変わっていなければ計算は省略する。
	if((pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileLastError == 0)
	{
		HANDLE hFind;
		WIN32_FIND_DATA FindData;

		hFind = FindFirstFile(szBuf, &FindData);
		if(hFind != INVALID_HANDLE_VALUE)
		{
			LONGLONG FileSize;
			LONGLONG FileLastWriteTime;
			LONGLONG LastWriteTime;

			FileSize  = FindData.nFileSizeLow;
			FileSize += (LONGLONG)FindData.nFileSizeHigh << 32;

			FileLastWriteTime  = FindData.ftLastWriteTime.dwLowDateTime;
			FileLastWriteTime += (LONGLONG)FindData.ftLastWriteTime.dwHighDateTime << 32;
			FindClose(hFind);

			LastWriteTime  = (pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileLastWriteTime.dwLowDateTime;
			LastWriteTime += (LONGLONG)(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileLastWriteTime.dwHighDateTime << 32;


			if(FileSize == (pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileSize &&
				FileLastWriteTime == LastWriteTime)
			{
				IsFileSkip	= TRUE;
				pMD5Thread->FileReadSize = (pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileSize;
				dwLasterror	= 0;
				bRet = TRUE;

#ifdef _DEBUG
				OutputDebugString(_T("MD5Thread: MD5Thread(): ファイルの計算は省略します。\r\n"));
#endif

				goto FILE_EXIT;
			}
			else
			{
				if(FileSize != (pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileSize)
				{
					pMD5Thread->MultiFile->FileAllSizeBackup	-= (pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileSize;
					pMD5Thread->MultiFile->FileAllSizeBackup	+= FileSize;
					pMD5Thread->MultiFile->FileAllSize			= pMD5Thread->MultiFile->FileAllSizeBackup;
					(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileSize = FileSize;
				}

				memcpy(&(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileLastWriteTime,
					&FileLastWriteTime,
					sizeof(FILETIME));
			}
		}
	}
	
	
	hFile = CreateFile(szBuf,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		dwCreateFileAttributes,
		NULL);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		dwLasterror = GetLastError();
		goto FILE_EXIT;
	}
	SendMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_MESSAGE_FILEOPEN, pMD5Thread->MultiFile->FileCount);

	if((CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET) &&
		CryptCreateHash(hCryptProv, HASH_ALG_SID, 0, 0, &hHash)) == FALSE)
	{
		dwLasterror = GetLastError();
		goto FILE_EXIT;
	}

	do
	{
		if(pMD5Thread->SuspendMessage & (APP_PRGRESS_EXIT | APP_PRGRESS_CANCEL))
		{
			dwLasterror = (DWORD)-1;
			bRet = FALSE;
			goto FILE_EXIT;
		}

		//64KBずつ処理
		bRet = ReadFile(hFile, bBuffer, dwBufferSize, &dwRead, NULL);
		if(bRet == FALSE)
		{
			dwLasterror = GetLastError();
			(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileLastError = dwLasterror;
			break;
		}

#ifdef _DEBUG
		Sleep(100);
#endif

		if(dwRead > 0)
		{
			pMD5Thread->FileReadSize += dwRead;

			//ハッシュ計算
			bRet = CryptHashData(hHash, bBuffer, dwRead, 0);
			if(bRet == FALSE)
			{
				dwLasterror = GetLastError();
				(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileLastError = dwLasterror;
				break;
			}
		}
		else
		{
			if(pMD5Thread->FileReadSize < pMD5Thread->FileSize)
			{
				(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileSize = pMD5Thread->FileReadSize;
				pMD5Thread->MultiFile->FileAllSize -= pMD5Thread->FileSize - pMD5Thread->FileReadSize;
				pMD5Thread->FileSize = pMD5Thread->FileReadSize;
				dwLasterror = GetLastError();
			}
			break;
		}
	}
	while(pMD5Thread->FileReadSize < pMD5Thread->FileSize);

	//ハッシュ取得
	if(bRet != FALSE)
	{
		if(!CryptGetHashParam(hHash, HP_HASHVAL, pbHash, &dwHashLen, 0)) 
		{
			dwLasterror = GetLastError();
			goto FILE_EXIT;
		}
		memcpy((pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileHashByte, pbHash, HASH_LEN);
	}

FILE_EXIT:
	if(hHash)
	{
		CryptDestroyHash(hHash);
		hHash = NULL;
	}
	if(hCryptProv)
	{
		CryptReleaseContext(hCryptProv, 0);
		hCryptProv = NULL;
	}
	if(hFile)
	{
		CloseHandle(hFile);
		hFile = NULL;
	}

	if(dwLasterror != 0)
	{
		pMD5Thread->FileSize = 0;
		(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileLastError = dwLasterror;
		pMD5Thread->MultiFile->FileAllSize -= (pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileSize;
		memset((pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileHashByte, 0, HASH_LEN);
	}
	else
	{
		(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileLastError = 0;
		pMD5Thread->MultiFile->FileAllCurrentSize += pMD5Thread->FileSize;

		if((pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->MD5FileHashLine != -1)
		{
			(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->MD5FileHashCmp = memcmp(
				(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileHashByte,
				(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->MD5FileHash,
				HASH_LEN);
		}
	}

#ifdef _DEBUG
	if((pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileLastError == 0)
	{
		TCHAR szText1[33];

		MD5_Thread_MD5ToString(szText1, (pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileHashByte, FALSE);
		_stprintf(szDebugText, _T("MD5Thread: MD5Thread(): %s\r\n"), szText1);
		OutputDebugString(szDebugText);
	}
#endif

	SendMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_MESSAGE_FILEEND, pMD5Thread->MultiFile->FileCurrentCount);
	if(pMD5Thread->SuspendMessage == 0 &&
		(pMD5Thread->MultiFile->FileCurrentCount + 1) < pMD5Thread->MultiFile->FileCount)
	{
		dwLasterror = 0;
		goto FILE_CHANGE;
	}

	free(bBuffer);

	WaitForSingleObject(pMD5Thread->ProgressHandle, INFINITE);
	CloseHandle(pMD5Thread->ProgressHandle);

	pMD5Thread->FileReadSize	= 0;
	pMD5Thread->FileSize		= 0;

	memcpy(pMD5Thread->HashByte,
		(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCurrentCount)->FileHashByte,
		HASH_LEN);
	SendMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_MESSAGE_FILECLOSE, pMD5Thread->MultiFile->FileCurrentCount);

	if(pMD5Thread->SuspendMessage & APP_PRGRESS_EXIT)
	{
		MD5_Thread_ClearBuffer(pMD5Thread);

#ifdef _DEBUG
		OutputDebugString(_T("MD5Thread: MD5Thread(): 緊急終了\r\n"));
#endif
	}

	if(bRet != FALSE)
	{
		PostMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_MESSAGE_END, pMD5Thread->MultiFile->FileCurrentCount);
		dwLasterror = 0;
	}
	else
	{
		SendMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_MESSAGE_LASTERROR, dwLasterror);
		PostMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_MESSAGE_ERROR, dwLasterror);
	}

	pMD5Thread->SuspendMessage = APP_PRGRESS_EXIT;
	if(pMD5Thread->MD5ThreadHandle != NULL)
	{
		pMD5Thread->MD5ThreadHandle = NULL;
		_endthreadex(dwLasterror);
	}
	return dwLasterror;
}

unsigned __stdcall Progress_MultiFile_Thread(void *lpThread)
{
	MD5Thread *pMD5Thread	= (MD5Thread*)lpThread;
	int dwPersent			= 0;
	int dwProgress			= 0;
#ifdef _DEBUG
	DWORD dwLoopCount	    = 0;		
	TCHAR szDebugText[MAX_LOADSTRING];
#endif

	SendMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_PRGRESSTHREAD_START, 1);

FILE_CHANGE:
	do
	{
		if(pMD5Thread->SuspendMessage != 0)
		{
			if(pMD5Thread->SuspendMessage & (APP_PRGRESS_EXIT | APP_PRGRESS_CANCEL))
			{
				goto EXIT;
			}
			if(pMD5Thread->SuspendMessage & APP_PRGRESS_PAUSE)
			{
				goto PAUSE;
			}
		}

		if(pMD5Thread->MultiFile->FileAllSize > 0)
		{
			dwPersent = (DWORD)((double)(pMD5Thread->MultiFile->FileAllCurrentSize + pMD5Thread->FileReadSize) / pMD5Thread->MultiFile->FileAllSize * 100);
		}

#ifdef _DEBUG
		if(dwLoopCount == 10)
		{
			_stprintf(szDebugText, _T("MD5Thraed: Progress_Thraed(): %d%%\t%lld / %lld\t\t…読み込み中\r\n"), dwPersent, pMD5Thread->FileReadSize, pMD5Thread->FileSize);
			OutputDebugString(szDebugText);
			dwLoopCount = 0;
		}
		else
			dwLoopCount++;
#endif

		if(dwPersent > dwProgress)
		{
			SendMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_MESSAGE_PROGRESS, (LPARAM)dwPersent);
			dwProgress = dwPersent;
		}
PAUSE:
		Sleep(APP_PROGRESS_SLEEP_TIME);
	}
	while(pMD5Thread->FileReadSize < pMD5Thread->FileSize);

EXIT:
#ifdef _DEBUG
	_stprintf(szDebugText, _T("MD5Thraed: Progress_Thraed(): 100%%\t%lld / %lld\t\t…読み込み完了\r\n"), pMD5Thread->FileReadSize, pMD5Thread->FileSize);
	szDebugText[MAX_LOADSTRING - 1] = '\0';
	OutputDebugString(szDebugText);
#endif

	if(pMD5Thread->SuspendMessage == 0 &&
		pMD5Thread->MultiFile->FileCount > 0 &&
		(pMD5Thread->MultiFile->FileCurrentCount + 1) < pMD5Thread->MultiFile->FileCount)
	{
		goto FILE_CHANGE;
	}
	PostMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_PRGRESSTHREAD_EXIT, (LPARAM)dwPersent);

	_endthreadex(0);
	return 0;
}

DWORD MD5_Thread_GetCurrentItem(MD5Thread *pMD5Thread)
{
	if(pMD5Thread->MultiFile->FileCurrentCount >= pMD5Thread->MultiFile->FileCount)
	{
		return pMD5Thread->MultiFile->FileCurrentCount - 1;
	}
	else
	{
		return pMD5Thread->MultiFile->FileCurrentCount;
	}
}

BOOL MD5_Thread_AddFileRecode(MD5Thread *pMD5Thread, const TCHAR *inFilePath, WIN32_FIND_DATA *hFind, DWORD dwHashLine)
{
	BOOL  bRet = TRUE;
	TCHAR szBuf[MAX_PATH_SIZE + 100];
#ifdef _DEBUG
	TCHAR szDebugText[1024];
#endif

	if(inFilePath != NULL)
	{
		_tcscpy((pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->FileName, inFilePath);
	}
	else
	{
		if(_tfullpath(szBuf, hFind->cFileName, MAX_PATH_SIZE + 100) != NULL)
		{
			*(szBuf + MAX_PATH_SIZE + 100 - 1) = '\0';
		}
		_tcscpy((pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->FileName, szBuf);
	}

	if(hFind != NULL)
	{
		(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->FileSize				= hFind->nFileSizeLow;
		if(hFind->nFileSizeHigh > 0)
		{
			(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->FileSize			+= (LONGLONG)hFind->nFileSizeHigh << 32;
		}
		(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->FileAttributes			= hFind->dwFileAttributes;
		memcpy(&(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->FileCreationTime,
			&hFind->ftCreationTime,
			3 * sizeof(FILETIME));
		pMD5Thread->MultiFile->FileAllSize += (pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->FileSize;
	}
	else
	{
		(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->FileSize				= 0;
		(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->FileAttributes			= 0;
		memset(&(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->FileCreationTime,
			NULL,
			3 * sizeof(FILETIME));
	}

	pMD5Thread->MultiFile->FileAllSizeBackup = pMD5Thread->MultiFile->FileAllSize;
	(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->FileLastError			= (DWORD)-1;
	(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->MD5FileHashLine			= dwHashLine;
	(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->MD5FileHashCmp			= (DWORD)-1;
	(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->NextFile				= pMD5Thread->MultiFile->FileCount + 1;

#ifdef _DEBUG
	_stprintf(szDebugText, _T("MD5Thraed: MD5_Thread_AddFileRecode(): %d番目のファイルが追加されました。\r\n"), pMD5Thread->MultiFile->FileCount);
	szDebugText[1023] = '\0';
	OutputDebugString(szDebugText);
	_stprintf(szDebugText, _T("%s\r\n"), (pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->FileName);
	szDebugText[1023] = '\0';
	OutputDebugString(szDebugText);
#endif

	return bRet;
}

DWORD MD5_Thread_MultiFile_Init(MD5Thread *pMD5Thread, const TCHAR *inFilePath, BOOL IsAddFile)
{
	HANDLE hFind = NULL;
	TCHAR *szBuf = NULL;
	WIN32_FIND_DATA FindData;
	DWORD dwRet;
#ifdef _DEBUG
	TCHAR szDebugText[1024];
#endif


	if(pMD5Thread->MultiFile->FileCount >= FILE_MAX_COUNTSIZE)
	{
		pMD5Thread->IsFileNoCheck = 1;
		return FILE_MAX_COUNTSIZE;
	}

	if(PathIsDirectory(inFilePath))
	{
		return MD5_Thread_Folder_Init(pMD5Thread, (TCHAR*)inFilePath, IsAddFile, TRUE);
	}

	if(IsAddFile == 0 ||
		pMD5Thread->MultiFile->FileCount == 0)
	{
		pMD5Thread->MultiFile->FileCount			= 0;
		pMD5Thread->MultiFile->FileAllSize			= 0;
		pMD5Thread->MultiFile->FileAllCurrentSize	= 0;
	}

	szBuf = (TCHAR*)malloc(MAX_PATH_SIZE * 2 * sizeof(TCHAR));
	if(szBuf == NULL)
	{
#ifdef _DEBUG
		OutputDebugString(_T("MD5Thraed: malloc() に失敗しました。\r\n"));
#endif
		goto FILE_EXIT;
	}
	*szBuf = '\0';


	dwRet = MD5_Thread_CreateBuffer(pMD5Thread, pMD5Thread->MultiFile->FileCount + 1, TRUE);
	if(dwRet == -1)
	{
		free(szBuf);
#ifdef _DEBUG
		_stprintf(szDebugText, _T("MD5Thraed: %s の関数を終了します。\r\n"), _T("MD5_Thread_CreateBuffer"));
		szDebugText[1023] = '\0';
		OutputDebugString(szDebugText);
#endif
		return (DWORD)-1;
	}


	if(ExpandEnvironmentStrings(inFilePath, szBuf, MAX_PATH_SIZE + 100) == 0)
	{
		_tcscpy(szBuf, inFilePath);
	}

	_tcscpy(pMD5Thread->MultiFile->FileCurrentFolder, szBuf);
	PathRemoveFileSpec(pMD5Thread->MultiFile->FileCurrentFolder);

	hFind = FindFirstFile(szBuf, &FindData);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		MD5_Thread_AddFileRecode(pMD5Thread, inFilePath, &FindData, MD5FILE_NOLINE);
		FindClose(hFind);

		// MD5チェックツール (1x) 形式のMD5ファイルが存在したら読み込みます。
		MD5_Thread_GetOldMD5File(pMD5Thread, szBuf);
		pMD5Thread->MultiFile->FileCount++;
	}


	if(pMD5Thread->MultiFile->FileCount > 0)
	{
		(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount - 1)->NextFile = (UINT)-1;
	}
	else
	{
		pMD5Thread->MultiFile->FileRecode->NextFile = (UINT)-1;
	}
	pMD5Thread->IsFileNoCheck = 1;

FILE_EXIT:
	if(szBuf != NULL)
	{
		free(szBuf);
	}

	return pMD5Thread->MultiFile->FileCount;
}

DWORD MD5_Thread_Folder_Init(MD5Thread *pMD5Thread, TCHAR *inFilePath, BOOL IsAddFile, BOOL IsSubFolder)
{
	DWORD dwCount;

	if(pMD5Thread->MultiFile->FileCount >= FILE_MAX_COUNTSIZE)
	{
		pMD5Thread->IsFileNoCheck = 1;
		return FILE_MAX_COUNTSIZE;
	}

#ifdef _DEBUG
	TCHAR szDebugText[1024];

	_stprintf(szDebugText, _T("MD5Thraed: フォルダーの一覧を取得します。\r\n%s\r\n"), inFilePath);
	szDebugText[1023] = '\0';
	OutputDebugString(szDebugText);
#endif

	SendMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_MESSAGE_FOLDER_INIT, 0);
	dwCount = MD5_Thread_Folder_Open(pMD5Thread, inFilePath, IsAddFile, IsSubFolder);
	SendMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_MESSAGE_FOLDER_INIT_EXIT, dwCount);

	return dwCount;
}

DWORD MD5_Thread_Folder_Open(MD5Thread *pMD5Thread, TCHAR *inFilePath, BOOL IsAddFile, BOOL IsSubFolder)
{
	HANDLE hFind = NULL;
	WIN32_FIND_DATA *FindData;
	size_t FilePathLen[2];
	TCHAR *szBuf = NULL;
	TCHAR *szPathBuf;
	DWORD dwRet = 0;

#ifdef _DEBUG
	TCHAR szDebugText[1024];
#endif

	if(IsAddFile == 0 ||
		pMD5Thread->MultiFile->FileCount == 0)
	{
		pMD5Thread->MultiFile->FileCount			= 0;
		pMD5Thread->MultiFile->FileAllSize			= 0;
		pMD5Thread->MultiFile->FileAllCurrentSize	= 0;
		pMD5Thread->IsFileNoCheck					= 1;
	}

	szBuf = (TCHAR*)malloc(
							(MAX_PATH_SIZE * 4 * sizeof(TCHAR)) +
							sizeof(WIN32_FIND_DATA)
							);
	if(szBuf == NULL)
	{
#ifdef _DEBUG
		OutputDebugString(_T("MD5Thraed: malloc() に失敗しました。\r\n"));
#endif
		goto FILE_EXIT;
	}
	szPathBuf = MAX_PATH_SIZE * 2 + szBuf;
	*szPathBuf = '\0';
	FindData = (WIN32_FIND_DATA*)(MAX_PATH_SIZE * 2 + szPathBuf);


	FilePathLen[0] = _tcslen(inFilePath);
	if(*(inFilePath + FilePathLen[0] - 1) == '\\')
	{
		FilePathLen[0]--;
	}

	dwRet = ExpandEnvironmentStrings(inFilePath, szPathBuf, MAX_PATH_SIZE + 100);
	if(dwRet == 0)
	{
		_tcscpy(szPathBuf, inFilePath);
	}
	_tcscpy(pMD5Thread->MultiFile->FileCurrentFolder, szPathBuf);
	SetCurrentDirectory(szPathBuf);
	FilePathLen[1] = _tcslen(szPathBuf);
	_tcscpy(szPathBuf + FilePathLen[1], _T("\\*"));
	_tcscpy(MD5_Thread_tcscpy(szBuf, inFilePath), _T("\\"));


	if(IsSubFolder == 0)
	{
		goto FILE_FIND;
	}

	hFind = FindFirstFile(szPathBuf, FindData);
	if(hFind == INVALID_HANDLE_VALUE)
	{
		goto FILE_FIND;
	}

	do
	{
		if(~FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ||
			FindData->dwFileAttributes & ~pMD5Thread->FileAttributeMask ||
			_tcscmp(FindData->cFileName, _T(".")) == 0 ||
			_tcscmp(FindData->cFileName, _T("..")) == 0)
		{
			continue;
		}

#ifdef _DEBUG
		_stprintf(szDebugText, _T("MD5Thraed: %s\r\n→%s のサブフォルダーが見つかりました。\r\n"), inFilePath, FindData->cFileName);
		szDebugText[1023] = '\0';
		OutputDebugString(szDebugText);
#endif

		_tcscpy(szBuf + FilePathLen[0] + 1, FindData->cFileName);
		dwRet = MD5_Thread_Folder_Open(pMD5Thread, szBuf, TRUE, TRUE);

		if(dwRet == -1)
		{
			free(szBuf);
			return (DWORD)-1;
		}
		SendMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_MESSAGE_SUBFOLDER_OPEN, pMD5Thread->MultiFile->FileCount);
	}
	while(FindNextFile(hFind, FindData));
	FindClose(hFind);


	*(szPathBuf + FilePathLen[1]) = '\0';
	_tcscpy(pMD5Thread->MultiFile->FileCurrentFolder, szPathBuf);
	SetCurrentDirectory(szPathBuf);
	_tcscpy(szPathBuf + FilePathLen[1], _T("\\*"));
	*(szBuf + FilePathLen[0]) = '\\';


FILE_FIND:
	hFind = FindFirstFile(szPathBuf, FindData);

	// 各ファイル名とファイルのサイズ取得
	if(hFind == INVALID_HANDLE_VALUE)
	{
		goto FILE_EXIT;
	}

	do
	{
		if(FindData->dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | ~pMD5Thread->FileAttributeMask))
		{

#ifdef _DEBUG

			if(~FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				TCHAR *szFileAttibute[] = {
					_T("無"),			//00000000
					_T("読"),			//00000001
					_T("隠"),			//00000010
					_T("読隠"),			//00000011
					_T("シ"),			//00000100
					_T("読シ"),			//00000101
					_T("隠シ"),			//00000110
					_T("隠読シ"),		//00000111
					_T("無"),			//00001000
					_T("読"),			//00001001
					_T("隠"),			//00001010
					_T("読隠"),			//00001011
					_T("シ"),			//00001100
					_T("読シ"),			//00001101
					_T("隠シ"),			//00001110
					_T("隠読シ"),		//00001111
					_T("デ"),			//00010000
					_T("読デ"),			//00010001
					_T("隠デ"),			//00010010
					_T("読隠デ"),		//00010011
					_T("シデ"),			//00010100
					_T("読シデ"),		//00010101
					_T("隠シデ"),		//00010110
					_T("隠読シデ"),		//00010111
					_T("デ"),			//00011000
					_T("読デ"),			//00011001
					_T("隠デ"),			//00011010
					_T("読隠デ"),		//00011011
					_T("シデ"),			//00011100
					_T("読シデ"),		//00011101
					_T("隠シデ"),		//00011110
					_T("隠読シデ"),		//00011111
					_T("ア"),			//00100000
					_T("読ア"),			//00100001
					_T("隠ア"),			//00100010
					_T("読隠ア"),		//00100011
					_T("シア"),			//00100100
					_T("読シア"),		//00100101
					_T("隠シア"),		//00100110
					_T("隠読シア"),		//00100111
					_T("ア"),			//00101000
					_T("読ア"),			//00101001
					_T("隠ア"),			//00101010
					_T("読隠ア"),		//00101011
					_T("シア"),			//00101100
					_T("読シア"),		//00101101
					_T("隠シア"),		//00101110
					_T("隠読シア"),		//00101111
					_T("デア"),			//00110000
					_T("読デア"),		//00110001
					_T("隠デア"),		//00110010
					_T("読隠デア"),		//00110011
					_T("シデア"),		//00110100
					_T("読シデア"),		//00110101
					_T("隠シデア"),		//00110110
					_T("隠読シデア"),	//00110111
					_T("デア"),			//00111000
					_T("読デア"),		//00111001
					_T("隠デア"),		//00111010
					_T("読隠デア"),		//00111011
					_T("シデア"),		//00111100
					_T("読シデア"),		//00111101
					_T("隠シデア"),		//00111110
					_T("隠読シデア"),	//00111111
				};

				OutputDebugString(_T("MD5Thraed: MD5_Thread_Folder_Open(): 以下のファイルは除外します。\r\n"));
				_stprintf(szDebugText, _T("MD5Thraed: MD5_Thread_Folder_Open(): [%s]\t%s\\%s\r\n"), szFileAttibute[FindData->dwFileAttributes & 0x3f], pMD5Thread->MultiFile->FileCurrentFolder, FindData->cFileName);
				szDebugText[1023] = '\0';
				OutputDebugString(szDebugText);
			}

#endif
			continue;
		}

		if(pMD5Thread->MultiFile->FileCount > FILE_MAX_COUNTSIZE)
		{
			pMD5Thread->MultiFile->MaxBuffer = FILE_MAX_COUNTSIZE;
			break;
		}

		dwRet = MD5_Thread_CreateBuffer(pMD5Thread, pMD5Thread->MultiFile->FileCount + 1, TRUE);
		if(dwRet == -1)
		{
			free(szBuf);
#ifdef _DEBUG
			_stprintf(szDebugText, _T("MD5Thraed: %s の関数を終了します。\r\n"), _T("MD5_Thread_CreateBuffer"));
			szDebugText[1023] = '\0';
			OutputDebugString(szDebugText);
#endif
			return (DWORD)-1;
		}

		_tcscpy(szBuf + FilePathLen[0] + 1, FindData->cFileName);

		if(!(_tcsicmp(PathFindExtension(FindData->cFileName), _T(".md5")) == 0 &&
			FindData->nFileSizeLow == HASH_LEN * 2) &&
			MD5_Thread_AddFileRecode(pMD5Thread, szBuf, FindData, MD5FILE_NOLINE))
		{
			if(_tcsicmp(PathFindExtension(FindData->cFileName), _T(".md5")) == 0)
			{
				dwRet = MD5_Thread_MD5File_Init(pMD5Thread, szBuf, TRUE, NULL);

				if(dwRet == -1)
				{
					free(szBuf);
					return (DWORD)-1;
				}
				else if(dwRet > 0)
				{
					pMD5Thread->MultiFile->FileCount--;
				}
			}
			else
			{
				// MD5チェックツール (1x) 形式のMD5ファイルが存在したら読み込みます。
				MD5_Thread_GetOldMD5File(pMD5Thread, szBuf);
			}
			pMD5Thread->MultiFile->FileCount++;
		}
	}
	while(FindNextFile(hFind, FindData));
	FindClose(hFind);

	if(pMD5Thread->MultiFile->FileCount > 0)
	{
		(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount - 1)->NextFile = (UINT)-1;
	}
	else
	{
		pMD5Thread->MultiFile->FileRecode->NextFile = (UINT)-1;
	}
	pMD5Thread->IsFileNoCheck = 1;


FILE_EXIT:
	if(szBuf != NULL)
	{
		free(szBuf);
	}

	return pMD5Thread->MultiFile->FileCount;
}

DWORD MD5_Thread_MD5File_Init(MD5Thread *pMD5Thread, const TCHAR *inFilePath, BOOL IsAddFile, const TCHAR *inOption)
{
	FILE *fp1 = NULL;
	BYTE *bBuf;
	TCHAR *szBuf = NULL;
	TCHAR *szPathBuf;
	TCHAR *szEnvironmentStrings;
	TCHAR *lpOpenOption	= _T("rb");
	TCHAR *lpRet;
	BOOL  IsAddMode		= FALSE;
	DWORD dwCharCode;
	DWORD dwLineCount	= 0;
	DWORD dwBufLen		= 0;
	DWORD dwRet;
	HANDLE hFind		= NULL;
	WIN32_FIND_DATA FindData;
#ifdef _DEBUG
	TCHAR szDebugText[1024];
#endif


	if(pMD5Thread->MultiFile->FileCount >= FILE_MAX_COUNTSIZE)
	{
		pMD5Thread->IsFileNoCheck = 1;
		return FILE_MAX_COUNTSIZE;
	}

	// オプションの解析
	if(inOption != NULL)
	{
		for(; *inOption != NULL;)
		{
			switch(*inOption)
			{
			case 'a':
				IsAddMode = TRUE;
#ifdef _DEBUG
				OutputDebugString(_T("MD5Thread: MD5File_Init(): 追記モード(計算済み)としてオープンします。\r\n"));
#endif
				break;
			}
			inOption++;
		}
	}

	szBuf = (TCHAR*)malloc(MAX_PATH_SIZE * 8 * sizeof(TCHAR));
	if(szBuf == NULL)
	{
		SendMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_MESSAGE_MD5FILE_INIT_EXIT, -1);
		return pMD5Thread->MultiFile->FileCount;
	}
	*szBuf					= '\0';
	bBuf					= MAX_PATH_SIZE * 2 + (BYTE*)szBuf;
	*(DWORD*)bBuf			= 0;
	szPathBuf				= MAX_PATH_SIZE * 2 + (TCHAR*)bBuf;
	szEnvironmentStrings	= MAX_PATH_SIZE * 2 + szPathBuf;

	dwRet = ExpandEnvironmentStrings(inFilePath, szBuf, MAX_PATH_SIZE + 100);
	if(dwRet == 0)
	{
		_tcscpy(szBuf, inFilePath);
	}
	MD5_Thread_FileNameCheck(szBuf);


	dwRet = MD5_Thread_GetCharCode(pMD5Thread, szBuf);
	if(dwRet == -1)
	{
		free(szBuf);
		return pMD5Thread->MultiFile->FileCount;
	}
	dwCharCode = pMD5Thread->MultiFile->MD5FileCharCode;
	pMD5Thread->IsFileNoCheck = 1;


	if(IsAddFile == 0 ||
		pMD5Thread->MultiFile->FileCount == 0)
	{
		pMD5Thread->MultiFile->FileCount			= 0;
		pMD5Thread->MultiFile->FileAllSize			= 0;
		pMD5Thread->MultiFile->FileAllCurrentSize	= 0;
	}
	SendMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_MESSAGE_MD5FILE_INIT, pMD5Thread->MultiFile->FileCount);


	if((fp1 = _tfopen(szBuf, lpOpenOption)) == NULL)
	{
		free(szBuf);
		SendMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_MESSAGE_MD5FILE_INIT_EXIT, -1);
		return pMD5Thread->MultiFile->FileCount;
	}

	_tcscpy(pMD5Thread->MultiFile->FileCurrentFolder, szBuf);
	PathRemoveFileSpec(pMD5Thread->MultiFile->FileCurrentFolder);
	SetCurrentDirectory(pMD5Thread->MultiFile->FileCurrentFolder);


	fseek(fp1, pMD5Thread->MultiFile->MD5FileBom, SEEK_SET);
	dwLineCount = pMD5Thread->MultiFile->FileCount;
	lpRet = (TCHAR*)bBuf;


	// 1行ずつ解析していく。
	while(fgets((char*)bBuf, MAX_PATH_SIZE * 2 * sizeof(BYTE), fp1) != NULL)
	{
		DWORD dwLineChk = 0;
		DWORD dwFileOpen = 0;
		size_t szLen;
		TCHAR *szFilePath;
		TCHAR *p1, *p2;
		BYTE *b = bBuf;

#ifdef _DEBUG
		_stprintf(szDebugText, _T("MD5Thraed: MD5File_Init(): %d行目の処理開始します。\r\n"), dwLineCount);
		szDebugText[1023] = '\0';
		OutputDebugString(szDebugText);
#endif


		if(pMD5Thread->MultiFile->FileCount >= FILE_MAX_COUNTSIZE)
		{
			pMD5Thread->MultiFile->MaxBuffer = FILE_MAX_COUNTSIZE;
			break;
		}

		dwRet = MD5_Thread_CreateBuffer(pMD5Thread, pMD5Thread->MultiFile->FileCount + 1, TRUE);
		if(dwRet == -1)
		{
			free(szBuf);
			SendMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_MESSAGE_MD5FILE_INIT_EXIT, -1);
#ifdef _DEBUG
			_stprintf(szDebugText, _T("MD5Thraed: MD5File_Init(): %s の関数を終了します。\r\n"), _T("MD5_Thread_CreateBuffer"));
			szDebugText[1023] = '\0';
			OutputDebugString(szDebugText);
#endif
			return (DWORD)-1;
		}

		switch(dwCharCode)
		{
		case MD5FILE_CHARCODE_SHIFT_JIS:
#ifdef _UNICODE
			dwRet = (DWORD)ShiftJisToUnicode(szBuf, (char*)bBuf);
#else
			dwRet = (DWORD)strcpy(szBuf, (char*)bBuf);
#endif
			break;
		case MD5FILE_CHARCODE_UNICODE:
			while(*b++ != '\n')
				;
			*(wchar_t*)b = '\0';
			fseek(fp1, 1, SEEK_CUR);

#ifdef _UNICODE
			dwRet = (DWORD)wcscpy(szBuf, (wchar_t*)bBuf);
#else
			dwRet = (DWORD)UnicodeToShiftJis(szBuf, (wchar_t*)bBuf);
#endif
			break;
		case MD5FILE_CHARCODE_UTF8:
#ifdef _UNICODE
			dwRet = (DWORD)UTF8ToUnicode(szBuf, (char*)bBuf);
#else
			dwRet = (DWORD)UTF8ToShiftJis(szBuf, (char*)bBuf);
#endif
		}
		dwBufLen = (DWORD)_tcslen(szBuf);


		// 改行コードがあれば削除します。
		if (dwBufLen > 2)
		{
			TCHAR *p3 = szBuf + dwBufLen - 2;

			// _tcsncmp(p3, _T("\r\n"), 2) の最適化
			if(
#ifdef _UNICODE
				*(DWORD*)p3 == 0x000A000D
#else
				*(WORD*)p3 == 0x0A0D
#endif
			)
			{
				memset(p3, 0, sizeof(TCHAR) * 2);
				dwBufLen -= 2;
#ifdef _DEBUG
				OutputDebugString(_T("MD5Thread: MD5File_Init(): \\r\\n (Windows形式) の改行文字を削除しました。\r\n"));
#endif
			}
			else
			{
				p3++;

				if (*p3 == '\r')
				{
					*p3 = '\0';
					dwBufLen--;
#ifdef _DEBUG
					OutputDebugString(_T("MD5Thread: MD5File_Init(): \\r (MacOS形式) の改行文字を削除しました。\r\n"));
#endif
				}

				if (*p3 == '\n')
				{
					*p3 = '\0';
					dwBufLen--;
#ifdef _DEBUG
					OutputDebugString(_T("MD5Thread: MD5File_Init(): \\n (Linux形式) の改行文字を削除しました。\r\n"));
#endif
				}
			}
		}


		if(dwBufLen < HASH_LEN * 2)
		{
			dwLineCount++;
			continue;
		}

		(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->FileSize		= 0;
		(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->MD5FileHashLine	= MD5FILE_NOLINE;
		*szEnvironmentStrings = '\0';

		if(dwBufLen == HASH_LEN * 2)
		{
MD5_1X_OPEN:

#ifdef _DEBUG
			OutputDebugString(_T("MD5Thread: MD5File_Init(): MD5チェックツール (1x) 形式の MD5ファイルとして開きます。\r\n"));
#endif
			if(_tcsicmp(PathFindExtension(inFilePath), _T(".md5")) == 0 &&
				MD5_Thread_MD5ToHash((pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->MD5FileHash, szBuf))
			{
				p1	= MD5_Thread_tcscpy(szBuf, inFilePath) - 4;// 4 == _tcslen(_T(".md5"));
				*p1	= '\0';
				dwLineChk = 1;
			}
		}
		else
		{
#ifdef _DEBUG
			OutputDebugString(_T("MD5Thread: MD5File_Init(): 通常の MD5ファイルとして開いています...\r\n"));
#endif

			if(MD5_Thread_MD5ToHash((pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->MD5FileHash, szBuf))
			{
#ifdef _DEBUG
				OutputDebugString(_T("MD5Thread: MD5File_Init(): MD5チェックツール 標準形式の MD5ファイルとして開きます。\r\n"));
#endif
				p1 = szBuf + HASH_LOADSTRING;
				szLen = _tcslen(p1);

				if(szLen == 0)
				{
					dwLineCount++;
					continue;
				}

				for(; *p1 != NULL; p1++)
				{
					if(!_istspace(*p1))
					{
						break;
					}
				}

				_tcscpy(szBuf, p1);
				dwLineChk = 3;
			}
			// BSDフォーマットの解析
			else if(
// _tcsncmp(szBuf, _T("MD5 ("), 5) の最適化
#ifdef _UNICODE
				*(DWORD*)szBuf == 0x0044004D &&
				*((DWORD*)szBuf + 1) == 0x00200035 &&
				(*((DWORD*)szBuf + 2) & 0x0000FFFF) == 0x00000028
#else
				*(DWORD*)szBuf == 0x2035444D &&
				(*((DWORD*)szBuf + 1) & 0x000000FF) == 0x00000028
#endif
			)
			{
#ifdef _DEBUG
				OutputDebugString(_T("MD5Thread: MD5File_Init(): BSD形式の MD5ファイルとして開きます。\r\n"));
#endif

				p1 = szBuf + 5;
				p2 = p1;

				for(; *p2 != NULL;)
				{
					if(
// _tcsncmp(p2, _T(") = "), 4) の最適化
#ifdef _UNICODE
						*(DWORD*)p2 == 0x00200029 &&
						*((DWORD*)p2 + 1) == 0x0020003D
#else
						*(DWORD*)p2 == 0x203D2029
#endif
					)
					{
						*p2 = '\0';
						_tcscpy(szBuf, p1);
						p2 += 4;
						if(MD5_Thread_MD5ToHash((pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->MD5FileHash, p2))
						{
							dwLineChk = 2;
						}
						break;
					}
					p2++;
				}
			}
			else
			{
				TCHAR *p3 = szBuf + dwBufLen - 1;
				DWORD dwIsDigitCount = 0;

#ifdef _DEBUG
				OutputDebugString(_T("MD5Thread: MD5File_Init(): フリーフォーマット形式の MD5ファイルとして開きます。\r\n"));
#endif

				for(; szBuf != p3; p3--)
				{
					if(_istxdigit(*p3))
					{
						dwIsDigitCount++;
					}
					else
					{
						dwIsDigitCount = 0;
					}

					if(dwIsDigitCount == (HASH_LEN * 2) &&
						_istxdigit(*(p3 - 1)) == FALSE &&
						MD5_Thread_MD5ToHash((pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->MD5FileHash, p3, FALSE))
					{
						TCHAR *p4;
						DWORD dwCount;
						DWORD dwLeftSpace			= p3 - szBuf;
						DWORD dwRightSpace			= dwBufLen - dwLeftSpace - (HASH_LEN * 2);// 必ずマイナスにならない
						double dLeftSpacePersent	= 0.;
						double dRightSpacePersent	= 0.;
						DWORD dwUnquote				= 0;

#ifdef _DEBUG
						OutputDebugString(_T("MD5Thread: MD5File_Init(): MD5ハッシュがヒットしました。\r\n"));
						_stprintf(szDebugText, _T("MD5Thread: MD5File_Init(): 開始文字: %d文字目, 右端までの文字数: %d\r\n"),
								dwLeftSpace, dwRightSpace);
						szDebugText[1023] = '\0';
						OutputDebugString(szDebugText);
#endif

						// 左側のスペースじゃない確率の計算
						if(dwLeftSpace > 1)
						{
							p1 = szBuf;
							dwCount = 0;
							for(; p1 != p3; p1++)
							{
								if(!_istspace(*p1))
								{
									dwCount++;
								}
							}
							dLeftSpacePersent = (double)dwCount / (double)dwLeftSpace;
						}
						// 右側のスペースじゃない確率の計算
						if(dwRightSpace > 1)
						{
							p1 = p3 + 1 + HASH_LEN * 2;
							dwCount = 0;
							for(; *p1 != NULL; p1++)
							{
								if(!_istspace(*p1))
								{
									dwCount++;
								}
							}
							dRightSpacePersent = (double)dwCount / (double)dwRightSpace;
						}
#ifdef _DEBUG
						_stprintf(szDebugText, _T("MD5Thread: MD5File_Init(): 左端までのスペース率: %.2f%%, 右端までのスペース率: %.2f%%\r\n"),
								dLeftSpacePersent * 100.,
								dRightSpacePersent * 100.);
						szDebugText[1023] = '\0';
						OutputDebugString(szDebugText);
#endif


						if(dLeftSpacePersent < dRightSpacePersent)
						{
							p2 = p3 + HASH_LEN * 2;
#ifdef _DEBUG
							OutputDebugString(_T("MD5Thread: MD5File_Init(): 右側の文字からファイル名の検索します\r\n"));
#endif
						}
						else if(dLeftSpacePersent > dRightSpacePersent)
						{
MD5_LEFT_TEXT:
							p2 = szBuf;
#ifdef _DEBUG
							OutputDebugString(_T("MD5Thread: MD5File_Init(): 左側の文字からファイル名の検索します。\r\n"));
#endif
						}
						else
						{
							if(dLeftSpacePersent == 1)
							{
								// 全てがスペースであれば、MD5チェックツール (1x) 形式の MD5ファイルとして開きます。
								goto MD5_1X_OPEN;
							}
							else
							{
								// 左側の文字が優先されます。
								goto MD5_LEFT_TEXT;
							}
						}


						for(; *p2 != NULL; p2++)
						{
							if(!_istspace(*p2))
							{
								if(*p2 == '"')
								{
									dwUnquote = 1;
								}
								break;
							}
						}
						p4 = p2;

						if(dwUnquote != 0)
						{
							p4++;
							for(; *p4 != NULL; p4++)
							{
								if(*p4 == '"')
								{
									*(p4 + 1) = '\0';
									dwUnquote = 0;
									break;
								}
							}

							// 『"』で閉じられていない処理
							if(dwUnquote != 0)
							{
								*(p4 - 2) = '"';
								*(p4 - 1) = '\0';
/*
								// 上記の最適化なんて書いていたら、
								// 逆アセンブル後のコードが同じでした（＾＾；
#ifdef _UNICODE
								*(DWORD*)(p4 - 2) = 0x00000022;
#else
								*(WORD*)(p4 - 2) = 0x0022;
#endif
*/
							}
						}
						else
						{
							for(; *p4 != NULL; p4++)
							{
								if(_istspace(*p4))
								{
									*p4 = '\0';
									break;
								}
							}
						}

						_tcscpy(szBuf, p2);
						dwLineChk = 4;
						break;
					}
				}
			}
		}

		if(dwLineChk == 0)
		{
			dwLineCount++;
			continue;
		}

		if(dwLineChk > 1)
		{
			dwRet = MD5_Thread_FileNameCheck(szBuf);
			_tcscpy(szPathBuf, szBuf);

			if(dwRet > 1)
			{
				dwRet = ExpandEnvironmentStrings(szPathBuf, szBuf, MAX_PATH_SIZE + 100);
				_tcscpy(dwRet != 0 ? szEnvironmentStrings : szBuf, szPathBuf);
			}


			// ファイルパスのチェック
			if(
				// _tcsncmp(szBuf + 1, _T("\\:"), 2) の最適化
#ifdef _UNICODE
				(*(DWORD*)(szBuf + 1) == 0x005C003A)
#else
				(*(WORD*)(szBuf + 1) == 0x5C3A)
#endif
				||
				// _tcsncmp(szBuf, _T("\\"), 2) の最適化
#ifdef _UNICODE
				(*(DWORD*)szBuf == 0x005C005C)
#else
				(*(WORD*)szBuf == 0x5C5C)
#endif
			)
			{
				// 絶対パスであれば何もしない
				;
			}
			else
			{
				// 相対パスは絶対パスに変換する。
				_tcscpy(szPathBuf, szBuf);
				if(_tfullpath(szBuf, szPathBuf, MAX_PATH_SIZE + 100) != NULL)
				{
					*(szBuf + MAX_PATH_SIZE + 100 - 1) = '\0';
				}
			}
		}
		szFilePath = *szEnvironmentStrings != '\0' ? szEnvironmentStrings : szBuf;

		hFind = FindFirstFile(szBuf, &FindData);
		if(hFind != INVALID_HANDLE_VALUE)
		{
			if(MD5_Thread_AddFileRecode(pMD5Thread, szFilePath, &FindData, dwLineCount))
			{
				if(IsAddMode != FALSE)
				{
					memcpy((pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->FileHashByte,
						(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->MD5FileHash,
						HASH_LEN);
					(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->FileLastError	= 0;
					(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->MD5FileHashCmp	= 0;
				}
				dwFileOpen++;
			}
			FindClose(hFind);
		}
		else
		{
			memset(&FindData, NULL, sizeof(WIN32_FIND_DATA));
			if(MD5_Thread_AddFileRecode(pMD5Thread, szFilePath, &FindData, dwLineCount))
			{
				dwFileOpen++;
			}
		}


		if(dwFileOpen != 0 &&
			_tcsicmp(PathFindExtension((pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->FileName), _T(".md5")) == 0)
		{
			if(_tcsicmp(inFilePath, (pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->FileName) != 0)
			{
				dwRet = MD5_Thread_MD5File_Init(pMD5Thread, (pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount)->FileName, TRUE, inOption);
					
				if(dwRet == -1)
				{
					return (DWORD)-1;
				}
			}
			else
			{
				dwLineCount--;
			}
			pMD5Thread->MultiFile->FileCount--;
		}
		pMD5Thread->MultiFile->FileCount++;
		dwLineCount++;
	}
	free(szBuf);
	fclose(fp1);

	if(IsAddMode != FALSE)
	{
		pMD5Thread->MultiFile->FileCurrentCount = pMD5Thread->MultiFile->FileCount;
	}
	if(pMD5Thread->MultiFile->FileCount > 0)
	{
		(pMD5Thread->MultiFile->FileRecode + pMD5Thread->MultiFile->FileCount - 1)->NextFile = (UINT)-1;
	}
	else
	{
		pMD5Thread->MultiFile->FileRecode->NextFile = (UINT)-1;
	}
	SendMessage(pMD5Thread->hWnd, pMD5Thread->MessageID, APP_MESSAGE_MD5FILE_INIT_EXIT, pMD5Thread->MultiFile->FileCount);

	return pMD5Thread->MultiFile->FileCount;
}

DWORD MD5_Thread_GetOldMD5File(MD5Thread *lpMD5Thread, const TCHAR *inFilePath)
{
	HANDLE hFind;
	WIN32_FIND_DATA FindData;
	TCHAR szBuf[MAX_PATH_SIZE + 100];
	DWORD dwRet = 0;

#ifdef _DEBUG
	TCHAR szDebugText[1024];
#endif

	dwRet = ExpandEnvironmentStrings(inFilePath, szBuf, MAX_PATH_SIZE + 100);
	if(dwRet == 0)
	{
		_tcscpy(szBuf, inFilePath);
	}
	_tcscat(szBuf, _T(".md5"));

	hFind = FindFirstFile(szBuf, &FindData);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	if(FindData.nFileSizeLow == HASH_LEN * 2)
	{
		FILE *fp1 = NULL;

#ifdef _DEBUG
		OutputDebugString(_T("MD5Thraed: GetOldMD5File(): MD5チェックツール (1x) 形式の MD5ファイルが見つかりました。\r\n"));
		_stprintf(szDebugText, _T("%s\r\n"), szBuf);
		szDebugText[1023] = '\0';
		OutputDebugString(szDebugText);
#endif

		if((fp1 = _tfopen(szBuf, _T("r"))) != NULL)
		{
			TCHAR szHash[HASH_LOADSTRING];

			if(_fgetts(szHash, HASH_LOADSTRING, fp1) != NULL)
			{
				if(MD5_Thread_MD5ToHash((lpMD5Thread->MultiFile->FileRecode + lpMD5Thread->MultiFile->FileCount)->MD5FileHash, szHash))
				{
					(lpMD5Thread->MultiFile->FileRecode + lpMD5Thread->MultiFile->FileCount)->MD5FileHashLine = 1;
					dwRet = TRUE;
#ifdef _DEBUG
					OutputDebugString(_T("MD5Thraed: GetOldMD5File(): MD5 の読み込みが完了しました。"));
#endif
				}
			}
			fclose(fp1);
		}

#ifdef _DEBUG
		else
		{
			OutputDebugString(_T("MD5Thraed: GetOldMD5File(): _tfopen() に失敗しました。"));
		}
#endif
	}
	FindClose(hFind);

	return dwRet;
}


DWORD MD5_Thread_GetCharCode(MD5Thread *lpMD5Thread, const TCHAR *inFilePath)
{
	FILE *fp1 = NULL;
	unsigned char *Buf;
	DWORD dwCharCode = lpMD5Thread->MultiFile->MD5FileCharCode = MD5FILE_CHARCODE_ANSI;
	DWORD dwHitCharCode[3] = {1, 0, 0};
	DWORD dwSize;


	Buf = (BYTE*)malloc(MD5FILE_CHARCODE_BUFFERSIZE);
	if(Buf == NULL)
	{
		return (DWORD)-1;
	}

	if((fp1 = _tfopen(inFilePath, _T("rb"))) == NULL)
	{
		free(Buf);
		return (DWORD)-1;
	}

	memset(Buf, NULL, MAX_PATH_SIZE);
	dwSize = (DWORD)fread(Buf, sizeof(unsigned char), MD5FILE_CHARCODE_BUFFERSIZE, fp1);
	fclose(fp1);

	// 文字コードの検出
	if(*(WORD*)Buf == 0xFEFF)
	{
		// UTF-16 Little Endian
		dwHitCharCode[1] += 2;
		lpMD5Thread->MultiFile->MD5FileBom = 2;
	}
	else if((*(DWORD*)Buf & 0x00FFFFFF) == 0x00BFBBEF)
	{
		// UTF-8 BOM
		dwHitCharCode[2] += 2;
		lpMD5Thread->MultiFile->MD5FileBom = 3;
	}
	else
	{
		unsigned char *b = Buf;

		lpMD5Thread->MultiFile->MD5FileBom = 0;


		for(UINT i = 0; i < dwSize;)
		{
			if((*b & 0xE0) == 0xE0 && (*(b+1) >= 0x80 && *(b+1) <= 0xBF) && (*(b+2) >= 0x80 && *(b+2) <= 0xBF))
			{
				dwHitCharCode[2]++;// UTF-8 3byte
				i += 3;
				b += 3;
			}
			else if((*b & 0xC0) == 0xC0 && (*(b+1) >= 0x80 && *(b+1) <= 0xBF))
			{
				dwHitCharCode[2]++;// UTF-8 2byte
				i += 2;
				b += 2;
			}
			else if(((*b >= 0x81 && *b <= 0x9F) || (*b >= 0xE0 && *b <= 0xEF)) && 
					((*(b+1) >= 0x40 && *(b+1) <= 0x7E) || (*(b+1) >= 0x80 && *(b+1) <= 0xFC)))
			{
				dwHitCharCode[0]++;// Shift-JIS
				i += 2;
				b += 2;
			}
			else if(*b >= 0x81 && *b <= 0xDF)
			{
				dwHitCharCode[0]++;// Shift_JIS 半角カナ
				i++;
				b++;
			}
			else if(*(b+1) == NULL)
			{
				if(!(i == (dwSize - 1)))
				{
					dwHitCharCode[1]++;// UTF-16 Little Endian
					i += 2;
					b += 2;
				}
				else
				{
					i++;
					b++;
				}
			}
			else
			{
				//dwHitCharCode[0]++;// Ansi
				i++;
				b++;
			}
		}
	}
	free(Buf);

	if(dwHitCharCode[1] > max(dwHitCharCode[0], dwHitCharCode[2]))
	{
		lpMD5Thread->MultiFile->MD5FileCharCode = dwCharCode = MD5FILE_CHARCODE_UNICODE;
	}


	if(dwHitCharCode[2] > max(dwHitCharCode[0], dwHitCharCode[1]))
	{
		lpMD5Thread->MultiFile->MD5FileCharCode = dwCharCode = MD5FILE_CHARCODE_UTF8;
	}

	return dwCharCode;
}

DWORD MD5_Thread_CreateBuffer(MD5Thread *lpMD5Thread, DWORD dwFileCount, BOOL IsBufferCopy)
{
	DWORD ___dwBufLen = 0;
	DWORD ___dwBufSize;

#ifdef _DEBUG
	DWORD dwTime1, dwTime2;
	TCHAR szDebugText[1024];
	TCHAR *pMalloc = _T("");
#endif


	if(lpMD5Thread->MultiFile == NULL)
	{
		lpMD5Thread->MultiFile = MD5_Thread_CreateMultiFileBuffer(lpMD5Thread, dwFileCount);

		if(lpMD5Thread->MultiFile == NULL)
		{
#ifdef _DEBUG
			pMalloc = _T("MD5_Thread_CreateMultiFileBuffer()");
#endif
			goto MALLOC_FAIL;
		}
		___dwBufLen = lpMD5Thread->MultiFile->MaxBuffer;
	}


	if(dwFileCount > lpMD5Thread->MultiFile->MaxBuffer)
	{
		tagMD5Thread_FileRecode	*___FileRecode1;
		tagMD5Thread_FileRecode	*___FileRecode2;
		BOOL ___IsFileMapping;

		___dwBufSize = lpMD5Thread->MultiFile->MaxBuffer;


		if(___dwBufSize > FILE_MAX_COUNTSIZE)
		{
#ifdef _DEBUG
			_stprintf(szDebugText, _T("MD5Thraed: CreateBuffer(): FILE_MAX_COUNTSIZE (%d) の上限が超えてます。\r\n"), FILE_MAX_COUNTSIZE);
			szDebugText[1023] = '\0';
			OutputDebugString(szDebugText);
#endif
			return (DWORD)-1;
		}

		if(___dwBufSize == 0)
		{
			___dwBufSize = FILE_DEF_COUNTSIZE;
			lpMD5Thread->MultiFile->FileRecode = NULL;
		}
		

		// 大きめにバッファのサイズを計算する
		while(___dwBufSize < dwFileCount)
		{
			if(___dwBufSize < 4096)
				___dwBufSize *= FILE_DEF_COUNTSIZE;// dwBufSize = dwBufSize << 4 同等
			else
				___dwBufSize *= 2;
		}



		___IsFileMapping	= lpMD5Thread->IsFileMapping;
		___dwBufLen			= lpMD5Thread->MultiFile->FileCount;
		___FileRecode1		= lpMD5Thread->MultiFile->FileRecode;
		___FileRecode2		= MD5_Thread_CreateFileRecodeBuffer(lpMD5Thread, ___dwBufSize);


		if(___FileRecode2 == NULL)
		{
#ifdef _DEBUG
			pMalloc = _T("MD5_Thread_CreateFileRecodeBuffer()");
#endif
			goto MALLOC_FAIL;
		}
		lpMD5Thread->MultiFile->FileRecode = ___FileRecode2;


		if(lpMD5Thread->IsFileMapping == 0 &&
			IsBufferCopy != 0)
		{
#ifdef _DEBUG
			___dwBufLen = lpMD5Thread->MultiFile->MaxBuffer;
			dwTime1 = timeGetTime();
#endif
			memcpy(___FileRecode2, ___FileRecode1, sizeof(tagMD5Thread_FileRecode) * ___dwBufLen);

#ifdef _DEBUG
			dwTime2 = timeGetTime();
			_stprintf(szDebugText, _T("MD5Thraed: CreateBuffer(): バッファを %d個コピーしました。%dミリ秒\r\n"), ___dwBufSize,
				dwTime2 - dwTime1);
			szDebugText[1023] = '\0';
			OutputDebugString(szDebugText);
#endif
		}


		if(___IsFileMapping == 0 &&
			___FileRecode1 != NULL)
		{
			free(___FileRecode1);
		}
		lpMD5Thread->MultiFile->MaxBuffer = ___dwBufSize;
	}

	return ___dwBufLen;


MALLOC_FAIL:

#ifdef _DEBUG
	_stprintf(szDebugText, _T("MD5Thraed: CreateBuffer(): バッファ確保に失敗しました。(%s)\r\n"), pMalloc);
	szDebugText[1023] = '\0';
	OutputDebugString(szDebugText);
#endif

	return (DWORD)-1;
}

tagMD5Thread_MultiFile *MD5_Thread_CreateMultiFileBuffer(MD5Thread *lpMD5Thread, DWORD dwBufSize)
{
	tagMD5Thread_MultiFile	*___MultiFileBuffer;

#ifdef _DEBUG
	TCHAR szDebugText[1024];
	TCHAR *pMalloc = _T("");
#endif


	// メモリー確保
	___MultiFileBuffer = (tagMD5Thread_MultiFile*)malloc(sizeof(tagMD5Thread_MultiFile));
	if(___MultiFileBuffer == NULL)
	{
#ifdef _DEBUG
		pMalloc = _T("___MultiFileBuffer");
#endif
		goto MALLOC_FAIL;
	}

	ZeroMemory(___MultiFileBuffer, sizeof(tagMD5Thread_MultiFile));
	___MultiFileBuffer->dwStructSize	= sizeof(tagMD5Thread_MultiFile);
	___MultiFileBuffer->MaxBuffer		= dwBufSize;
	___MultiFileBuffer->FileRecode		= MD5_Thread_CreateFileRecodeBuffer(lpMD5Thread, dwBufSize);

	return ___MultiFileBuffer;


MALLOC_FAIL:

	if(___MultiFileBuffer != NULL)
		free(___MultiFileBuffer);

#ifdef _DEBUG
	_stprintf(szDebugText, _T("MD5Thraed: CreateMultiFileBuffer(): バッファ確保に失敗しました。(%s)\r\n"), pMalloc);
	szDebugText[1023] = '\0';
	OutputDebugString(szDebugText);
#endif

	return NULL;
}

tagMD5Thread_FileRecode *MD5_Thread_CreateFileRecodeBuffer(MD5Thread *lpMD5Thread, DWORD dwBufSize)
{
	tagMD5Thread_FileRecode	*___Buf = NULL;
	size_t ___BufSize = 0;

#ifdef _DEBUG
	TCHAR szDebugText[1024];
	TCHAR *pMalloc = _T("");
#endif

	if(lpMD5Thread == NULL)
	{
		return NULL;
	}


	___BufSize = dwBufSize * sizeof(tagMD5Thread_FileRecode);

	if(lpMD5Thread->IsFileMapping != FALSE)
	{
FILE_MAPPING:

		if(lpMD5Thread->hFileMapping != NULL)
		{
			UnmapViewOfFile(lpMD5Thread->MultiFile->FileRecode);
			CloseHandle(lpMD5Thread->hFileMapping);
			CloseHandle(lpMD5Thread->hFile);
		}

		lpMD5Thread->hFile = CreateFile(lpMD5Thread->FileMapping, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
		if(lpMD5Thread->hFile == INVALID_HANDLE_VALUE)
		{
			goto MAPPING_FAIL;
		}
	
		lpMD5Thread->hFileMapping = CreateFileMapping(lpMD5Thread->hFile, 0, PAGE_READWRITE, 0, (DWORD)___BufSize, NULL);
		if(lpMD5Thread->hFileMapping <= 0)
		{
			CloseHandle(lpMD5Thread->hFile);
			lpMD5Thread->hFile = NULL;
			goto MAPPING_FAIL;
		}

		___Buf = (tagMD5Thread_FileRecode*)MapViewOfFile(lpMD5Thread->hFileMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);

	}
	else
	{
		___Buf = (tagMD5Thread_FileRecode*)malloc(___BufSize);

		if(___Buf == NULL)
		{
#ifdef _DEBUG
			pMalloc = _T("___Buf");
#endif
			lpMD5Thread->IsFileMapping = 1;
			goto FILE_MAPPING;
		}
		lpMD5Thread->IsFileMapping = 0;
	}

#ifdef _DEBUG
	_stprintf(szDebugText, _T("MD5Thraed: CreateFileRecodeBuffer(): バッファが %d個確保できました。(0x%08p, %dByte)\r\n"),
		dwBufSize, &___Buf, ___BufSize);
	szDebugText[1023] = '\0';
	OutputDebugString(szDebugText);
#endif

	return ___Buf;


MAPPING_FAIL:

#ifdef _DEBUG
	_stprintf(szDebugText, _T("MD5Thraed: CreateFileRecodeBuffer(): ファイルマッピングに失敗しました。(%s)\r\n"), pMalloc);
	szDebugText[1023] = '\0';
	OutputDebugString(szDebugText);
#endif

	return NULL;
}

BOOL MD5_Thread_ClearBuffer(MD5Thread *lpMD5Thread)
{
	BOOL ret;

	ret = MD5_Thread_ClearBuffer_Core(lpMD5Thread, FALSE);
	lpMD5Thread->MultiFile->FileCurrentCount	= 0;
	lpMD5Thread->MultiFile->FileCount			= 0;
	lpMD5Thread->MultiFile->MaxBuffer			= 0;

#ifdef _DEBUG
	OutputDebugString(_T("MD5Thread: ClearBuffer(): バッファのクリアしました。\r\n"));
#endif
	return ret;
}

BOOL MD5_Thread_ClearBuffer_Core(MD5Thread *lpMD5Thread, BOOL IsMultiFileClear)
{
	if(lpMD5Thread->MultiFile == NULL)
	{
		return FALSE;
	}

	if(lpMD5Thread->MultiFile->FileRecode != NULL &&
		lpMD5Thread->IsFileMapping == FALSE)
	{
		free(lpMD5Thread->MultiFile->FileRecode);
		lpMD5Thread->MultiFile->FileRecode = NULL;
	}

	if(IsMultiFileClear != FALSE)
	{
		free(lpMD5Thread->MultiFile);
		lpMD5Thread->MultiFile = NULL;
	}

#ifdef _DEBUG
	OutputDebugString(_T("MD5Thread: ClearBuffer_Core(): バッファのクリアしました。\r\n"));
#endif
	return TRUE;
}

BOOL MD5_Thread_DeleteItemBuffer(MD5Thread *lpMD5Thread, DWORD dwItem)
{
	DWORD ___dwBufLen;
	LONGLONG ___FileAllSize;
#ifdef _DEBUG
	DWORD dwTime1, dwTime2;
	TCHAR szDebugText[1024];
#endif


	if(lpMD5Thread->MultiFile == NULL ||
		dwItem > lpMD5Thread->MultiFile->FileCount)
	{
		return FALSE;
	}
	if(lpMD5Thread->MultiFile->FileCount == 1)
	{
		lpMD5Thread->MultiFile->FileCurrentCount	= 0;
		lpMD5Thread->MultiFile->FileCount			= 0;
		lpMD5Thread->MultiFile->FileAllSize			= 0;
		lpMD5Thread->MultiFile->FileAllCurrentSize	= 0;
		return FALSE;
	}

	___dwBufLen		= lpMD5Thread->MultiFile->FileCount - dwItem;
	___FileAllSize	= (lpMD5Thread->MultiFile->FileRecode + dwItem)->FileSize;

#ifdef _DEBUG
	dwTime1 = timeGetTime();
#endif
	memmove(lpMD5Thread->MultiFile->FileRecode + dwItem,
			lpMD5Thread->MultiFile->FileRecode + dwItem + 1,
			___dwBufLen * sizeof(tagMD5Thread_FileRecode));

#ifdef _DEBUG
	dwTime2 = timeGetTime();
	_stprintf(szDebugText, _T("MD5Thraed: DeleteItemBuffer(): バッファの項目を 1個削除しました。項目(%d)メモリームーブ(%d個) %dミリ秒\r\n"), dwItem, ___dwBufLen,
		dwTime2 - dwTime1);
	szDebugText[1023] = '\0';
	OutputDebugString(szDebugText);
#endif
	lpMD5Thread->MultiFile->FileAllSize -= ___FileAllSize;
	if(lpMD5Thread->MultiFile->FileAllSize < 0)
	{
		lpMD5Thread->MultiFile->FileAllSize = 0;
	}
	lpMD5Thread->MultiFile->FileAllSizeBackup = lpMD5Thread->MultiFile->FileAllSize;
	lpMD5Thread->MultiFile->FileCount--;

	return TRUE;
}

BOOL MD5_Thread_SwapItemBuffer(MD5Thread *lpMD5Thread, DWORD dwItem1, DWORD dwItem2)
{
	tagMD5Thread_FileRecode ___FileRecode;

#ifdef _DEBUG
	TCHAR szDebugText[1024];
#endif


	if(dwItem1 > lpMD5Thread->MultiFile->FileCount ||
		dwItem2 > lpMD5Thread->MultiFile->FileCount)
	{
		return FALSE;
	}


	memcpy(&___FileRecode,
		lpMD5Thread->MultiFile->FileRecode + dwItem1,
		sizeof(tagMD5Thread_FileRecode));

	memcpy(lpMD5Thread->MultiFile->FileRecode + dwItem1,
		lpMD5Thread->MultiFile->FileRecode + dwItem2,
		sizeof(tagMD5Thread_FileRecode));

	memcpy(lpMD5Thread->MultiFile->FileRecode + dwItem2,
		&___FileRecode,
		sizeof(tagMD5Thread_FileRecode));

#ifdef _DEBUG
	_stprintf(szDebugText, _T("MD5Thraed: SwapItemBuffer(): バッファの項目を入れ替えました。%d←→%d\r\n"), dwItem1, dwItem2);
	szDebugText[1023] = '\0';
	OutputDebugString(szDebugText);
#endif
	return TRUE;
}

BOOL MD5_Thread_CreateMD5File(MD5Thread *lpMD5Thread, const TCHAR *inFilePath, const TCHAR *inOption)
{
	TCHAR *szBuf;
	TCHAR *szFilePath;
	TCHAR *szCurrentFolder;
	TCHAR *lpSaveOption = _T("wb");;
	BYTE  *bBuf;
	DWORD dwCharCode	= 0;
	BOOL  IsoldMD5File	= FALSE;
	BOOL  IsMD5Upper	= FALSE;
	size_t	szLen;


#ifdef _DEBUG
	int nRet;
	TCHAR szDebugText[1024];
	TCHAR *szCharCodeText[] = {_T("Shift_JIS"), _T("Unicode"), _T("UTF-8")};
#endif


	szBuf = (TCHAR*)malloc((MAX_PATH_SIZE * 8) * sizeof(TCHAR));
	if(szBuf == NULL)
	{
		return 0;
	}
	*szBuf				= '\0';
	bBuf				= (BYTE*)(MAX_PATH_SIZE * 2 + szBuf);
	*bBuf				= '\0';
	szFilePath			= MAX_PATH_SIZE * 2 + (TCHAR*)bBuf;
	*szFilePath			= '\0';
	szCurrentFolder		= MAX_PATH_SIZE * 2 + szFilePath;
	*szCurrentFolder	= '\0';


	dwCharCode = lpMD5Thread->MultiFile->MD5FileCharCode;

	// オプションの解析
	if(inOption != NULL)
	{
		for(; *inOption != NULL;)
		{
			switch(*inOption)
			{
			case 'c':
				inOption++;
				dwCharCode = *inOption - '0';

				if(dwCharCode > 2)
				{
					dwCharCode = 0;
				}
#ifdef _DEBUG
				_stprintf(szDebugText, _T("MD5Thread: CreateMD5File(): %sの文字コード出力が選択されました。\r\n"), szCharCodeText[dwCharCode]);
				szDebugText[1023] = '\0';
				OutputDebugString(szDebugText);
#endif
				break;
			case 'l':
				IsMD5Upper = FALSE;
#ifdef _DEBUG
				OutputDebugString(_T("MD5Thread: CreateMD5File(): MD5の小文字出力が選択されました。\r\n"));
#endif
				break;
			case 'u':
				IsMD5Upper = TRUE;
#ifdef _DEBUG
				OutputDebugString(_T("MD5Thread: CreateMD5File(): MD5の大文字出力が選択されました。\r\n"));
#endif
				break;
			case 'o':
				IsoldMD5File = TRUE;
#ifdef _DEBUG
				OutputDebugString(_T("MD5Thread: CreateMD5File(): MD5チェックツール (1x) 形式の MD5ファイル出力が選択されました。\r\n"));
#endif
				break;
			}
			inOption++;
		}
	}



	if(IsoldMD5File != 0)
	{
		for(DWORD dwCount = 0,
			dwFileCount = lpMD5Thread->MultiFile->FileCount; dwCount < dwFileCount; dwCount++)
		{
			if((lpMD5Thread->MultiFile->FileRecode + dwCount)->FileLastError == 0 ||
				(lpMD5Thread->MultiFile->FileRecode + dwCount)->MD5FileHashLine != -1)
			{
				szLen = _tcslen((lpMD5Thread->MultiFile->FileRecode + dwCount)->FileName);

				if((szLen + 4) < MAX_PATH_SIZE)
				{
					FILE *fp1;

					_tcscpy(szFilePath, (lpMD5Thread->MultiFile->FileRecode + dwCount)->FileName);
					_tcscpy(szFilePath + szLen, _T(".md5"));

					if((fp1 = _tfopen(szFilePath, lpSaveOption)) != NULL)
					{
						if((lpMD5Thread->MultiFile->FileRecode + dwCount)->FileLastError != 0 &&
							(lpMD5Thread->MultiFile->FileRecode + dwCount)->MD5FileHashLine != -1)
						{
							memcpy((lpMD5Thread->MultiFile->FileRecode + dwCount)->FileHashByte,
								(lpMD5Thread->MultiFile->FileRecode + dwCount)->MD5FileHash,
								HASH_LEN);
						}

						MD5_Thread_MD5ToString(szBuf, (lpMD5Thread->MultiFile->FileRecode + dwCount)->FileHashByte, IsMD5Upper);

						switch(dwCharCode)
						{
						case MD5FILE_CHARCODE_SHIFT_JIS:
						case MD5FILE_CHARCODE_UTF8:
#ifdef _UNICODE
							szLen = UnicodeToShiftJis((char*)bBuf, szBuf) - 1;
							if(szLen > 0)
								fwrite(bBuf, sizeof(char), HASH_LEN * 2, fp1);
#else
							fwrite(szBuf, sizeof(char), HASH_LEN * 2, fp1);
#endif
							break;
						case MD5FILE_CHARCODE_UNICODE:
							// BOM の書き込みします。
							*(WORD*)bBuf = 0xFEFF;
							fwrite(bBuf, 1, 2, fp1);

#ifdef _UNICODE
							fwrite(szBuf, sizeof(wchar_t), HASH_LEN * 2, fp1);
#else
							szLen = ShiftJisToUnicode((wchar_t*)bBuf, szBuf) - 1;
							if(szLen > 0)
								fwrite(bBuf, sizeof(wchar_t), HASH_LEN * 2, fp1);
#endif
							break;
						}
						fclose(fp1);

						(lpMD5Thread->MultiFile->FileRecode + dwCount)->MD5FileHashLine = 1;
						(lpMD5Thread->MultiFile->FileRecode + dwCount)->MD5FileHashCmp  = 0;
						memcpy((lpMD5Thread->MultiFile->FileRecode + dwCount)->MD5FileHash,
							(lpMD5Thread->MultiFile->FileRecode + dwCount)->FileHashByte,
							HASH_LEN);
					}
#ifdef _DEBUG
					else
					{
						nRet = errno;
						OutputDebugString(_T("MD5Thread: CreateMD5File(): _ffopen() のエラーが発生しました。\r\n"));
						OutputDebugString(_tcserror(nRet));
						OutputDebugString(_T("\r\n"));
					}
#endif
				}
			}
		}
	}
	else
	{
		HANDLE	hFile;
		DWORD	dwWrite;
		LARGE_INTEGER liSize;

#if _MSC_VER < 1500
		UINT  nRet;
		UINT  nLen;
		TCHAR *szPath;
		const TCHAR *szFile = _T("KERNEL32.DLL");
		const char  *szFunc = "SetFilePointerEx";
#endif /* _MSC_VER < 1500 */


		DeleteFile(inFilePath);

		hFile = CreateFile(
			inFilePath, GENERIC_WRITE, 0, NULL,
			CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL
		);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			free(szBuf);

#ifdef _DEBUG
			OutputDebugString(_T("MD5Thread: CreateMD5File(): CreateFile() のエラーが発生しました。\r\n"));
#endif
			return 0;
		}


#if _MSC_VER < 1500
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

					if(dwCharCode == 0)
						liSize.QuadPart = MAX_PATH + 100;
					else
						liSize.QuadPart = MAX_PATH;

					liSize.QuadPart = (liSize.QuadPart * (dwCharCode + 1) + HASH_LEN * 2 + 2) * lpMD5Thread->MultiFile->FileCount;
					SetFilePointerEx(hFile, liSize, NULL, FILE_BEGIN);
					SetEndOfFile(hFile);
					liSize.QuadPart = 0;
					SetFilePointerEx(hFile, liSize, NULL, FILE_BEGIN);

#if _MSC_VER < 1500
				}
				FreeLibrary(hDll);
			}
		}
DLLFOUL:
		free(szPath);
#endif /* _MSC_VER < 1500 */


		if(dwCharCode == MD5FILE_CHARCODE_UNICODE)
		{
			*(WORD*)bBuf = 0xFEFF;
			WriteFile(hFile, bBuf, 2, &dwWrite, FALSE);
		}
		GetCurrentDirectory(MAX_PATH_SIZE, szCurrentFolder);

		for(DWORD dwCount = 0,
			dwFileCount = lpMD5Thread->MultiFile->FileCount; dwCount < dwFileCount; dwCount++)
		{
			if((lpMD5Thread->MultiFile->FileRecode + dwCount)->FileLastError == 0 ||
				(lpMD5Thread->MultiFile->FileRecode + dwCount)->MD5FileHashLine != -1)
			{
				TCHAR *p = szBuf;

				if((lpMD5Thread->MultiFile->FileRecode + dwCount)->FileLastError != 0 &&
					(lpMD5Thread->MultiFile->FileRecode + dwCount)->MD5FileHashLine != -1)
				{
					memcpy((lpMD5Thread->MultiFile->FileRecode + dwCount)->FileHashByte,
						(lpMD5Thread->MultiFile->FileRecode + dwCount)->MD5FileHash,
						HASH_LEN);
				}

				(lpMD5Thread->MultiFile->FileRecode + dwCount)->MD5FileHashLine = dwCount;
				(lpMD5Thread->MultiFile->FileRecode + dwCount)->MD5FileHashCmp  = 0;
				memcpy((lpMD5Thread->MultiFile->FileRecode + dwCount)->MD5FileHash,
					(lpMD5Thread->MultiFile->FileRecode + dwCount)->FileHashByte,
					HASH_LEN);

				if(!PathRelativePathTo(szFilePath,
					szCurrentFolder, FILE_ATTRIBUTE_DIRECTORY,
					(lpMD5Thread->MultiFile->FileRecode + dwCount)->FileName,
					FILE_ATTRIBUTE_ARCHIVE))
				{
					_tcscpy(szFilePath, (lpMD5Thread->MultiFile->FileRecode + dwCount)->FileName);
				}


				MD5_Thread_MD5ToString(p, (lpMD5Thread->MultiFile->FileRecode + dwCount)->FileHashByte, IsMD5Upper);
#ifdef _UNICODE
				p += HASH_LEN * 2;
				*(DWORD*)p = 0x00200020;
				p += 2;
				p = MD5_Thread_tcscpy(p, szFilePath);
				*(DWORD*)p = 0x000A000D;
				p += 2;
				*p = '\0';
#else
				p += HASH_LEN * 2;
				*(WORD*)p = 0x2020;
				p += 2;
				p = MD5_Thread_tcscpy(p, szFilePath);
				*(DWORD*)p = 0x0A0D;
				p += 2;
				*p = '\0';
#endif

				switch(dwCharCode)
				{
				case MD5FILE_CHARCODE_SHIFT_JIS:
#ifdef _UNICODE
					szLen = UnicodeToShiftJis((char*)bBuf, szBuf) - 1;
					if(szLen > 0)
						WriteFile(hFile, bBuf, szLen, &dwWrite, FALSE);
#else
					szLen = strlen(szBuf);
					WriteFile(hFile, szBuf, szLen, &dwWrite, FALSE);
#endif
					break;
				case MD5FILE_CHARCODE_UNICODE:
#ifdef _UNICODE
					szLen = wcslen(szBuf);
					WriteFile(hFile, szBuf, szLen * sizeof(TCHAR), &dwWrite, FALSE);
#else
					szLen = ShiftJisToUnicode((wchar_t*)bBuf, szBuf) - 1;
					if(szLen > 0)
						WriteFile(hFile, bBuf, szLen * sizeof(wchar_t), &dwWrite, FALSE);
#endif
					break;
				case MD5FILE_CHARCODE_UTF8:
#ifdef _UNICODE
					szLen = UnicodeToUTF8((char*)bBuf, szBuf) - 1;
#else
					szLen = ShiftJisToUTF8((char*)bBuf, szBuf) - 1;
#endif
					if(szLen > 0)
						WriteFile(hFile, bBuf, szLen, &dwWrite, FALSE);
					break;
				}
			}
		}
		SetEndOfFile(hFile);
		CloseHandle(hFile);
	}
	free(szBuf);

	return 0;
}

BOOL MD5_Thread_FileNameCheck(TCHAR *inFile)
{
	wchar_t *szBuf;
	wchar_t *p;
	int EnvironmentCount	= 0;
	BOOL IsEnvironment		= 0;

#ifndef _UNICODE
	size_t szLen;
#endif

	szBuf = p = (wchar_t*)malloc(MAX_PATH_SIZE * 2 * sizeof(wchar_t));
	if(szBuf == NULL)
	{
		return 0;
	}


#ifdef _UNICODE
	_tcscpy(szBuf, inFile);
#else
	// マルチバイト特有の不具合防止に一度 Unicode に変換します。

	szLen = ShiftJisToUnicode(szBuf, inFile) - 1;
	if(!(szLen > 1))
	{
		free(szBuf);
		return 0;
	}
#endif

	// "ファイル名" に指定された『"』の削除
	if(*p == '"')
	{
#ifdef _DEBUG
		OutputDebugString(_T("MD5Thread: MD5_Thread_FileNameCheck(): ファイル名のダブルコーテーションを削除します。\r\n"));
#endif

		PathUnquoteSpacesW(szBuf);
	}

/*
	if(*p == '"')
	{
		TCHAR *p1 = p;

		while(*p1 != NULL)
		{
			*p1++ = *(p1 + 1);
		}

		if(*(p1 - 2) == '"')
		{
			*(p1 - 2) = '\0';
		}
*/

	while(*p != NULL)
	{
		if(0 <= *p && *p < 32)
		{
			*p = '_';
		}
		else
		{
			switch(*p)
			{
			case '"':
			case '<':
			case '>':
			case '|':
			case '?':
				*p = '\\';
				break;
			case '%':
				if(EnvironmentCount == 0)
				{
					EnvironmentCount++;
				}
				else
				{
					EnvironmentCount = 0;
					IsEnvironment++;
				}
				break;
			}
		}
		p++;
	}


#ifdef _UNICODE
	_tcscpy(inFile, szBuf);
#else
	szLen = UnicodeToShiftJis(inFile, szBuf) - 1;
	if(!(szLen > 1))
	{
		free(szBuf);
		return 0;
	}
#endif

	free(szBuf);

	return IsEnvironment + 1;
}

TCHAR *MD5_Thread_MD5ToString_no_sprintf(TCHAR *inString, const BYTE *inHashByte, BOOL IsMD5Upper)
{
	static TCHAR hex[] = _T("0123456789abcdef");
	TCHAR	*p = (TCHAR*)inString;
	DWORD	dwCount;
	DWORD	HashDWord;
	DWORD	*b = (DWORD*)inHashByte;


	for(dwCount = 0; dwCount < HASH_LEN; b++, dwCount += 4)
	{
		HashDWord	= *b & 0x000000FF;
		*p++		= hex[HashDWord >> 4];
		*p++		= hex[HashDWord & 0x0F];
		HashDWord	= (*b & 0x0000FF00) >> 8;
		*p++		= hex[HashDWord >> 4];
		*p++		= hex[HashDWord & 0x0F];
		HashDWord	= (*b & 0x00FF0000) >> 16;
		*p++		= hex[HashDWord >> 4];
		*p++		= hex[HashDWord & 0x0F];
		HashDWord	= (*b & 0xFF000000) >> 24;
		*p++		= hex[HashDWord >> 4];
		*p++		= hex[HashDWord & 0x0F];
	}
	*p = '\0';

	if(IsMD5Upper)
	{
		_tcsupr(inString);
	}

	return inString;
}

BYTE *MD5_Thread_MD5ToHash_no_scanf(BYTE *inHashByte, const TCHAR *inString, BOOL DigitCheck)
{
	TCHAR	szBuf[9];
	TCHAR	*p = (TCHAR*)inString;
	TCHAR	*endptr;
	DWORD	dwCount;
	DWORD	HashDWord;
	DWORD	*b = (DWORD*)inHashByte;


	szBuf[8] = '\0';
	for(dwCount = 0; dwCount < HASH_LEN; dwCount += 4, b++, p += 8)
	{
		memcpy(szBuf, p, 8 * sizeof(TCHAR));

		if(!DigitCheck ||
			_istxdigit(szBuf[0]) && _istxdigit(szBuf[1]) && _istxdigit(szBuf[2]) && _istxdigit(szBuf[3]) &&
			_istxdigit(szBuf[4]) && _istxdigit(szBuf[5]) && _istxdigit(szBuf[6]) && _istxdigit(szBuf[7]))
		{
			HashDWord = _tcstoul(szBuf, &endptr, 16);
			HashDWord = (HashDWord >> 24) | (HashDWord << 24) | ((HashDWord >> 8) & 0xFF00) | ((HashDWord << 8) & 0xFF0000);
			*b = HashDWord;
		}
		else
		{
			return NULL;
		}
	}

	return inHashByte;
}

TCHAR *MD5_Thread_MD5ToString_Default(TCHAR *inString, const BYTE *inHashByte, BOOL IsMD5Upper)
{
	TCHAR	szBuf[9];
	TCHAR	*lpFormat;
	TCHAR	*p = (TCHAR*)inString;
	DWORD	dwCount;
	DWORD	HashDWord;
	DWORD	*b = (DWORD*)inHashByte;

	lpFormat = IsMD5Upper != 0 ? _T("%08X") : _T("%08x");

	for(dwCount = 0; dwCount < HASH_LEN; dwCount += 4, b++)
	{
		HashDWord = *b;
		HashDWord = (HashDWord >> 24) | (HashDWord << 24) | ((HashDWord >> 8) & 0xFF00) | ((HashDWord << 8) & 0xFF0000);
		_stprintf(szBuf, lpFormat, HashDWord);
		memcpy(p, szBuf, 8 * sizeof(TCHAR));
		p += 8;
	}
	*p = '\0';

	return inString;
}

BYTE *MD5_Thread_MD5ToHash_Default(BYTE *inHashByte, const TCHAR *inString, BOOL DigitCheck)
{
	TCHAR	szBuf[9];
	TCHAR	*p = (TCHAR*)inString;
	DWORD	dwCount;
	DWORD	HashDWord;
	DWORD	*b = (DWORD*)inHashByte;


	szBuf[8] = '\0';
	for(dwCount = 0; dwCount < HASH_LEN; dwCount += 4, b++, p += 8)
	{
		memcpy(szBuf, p, 8 * sizeof(TCHAR));

		if(!DigitCheck ||
			_istxdigit(szBuf[0]) && _istxdigit(szBuf[1]) && _istxdigit(szBuf[2]) && _istxdigit(szBuf[3]) &&
			_istxdigit(szBuf[4]) && _istxdigit(szBuf[5]) && _istxdigit(szBuf[6]) && _istxdigit(szBuf[7]))
		{
			_stscanf(szBuf, _T("%8x"), &HashDWord);
			HashDWord = (HashDWord >> 24) | (HashDWord << 24) | ((HashDWord >> 8) & 0xFF00) | ((HashDWord << 8) & 0xFF0000);
			*b = HashDWord;
		}
		else
		{
			return NULL;
		}
	}

	return inHashByte;
}

TCHAR *MD5_Thread_MD5ToString_Old(TCHAR *inString, const BYTE *inHashByte, BOOL IsMD5Upper)
{
	TCHAR szBuf[3];
	TCHAR *lpFormat = IsMD5Upper != 0 ? _T("%02X") : _T("%02x");
	TCHAR *p = inString;
	BYTE  *b = (BYTE*)inHashByte;


	for(DWORD dwCount = 0; dwCount < HASH_LEN; dwCount++, b++)
	{
		_stprintf(szBuf, lpFormat, *b);
		*(p++) = *szBuf;
		*(p++) = *(szBuf + 1);
	}
	*p = '\0';

	return inString;
}

BYTE *MD5_Thread_MD5ToHash_Old(BYTE *inHashByte, const TCHAR *inString)
{
	TCHAR *p = (TCHAR*)inString;
	TCHAR szBuf[3];
	BOOL  Val;
	BYTE  *b = inHashByte;

	szBuf[2] = '\0';
	for(DWORD dwCount = 0; dwCount < HASH_LEN; dwCount++, b++)
	{
		memcpy(szBuf, p, 2 * sizeof(TCHAR)); 
		if(isxdigit(szBuf[0]) && isxdigit(szBuf[1]))
		{
			_stscanf(szBuf, _T("%x"), &Val);
			*b = (BYTE)Val;
			p += 2;
		}
		else
		{
			return NULL;
		}
	}

	return inHashByte;
}

TCHAR *MD5_Thread_tcscpy(TCHAR *dst, const TCHAR *src)
{
	while(*src)
		*dst++ = *src++;

	*dst = '\0';
	return dst;
}
