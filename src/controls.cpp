#include "../inc/controls.h"
#include "../inc/window.h"
#include "../inc/utils.h"
#include <commdlg.h>
#include <stdarg.h>

// 控件句柄定义
HWND hEditMihomo = NULL;
HWND hEditConfig = NULL;
HWND hBtnBrowseMihomo = NULL;
HWND hBtnBrowseConfig = NULL;
HWND hBtnStart = NULL;
HWND hBtnStop = NULL;
HWND hBtnAutostart = NULL;
HWND hEditLog = NULL;

void CreateControls(HWND hwnd)
{
    // 创建现代字体
    HFONT hModernFont = CreateFontW(
        -14,                    // 更大的字体高度
        0, 0, 0,
        FW_NORMAL,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,      
        DEFAULT_PITCH | FF_DONTCARE,
        L"Microsoft YaHei UI"   
    );

    // 创建标签和输入框
    HWND hStaticMihomo = CreateWindowW(L"STATIC", L"Mihomo路径:", 
        WS_VISIBLE | WS_CHILD | SS_LEFTNOWORDWRAP,
        20, 20, 90, 24, hwnd, NULL, NULL, NULL);
    
    hEditMihomo = CreateWindowW(L"EDIT", L"", 
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        120, 20, 280, 24, hwnd, NULL, NULL, NULL);
    
    hBtnBrowseMihomo = CreateWindowW(L"BUTTON", L"浏览", 
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        410, 20, 70, 24, hwnd, (HMENU)1001, NULL, NULL);

    HWND hStaticConfig = CreateWindowW(L"STATIC", L"配置文件路径:", 
        WS_VISIBLE | WS_CHILD | SS_LEFTNOWORDWRAP,  
        20, 54, 90, 24, hwnd, NULL, NULL, NULL);
    
    hEditConfig = CreateWindowW(L"EDIT", L"", 
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        120, 54, 280, 24, hwnd, NULL, NULL, NULL);
    
    hBtnBrowseConfig = CreateWindowW(L"BUTTON", L"浏览", 
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        410, 54, 70, 24, hwnd, (HMENU)1002, NULL, NULL);

    // 操作按钮
    hBtnStart = CreateWindowW(L"BUTTON", L"启动", 
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        120, 88, 110, 32, hwnd, (HMENU)1003, NULL, NULL);
    
    hBtnStop = CreateWindowW(L"BUTTON", L"停止", 
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        240, 88, 110, 32, hwnd, (HMENU)1004, NULL, NULL);

    hBtnAutostart = CreateWindowW(L"BUTTON", L"设置开机自启", 
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        360, 88, 110, 32, hwnd, (HMENU)ID_AUTOSTART, NULL, NULL);

    // 为所有控件设置现代字体
    HWND controls[] = {
        hStaticMihomo, hEditMihomo, hBtnBrowseMihomo,
        hStaticConfig, hEditConfig, hBtnBrowseConfig,
        hBtnStart, hBtnStop, hBtnAutostart
    };
    
    for(HWND ctrl : controls) {
        SendMessageW(ctrl, WM_SETFONT, (WPARAM)hModernFont, TRUE);
    }
}

void BrowseFile(HWND hwndEdit, const wchar_t* filter)
{
    wchar_t filename[MAX_PATH] = L"";
    OPENFILENAMEW ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetParent(hwndEdit);
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    if (GetOpenFileNameW(&ofn))
    {
        SetWindowTextW(hwndEdit, filename);
        SavePaths();
    }
}

void AddLog(const wchar_t* format, ...)
{
    va_list args;
    va_start(args, format);
    
    wchar_t buffer[1024];
    vswprintf_s(buffer, format, args);
    
    va_end(args);

    // 获取当前文本长度
    int len = GetWindowTextLengthW(hEditLog);
    
    // 移动到末尾
    SendMessageW(hEditLog, EM_SETSEL, len, len);
    
    // 添加时间戳
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t timeStr[32];
    swprintf_s(timeStr, L"[%02d:%02d:%02d] ", st.wHour, st.wMinute, st.wSecond);
    
    // 插入新文本
    SendMessageW(hEditLog, EM_REPLACESEL, FALSE, (LPARAM)timeStr);
    SendMessageW(hEditLog, EM_REPLACESEL, FALSE, (LPARAM)buffer);
    SendMessageW(hEditLog, EM_REPLACESEL, FALSE, (LPARAM)L"\r\n");
    
    // 滚动到底部
    SendMessageW(hEditLog, EM_SCROLLCARET, 0, 0);
}

void UpdateButtonState(bool isRunning)
{
    // 设置按钮状态
    EnableWindow(hBtnStart, !isRunning);
    SendMessage(hBtnStart, WM_SETTEXT, 0, (LPARAM)(isRunning ? L"运行中" : L"启动"));
    InvalidateRect(hBtnStart, NULL, TRUE);

    EnableWindow(hBtnStop, isRunning);
    InvalidateRect(hBtnStop, NULL, TRUE);

    // 更新托盘图标提示文本
    wcscpy_s(nid.szTip, isRunning ? L"Mihomo 运行中" : L"Mihomo 已停止");
    Shell_NotifyIconW(NIM_MODIFY, &nid);
} 