
// WindowSize.h
//
//
// �ȉ��̃T�C�g�̃R�[�h���Q�l�ɂ��܂����B
//
// ���E�B���h�E�T�C�Y�̕���
//
//   http://oldworldgarage.web.fc2.com/programing/tip0006_RestoreWindow.html
//

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

#ifndef __WINDOWSIZE_H__
#define __WINDOWSIZE_H__

#include <windows.h>


#ifdef __cplusplus
extern "C" {
#endif

VOID RestoreWindow_SaveState(HWND hWnd, LPCTSTR inWindowName, LPCTSTR inIniFile);// �E�C���h�E�̃T�C�Y�𕜌����܂��B
VOID RestoreWindow_LoadState(HWND hWnd, LPCTSTR inWindowName, LPCTSTR inIniFile);// �E�C���h�E�̃T�C�Y��ۑ����܂��B

#ifdef __cplusplus
}
#endif
#endif /* __WINDOWSIZE_H__ */
