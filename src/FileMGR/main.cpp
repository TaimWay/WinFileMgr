#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <stdio.h>
#include <vector>
#include <string>
#include "res.h"

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")

#define FULL_PATH 32768

const int buttonHeight = 32;
const int buttonWidth = 32;
const int buttonSpacing = 8;

HWND hwnd;
HWND hListView;
HWND hAddress;
HWND hEdit;
HWND hUp, hRefresh;
HWND hYes, hNo;
HWND hNDir, hNFile, hDelete, hCopy, hCut, hPaste, hRename;
HFONT hFont;

char operation = 0;

wchar_t browsingPath[FULL_PATH];

int ErrorMsg(const wchar_t* info, const UINT uType) {
    wchar_t msg[FULL_PATH];
    DWORD err = GetLastError();
    swprintf_s(msg, FULL_PATH, L"%s\n[0x%08X]", info, err);
    size_t len = wcslen(msg);
    FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        msg + len,
        FULL_PATH - len,
        NULL
    );
    return MessageBoxW(hwnd, msg, L"Error", uType);
}

BOOL CopyTextToClipboard(const std::wstring& strText) {
    if (strText.empty()) return FALSE;

    // 1. 打开剪切板
    // 如果另一个进程已经打开剪切板，OpenClipboard 会失败
    if (!OpenClipboard(NULL)) {
        ErrorMsg(L"OpenClipboard failed", MB_ICONERROR | MB_OK);
        return FALSE;
    }

    // 2. 清空剪切板
    // 这会释放当前剪切板中的数据，并将剪切板所有权分配给当前窗口
    if (!EmptyClipboard()) {
        CloseClipboard();
        return FALSE;
    }

    // 3. 分配全局内存
    // 计算需要的字节大小：(字符长度 + 1 个空字符) * sizeof(WCHAR)
    // 使用 GMEM_MOVEABLE 分配可移动内存，这是剪切板要求的
    SIZE_T cbSize = (strText.length() + 1) * sizeof(WCHAR);
    HGLOBAL hClipboardData = GlobalAlloc(GMEM_MOVEABLE, cbSize);

    if (hClipboardData == NULL) {
        CloseClipboard();
        return FALSE;
    }

    // 4. 锁定内存并复制数据
    // GlobalLock 返回指向内存块的指针
    wchar_t* pchData = (wchar_t*)GlobalLock(hClipboardData);
    if (pchData != NULL) {
        // 将字符串复制到全局内存中
        wcscpy_s(pchData, strText.length() + 1, strText.c_str());
        // 解锁内存
        GlobalUnlock(hClipboardData);
    } else {
        // 锁定失败，释放内存并退出
        GlobalFree(hClipboardData);
        CloseClipboard();
        return FALSE;
    }

    // 5. 设置剪切板数据
    // CF_UNICODETEXT 表示数据格式是 Unicode 文本
    // 此时 hClipboardData 的所有权移交给了系统，不要在这里调用 GlobalFree
    if (SetClipboardData(CF_UNICODETEXT, hClipboardData) == NULL) {
        // 如果设置失败，我们需要手动释放内存（因为系统没接管成功）
        GlobalFree(hClipboardData);
        CloseClipboard();
        return FALSE;
    }

    // 6. 关闭剪切板
    CloseClipboard();

    return TRUE;
}

