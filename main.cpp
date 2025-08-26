#define UNICODE
#define _UNICODE

#include <windows.h>
#include <ntverp.h> // For VER_PRODUCTBUILD
#include <stdio.h>
#include <shellapi.h>
#include "resource.h"
#include "Settings.h"
#include "SettingsDialog.h"
#include "ToastNotifier.h"

#define WM_TRAYICON (WM_USER + 1)
#define IDT_TIMER1 1

// Global variables
HINSTANCE g_hInst = NULL;
HWND g_hWnd = NULL;
ToastNotifier g_notifier;
const wchar_t CLASS_NAME[] = L"MemAlertWindowClass";
const wchar_t WINDOW_TITLE[] = L"MemAlert";

// Forward declarations
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void AddTrayIcon(HWND hWnd);
void RemoveTrayIcon(HWND hWnd);
void ShowContextMenu(HWND hWnd, POINT pt);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // --- SDK Version Check ---
    wchar_t sdk_msg[256];
    swprintf_s(sdk_msg, L"DEBUG: Windows SDK Build Version: %d\n", VER_PRODUCTBUILD);
    OutputDebugStringW(sdk_msg);
    // -------------------------

    g_hInst = hInstance;

    InitSettings();

    // Register the window class.
    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_APPICON));

    if (!RegisterClass(&wc))
    {
        MessageBox(NULL, L"Window Registration Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Create the window.
    g_hWnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        WINDOW_TITLE,                   // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (g_hWnd == NULL)
    {
        MessageBox(NULL, L"Window Creation Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // We don't want to show the main window.
    // ShowWindow(g_hWnd, nCmdShow);

    AddTrayIcon(g_hWnd);
    SetTimer(g_hWnd, IDT_TIMER1, 5000, NULL); // Trigger every 5 seconds

    // Run the message loop.
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        KillTimer(hWnd, IDT_TIMER1);
        RemoveTrayIcon(hWnd);
        PostQuitMessage(0);
        return 0;

    case WM_TIMER:
        if (wParam == IDT_TIMER1)
        {
            MEMORYSTATUSEX statex;
            statex.dwLength = sizeof(statex);
            GlobalMemoryStatusEx(&statex);

            DWORDLONG totalVirtual = statex.ullTotalPageFile;
            DWORDLONG usedVirtual = statex.ullTotalPageFile - statex.ullAvailPageFile;
            
            if (totalVirtual > 0) {
                int percentage = (int)((usedVirtual * 100) / totalVirtual);
                int threshold = GetAlertThreshold();

                wchar_t buffer[256];
                swprintf_s(buffer, L"Commit Charge: %d%%, Threshold: %d%%\n", percentage, threshold);
                OutputDebugStringW(buffer);

                if (percentage >= threshold)
                {
                    g_notifier.ShowToast(percentage);
                }
            }
        }
        return 0;

    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP)
        {
            POINT curPoint;
            GetCursorPos(&curPoint);
            ShowContextMenu(hWnd, curPoint);
        }
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_SETTINGS:
            ShowSettingsDialog(hWnd);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        }
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        EndPaint(hWnd, &ps);
    }
    return 0;

    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void AddTrayIcon(HWND hWnd)
{
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_APPICON));
    wcscpy_s(nid.szTip, L"MemAlert");

    Shell_NotifyIcon(NIM_ADD, &nid);
}

void RemoveTrayIcon(HWND hWnd)
{
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1;

    Shell_NotifyIcon(NIM_DELETE, &nid);
}

void ShowContextMenu(HWND hWnd, POINT pt)
{
    HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXTMENU));
    if (hMenu)
    {
        HMENU hSubMenu = GetSubMenu(hMenu, 0);
        if (hSubMenu)
        {
            // To make the menu disappear when clicking outside
            SetForegroundWindow(hWnd);
            TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
        }
        DestroyMenu(hMenu);
    }
}