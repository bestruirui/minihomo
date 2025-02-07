#pragma once
#include <windows.h>
#include <string>

// 进程相关变量声明
extern HANDLE hProcess;
extern DWORD g_ProcessId;

// 进程相关函数声明
void StartMihomo();
void StopMihomo();
bool IsRunAsAdmin();
bool RunAsAdmin(HWND hwnd, const std::wstring& cmd); 