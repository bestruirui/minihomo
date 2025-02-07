#pragma once
#include <windows.h>
#include <string>

// 窗口相关常量
extern const wchar_t* CLASS_NAME;
extern const wchar_t* WINDOW_TITLE;

// 全局变量声明
extern HWND hwnd;
extern NOTIFYICONDATAW nid;
extern HMENU hTrayMenu;

// 菜单ID
#define ID_TRAY_START    2001
#define ID_TRAY_STOP     2002
#define ID_TRAY_EXIT     2003
#define ID_AUTOSTART     1005
#define ID_HOTKEY_TOGGLE 1

// 窗口过程函数声明
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// 托盘菜单相关函数
void ShowTrayMenu(HWND hwnd);
void ShowBalloonTip(const wchar_t* title, const wchar_t* message);

// 在文件末尾添加
int GetWindowDpiScale(HWND hwnd);
int ScaleForDpi(int value, int dpi); 