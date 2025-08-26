#pragma once
#include <windows.h>

// Global instance handle, defined in main.cpp
extern HINSTANCE g_hInst;

// Handle to the modeless settings dialog, NULL if not open
extern HWND g_hSettingsDialog;