#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <Shlwapi.h>
#include "../inc/window.h"
#include "../inc/controls.h"
#include "../inc/process.h"
#include "../inc/utils.h"
#include "../inc/resource.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "Shlwapi.lib")

// 全局变量定义
const wchar_t* CLASS_NAME = L"TrayAppWindow";
const wchar_t* WINDOW_TITLE = L"Mihomo Control";
const wchar_t* CONFIG_FILE = L"config.ini";

HWND hwnd = NULL;
NOTIFYICONDATAW nid = {0};
HMENU hTrayMenu = NULL;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    InitCommonControls();

    // 注册窗口类
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_ICON1));
    RegisterClassW(&wc);

    // 创建窗口
    hwnd = CreateWindowExW(
        0, CLASS_NAME, WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 160,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    // 检查配置文件并自动启动
    wchar_t mihomo_path[MAX_PATH] = L"";
    wchar_t config_path[MAX_PATH] = L"";
    
    // 获取程序当前目录
    wchar_t exe_path[MAX_PATH];
    GetModuleFileNameW(NULL, exe_path, MAX_PATH);
    PathRemoveFileSpecW(exe_path);

    // 构建配置文件完整路径
    wchar_t config_file_path[MAX_PATH];
    PathCombineW(config_file_path, exe_path, CONFIG_FILE);

    // 读取配置
    GetPrivateProfileStringW(L"Paths", L"MihomoPath", L"", mihomo_path, MAX_PATH, config_file_path);
    GetPrivateProfileStringW(L"Paths", L"ConfigPath", L"", config_path, MAX_PATH, config_file_path);

    // 如果两个路径都存在，自动启动但不显示窗口
    if (wcslen(mihomo_path) > 0 && wcslen(config_path) > 0) {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        StartMihomo();
    } else {
        ShowWindow(hwnd, nCmdShow);
    }

    // 消息循环
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
} 