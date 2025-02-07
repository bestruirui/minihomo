#include "../inc/window.h"
#include "../inc/controls.h"
#include "../inc/process.h"
#include "../inc/utils.h"
#include "../inc/resource.h"
#include <shellapi.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_CREATE:
        {
            // 初始化托盘图标
            nid.cbSize = sizeof(NOTIFYICONDATAW);
            nid.hWnd = hwnd;
            nid.uID = 1;
            nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
            nid.uCallbackMessage = WM_USER + 1;
            nid.hIcon = LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_ICON1));
            wcscpy_s(nid.szTip, L"Mihomo Control");
            Shell_NotifyIconW(NIM_ADD, &nid);

            CreateControls(hwnd);
            LoadPaths();
            UpdateButtonState(false);

            // 检查并更新自启按钮文本
            if (IsAutoStartEnabled()) {
                SetWindowTextW(hBtnAutostart, L"取消开机自启");
            }
            
            // 注册热键
            if (!RegisterHotKey(hwnd, ID_HOTKEY_TOGGLE, MOD_CONTROL | MOD_ALT, 'M')) {
                //AddLog(L"注册热键失败");
                MessageBoxW(hwnd, L"注册热键失败", L"错误", MB_OK | MB_ICONERROR);
            } else {
                //AddLog(L"已注册热键: Ctrl + Alt + M");
            }
            return 0;
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case 1001: // Browse Mihomo
                    BrowseFile(hEditMihomo, L"可执行文件\0*.exe\0所有文件\0*.*\0");
                    break;
                case 1002: // Browse Config
                    BrowseFile(hEditConfig, L"YAML文件\0*.yaml;*.yml\0所有文件\0*.*\0");
                    break;
                case 1003: // Start
                case ID_TRAY_START:
                    StartMihomo();
                    break;
                case 1004: // Stop
                case ID_TRAY_STOP:
                    StopMihomo();
                    break;
                case ID_TRAY_EXIT:
                    DestroyWindow(hwnd);
                    break;
                case ID_AUTOSTART:
                    ToggleAutoStart(hwnd);
                    break;
            }
            return 0;
        }

        case WM_DESTROY:
        {
            // 删除托盘图标
            Shell_NotifyIconW(NIM_DELETE, &nid);
            if (hProcess != NULL) {
                TerminateProcess(hProcess, 0);
                CloseHandle(hProcess);
            }
            if (hTrayMenu) {
                DestroyMenu(hTrayMenu);
            }
            PostQuitMessage(0);

            // 在窗口销毁前注销热键
            UnregisterHotKey(hwnd, ID_HOTKEY_TOGGLE);
            
            return 0;
        }

        case WM_SIZE:
        {
            // 当窗口最小化时，隐藏窗口
            if (wParam == SIZE_MINIMIZED) {
                ShowWindow(hwnd, SW_HIDE);
            }
            return 0;
        }

        case WM_INITMENUPOPUP:
        {
            // 动态更新菜单项状态
            bool isRunning = (hProcess != NULL);
            EnableMenuItem((HMENU)wParam, ID_TRAY_START, isRunning ? MF_GRAYED : MF_ENABLED);
            EnableMenuItem((HMENU)wParam, ID_TRAY_STOP, isRunning ? MF_ENABLED : MF_GRAYED);
            return 0;
        }

        case WM_HOTKEY:
        {
            if (wParam == ID_HOTKEY_TOGGLE) {
                // 如果进程正在运行，则停止；否则启动
                if (hProcess != NULL) {
                    StopMihomo();
                } else {
                    StartMihomo();
                }
            }
            return 0;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetBkMode(hdcStatic, TRANSPARENT);  // 设置背景透明
            return (LRESULT)GetStockObject(NULL_BRUSH);  // 返回空画刷
        }
    }

    // 修改托盘图标的消息处理
    if (uMsg == WM_USER + 1) {
        switch (LOWORD(lParam)) {
            case WM_LBUTTONDBLCLK:
                ShowWindow(hwnd, SW_RESTORE);
                SetForegroundWindow(hwnd);
                break;
            case WM_RBUTTONUP:
                 ShowTrayMenu(hwnd);
                break;
        }
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void ShowTrayMenu(HWND hwnd)
{
    POINT pt;
    GetCursorPos(&pt);

    // 如果菜单已存在，先销毁
    if (hTrayMenu) {
        DestroyMenu(hTrayMenu);
    }

    // 创建新菜单
    hTrayMenu = CreatePopupMenu();
    bool isRunning = (hProcess != NULL);

    AppendMenuW(hTrayMenu, MF_STRING | (isRunning ? MF_GRAYED : 0), ID_TRAY_START, L"运行 Mihomo");
    AppendMenuW(hTrayMenu, MF_STRING | (isRunning ? 0 : MF_GRAYED), ID_TRAY_STOP, L"停止 Mihomo");
    AppendMenuW(hTrayMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hTrayMenu, MF_STRING, ID_TRAY_EXIT, L"退出");

    // 确保窗口在前台
    SetForegroundWindow(hwnd);

    // 显示菜单
    TrackPopupMenu(hTrayMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON,
        pt.x, pt.y, 0, hwnd, NULL);

    PostMessage(hwnd, WM_NULL, 0, 0);  // 处理菜单消失后的窗口激活问题
}

void ShowBalloonTip(const wchar_t* title, const wchar_t* message)
{
    // 保存原始标志和提示文本
    UINT originalFlags = nid.uFlags;
    wchar_t originalTip[128];
    wcscpy_s(originalTip, nid.szTip);

    // 设置气泡通知
    nid.uFlags = NIF_INFO;  // 只使用 NIF_INFO 标志
    wcscpy_s(nid.szInfoTitle, title);
    wcscpy_s(nid.szInfo, message);
    nid.dwInfoFlags = NIIF_USER;  // 使用基本的 USER 图标
    nid.hIcon = LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_ICON1));

    // 发送通知
    Shell_NotifyIconW(NIM_MODIFY, &nid);

    // 恢复原始状态
    nid.uFlags = originalFlags;
    wcscpy_s(nid.szTip, originalTip);
    Shell_NotifyIconW(NIM_MODIFY, &nid);
} 