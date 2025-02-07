#pragma once
#include <windows.h>

// 控件句柄声明
extern HWND hEditMihomo, hEditConfig;
extern HWND hBtnBrowseMihomo, hBtnBrowseConfig;
extern HWND hBtnStart, hBtnStop;
extern HWND hBtnAutostart;

// 控件相关函数声明
void CreateControls(HWND hwnd);
void BrowseFile(HWND hwndEdit, const wchar_t* filter);
void UpdateButtonState(bool isRunning); 