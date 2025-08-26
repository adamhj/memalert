#include "StartupManager.h"
#include <windows.h>
#include <shlobj.h>
#include <wrl/client.h>

// Helper function to get the full path to the shortcut in the Startup folder.
HRESULT GetStartupShortcutPath(wchar_t* path, size_t pathSize)
{
    HRESULT hr = SHGetFolderPathW(NULL, CSIDL_STARTUP, NULL, 0, path);
    if (FAILED(hr))
    {
        return hr;
    }
    wcscat_s(path, pathSize, L"\\MemAlert.lnk");
    return S_OK;
}

void UpdateStartupShortcut(bool create)
{
    wchar_t shortcutPath[MAX_PATH];
    if (FAILED(GetStartupShortcutPath(shortcutPath, MAX_PATH)))
    {
        return;
    }

    if (create)
    {
        wchar_t exePath[MAX_PATH];
        DWORD charCount = GetModuleFileNameW(NULL, exePath, ARRAYSIZE(exePath));
        if (charCount == 0) return;

        Microsoft::WRL::ComPtr<IShellLinkW> shellLink;
        HRESULT hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&shellLink));
        if (FAILED(hr)) return;

        shellLink->SetPath(exePath);
        
        Microsoft::WRL::ComPtr<IPersistFile> persistFile;
        hr = shellLink.As(&persistFile);
        if (FAILED(hr)) return;

        persistFile->Save(shortcutPath, TRUE);
    }
    else
    {
        DeleteFileW(shortcutPath);
    }
}

bool IsStartupShortcutEnabled()
{
    wchar_t shortcutPath[MAX_PATH];
    if (FAILED(GetStartupShortcutPath(shortcutPath, MAX_PATH)))
    {
        return false;
    }
    DWORD attributes = GetFileAttributesW(shortcutPath);
    return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
}