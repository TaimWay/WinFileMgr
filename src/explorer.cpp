#include "explorer.h"
#include <stdio.h>

Explorer::Explorer()
    : m_hwnd(nullptr), m_hListView(nullptr), m_hAddress(nullptr), m_hEdit(nullptr),
      m_hUp(nullptr), m_hRefresh(nullptr), m_hSuper(nullptr),
      m_hYes(nullptr), m_hNo(nullptr), m_hNDir(nullptr), m_hNFile(nullptr),
      m_hDelete(nullptr), m_hCopy(nullptr), m_hCut(nullptr), m_hPaste(nullptr),
      m_hRename(nullptr), m_hFont(nullptr), m_hInst(nullptr),
      m_operation(0), m_super(0), m_rm(FALSE) {
    m_browsingPath[0] = L'\0';
    m_fromPath[0] = L'\0';
}

Explorer::~Explorer() {
    if (m_hFont) DeleteObject(m_hFont);
}

bool Explorer::Create(HINSTANCE hInstance, int nCmdShow) {
    m_hInst = hInstance;

    // 注册窗口类
    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = StaticWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"MainClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    if (!RegisterClassW(&wc))
        return false;

    // 创建字体
    m_hFont = CreateFontW(0, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");

    // 创建窗口
    m_hwnd = CreateWindowExW(0, L"MainClass", L"文件资源管理器",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, hInstance, this);  // 传递 this 到 WM_NCCREATE

    if (!m_hwnd)
        return false;

    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);
    return true;
}

