#include "SettingsDialog.h"
#include "resource.h"
#include "Settings.h"
#include "Globals.h"
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

        // Load current frequency and set it to the edit box
        int frequency = GetCheckFrequency();
        SetDlgItemInt(hDlg, IDC_FREQUENCY_EDIT, frequency, FALSE);

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

            // Get and save frequency
            GetDlgItemTextW(hDlg, IDC_FREQUENCY_EDIT, buffer, 16);
            int newFrequency = _wtoi(buffer);
            if (newFrequency >= 1) // At least 1 second
            {
                SetCheckFrequency(newFrequency);
            }

            DestroyWindow(hDlg);
            return (INT_PTR)TRUE;
        }
        case IDCANCEL:
            DestroyWindow(hDlg);
            return (INT_PTR)TRUE;
        }
        break;
    case WM_DESTROY:
        // This is important for modeless dialogs to signal they are closed.
        g_hSettingsDialog = NULL;
        break;
    }
    return (INT_PTR)FALSE;
}