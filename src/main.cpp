#include "explorer.h"

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    InitCommonControls();

    Explorer explorer;
    if (!explorer.Create(hInstance, nCmdShow))
        return 0;

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}