void BrowseTo(const wchar_t* path) {
    if (!*path || wcscmp(path, L"\\??\\") == 0 || wcscmp(path, L"\\\\?\\") == 0 || wcscmp(path, L"\\\\.\\") == 0) {
        wchar_t buffer[128];
        DWORD dwResult = GetLogicalDriveStringsW(128, buffer);
        if (dwResult == 0) {
            ErrorMsg(L"GetLogicalDriveStrings", MB_ICONERROR | MB_OK);
            return;
        }
        wcscpy_s(browsingPath, FULL_PATH, path);
        SetWindowTextW(hAddress, path);
        ListView_DeleteAllItems(hListView);
        LVITEMW lvi = { 0 };
        for (wchar_t* p = buffer; *p; p += wcslen(p) + 1) {
            SHFILEINFOW sfi = { 0 };
            SHGetFileInfoW(p, FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi),
                SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

            lvi.mask = LVIF_TEXT | LVIF_IMAGE;
            lvi.iItem = ListView_GetItemCount(hListView);
            lvi.pszText = p;
            lvi.iImage = sfi.iIcon;
            ListView_InsertItem(hListView, &lvi);
        }
        if (!*path) {
            lvi.mask = LVIF_TEXT;

            lvi.iItem = ListView_GetItemCount(hListView);
            wchar_t itemName1[] = L"\\??\\";
            lvi.pszText = itemName1;
            ListView_InsertItem(hListView, &lvi);

            lvi.iItem = ListView_GetItemCount(hListView);
            wchar_t itemName2[] = L"\\\\.\\";
            lvi.pszText = itemName2;
            ListView_InsertItem(hListView, &lvi);
        
            lvi.iItem = ListView_GetItemCount(hListView);
            wchar_t itemName3[] = L"\\\\?\\";
            lvi.pszText = itemName3;
            ListView_InsertItem(hListView, &lvi);
        }
        return;
    }
    wchar_t locPath[FULL_PATH];
    DWORD expandLen = ExpandEnvironmentStringsW(path, locPath, FULL_PATH);
    if (expandLen == 0 || expandLen > FULL_PATH) {
        ErrorMsg(path, MB_ICONERROR | MB_OK);
        return;
    }
    DWORD attr = GetFileAttributesW(locPath);

    size_t len = wcslen(locPath);
    if (locPath[len-1] != L'\\')
        locPath[len++] = L'\\';
    locPath[len] = L'*';
    locPath[len+1] = L'\0';

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(locPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        locPath[--len] = L'\0';

        if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY))
        if (ShellExecuteW(NULL, L"open", locPath, NULL, NULL, SW_SHOWNORMAL) > (HINSTANCE)32) {
            SetWindowTextW(hAddress, browsingPath);
            return;
        }
        if (locPath[--len] != L':') {
            ErrorMsg(locPath, MB_ICONERROR | MB_OK);
            SetWindowTextW(hAddress, browsingPath);
            return;
        }
        locPath[len] = L'\0';
        WIN32_FIND_STREAM_DATA findStreamData;
        HANDLE hFind = FindFirstStreamW(locPath, FindStreamInfoStandard, &findStreamData, 0);
        if (hFind == INVALID_HANDLE_VALUE) {
            if (GetLastError() == 0x26) {
                locPath[len] = L':';
                wcscpy_s(browsingPath, FULL_PATH, locPath);
                SetWindowTextW(hAddress, locPath);

                ListView_DeleteAllItems(hListView);
                return;
            }
            ErrorMsg(locPath, MB_ICONERROR | MB_OK);
            SetWindowTextW(hAddress, browsingPath);
            return;
        }
        locPath[len] = L':';
        wcscpy_s(browsingPath, FULL_PATH, locPath);
        SetWindowTextW(hAddress, locPath);

        ListView_DeleteAllItems(hListView);
        do {
            if (findStreamData.cStreamName[1] != L':') {
                wchar_t* p = wcschr(findStreamData.cStreamName + 1, L':');
                if (p != NULL) *p = L'\0';
            }
            LVITEMW lvi = { 0 };
            lvi.mask = LVIF_TEXT | LVIF_IMAGE;
            lvi.iItem = ListView_GetItemCount(hListView);
            lvi.pszText = findStreamData.cStreamName+1;

            // 设置图标
            SHFILEINFOW sfi = { 0 };
            SHGetFileInfoW(findStreamData.cStreamName + 1, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi),
                SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
            lvi.iImage = sfi.iIcon;
            ListView_InsertItem(hListView, &lvi);

            // 添加流大小
            wchar_t size[32];
            swprintf_s(size, 32, L"%lld KB", findStreamData.StreamSize.QuadPart / 1024);
            ListView_SetItemText(hListView, lvi.iItem, 1, size);
        } while (FindNextStreamW(hFind, &findStreamData));
        FindClose(hFind);
        return;
    }
    locPath[len] = L'\0';

    wcscpy_s(browsingPath, FULL_PATH, locPath);
    SetWindowTextW(hAddress, locPath);
    SetCurrentDirectoryW(locPath);

    ListView_DeleteAllItems(hListView);
    do {
        if (wcscmp(findData.cFileName, L".")==0 || wcscmp(findData.cFileName, L"..")==0) continue;

        LVITEMW lvi = { 0 };
        lvi.mask = LVIF_TEXT | LVIF_IMAGE;
        lvi.iItem = ListView_GetItemCount(hListView);
        lvi.pszText = findData.cFileName;

        // 设置图标
        SHFILEINFOW sfi = { 0 };
        SHGetFileInfoW(findData.cFileName, findData.dwFileAttributes, &sfi, sizeof(sfi),
            SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
        lvi.iImage = sfi.iIcon;
        ListView_InsertItem(hListView, &lvi);

        // 添加文件大小
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            wchar_t size[32];
            swprintf_s(size, 32, L"%ld KB", findData.nFileSizeLow / 1024);
            ListView_SetItemText(hListView, lvi.iItem, 1, size);
        }

        // 添加修改日期
        SYSTEMTIME st;
        FileTimeToSystemTime(&findData.ftLastWriteTime, &st);
        wchar_t date[32];
        swprintf_s(date, 32, L"%04d/%02d/%02d", st.wYear, st.wMonth, st.wDay);
        ListView_SetItemText(hListView, lvi.iItem, 2, date);
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
}

