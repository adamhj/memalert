#include "Settings.h"
#include "StartupManager.h"
#include <windows.h>
#include <shlobj.h> // For SHGetFolderPathW
#include <stdio.h>  // For swprintf_s

// Path to the INI file, e.g., C:\Users\Username\AppData\Roaming\MemAlert\config.ini
wchar_t g_iniPath[MAX_PATH];

void InitSettings()
{
    // Get the path to the AppData\Roaming folder
    if (FAILED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, g_iniPath)))
    {
        // Fallback to current directory if AppData is not available
        wcscpy_s(g_iniPath, L".\\config.ini");
        return;
    }

    // Append our application's folder
    wcscat_s(g_iniPath, MAX_PATH, L"\\MemAlert");

    // Create the directory if it doesn't exist
    CreateDirectoryW(g_iniPath, NULL);

    // Append the file name
    wcscat_s(g_iniPath, MAX_PATH, L"\\config.ini");
}

int GetAlertThreshold()
{
    // Reads the threshold from the "[Settings]" section, "Threshold" key.
    // If not found, it returns the default value of 85.
    return GetPrivateProfileIntW(L"Settings", L"Threshold", 85, g_iniPath);
}

void SetAlertThreshold(int threshold)
{
    wchar_t buffer[16];
    swprintf_s(buffer, L"%d", threshold);
    WritePrivateProfileStringW(L"Settings", L"Threshold", buffer, g_iniPath);
}

bool GetStartupEnabled()
{
    return GetPrivateProfileIntW(L"Settings", L"Startup", 0, g_iniPath) == 1;
}

void SetStartupEnabled(bool enabled)
{
    WritePrivateProfileStringW(L"Settings", L"Startup", enabled ? L"1" : L"0", g_iniPath);
    UpdateStartupShortcut(enabled);
}

int GetCheckFrequency()
{
    // Default to 5 seconds
    return GetPrivateProfileIntW(L"Settings", L"Frequency", 5, g_iniPath);
}

void SetCheckFrequency(int seconds)
{
    wchar_t buffer[16];
    swprintf_s(buffer, L"%d", seconds);
    WritePrivateProfileStringW(L"Settings", L"Frequency", buffer, g_iniPath);
}