#include "../inc/process.h"
#include "../inc/controls.h"
#include "../inc/window.h"
#include <TlHelp32.h>
#include <Shlwapi.h>
#include <shellapi.h>

// 进程相关变量定义
HANDLE hProcess = NULL;
DWORD g_ProcessId = 0;

void StartMihomo()
{
    if (hProcess != NULL)
    {
        ShowBalloonTip(L"Mihomo", L"Mihomo 已在运行中");
        MessageBoxW(hwnd, L"Mihomo 已在运行中", L"提示", MB_OK | MB_ICONINFORMATION);
        return;
    }

    wchar_t mihomo_path[MAX_PATH], config_path[MAX_PATH];
    GetWindowTextW(hEditMihomo, mihomo_path, MAX_PATH);
    GetWindowTextW(hEditConfig, config_path, MAX_PATH);

    if (wcslen(mihomo_path) == 0 || wcslen(config_path) == 0)
    {
        MessageBoxW(hwnd, L"请选择 Mihomo 和配置文件路径", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    // 获取配置文件所在目录
    wchar_t config_dir[MAX_PATH];
    wcscpy_s(config_dir, config_path);
    PathRemoveFileSpecW(config_dir);

    if (!IsRunAsAdmin())
    {
        SHELLEXECUTEINFOW sei = {0};
        sei.cbSize = sizeof(sei);
        sei.lpVerb = L"runas";
        sei.lpFile = mihomo_path;
        std::wstring params = L"-d \"" + std::wstring(config_dir) + L"\" -f \"" + config_path + L"\"";
        sei.lpParameters = params.c_str();
        sei.hwnd = hwnd;
        sei.nShow = SW_HIDE;
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;

        if (ShellExecuteExW(&sei))
        {
            hProcess = sei.hProcess;
            g_ProcessId = GetProcessId(sei.hProcess);
            
            // 发送通知
            ShowBalloonTip(L"Mihomo", L"代理服务已成功启动");
            
            UpdateButtonState(true);
        }
        else
        {
            DWORD error = GetLastError();
            wchar_t errorMsg[256];
            swprintf_s(errorMsg, L"启动失败，错误代码: %d", error);
            MessageBoxW(hwnd, errorMsg, L"错误", MB_OK | MB_ICONERROR);
        }
        return;
    }

    std::wstring cmd = std::wstring(L"\"") + mihomo_path + L"\" -d \"" + config_dir + L"\" -f \"" + config_path + L"\"";

    STARTUPINFOW si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    if (CreateProcessW(
        NULL,
        (LPWSTR)cmd.c_str(),
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW,
        NULL,
        config_dir,
        &si,
        &pi
    ))
    {
        hProcess = pi.hProcess;
        CloseHandle(pi.hThread);
        
        // 发送通知
        ShowBalloonTip(L"Mihomo", L"代理服务已成功启动");
        
        UpdateButtonState(true);
    }
    else
    {
        DWORD error = GetLastError();
        wchar_t errorMsg[256];
        swprintf_s(errorMsg, L"启动失败，错误代码: %d", error);
        MessageBoxW(hwnd, errorMsg, L"错误", MB_OK | MB_ICONERROR);
    }
}

void StopMihomo()
{
    bool stopped = false;

    // 首先尝试使用进程句柄终止
    if (hProcess != NULL)
    {
        if (TerminateProcess(hProcess, 0))
        {
            stopped = true;
        }
        CloseHandle(hProcess);
        hProcess = NULL;
    }

    // 如果没有成功终止，则查找并终止所有 mihomo.exe 进程
    if (!stopped)
    {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot != INVALID_HANDLE_VALUE)
        {
            PROCESSENTRY32W pe32 = { sizeof(PROCESSENTRY32W) };
            if (Process32FirstW(snapshot, &pe32))
            {
                do
                {
                    if (_wcsicmp(pe32.szExeFile, L"mihomo.exe") == 0)
                    {
                        HANDLE tempProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
                        if (tempProcess != NULL)
                        {
                            if (TerminateProcess(tempProcess, 0))
                            {
                                stopped = true;
                            }
                            CloseHandle(tempProcess);
                        }
                    }
                } while (Process32NextW(snapshot, &pe32));
            }
            CloseHandle(snapshot);
        }
    }

    if (stopped)
    {
        ShowBalloonTip(L"Mihomo", L"代理服务已成功停止");
        UpdateButtonState(false);
        g_ProcessId = 0;
    }
    else
    {
        UpdateButtonState(false);
    }
}

bool IsRunAsAdmin()
{
    BOOL isAdmin = FALSE;
    PSID adminGroup;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup))
    {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }

    return isAdmin != FALSE;
}

bool RunAsAdmin(HWND hwnd, const std::wstring& cmd)
{
    SHELLEXECUTEINFOW sei = {0};
    sei.cbSize = sizeof(sei);
    sei.lpVerb = L"runas";
    sei.lpFile = L"cmd.exe";
    sei.lpParameters = (L"/c " + cmd).c_str();
    sei.hwnd = hwnd;
    sei.nShow = SW_HIDE;

    return ShellExecuteExW(&sei);
} 