void OnListViewDblClick() {
    int index = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
    if (index == -1) return;

    wchar_t path[FULL_PATH];
    wcscpy_s(path, FULL_PATH, browsingPath);
    size_t len = wcslen(path);
    ListView_GetItemText(hListView, index, 0, path+len, FULL_PATH-len);
    BrowseTo(path);
}

BOOL Delete(wchar_t *path) {
    wcscat_s(path, FULL_PATH, L"\\*");
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(path, &findData);
    size_t len = wcslen(path);
    if (hFind == INVALID_HANDLE_VALUE) {
        path[len-2] = L'\0';
        while (!DeleteFileW(path)) {
            int c = ErrorMsg(path, MB_ICONWARNING | MB_CANCELTRYCONTINUE);
            if (c == IDCANCEL) return FALSE;
            if (c == IDCONTINUE) return TRUE;
        } return TRUE;
    }
    path[--len] = L'\0';
    do if (wcscmp(findData.cFileName, L".") && wcscmp(findData.cFileName, L"..")) {
        wcscpy_s(path + len, FULL_PATH - len, findData.cFileName);
        if (!Delete(path)) {
            FindClose(hFind);
            return FALSE;
        }
    } while (FindNextFileW(hFind, &findData));
    FindClose(hFind);
    path[len] = L'\0';
    while (!RemoveDirectoryW(path)) {
        int c = ErrorMsg(path, MB_ICONWARNING | MB_CANCELTRYCONTINUE);
        if (c == IDCANCEL) return FALSE;
        if (c == IDCONTINUE) return TRUE;
    } return TRUE;
}

BOOL ManualCopy(const wchar_t* from, const wchar_t* to, bool overwrite) {
#define BUFFER_SIZE 8192

    char buffer[BUFFER_SIZE];
    DWORD bytesRead, bytesWritten;
    DWORD error;
    HANDLE hfrom = CreateFileW(from, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hfrom == INVALID_HANDLE_VALUE) return FALSE;

    HANDLE hto = CreateFileW(to, GENERIC_WRITE, 0, NULL, overwrite ? CREATE_ALWAYS : CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hto == INVALID_HANDLE_VALUE) {
        error = GetLastError();
        CloseHandle(hfrom);
        SetLastError(error);
        return FALSE;
    }

    while (TRUE) {
        if (!ReadFile(hfrom, buffer, BUFFER_SIZE, &bytesRead, NULL)) goto L1;
        if (bytesRead == 0) break;
        if (!WriteFile(hto, buffer, bytesRead, &bytesWritten, NULL)) goto L1;
        if (bytesRead != bytesWritten) goto L1;
    }

    CloseHandle(hto);
    CloseHandle(hfrom);
    return TRUE;

L1: error = GetLastError();
    CloseHandle(hto);
    CloseHandle(hfrom);
    SetLastError(error);
    return FALSE;
}

