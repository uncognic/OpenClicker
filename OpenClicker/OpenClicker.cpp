#include <windows.h>
#pragma comment(linker, \
"\"/manifestdependency:type='win32' "\
"name='Microsoft.Windows.Common-Controls' "\
"version='6.0.0.0' "\
"processorArchitecture='*' "\
"publicKeyToken='6595b64144ccf1df' "\
"language='*'\"")
#define ID_BUTTON_CLICK 1
bool clicking = false;
HWND hwndEdit;
HWND hwndButton;
HANDLE thread = nullptr;
DWORD lastToggleTime = 0;
const DWORD toggleCooldown = 1000;
DWORD WINAPI Click(LPVOID)
{
    while (clicking) {
        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;
        SendInput(1, &input, sizeof(INPUT));
        wchar_t buffer[16]; 
        GetWindowText(hwndEdit, buffer, 16);
        int interval = max(1, min(_wtoi(buffer), 10000));
        Sleep(interval);
    }

    return 0;
}
void ToggleClicking()
{
    DWORD now = GetTickCount();
    if (now - lastToggleTime < toggleCooldown)
        return; 

    lastToggleTime = now;

    clicking = !clicking;
    SetWindowText(hwndButton, clicking ? L"Stop" : L"Start");
    EnableWindow(hwndEdit, !clicking);

    if (clicking)
        thread = CreateThread(nullptr, 0, Click, nullptr, 0, nullptr);
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg) {
        case WM_DESTROY:
            clicking = false;
            if (thread) {
                WaitForSingleObject(thread, INFINITE);
                CloseHandle(thread);
            }
            PostQuitMessage(0);
            return 0;
        case WM_COMMAND:
            if (LOWORD(wParam) == ID_BUTTON_CLICK)
                ToggleClicking();
            return 0;
        case WM_HOTKEY:
            if (wParam == 1)
                ToggleClicking();
            return 0;
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetBkMode(hdcStatic, TRANSPARENT);  
            SetTextColor(hdcStatic, RGB(0, 0, 0)); 
            return (LRESULT)GetStockObject(NULL_BRUSH); 
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"OpenClicker";

    RegisterClass(&wc);

    HFONT hUIFont = CreateFontW(
        -11, 0, 0, 0,
        FW_NORMAL,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"Segoe UI"
    );
    HFONT hItalicFont = CreateFontW(
        -11, 0, 0, 0,
        FW_NORMAL,
        TRUE, 
        FALSE,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"Segoe UI"
    );



    HWND hwnd = CreateWindowEx(
        0,
        L"OpenClicker", L"OpenClicker",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 200, 300,
        nullptr, nullptr, hInst, nullptr
    );


    RegisterHotKey(hwnd, 1, 0, VK_F5);

    HWND hwndLabel = CreateWindow(
        L"STATIC", 
        L"Interval (ms):",
        WS_CHILD | WS_VISIBLE | SS_RIGHT,
        10, 30, 80, 25,
        hwnd,
        nullptr,
        hInst, 
        nullptr
    );
    SendMessage(hwndLabel, WM_SETFONT, (WPARAM)hUIFont, TRUE);
    SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_MAXIMIZEBOX);
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

    hwndEdit = CreateWindow(L"EDIT",
        L"50",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
        100, 30, 60, 25,
        hwnd, 
        (HMENU)2,
        hInst,
        nullptr
    );
    SendMessage(hwndEdit, WM_SETFONT, (WPARAM)hUIFont, TRUE);

    hwndButton = CreateWindow(
        L"BUTTON",
        L"Start",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        45, 90, 100, 100,
        hwnd, 
        (HMENU)ID_BUTTON_CLICK, 
        hInst, 
        nullptr
    );
    SendMessage(hwndButton, WM_SETFONT, (WPARAM)hUIFont, TRUE);

    HWND hwndHint = CreateWindow(
        L"STATIC",
        L"or F5",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        45, 200, 100, 20,
        hwnd, 
        nullptr,
        hInst,
        nullptr
    );
    SendMessage(hwndHint, WM_SETFONT, (WPARAM)hItalicFont, TRUE);

    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
