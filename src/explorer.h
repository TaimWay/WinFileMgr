#pragma once

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <vector>
#include <string>

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")

#define FULL_PATH 32768
#include "resource.h"

class Explorer {
public:
    Explorer();
    ~Explorer();

    bool Create(HINSTANCE hInstance, int nCmdShow);

private:
    // 窗口和控件句柄
    HWND m_hwnd;
    HWND m_hListView;
    HWND m_hAddress;
    HWND m_hEdit;
    HWND m_hUp, m_hRefresh, m_hSuper;
    HWND m_hYes, m_hNo;
    HWND m_hNDir, m_hNFile, m_hDelete, m_hCopy, m_hCut, m_hPaste, m_hRename;
    HFONT m_hFont;
    HINSTANCE m_hInst;

    // 状态变量
    char m_operation;               // 当前操作类型
    char m_super;                   // 超级路径模式
    wchar_t m_browsingPath[FULL_PATH]; // 当前浏览路径

    // 剪切板相关
    std::vector<std::wstring> m_items;
    wchar_t m_fromPath[FULL_PATH];
    BOOL m_rm;                       // 是否为剪切操作

    // 按钮尺寸常量
    static const int buttonHeight = 32;
    static const int buttonWidth = 32;
    static const int buttonSpacing = 8;

    // 静态窗口过程转发器
    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // 辅助功能函数
    int ErrorMsg(const wchar_t* info, UINT uType);
    BOOL CopyTextToClipboard(const std::wstring& strText);
    void BrowseTo(const wchar_t* path);
    void OnListViewDblClick();
    BOOL Delete(wchar_t* path);
    BOOL ManualCopy(const wchar_t* from, const wchar_t* to, bool overwrite);
    BOOL Copy(wchar_t* from, wchar_t* to);
};