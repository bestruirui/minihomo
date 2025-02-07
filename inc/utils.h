#pragma once
#include <windows.h>

// 配置文件常量
extern const wchar_t* CONFIG_FILE;

// 工具函数声明
void SavePaths();
void LoadPaths();
bool IsAutoStartEnabled();
void ToggleAutoStart(HWND hwnd); 