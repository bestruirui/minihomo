#include "../inc/utils.h"
#include "../inc/controls.h"
#include "../inc/window.h"
#include <Shlwapi.h>

void SavePaths()
{
    wchar_t mihomo_path[MAX_PATH], config_path[MAX_PATH];
    GetWindowTextW(hEditMihomo, mihomo_path, MAX_PATH);
    GetWindowTextW(hEditConfig, config_path, MAX_PATH);

    // 获取程序当前目录
    wchar_t exe_path[MAX_PATH];
    GetModuleFileNameW(NULL, exe_path, MAX_PATH);
    PathRemoveFileSpecW(exe_path);

    // 构建配置文件完整路径
    wchar_t config_file_path[MAX_PATH];
    PathCombineW(config_file_path, exe_path, CONFIG_FILE);

    // 保存路径
    WritePrivateProfileStringW(L"Paths", L"MihomoPath", mihomo_path, config_file_path);
    WritePrivateProfileStringW(L"Paths", L"ConfigPath", config_path, config_file_path);

    //AddLog(L"已保存路径配置");
}

void LoadPaths()
{
    // 获取程序当前目录
    wchar_t exe_path[MAX_PATH];
    GetModuleFileNameW(NULL, exe_path, MAX_PATH);
    PathRemoveFileSpecW(exe_path);

    // 构建配置文件完整路径
    wchar_t config_file_path[MAX_PATH];
    PathCombineW(config_file_path, exe_path, CONFIG_FILE);

    // 读取路径
    wchar_t mihomo_path[MAX_PATH] = L"";
    wchar_t config_path[MAX_PATH] = L"";
    GetPrivateProfileStringW(L"Paths", L"MihomoPath", L"", mihomo_path, MAX_PATH, config_file_path);
    GetPrivateProfileStringW(L"Paths", L"ConfigPath", L"", config_path, MAX_PATH, config_file_path);

    // 设置到输入框
    if (wcslen(mihomo_path) > 0)
    {
        SetWindowTextW(hEditMihomo, mihomo_path);
        //AddLog(L"已加载 Mihomo 路径: %s", mihomo_path);
    }
    if (wcslen(config_path) > 0)
    {
        SetWindowTextW(hEditConfig, config_path);
        //AddLog(L"已加载配置文件路径: %s", config_path);
    }
}

bool IsAutoStartEnabled()
{
    HKEY hKey;
    bool exists = false;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 
        0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        wchar_t path[MAX_PATH];
        DWORD size = sizeof(path);
        if (RegQueryValueExW(hKey, L"MihomoControl", NULL, NULL, (LPBYTE)path, &size) == ERROR_SUCCESS)
        {
            exists = true;
        }
        RegCloseKey(hKey);
    }
    return exists;
}

void ToggleAutoStart(HWND hwnd)
{
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 
        0, KEY_SET_VALUE | KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
    {
        if (IsAutoStartEnabled())
        {
            // 取消自启
            if (RegDeleteValueW(hKey, L"MihomoControl") == ERROR_SUCCESS)
            {
                //AddLog(L"已取消开机自启");
                ShowBalloonTip(L"Mihomo", L"代理服务已成功取消开机自启");
                SetWindowTextW(hBtnAutostart, L"设置开机自启");
            }
            else
            {
                //AddLog(L"取消开机自启失败");
                MessageBoxW(hwnd, L"错误", L"操作失败", MB_OK | MB_ICONERROR);
            }
        }
        else
        {
            // 设置自启
            if (RegSetValueExW(hKey, L"MihomoControl", 0, REG_SZ, 
                (BYTE*)exePath, (wcslen(exePath) + 1) * sizeof(wchar_t)) == ERROR_SUCCESS)
            {
                //AddLog(L"已设置开机自启");
                ShowBalloonTip(L"Mihomo", L"代理服务已成功设置开机自启");
                SetWindowTextW(hBtnAutostart, L"取消开机自启");
            }
            else
            {
                //AddLog(L"设置开机自启失败");
                MessageBoxW(hwnd, L"错误", L"操作失败", MB_OK | MB_ICONERROR);
            }
        }
        RegCloseKey(hKey);
    }
    else
    {
        //AddLog(L"访问注册表失败");
        MessageBoxW(hwnd, L"访问注册表失败", L"错误", MB_OK | MB_ICONERROR);
    }
} 