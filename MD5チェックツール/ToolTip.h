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

#ifndef __TOOLTIP__
#define __TOOLTIP__

#include <windows.h>
#include <commctrl.h>

#ifdef __cplusplus
extern "C" {
#endif

// ���̃R�[�h ���W���[���Ɋ܂܂��֐��̐錾��]�����܂�:
VOID SetToolTip(const HWND hTool, const HWND hWnd, const TCHAR *lpString);
VOID SetToolTipText(const HWND hTool, const HWND hWnd, const TCHAR *lpString);

#ifdef __cplusplus
}
#endif
#endif /* __TOOLTIP_H__ */