LRESULT CALLBACK Explorer::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Explorer* pThis = nullptr;
    if (msg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (Explorer*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
    } else {
        pThis = (Explorer*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (pThis)
        return pThis->WndProc(hwnd, msg, wParam, lParam);
    else
        return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT Explorer::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // 处理通用控件颜色消息
    if (msg >= WM_CTLCOLORMSGBOX && msg <= WM_CTLCOLORSTATIC)
        return (LRESULT)GetStockObject(WHITE_BRUSH);

    wchar_t path[FULL_PATH];
    wchar_t path2[FULL_PATH];

    switch (msg) {
    case WM_CREATE: {
        // 创建地址栏编辑框
        m_hAddress = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", nullptr,
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 0, 0,
            hwnd, (HMENU)ID_ADDRESS, m_hInst, nullptr);
        SendMessageW(m_hAddress, WM_SETFONT, (WPARAM)m_hFont, TRUE);

        // 创建操作编辑框
        m_hEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", nullptr,
            WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0,
            hwnd, (HMENU)ID_EDIT, m_hInst, nullptr);
        SendMessageW(m_hEdit, WM_SETFONT, (WPARAM)m_hFont, TRUE);

        // 创建导航按钮
        m_hUp = CreateWindowExW(0, L"BUTTON", nullptr,
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            buttonSpacing * 1, buttonSpacing, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_UP, m_hInst, nullptr);
        m_hRefresh = CreateWindowExW(0, L"BUTTON", nullptr,
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            buttonSpacing * 2 + buttonWidth * 1, buttonSpacing, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_REFRESH, m_hInst, nullptr);
        m_hSuper = CreateWindowExW(0, L"BUTTON", nullptr,
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            buttonSpacing * 3 + buttonWidth * 2, buttonSpacing, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_SUPER, m_hInst, nullptr);

        // 创建确认/取消按钮
        m_hYes = CreateWindowExW(0, L"BUTTON", nullptr,
            WS_CHILD | BS_OWNERDRAW, 0, 0, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_YES, m_hInst, nullptr);
        m_hNo = CreateWindowExW(0, L"BUTTON", nullptr,
            WS_CHILD | BS_OWNERDRAW, 0, 0, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_NO, m_hInst, nullptr);

        // 创建文件操作按钮
        m_hNDir = CreateWindowExW(0, L"BUTTON", nullptr,
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            buttonSpacing, buttonSpacing * 2 + buttonHeight, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_NDIR, m_hInst, nullptr);
        m_hNFile = CreateWindowExW(0, L"BUTTON", nullptr,
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            buttonSpacing * 2 + buttonWidth, buttonSpacing * 2 + buttonHeight, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_NFILE, m_hInst, nullptr);
        m_hDelete = CreateWindowExW(0, L"BUTTON", nullptr,
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            buttonSpacing * 3 + buttonWidth * 2, buttonSpacing * 2 + buttonHeight, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_DELETE, m_hInst, nullptr);
        m_hRename = CreateWindowExW(0, L"BUTTON", nullptr,
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            buttonSpacing * 4 + buttonWidth * 3, buttonSpacing * 2 + buttonHeight, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_RENAME, m_hInst, nullptr);
        m_hCopy = CreateWindowExW(0, L"BUTTON", nullptr,
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            buttonSpacing * 5 + buttonWidth * 4, buttonSpacing * 2 + buttonHeight, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_COPY, m_hInst, nullptr);
        m_hCut = CreateWindowExW(0, L"BUTTON", nullptr,
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            buttonSpacing * 6 + buttonWidth * 5, buttonSpacing * 2 + buttonHeight, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_CUT, m_hInst, nullptr);
        m_hPaste = CreateWindowExW(0, L"BUTTON", nullptr,
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            buttonSpacing * 7 + buttonWidth * 6, buttonSpacing * 2 + buttonHeight, buttonWidth, buttonHeight,
            hwnd, (HMENU)ID_PASTE, m_hInst, nullptr);

        // 创建列表视图
        m_hListView = CreateWindowExW(0, WC_LISTVIEWW, L"",
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS,
            0, 0, 0, 0, hwnd, (HMENU)ID_LISTVIEW, m_hInst, nullptr);

        // 设置列表视图图像列表
        SHFILEINFOW sfi = { 0 };
        HIMAGELIST hImageList = (HIMAGELIST)SHGetFileInfoW(L"", 0, &sfi, sizeof(sfi),
            SHGFI_SMALLICON | SHGFI_SYSICONINDEX);
        ListView_SetImageList(m_hListView, hImageList, LVSIL_SMALL);

        // 插入列
        LVCOLUMNW lvc = { 0 };
        lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        lvc.pszText = (LPWSTR)L"名称";
        lvc.cx = 200;
        lvc.iSubItem = 0;
        ListView_InsertColumn(m_hListView, 0, &lvc);
        lvc.pszText = (LPWSTR)L"大小";
        lvc.cx = 100;
        lvc.iSubItem = 1;
        ListView_InsertColumn(m_hListView, 1, &lvc);
        lvc.pszText = (LPWSTR)L"修改日期";
        lvc.cx = 120;
        lvc.iSubItem = 2;
        ListView_InsertColumn(m_hListView, 2, &lvc);

        ListView_SetExtendedListViewStyle(m_hListView, LVS_EX_FULLROWSELECT);

        // 初始浏览到根驱动器
        BrowseTo(L"");
        break;
    }

    case WM_SIZE: {
        RECT rc;
        GetClientRect(hwnd, &rc);

        MoveWindow(m_hAddress, buttonSpacing * 4 + buttonWidth * 3, buttonSpacing,
            rc.right - buttonSpacing * 3 - buttonWidth * 1, buttonHeight, TRUE);
        MoveWindow(m_hEdit, buttonSpacing * 2 + buttonWidth * 1,
            buttonSpacing * 2 + buttonHeight, rc.right - buttonSpacing * 5 - buttonWidth * 3,
            buttonHeight, TRUE);
        SetWindowPos(m_hYes, nullptr, rc.right - buttonSpacing - buttonWidth,
            buttonSpacing * 2 + buttonHeight, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
        SetWindowPos(m_hNo, nullptr, rc.right - buttonSpacing * 2 - buttonWidth * 2,
            buttonSpacing * 2 + buttonHeight, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
        int listY = buttonSpacing * 3 + buttonHeight * 2;
        MoveWindow(m_hListView, buttonSpacing, listY,
            rc.right - buttonSpacing * 2, rc.bottom - listY - buttonSpacing, TRUE);
        break;
    }

    case WM_CONTEXTMENU: {
        POINT pt;
        GetCursorPos(&pt);
        HMENU hMenu = CreatePopupMenu();

        int index = ListView_GetNextItem(m_hListView, -1, LVNI_SELECTED);
        if (index == -1) {
            AppendMenuW(hMenu, MF_STRING, IDM_REFRESH, L"刷新(&R)");
            AppendMenuW(hMenu, MF_STRING, IDM_CMD, L"CMD(&C)");
            AppendMenuW(hMenu, MF_STRING, IDM_EXPLORER, L"Explorer(&E)");
            AppendMenuW(hMenu, MF_STRING, IDM_CPDIRPATH, L"复制路径(&P)");
        } else if (ListView_GetNextItem(m_hListView, index, LVNI_SELECTED) == -1) {
            AppendMenuW(hMenu, MF_STRING, IDM_OPEN, L"打开(&O)");
            AppendMenuW(hMenu, MF_STRING, IDM_NOTEPAD, L"记事本(&N)");
            AppendMenuW(hMenu, MF_STRING, IDM_CPFILEPATH, L"复制路径(&P)");
            AppendMenuW(hMenu, MF_STRING, IDM_STREAM, L"数据流(&S)");
        } else {
            DestroyMenu(hMenu);
            break;
        }

        int choice = TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RETURNCMD,
            pt.x, pt.y, 0, hwnd, nullptr);
        STARTUPINFO si = { sizeof(si) };
        PROCESS_INFORMATION pi;

        if (index == -1) {
            switch (choice) {
            case IDM_REFRESH:
                BrowseTo(m_browsingPath);
                break;
            case IDM_CMD: {
                wchar_t cmd[] = L"cmd.exe";
                if (CreateProcessW(nullptr, cmd, nullptr, nullptr, FALSE,
                    CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi)) {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
                break;
            }
            case IDM_EXPLORER:
                wcscpy_s(path, FULL_PATH, L"explorer.exe ");
                wcscat_s(path, FULL_PATH, m_browsingPath);
                if (CreateProcessW(nullptr, path, nullptr, nullptr, FALSE,
                    CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi)) {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
                break;
            case IDM_CPDIRPATH:
                CopyTextToClipboard(m_browsingPath);
                break;
            }
        } else {
            size_t len;
            switch (choice) {
            case IDM_OPEN:
                wcscpy_s(path, FULL_PATH, m_browsingPath);
                len = wcslen(path);
                ListView_GetItemText(m_hListView, index, 0, path + len, FULL_PATH - len);
                BrowseTo(path);
                break;
            case IDM_NOTEPAD:
                wcscpy_s(path, FULL_PATH, L"notepad.exe ");
                wcscat_s(path, FULL_PATH, m_browsingPath);
                len = wcslen(path);
                ListView_GetItemText(m_hListView, index, 0, path + len, FULL_PATH - len);
                if (CreateProcessW(nullptr, path, nullptr, nullptr, FALSE,
                    CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi)) {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
                break;
            case IDM_CPFILEPATH:
                wcscpy_s(path, FULL_PATH, m_browsingPath);
                len = wcslen(path);
                ListView_GetItemText(m_hListView, index, 0, path + len, FULL_PATH - len);
                CopyTextToClipboard(path);
                break;
            case IDM_STREAM:
                wcscpy_s(path, FULL_PATH, m_browsingPath);
                len = wcslen(path);
                ListView_GetItemText(m_hListView, index, 0, path + len, FULL_PATH - len);
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
            wcscpy_s(path, FULL_PATH, m_browsingPath);
            int p = m_super * 4;
            for (int i = 1 + m_super * 4; i < FULL_PATH; i++) {
                if (path[i - 1] == L'\\' && path[i] != L'\0')
                    p = i;
            }
            path[p] = L'\0';
            BrowseTo(path);
            break;
        }
        case ID_REFRESH: {
            if (m_super) {
                wcscpy_s(path, FULL_PATH, L"\\\\?\\");
                GetWindowTextW(m_hAddress, path + 4, FULL_PATH - 4);
            } else {
                GetWindowTextW(m_hAddress, path, FULL_PATH);
            }
            BrowseTo(path);
            break;
        }
        case ID_SUPER:
            m_super = 1 - m_super;
            InvalidateRect(m_hSuper, nullptr, TRUE);
            if (m_super) {
                wcscpy_s(path, FULL_PATH, L"\\\\?\\");
                wcscat_s(path, FULL_PATH, m_browsingPath);
                BrowseTo(path);
            } else {
                wcscpy_s(path, FULL_PATH, m_browsingPath);
                for (char i = 0;; i++) {
                    if (i >= 4) {
                        BrowseTo(path + 4);
                        break;
                    }
                    if (path[i] != L"\\\\?\\"[i]) break;
                }
            }
            break;

        case ID_NDIR:
            MoveWindow(m_hNDir, buttonSpacing, buttonSpacing * 2 + buttonHeight,
                buttonWidth, buttonHeight, TRUE);
            ShowWindow(m_hNFile, SW_HIDE);
            ShowWindow(m_hDelete, SW_HIDE);
            ShowWindow(m_hRename, SW_HIDE);
            ShowWindow(m_hCopy, SW_HIDE);
            ShowWindow(m_hCut, SW_HIDE);
            ShowWindow(m_hPaste, SW_HIDE);
            m_operation = 1;
            goto L1;

        case ID_NFILE:
            MoveWindow(m_hNFile, buttonSpacing, buttonSpacing * 2 + buttonHeight,
                buttonWidth, buttonHeight, TRUE);
            ShowWindow(m_hNDir, SW_HIDE);
            ShowWindow(m_hDelete, SW_HIDE);
            ShowWindow(m_hRename, SW_HIDE);
            ShowWindow(m_hCopy, SW_HIDE);
            ShowWindow(m_hCut, SW_HIDE);
            ShowWindow(m_hPaste, SW_HIDE);
            SetWindowTextW(m_hEdit, L"");
            m_operation = 2;
            goto L1;

        case ID_DELETE:
            if (ListView_GetNextItem(m_hListView, -1, LVNI_SELECTED) != -1) {
                if (MessageBoxW(hwnd, L"是否删除选中项目？", L"警告",
                    MB_YESNO | MB_ICONWARNING) == IDYES) {
                    wcscpy_s(path, FULL_PATH, m_browsingPath);
                    size_t len = wcslen(m_browsingPath);
                    int index = -1;
                    while ((index = ListView_GetNextItem(m_hListView, index, LVNI_SELECTED)) != -1) {
                        ListView_GetItemText(m_hListView, index, 0, path + len, FULL_PATH - len);
                        if (!Delete(path)) break;
                    }
                }
                BrowseTo(m_browsingPath);
                break;
            }
            MoveWindow(m_hDelete, buttonSpacing, buttonSpacing * 2 + buttonHeight,
                buttonWidth, buttonHeight, TRUE);
            ShowWindow(m_hNDir, SW_HIDE);
            ShowWindow(m_hNFile, SW_HIDE);
            ShowWindow(m_hRename, SW_HIDE);
            ShowWindow(m_hCopy, SW_HIDE);
            ShowWindow(m_hCut, SW_HIDE);
            ShowWindow(m_hPaste, SW_HIDE);
            m_operation = 3;
            goto L1;

        case ID_RENAME: {
            MoveWindow(m_hRename, buttonSpacing, buttonSpacing * 2 + buttonHeight,
                buttonWidth, buttonHeight, TRUE);
            ShowWindow(m_hNDir, SW_HIDE);
            ShowWindow(m_hNFile, SW_HIDE);
            ShowWindow(m_hDelete, SW_HIDE);
            ShowWindow(m_hCopy, SW_HIDE);
            ShowWindow(m_hCut, SW_HIDE);
            ShowWindow(m_hPaste, SW_HIDE);
            int index = ListView_GetNextItem(m_hListView, -1, LVNI_SELECTED);
            if (index == -1) goto L1;
            ListView_GetItemText(m_hListView, index, 0, path, FULL_PATH);
            SetWindowTextW(m_hEdit, path);
            m_operation = 4;
            goto L1;
        }

        case ID_COPY:
            wcscpy_s(m_fromPath, FULL_PATH, m_browsingPath);
            m_items.clear();
            for (int index = -1; (index = ListView_GetNextItem(m_hListView, index, LVNI_SELECTED)) != -1;) {
                ListView_GetItemText(m_hListView, index, 0, path, FULL_PATH);
                m_items.push_back(path);
            }
            m_rm = FALSE;
            break;

        case ID_CUT:
            wcscpy_s(m_fromPath, FULL_PATH, m_browsingPath);
            m_items.clear();
            for (int index = -1; (index = ListView_GetNextItem(m_hListView, index, LVNI_SELECTED)) != -1;) {
                ListView_GetItemText(m_hListView, index, 0, path, FULL_PATH);
                m_items.push_back(path);
            }
            m_rm = TRUE;
            break;

        case ID_PASTE: {
            wcscpy_s(path, FULL_PATH, m_fromPath);
            size_t len = wcslen(path);
            wcscpy_s(path2, FULL_PATH, m_browsingPath);
            size_t len2 = wcslen(path2);
            if (m_rm) {
                for (const std::wstring& item : m_items) {
                    wcscpy_s(path + len, FULL_PATH - len, item.c_str());
                    wcscpy_s(path2 + len2, FULL_PATH - len2, item.c_str());
                    while (!MoveFileExW(path, path2, MOVEFILE_COPY_ALLOWED)) {
                        if (GetLastError() == ERROR_ALREADY_EXISTS) {
                            while (TRUE) {
                                wcscat_s(path2, FULL_PATH, L" -\"重试\"以覆盖文件");
                                int c = ErrorMsg(path2, MB_ICONWARNING | MB_CANCELTRYCONTINUE);
                                path2[len2] = 0;
                                if (c == IDCANCEL) return 0;
                                if (c == IDCONTINUE) goto L2;
                                if (MoveFileExW(path, path2,
                                    MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING))
                                    goto L2;
                            }
                        }
                        int c = ErrorMsg(path, MB_ICONWARNING | MB_CANCELTRYCONTINUE);
                        if (c == IDCANCEL) return 0;
                        if (c == IDCONTINUE) break;
                    }
                L2:;
                }
            } else {
                for (const std::wstring& item : m_items) {
                    wcscpy_s(path + len, FULL_PATH - len, item.c_str());
                    wcscpy_s(path2 + len2, FULL_PATH - len2, item.c_str());
                    if (!Copy(path, path2)) break;
                }
            }
            BrowseTo(m_browsingPath);
            break;
        }

        L1:
            ShowWindow(m_hYes, SW_SHOW);
            ShowWindow(m_hNo, SW_SHOW);
            ShowWindow(m_hEdit, SW_SHOW);
            SetFocus(m_hEdit);
            break;

        case ID_YES: {
            wcscpy_s(path, FULL_PATH, m_browsingPath);
            size_t len = wcslen(m_browsingPath);
            GetWindowTextW(m_hEdit, path + len, FULL_PATH - len);
            switch (m_operation) {
            case 1:
                if (!CreateDirectoryW(path, nullptr)) {
                    if (ErrorMsg(path, MB_ICONERROR | MB_OKCANCEL) == IDOK)
                        return 0;
                }
                break;
            case 2: {
                HANDLE hFile = CreateFileW(path, GENERIC_WRITE, 7, nullptr,
                    CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
                if (hFile == INVALID_HANDLE_VALUE) {
                    if (ErrorMsg(path, MB_ICONERROR | MB_OKCANCEL) == IDOK)
                        return 0;
                } else {
                    CloseHandle(hFile);
                }
                break;
            }
            case 3:
                Delete(path);
                break;
            case 4: {
                int index = ListView_GetNextItem(m_hListView, -1, LVNI_SELECTED);
                if (index == -1) break;
                wcscpy_s(path2, FULL_PATH, m_browsingPath);
                size_t len2 = wcslen(path2);
                ListView_GetItemText(m_hListView, index, 0, path2 + len2, FULL_PATH - len2);
                if (!MoveFileW(path2, path)) {
                    if (ErrorMsg(path, MB_ICONERROR | MB_OKCANCEL) == IDOK)
                        return 0;
                }
            }
            }
            BrowseTo(m_browsingPath);
            // 继续到 ID_NO 以隐藏编辑控件
        }

        case ID_NO:
            SetWindowTextW(m_hEdit, L"");
            SetWindowPos(m_hNDir, nullptr, buttonSpacing, buttonSpacing * 2 + buttonHeight,
                0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
            SetWindowPos(m_hNFile, nullptr, buttonSpacing * 2 + buttonWidth,
                buttonSpacing * 2 + buttonHeight, 0, 0,
                SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
            SetWindowPos(m_hDelete, nullptr, buttonSpacing * 3 + buttonWidth * 2,
                buttonSpacing * 2 + buttonHeight, 0, 0,
                SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
            SetWindowPos(m_hRename, nullptr, buttonSpacing * 4 + buttonWidth * 3,
                buttonSpacing * 2 + buttonHeight, 0, 0,
                SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
            SetWindowPos(m_hCopy, nullptr, buttonSpacing * 5 + buttonWidth * 4,
                buttonSpacing * 2 + buttonHeight, 0, 0,
                SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
            SetWindowPos(m_hCut, nullptr, buttonSpacing * 6 + buttonWidth * 5,
                buttonSpacing * 2 + buttonHeight, 0, 0,
                SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
            SetWindowPos(m_hPaste, nullptr, buttonSpacing * 7 + buttonWidth * 6,
                buttonSpacing * 2 + buttonHeight, 0, 0,
                SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
            ShowWindow(m_hNDir, SW_SHOW);
            ShowWindow(m_hNFile, SW_SHOW);
            ShowWindow(m_hDelete, SW_SHOW);
            ShowWindow(m_hRename, SW_SHOW);
            ShowWindow(m_hCopy, SW_SHOW);
            ShowWindow(m_hCut, SW_SHOW);
            ShowWindow(m_hPaste, SW_SHOW);
            ShowWindow(m_hYes, SW_HIDE);
            ShowWindow(m_hNo, SW_HIDE);
            ShowWindow(m_hEdit, SW_HIDE);
            m_operation = 0;
            break;
        }
        break;

    case WM_NOTIFY: {
        LPNMHDR pnmh = (LPNMHDR)lParam;
        if (pnmh->hwndFrom == m_hListView) {
            switch (pnmh->code) {
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

        COLORREF bgColor =
            (pDIS->itemState & ODS_SELECTED) ? 0xF0F0F0 :
            (pDIS->itemState & ODS_DISABLED) ? 0xC0C0C0 : 0xFFFFFF;

        HBRUSH hBrush = CreateSolidBrush(bgColor);
        FillRect(pDIS->hDC, &rcClient, hBrush);
        DeleteObject(hBrush);

        HICON hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(
            (pDIS->CtlID == ID_SUPER) ? IDI_SUPERN + m_super : pDIS->CtlID
        ));
        DrawIconEx(pDIS->hDC, 0, 0, hIcon, rcClient.right, rcClient.bottom,
            0, nullptr, DI_NORMAL);
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

int Explorer::ErrorMsg(const wchar_t* info, UINT uType) {
    wchar_t msg[FULL_PATH];
    DWORD err = GetLastError();
    swprintf_s(msg, FULL_PATH, L"%s\n[0x%08X]", info, err);
    size_t len = wcslen(msg);
    FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        msg + len, FULL_PATH - len, nullptr
    );
    return MessageBoxW(m_hwnd, msg, L"Error", uType);
}

BOOL Explorer::CopyTextToClipboard(const std::wstring& strText) {
    if (strText.empty()) return FALSE;
    if (!OpenClipboard(nullptr)) {
        ErrorMsg(L"OpenClipboard failed", MB_ICONERROR | MB_OK);
        return FALSE;
    }
    if (!EmptyClipboard()) {
        CloseClipboard();
        return FALSE;
    }

    SIZE_T cbSize = (strText.length() + 1) * sizeof(WCHAR);
    HGLOBAL hClipboardData = GlobalAlloc(GMEM_MOVEABLE, cbSize);
    if (hClipboardData == nullptr) {
        CloseClipboard();
        return FALSE;
    }

    wchar_t* pchData = (wchar_t*)GlobalLock(hClipboardData);
    if (pchData != nullptr) {
        wcscpy_s(pchData, strText.length() + 1, strText.c_str());
        GlobalUnlock(hClipboardData);
    } else {
        GlobalFree(hClipboardData);
        CloseClipboard();
        return FALSE;
    }

    if (SetClipboardData(CF_UNICODETEXT, hClipboardData) == nullptr) {
        GlobalFree(hClipboardData);
        CloseClipboard();
        return FALSE;
    }

    CloseClipboard();
    return TRUE;
}

void Explorer::BrowseTo(const wchar_t* path) {
    wchar_t locPath[FULL_PATH];
    if (!*path || wcscmp(path, L"\\\\?\\") == 0) {
        DWORD dwResult = GetLogicalDriveStringsW(256, locPath);
        if (dwResult == 0) {
            ErrorMsg(L"GetLogicalDriveStrings", MB_ICONERROR | MB_OK);
            return;
        }
        wcscpy_s(m_browsingPath, FULL_PATH, path);
        SetWindowTextW(m_hAddress, m_super ? path + 4 : path);
        ListView_DeleteAllItems(m_hListView);
        for (wchar_t* p = locPath; *p; p += wcslen(p) + 1) {
            LVITEMW lvi = { 0 };
            SHFILEINFOW sfi = { 0 };
            SHGetFileInfoW(p, FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi),
                SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
            lvi.mask = LVIF_TEXT | LVIF_IMAGE;
            lvi.iItem = ListView_GetItemCount(m_hListView);
            lvi.pszText = p;
            lvi.iImage = sfi.iIcon;
            ListView_InsertItem(m_hListView, &lvi);
        }
        return;
    }

    if (!m_super) {
        DWORD expandLen = ExpandEnvironmentStringsW(path, locPath, FULL_PATH);
        if (expandLen == 0 || expandLen > FULL_PATH) {
            ErrorMsg(path, MB_ICONERROR | MB_OK);
            return;
        }
    } else {
        wcscpy_s(locPath, FULL_PATH, path);
    }

    DWORD attr = GetFileAttributesW(locPath);
    size_t len = wcslen(locPath);
    if (locPath[len - 1] != L'\\')
        locPath[len++] = L'\\';
    locPath[len] = L'*';
    locPath[len + 1] = L'\0';

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(locPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        locPath[--len] = L'\0';

        if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
            if (ShellExecuteW(nullptr, L"open", locPath, nullptr, nullptr, SW_SHOWNORMAL) > (HINSTANCE)32) {
                SetWindowTextW(m_hAddress, m_super ? m_browsingPath + 4 : m_browsingPath);
                return;
            }
        }

        if (locPath[--len] != L':') {
            ErrorMsg(locPath, MB_ICONERROR | MB_OK);
            SetWindowTextW(m_hAddress, m_super ? m_browsingPath + 4 : m_browsingPath);
            return;
        }
        locPath[len] = L'\0';
        WIN32_FIND_STREAM_DATA findStreamData;
        HANDLE hFindStream = FindFirstStreamW(locPath, FindStreamInfoStandard, &findStreamData, 0);
        if (hFindStream == INVALID_HANDLE_VALUE) {
            if (GetLastError() == 0x26) {
                locPath[len] = L':';
                wcscpy_s(m_browsingPath, FULL_PATH, locPath);
                SetWindowTextW(m_hAddress, m_super ? locPath + 4 : locPath);
                ListView_DeleteAllItems(m_hListView);
                return;
            }
            ErrorMsg(locPath, MB_ICONERROR | MB_OK);
            SetWindowTextW(m_hAddress, m_super ? m_browsingPath + 4 : m_browsingPath);
            return;
        }
        locPath[len] = L':';
        wcscpy_s(m_browsingPath, FULL_PATH, locPath);
        SetWindowTextW(m_hAddress, m_super ? locPath + 4 : locPath);
        ListView_DeleteAllItems(m_hListView);

        do {
            if (findStreamData.cStreamName[1] != L':') {
                wchar_t* p = wcschr(findStreamData.cStreamName + 1, L':');
                if (p != nullptr) *p = L'\0';
            }
            LVITEMW lvi = { 0 };
            lvi.mask = LVIF_TEXT | LVIF_IMAGE;
            lvi.iItem = ListView_GetItemCount(m_hListView);
            lvi.pszText = findStreamData.cStreamName + 1;

            SHFILEINFOW sfi = { 0 };
            SHGetFileInfoW(findStreamData.cStreamName + 1, FILE_ATTRIBUTE_NORMAL,
                &sfi, sizeof(sfi), SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
            lvi.iImage = sfi.iIcon;
            ListView_InsertItem(m_hListView, &lvi);

            wchar_t size[32];
            swprintf_s(size, 32, L"%lld KB", findStreamData.StreamSize.QuadPart / 1024);
            ListView_SetItemText(m_hListView, lvi.iItem, 1, size);
        } while (FindNextStreamW(hFindStream, &findStreamData));
        FindClose(hFindStream);
        return;
    }

    locPath[len] = L'\0';
    wcscpy_s(m_browsingPath, FULL_PATH, locPath);
    SetWindowTextW(m_hAddress, m_super ? locPath + 4 : locPath);
    SetCurrentDirectoryW(locPath);
    ListView_DeleteAllItems(m_hListView);

    do {
        if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0)
            continue;

        LVITEMW lvi = { 0 };
        lvi.mask = LVIF_TEXT | LVIF_IMAGE;
        lvi.iItem = ListView_GetItemCount(m_hListView);
        lvi.pszText = findData.cFileName;

        SHFILEINFOW sfi = { 0 };
        SHGetFileInfoW(findData.cFileName, findData.dwFileAttributes, &sfi, sizeof(sfi),
            SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
        lvi.iImage = sfi.iIcon;
        ListView_InsertItem(m_hListView, &lvi);

        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            wchar_t size[32];
            swprintf_s(size, 32, L"%ld KB", findData.nFileSizeLow / 1024);
            ListView_SetItemText(m_hListView, lvi.iItem, 1, size);
        }

        SYSTEMTIME st;
        FileTimeToSystemTime(&findData.ftLastWriteTime, &st);
        wchar_t date[32];
        swprintf_s(date, 32, L"%04d/%02d/%02d", st.wYear, st.wMonth, st.wDay);
        ListView_SetItemText(m_hListView, lvi.iItem, 2, date);
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
}

void Explorer::OnListViewDblClick() {
    int index = ListView_GetNextItem(m_hListView, -1, LVNI_SELECTED);
    if (index == -1) return;

    wchar_t path[FULL_PATH];
    wcscpy_s(path, FULL_PATH, m_browsingPath);
    size_t len = wcslen(path);
    ListView_GetItemText(m_hListView, index, 0, path + len, FULL_PATH - len);
    BrowseTo(path);
}

BOOL Explorer::Delete(wchar_t* path) {
    wcscat_s(path, FULL_PATH, L"\\*");
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(path, &findData);
    size_t len = wcslen(path);
    if (hFind == INVALID_HANDLE_VALUE) {
        path[len - 2] = L'\0';
        while (!DeleteFileW(path)) {
            int c = ErrorMsg(path, MB_ICONWARNING | MB_CANCELTRYCONTINUE);
            if (c == IDCANCEL) return FALSE;
            if (c == IDCONTINUE) return TRUE;
        }
        return TRUE;
    }
    path[--len] = L'\0';
    do {
        if (wcscmp(findData.cFileName, L".") && wcscmp(findData.cFileName, L"..")) {
            wcscpy_s(path + len, FULL_PATH - len, findData.cFileName);
            if (!Delete(path)) {
                FindClose(hFind);
                return FALSE;
            }
        }
    } while (FindNextFileW(hFind, &findData));
    FindClose(hFind);
    path[len] = L'\0';
    while (!RemoveDirectoryW(path)) {
        int c = ErrorMsg(path, MB_ICONWARNING | MB_CANCELTRYCONTINUE);
        if (c == IDCANCEL) return FALSE;
        if (c == IDCONTINUE) return TRUE;
    }
    return TRUE;
}

BOOL Explorer::ManualCopy(const wchar_t* from, const wchar_t* to, bool overwrite) {
#define BUFFER_SIZE 8192
    char buffer[BUFFER_SIZE];
    DWORD bytesRead, bytesWritten;
    DWORD error;
    HANDLE hfrom = CreateFileW(from, GENERIC_READ, FILE_SHARE_READ, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hfrom == INVALID_HANDLE_VALUE) return FALSE;

    HANDLE hto = CreateFileW(to, GENERIC_WRITE, 0, nullptr,
        overwrite ? CREATE_ALWAYS : CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hto == INVALID_HANDLE_VALUE) {
        error = GetLastError();
        CloseHandle(hfrom);
        SetLastError(error);
        return FALSE;
    }

    while (TRUE) {
        if (!ReadFile(hfrom, buffer, BUFFER_SIZE, &bytesRead, nullptr)) goto L1;
        if (bytesRead == 0) break;
        if (!WriteFile(hto, buffer, bytesRead, &bytesWritten, nullptr)) goto L1;
        if (bytesRead != bytesWritten) goto L1;
    }

    CloseHandle(hto);
    CloseHandle(hfrom);
    return TRUE;

L1:
    error = GetLastError();
    CloseHandle(hto);
    CloseHandle(hfrom);
    SetLastError(error);
    return FALSE;
#undef BUFFER_SIZE
}

BOOL Explorer::Copy(wchar_t* from, wchar_t* to) {
    wcscat_s(from, FULL_PATH, L"\\*");
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(from, &findData);
    size_t fromLen = wcslen(from);
    size_t toLen = wcslen(to);
    if (hFind == INVALID_HANDLE_VALUE) {
        from[fromLen -= 2] = L'\0';
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
            if (GetLastError() == ERROR_FILE_EXISTS) {
                while (TRUE) {
                    wcscat_s(to, FULL_PATH, L" -\"重试\"以覆盖文件并忽略文件属性");
                    int c = ErrorMsg(from, MB_ICONWARNING | MB_CANCELTRYCONTINUE);
                    to[toLen] = L'\0';
                    if (c == IDCANCEL) return FALSE;
                    if (c == IDCONTINUE) return TRUE;
                    if (ManualCopy(from, to, TRUE)) return TRUE;
                }
            }
        }
    }

    from[--fromLen] = L'\0';
    wcscat_s(to, FULL_PATH, L"\\");
    toLen++;
    while (!CreateDirectoryW(to, nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) {
        int c = ErrorMsg(to, MB_ICONWARNING | MB_CANCELTRYCONTINUE);
        if (c == IDCANCEL) return FALSE;
        if (c == IDCONTINUE) return TRUE;
    }

    do {
        if (wcscmp(findData.cFileName, L".") && wcscmp(findData.cFileName, L"..")) {
            wcscpy_s(from + fromLen, FULL_PATH - fromLen, findData.cFileName);
            wcscpy_s(to + toLen, FULL_PATH - toLen, findData.cFileName);
            if (!Copy(from, to)) {
                FindClose(hFind);
                return FALSE;
            }
        }
    } while (FindNextFileW(hFind, &findData));
    FindClose(hFind);
    return TRUE;
}