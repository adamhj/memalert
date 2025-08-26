#include "SettingsDialog.h"
#include "resource.h"
#include "Settings.h"
#include <wchar.h> // For _wtoi

INT_PTR CALLBACK SettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        // Load current threshold and set it to the edit box
        int threshold = GetAlertThreshold();
        SetDlgItemInt(hDlg, IDC_THRESHOLD_EDIT, threshold, FALSE);

        // Load and set the startup check state
        if (GetStartupEnabled())
        {
            CheckDlgButton(hDlg, IDC_STARTUP_CHECK, BST_CHECKED);
        }
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            // Get value from edit box
            wchar_t buffer[16];
            GetDlgItemTextW(hDlg, IDC_THRESHOLD_EDIT, buffer, 16);
            int newThreshold = _wtoi(buffer);

            // Validate and save
            if (newThreshold > 0 && newThreshold <= 100)
            {
                SetAlertThreshold(newThreshold);
            }
            
            // Save startup check state
            bool startupEnabled = (IsDlgButtonChecked(hDlg, IDC_STARTUP_CHECK) == BST_CHECKED);
            SetStartupEnabled(startupEnabled);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


void ShowSettingsDialog(HWND hParent)
{
    extern HINSTANCE g_hInst; // Use the global instance handle from main.cpp
    DialogBox(g_hInst, MAKEINTRESOURCE(IDD_SETTINGS), hParent, SettingsDlgProc);
}