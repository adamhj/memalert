#pragma once
#include <windows.h>

// Custom Window Messages
#define WM_TRAYICON (WM_USER + 1)
#define WM_APP_SHOWSETTINGS (WM_APP + 0)
#define WM_SETTINGS_CHANGED (WM_APP + 1)

// Timer ID
#define IDT_TIMER1 1

// Global instance handle, defined in main.cpp
extern HINSTANCE g_hInst;

// Handle to the main window, defined in main.cpp
extern HWND g_hWnd;

// Handle to the modeless settings dialog, NULL if not open
extern HWND g_hSettingsDialog;