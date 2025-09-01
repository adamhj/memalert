#define UNICODE
#define _UNICODE

#include <windows.h>
#include <ntverp.h> // For VER_PRODUCTBUILD
#include <stdio.h>
#include <shellapi.h>
#include "resource.h"
#include "Settings.h"
#include "ToastNotifier.h"
#include "Globals.h"

// Forward declarations
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void AddTrayIcon(HWND hWnd);
void RemoveTrayIcon(HWND hWnd);
void ShowContextMenu(HWND hWnd, POINT pt);
INT_PTR CALLBACK SettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void OpenOrFocusSettingsDialog();
void PerformMemoryCheck();

// Global variables
HINSTANCE g_hInst = NULL;
HWND g_hWnd = NULL;
HWND g_hSettingsDialog = NULL;
ToastNotifier g_notifier;
const wchar_t CLASS_NAME[] = L"MemAlertWindowClass";
const wchar_t WINDOW_TITLE[] = L"MemAlert";

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // Single instance check
    HANDLE hMutex = CreateMutexW(NULL, TRUE, L"MemAlertMutex");
    if (hMutex != NULL && GetLastError() == ERROR_ALREADY_EXISTS)
    {
        HWND hWndExisting = FindWindowW(CLASS_NAME, NULL);
        if (hWndExisting)
        {
            // Use a defined constant for clarity
            PostMessage(hWndExisting, WM_APP_SHOWSETTINGS, 0, 0);
        }
        CloseHandle(hMutex);
        return 0;
    }

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

    // Create the main hidden window.
    g_hWnd = CreateWindowEx(0, CLASS_NAME, WINDOW_TITLE, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);

    if (g_hWnd == NULL)
    {
        MessageBox(NULL, L"Window Creation Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    AddTrayIcon(g_hWnd);
    SetTimer(g_hWnd, IDT_TIMER1, GetCheckFrequency() * 1000, NULL);

    // Perform an initial check immediately on startup
    PerformMemoryCheck();

    // Main message loop
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        // This allows keyboard navigation (Tab, Enter, etc.) in the modeless dialog.
        if (g_hSettingsDialog == NULL || !IsDialogMessage(g_hSettingsDialog, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    CloseHandle(hMutex);
    return 0;
}

void OpenOrFocusSettingsDialog()
{
    if (g_hSettingsDialog)
    {
        // If dialog is already open, just bring it to the front.
        SetForegroundWindow(g_hSettingsDialog);
    }
    else
    {
        // Create the modeless dialog.
        g_hSettingsDialog = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_SETTINGS), g_hWnd, SettingsDlgProc);
        if (g_hSettingsDialog)
        {
            ShowWindow(g_hSettingsDialog, SW_SHOW);
        }
    }
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_APP_SHOWSETTINGS:
        OpenOrFocusSettingsDialog();
        return 0;

    case WM_SETTINGS_CHANGED:
        // Kill the old timer and create a new one with the updated frequency
        KillTimer(hWnd, IDT_TIMER1);
        SetTimer(hWnd, IDT_TIMER1, GetCheckFrequency() * 1000, NULL);
        // Perform a check immediately after settings change
        PerformMemoryCheck();
        return 0;

    case WM_DESTROY:
        KillTimer(hWnd, IDT_TIMER1);
        RemoveTrayIcon(hWnd);
        PostQuitMessage(0);
        return 0;

    case WM_TIMER:
        if (wParam == IDT_TIMER1)
        {
            PerformMemoryCheck();
        }
        return 0;

    case WM_TRAYICON:
        switch (lParam)
        {
        case WM_RBUTTONUP:
        {
            POINT curPoint;
            GetCursorPos(&curPoint);
            ShowContextMenu(hWnd, curPoint);
            break;
        }
        case WM_LBUTTONDBLCLK:
            OpenOrFocusSettingsDialog();
            break;
        }
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_SETTINGS:
            OpenOrFocusSettingsDialog();
            break;
        case IDM_ABOUT:
            MessageBox(hWnd, L"MemAlert Version 1.0", L"About MemAlert", MB_OK | MB_ICONINFORMATION);
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
            SetForegroundWindow(hWnd);
            TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
        }
        DestroyMenu(hMenu);
    }
}

void PerformMemoryCheck()
{
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);

    DWORDLONG totalVirtual = statex.ullTotalPageFile;
    DWORDLONG usedVirtual = statex.ullTotalPageFile - statex.ullAvailPageFile;
    
    if (totalVirtual > 0) {
        int percentage = (int)((usedVirtual * 100) / totalVirtual);
        int threshold = GetAlertThreshold();
        if (percentage >= threshold)
        {
            g_notifier.ShowToast(percentage);
        }
    }
}