BOOL Copy(wchar_t* from, wchar_t* to) {
    wcscat_s(from, FULL_PATH, L"\\*");
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(from, &findData);
    size_t fromLen = wcslen(from);
    size_t toLen = wcslen(to);
    if (hFind == INVALID_HANDLE_VALUE) {
        from[fromLen-=2] = L'\0';
        if (CopyFileW(from, to, TRUE)) return TRUE;
        if (GetLastError() == ERROR_FILE_EXISTS) {
            wcscat_s(to, FULL_PATH, L" -\"重试\"以覆盖文件");
            int c = ErrorMsg(to, MB_ICONWARNING | MB_CANCELTRYCONTINUE);
            to[toLen] = L'\0';
            if (c == IDCANCEL) return FALSE;
            if (c == IDCONTINUE) return TRUE;
            if (CopyFileW(from, to, FALSE)) return TRUE;
        }
        while (TRUE) {
            wcscat_s(from, FULL_PATH, L" -\"重试\"以忽略文件属性");
            int c = ErrorMsg(from, MB_ICONWARNING | MB_CANCELTRYCONTINUE);
            from[fromLen] = L'\0';
            if (c == IDCANCEL) return FALSE;
            if (c == IDCONTINUE) return TRUE;
            if (ManualCopy(from, to, FALSE)) return TRUE;
            if (GetLastError() == ERROR_FILE_EXISTS) while (TRUE) {
                wcscat_s(to, FULL_PATH, L" -\"重试\"以覆盖文件并忽略文件属性");
                int c = ErrorMsg(from, MB_ICONWARNING | MB_CANCELTRYCONTINUE);
                to[toLen] = L'\0';
                if (c == IDCANCEL) return FALSE;
                if (c == IDCONTINUE) return TRUE;
                if (ManualCopy(from, to, TRUE)) return TRUE;
            }
        }
    }
    from[--fromLen] = L'\0';
    wcscat_s(to, FULL_PATH, L"\\");
    toLen++;
    while (!CreateDirectoryW(to, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        int c = ErrorMsg(to, MB_ICONWARNING | MB_CANCELTRYCONTINUE);
        if (c == IDCANCEL) return FALSE;
        if (c == IDCONTINUE) return TRUE;
    }
    do if (wcscmp(findData.cFileName, L".") && wcscmp(findData.cFileName, L"..")) {
        wcscpy_s(from + fromLen, FULL_PATH - fromLen, findData.cFileName);
        wcscpy_s(to + toLen, FULL_PATH - toLen, findData.cFileName);
        if (!Copy(from, to)) {
            FindClose(hFind);
            return FALSE;
        }
    } while (FindNextFileW(hFind, &findData));
    FindClose(hFind);
    return TRUE;
}

// 窗口过程
std::vector<std::wstring> items;
wchar_t fromPath[FULL_PATH];
BOOL rm;
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    wchar_t path[FULL_PATH];
    wchar_t path2[FULL_PATH];
    if (msg >= WM_CTLCOLORMSGBOX && msg <= WM_CTLCOLORSTATIC) return (LRESULT)GetStockObject(WHITE_BRUSH);
    switch (msg) {
    case WM_CREATE: {
        hAddress = CreateWindowExW(
            WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            0, 0, 0, 0, hwnd, (HMENU)ID_ADDRESS, NULL, NULL);
        SendMessageW(hAddress, WM_SETFONT, (WPARAM)hFont, TRUE);
        hEdit = CreateWindowExW(
            WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | ES_AUTOHSCROLL,
            0, 0, 0, 0, hwnd, (HMENU)ID_EDIT, NULL, NULL);
        SendMessageW(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

        hUp = CreateWindowExW(
            0, L"BUTTON", NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            buttonSpacing * 1, buttonSpacing, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_UP, NULL, NULL);
        hRefresh = CreateWindowExW(
            0, L"BUTTON", NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            buttonSpacing * 2 + buttonWidth * 1, buttonSpacing, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_REFRESH, NULL, NULL);

        hYes = CreateWindowExW(
            0, L"BUTTON", NULL, WS_CHILD | BS_OWNERDRAW,
            0, 0, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_YES, NULL, NULL);
        hNo = CreateWindowExW(
            0, L"BUTTON", NULL, WS_CHILD | BS_OWNERDRAW,
            0, 0, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_NO, NULL, NULL);

        hNDir = CreateWindowExW(
            0, L"BUTTON", NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            buttonSpacing, buttonSpacing * 2 + buttonHeight, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_NDIR, NULL, NULL);
        hNFile = CreateWindowExW(
            0, L"BUTTON", NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            buttonSpacing * 2 + buttonWidth, buttonSpacing * 2 + buttonHeight, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_NFILE, NULL, NULL);
        hDelete = CreateWindowExW(
            0, L"BUTTON", NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            buttonSpacing * 3 + buttonWidth * 2, buttonSpacing * 2 + buttonHeight, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_DELETE, NULL, NULL);
        hRename = CreateWindowExW(
            0, L"BUTTON", NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            buttonSpacing * 4 + buttonWidth * 3, buttonSpacing * 2 + buttonHeight, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_RENAME, NULL, NULL);
        hCopy = CreateWindowExW(
            0, L"BUTTON", NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            buttonSpacing * 5 + buttonWidth * 4, buttonSpacing * 2 + buttonHeight, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_COPY, NULL, NULL);
        hCut = CreateWindowExW(
            0, L"BUTTON", NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            buttonSpacing * 6 + buttonWidth * 5, buttonSpacing * 2 + buttonHeight, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_CUT, NULL, NULL);
        hPaste = CreateWindowExW(
            0, L"BUTTON", NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            buttonSpacing * 7 + buttonWidth * 6, buttonSpacing * 2 + buttonHeight, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_PASTE, NULL, NULL);

        hListView = CreateWindowExW(
            0, WC_LISTVIEWW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS,
            0, 0, 0, 0, hwnd, (HMENU)ID_LISTVIEW, NULL, NULL);

        SHFILEINFOW sfi = { 0 };
        HIMAGELIST hImageList = (HIMAGELIST)SHGetFileInfoW(L"", 0, &sfi, sizeof(sfi), SHGFI_SMALLICON | SHGFI_SYSICONINDEX);
        ListView_SetImageList(hListView, hImageList, LVSIL_SMALL);

        LVCOLUMNW lvc = { 0 };
        lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        lvc.pszText = (LPWSTR)L"名称";
        lvc.cx = 200;
        lvc.iSubItem = 0;
        ListView_InsertColumn(hListView, 0, &lvc);
        lvc.pszText = (LPWSTR)L"大小";
        lvc.cx = 100;
        lvc.iSubItem = 1;
        ListView_InsertColumn(hListView, 1, &lvc);
        lvc.pszText = (LPWSTR)L"修改日期";
        lvc.cx = 120;
        lvc.iSubItem = 2;
        ListView_InsertColumn(hListView, 2, &lvc);

        ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT);

        BrowseTo(L"");

        break;
    }

    case WM_SIZE: {
        RECT rc;
        GetClientRect(hwnd, &rc);

        MoveWindow(hAddress, buttonSpacing * 3 + buttonWidth * 2, buttonSpacing, rc.right - buttonSpacing * 4 - buttonWidth * 2, buttonHeight, TRUE);
        MoveWindow(hEdit, buttonSpacing * 2 + buttonWidth * 1, buttonSpacing * 2 + buttonHeight, rc.right - buttonSpacing * 5 - buttonWidth * 3, buttonHeight, TRUE);
        SetWindowPos(hYes, NULL, rc.right - buttonSpacing - buttonWidth, buttonSpacing * 2 + buttonHeight, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
        SetWindowPos(hNo, NULL, rc.right - buttonSpacing * 2 - buttonWidth * 2, buttonSpacing * 2 + buttonHeight, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
        int listY = buttonSpacing * 3 + buttonHeight * 2;
        MoveWindow(hListView, buttonSpacing, listY, rc.right - buttonSpacing * 2, rc.bottom - listY - buttonSpacing, TRUE);
        break;
    }

    case WM_CONTEXTMENU: {
        POINT pt;
        GetCursorPos(&pt);
        HMENU hMenu = CreatePopupMenu();

        int index;
        if ((index=ListView_GetNextItem(hListView, -1, LVNI_SELECTED)) == -1) {
            AppendMenuW(hMenu, MF_STRING, IDM_REFRESH, L"刷新(&R)");
            AppendMenuW(hMenu, MF_STRING, IDM_CMD, L"CMD(&C)");
            AppendMenuW(hMenu, MF_STRING, IDM_EXPLORER, L"Explorer(&E)");
            AppendMenuW(hMenu, MF_STRING, IDM_CPDIRPATH, L"复制路径(&P)");
        } else if (ListView_GetNextItem(hListView, index, LVNI_SELECTED) == -1) {
            AppendMenuW(hMenu, MF_STRING, IDM_OPEN, L"打开(&O)");
            AppendMenuW(hMenu, MF_STRING, IDM_NOTEPAD, L"记事本(&N)");
            AppendMenuW(hMenu, MF_STRING, IDM_CPFILEPATH, L"复制路径(&P)");
            AppendMenuW(hMenu, MF_STRING, IDM_STREAM, L"数据流(&S)");
        } else {
            DestroyMenu(hMenu);
            break;
        }

        int choice = TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);
        STARTUPINFO si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        if (index == -1) {
            switch (choice) {
            case IDM_REFRESH:
                BrowseTo(browsingPath);
                break;

            case IDM_CMD: {
                wchar_t cmd[] = L"cmd.exe";
                if (CreateProcessW(NULL, cmd, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
                break;
            }
            
            case IDM_EXPLORER:
                wcscpy_s(path, FULL_PATH, L"explorer.exe ");
                wcscat_s(path, FULL_PATH, browsingPath);
                if (CreateProcessW(NULL, path, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
                break;
            
            case IDM_CPDIRPATH:
                CopyTextToClipboard(browsingPath);
                break;
            }
        } else {
            size_t len;
            switch (choice) {
            case IDM_OPEN:
                wcscpy_s(path, FULL_PATH, browsingPath);
                len = wcslen(path);
                ListView_GetItemText(hListView, index, 0, path + len, FULL_PATH - len);
                BrowseTo(path);
                break;

            case IDM_NOTEPAD: 
                wcscpy_s(path, FULL_PATH, L"notepad.exe ");
                wcscat_s(path, FULL_PATH, browsingPath);
                len = wcslen(path);
                ListView_GetItemText(hListView, index, 0, path + len, FULL_PATH - len);
                if (CreateProcessW(NULL, path, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
                break;
            
            case IDM_CPFILEPATH:
                wcscpy_s(path, FULL_PATH, browsingPath);
                len = wcslen(path);
                ListView_GetItemText(hListView, index, 0, path + len, FULL_PATH - len);
                CopyTextToClipboard(path);
                break;

            case IDM_STREAM:
                wcscpy_s(path, FULL_PATH, browsingPath);
                len = wcslen(path);
                ListView_GetItemText(hListView, index, 0, path + len, FULL_PATH - len);
                wcscat_s(path, FULL_PATH, L":");
                BrowseTo(path);
                break;
            }
        }
        DestroyMenu(hMenu);
        break;
    }


    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_UP: {
            if (wcscmp(browsingPath, L"\\??\\")==0 || wcscmp(browsingPath, L"\\\\?\\")==0 || wcscmp(browsingPath, L"\\\\.\\")==0) {
                BrowseTo(L"");
                break;
            }
            wcscpy_s(path, FULL_PATH, browsingPath);
            int p = 0;
            for (int i = 1; i < FULL_PATH; i++) {
                if (path[i-1] == L'\\' && path[i] != L'\0')
                    p = i;
            }
            path[p] = L'\0';
            BrowseTo(path);
            break;
        }

        case ID_REFRESH: {
            GetWindowTextW(hAddress, path, FULL_PATH);
            BrowseTo(path);
            break;
        }

        case ID_NDIR:
            MoveWindow(hNDir, buttonSpacing, buttonSpacing * 2 + buttonHeight, buttonWidth, buttonHeight, TRUE);
            ShowWindow(hNFile, SW_HIDE);
            ShowWindow(hDelete, SW_HIDE);
            ShowWindow(hRename, SW_HIDE);
            ShowWindow(hCopy, SW_HIDE);
            ShowWindow(hCut, SW_HIDE);
            ShowWindow(hPaste, SW_HIDE);
            operation = 1;
            goto L1;
        
        case ID_NFILE:
            MoveWindow(hNFile, buttonSpacing, buttonSpacing * 2 + buttonHeight, buttonWidth, buttonHeight, TRUE);
            ShowWindow(hNDir, SW_HIDE);
            ShowWindow(hDelete, SW_HIDE);
            ShowWindow(hRename, SW_HIDE);
            ShowWindow(hCopy, SW_HIDE);
            ShowWindow(hCut, SW_HIDE);
            ShowWindow(hPaste, SW_HIDE);

            SetWindowTextW(hEdit, L"");
            operation = 2;
            goto L1;

        case ID_DELETE:
            if (ListView_GetNextItem(hListView, -1, LVNI_SELECTED) != -1) {
                if (MessageBoxW(hwnd, L"是否删除选中项目？", L"警告", MB_YESNO | MB_ICONWARNING) == IDYES) {
                    wcscpy_s(path, FULL_PATH, browsingPath);
                    size_t len = wcslen(browsingPath);
                    int index = -1;
                    while ((index = ListView_GetNextItem(hListView, index, LVNI_SELECTED)) != -1) {
                        ListView_GetItemText(hListView, index, 0, path + len, FULL_PATH - len);
                        if (!Delete(path)) break;
                    }
                }
                BrowseTo(browsingPath);
                break;
            }
            MoveWindow(hDelete, buttonSpacing, buttonSpacing * 2 + buttonHeight, buttonWidth, buttonHeight, TRUE);
            ShowWindow(hNDir, SW_HIDE);
            ShowWindow(hNFile, SW_HIDE);
            ShowWindow(hRename, SW_HIDE);
            ShowWindow(hCopy, SW_HIDE);
            ShowWindow(hCut, SW_HIDE);
            ShowWindow(hPaste, SW_HIDE);
            operation = 3;
            goto L1;

        case ID_RENAME: {
            MoveWindow(hRename, buttonSpacing, buttonSpacing * 2 + buttonHeight, buttonWidth, buttonHeight, TRUE);
            ShowWindow(hNDir, SW_HIDE);
            ShowWindow(hNFile, SW_HIDE);
            ShowWindow(hDelete, SW_HIDE);
            ShowWindow(hCopy, SW_HIDE);
            ShowWindow(hCut, SW_HIDE);
            ShowWindow(hPaste, SW_HIDE);
            int index = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
            if (index == -1) goto L1;
            ListView_GetItemText(hListView, index, 0, path, FULL_PATH);
            SetWindowTextW(hEdit, path);
            operation = 4;
            goto L1;
        }

        case ID_COPY:
            wcscpy_s(fromPath, FULL_PATH, browsingPath);
            items.clear();
            for (int index = -1; (index = ListView_GetNextItem(hListView, index, LVNI_SELECTED)) != -1;) {
                ListView_GetItemText(hListView, index, 0, path, FULL_PATH);
                items.push_back(path);
            }
            rm=0;
            break;

        case ID_CUT:
            wcscpy_s(fromPath, FULL_PATH, browsingPath);
            items.clear();
            for (int index = -1; (index = ListView_GetNextItem(hListView, index, LVNI_SELECTED)) != -1;) {
                ListView_GetItemText(hListView, index, 0, path, FULL_PATH);
                items.push_back(path);
            }
            rm = 1;
            break;

        case ID_PASTE: {
            wcscpy_s(path, FULL_PATH, fromPath);
            size_t len = wcslen(path);
            wcscpy_s(path2, FULL_PATH, browsingPath);
            size_t len2 = wcslen(path2);
            if (rm) {
                for (std::wstring item : items) {
                    wcscpy_s(path + len, FULL_PATH - len, item.c_str());
                    wcscpy_s(path2 + len2, FULL_PATH - len2, item.c_str());
                    while (!MoveFileExW(path, path2, MOVEFILE_COPY_ALLOWED)) {
                        if (GetLastError() == ERROR_ALREADY_EXISTS) while (TRUE) {
                            wcscat_s(path2, FULL_PATH, L" -\"重试\"以覆盖文件");
                            int c = ErrorMsg(path2, MB_ICONWARNING | MB_CANCELTRYCONTINUE);
                            path2[len2] = 0;
                            if (c == IDCANCEL) return 0;
                            if (c == IDCONTINUE) goto L2;
                            if (MoveFileExW(path, path2, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING)) goto L2;
                        }
                        int c = ErrorMsg(path, MB_ICONWARNING | MB_CANCELTRYCONTINUE);
                        if (c == IDCANCEL) return 0;
                        if (c == IDCONTINUE) break;
                    }
                L2:;
                }
            }
            else {
                for (std::wstring item : items) {
                    wcscpy_s(path + len, FULL_PATH - len, item.c_str());
                    wcscpy_s(path2 + len2, FULL_PATH - len2, item.c_str());
                    if (!Copy(path, path2)) break;
                }
            }
            BrowseTo(browsingPath);
            break;
        }
        L1:
            ShowWindow(hYes, SW_SHOW);
            ShowWindow(hNo, SW_SHOW);
            ShowWindow(hEdit, SW_SHOW);
            SetFocus(hEdit);
            break;
            
        case ID_YES: {
            wcscpy_s(path, FULL_PATH, browsingPath);
            size_t len = wcslen(browsingPath);
            GetWindowTextW(hEdit, path + len, FULL_PATH - len);
            switch (operation) {
            case 1:
                wcscat_s(path, FULL_PATH, L"\\");
                if (!CreateDirectoryW(path, NULL))
                    if (ErrorMsg(path, MB_ICONERROR | MB_OKCANCEL) == IDOK) return 0;
                break;

            case 2: {
                HANDLE hFile = CreateFileW(path, GENERIC_WRITE, 7, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile == INVALID_HANDLE_VALUE) {
                    if (ErrorMsg(path, MB_ICONERROR | MB_OKCANCEL) == IDOK) return 0;
                } else CloseHandle(hFile);
                break;
            }
            case 3:
                Delete(path);
                break;
            
            case 4: {
                int index = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
                if (index == -1) break;
                wcscpy_s(path2, FULL_PATH, browsingPath);
                size_t len2 = wcslen(path2);
                ListView_GetItemText(hListView, index, 0, path2 + len2, FULL_PATH - len2);
                if (!MoveFileW(path2, path))
                    if (ErrorMsg(path, MB_ICONERROR | MB_OKCANCEL) == IDOK) return 0;
            }
            }
            BrowseTo(browsingPath);
        }

        case ID_NO:
            SetWindowTextW(hEdit, L"");
            SetWindowPos(hNDir, NULL, buttonSpacing, buttonSpacing * 2 + buttonHeight, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
            SetWindowPos(hNFile, NULL, buttonSpacing * 2 + buttonWidth, buttonSpacing * 2 + buttonHeight * 1, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
            SetWindowPos(hDelete, NULL, buttonSpacing * 3 + buttonWidth * 2, buttonSpacing * 2 + buttonHeight * 1, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
            SetWindowPos(hRename, NULL, buttonSpacing * 4 + buttonWidth * 3, buttonSpacing * 2 + buttonHeight * 1, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
            SetWindowPos(hCopy, NULL, buttonSpacing * 5 + buttonWidth * 4, buttonSpacing * 2 + buttonHeight * 1, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
            SetWindowPos(hCut, NULL, buttonSpacing * 6 + buttonWidth * 5, buttonSpacing * 2 + buttonHeight * 1, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
            SetWindowPos(hPaste, NULL, buttonSpacing * 7 + buttonWidth * 6, buttonSpacing * 2 + buttonHeight * 1, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
            ShowWindow(hNDir, SW_SHOW);
            ShowWindow(hNFile, SW_SHOW);
            ShowWindow(hDelete, SW_SHOW);
            ShowWindow(hRename, SW_SHOW);
            ShowWindow(hCopy, SW_SHOW);
            ShowWindow(hCut, SW_SHOW);
            ShowWindow(hPaste, SW_SHOW);
            ShowWindow(hYes, SW_HIDE);
            ShowWindow(hNo, SW_HIDE);
            ShowWindow(hEdit, SW_HIDE);
            operation = 0;
            break;
        }
        break;

    case WM_NOTIFY: {
        if (((LPNMHDR)lParam)->hwndFrom == hListView) {
            switch (((LPNMHDR)lParam)->code) {
            case NM_DBLCLK:
                OnListViewDblClick();
                break;
            }
        }
        break;
    }

    case WM_DRAWITEM: {
        LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
        if (pDIS->CtlID < ID_UP || pDIS->CtlID > ID_PASTE)
            return DefWindowProc(hwnd, msg, wParam, lParam);
        RECT rcClient = pDIS->rcItem;
        
        // 根据按钮状态决定颜色
        COLORREF bgColor =
            pDIS->itemState & ODS_SELECTED ? 0xF0F0F0 :
            pDIS->itemState & ODS_DISABLED ? 0xC0C0C0 :
            0xFFFFFF ;

        HBRUSH hBrush = CreateSolidBrush(bgColor);
        FillRect(pDIS->hDC, &rcClient, hBrush);
        DeleteObject(hBrush);

        //// 绘制焦点矩形
        //if (pDIS->itemState & ODS_FOCUS)
        //    DrawFocusRect(pDIS->hDC, &rcClient);

        HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(pDIS->CtlID));
        DrawIconEx(pDIS->hDC, 0, 0, hIcon, rcClient.right, rcClient.bottom, 0, NULL, DI_NORMAL);
        DestroyIcon(hIcon);

        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    InitCommonControls();

    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"FileExplorer";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    hFont = CreateFontW(0, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");

    hwnd = CreateWindowExW(0, L"FileExplorer", L"文件资源管理器", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);
    if (!hwnd) return 0;
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
