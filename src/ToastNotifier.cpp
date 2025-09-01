// System / COM
#include <windows.h>
#include <objbase.h> // For CoCreateInstance
#include <roapi.h>   // For RoInitialize, GetActivationFactory

// Shell
#include <shobjidl.h> // For IShellLink
#include <shlobj.h>   // For SHGetFolderPathW, CSIDL_STARTMENU
#include <propkey.h>  // For PKEY_AppUserModel_ID
#include <propvarutil.h> // For InitPropVariantFromString

// WRL
#include <wrl/client.h>
#include <wrl/implements.h>
#include <wrl/wrappers/corewrappers.h>

// WinRT Notifications
#include <windows.ui.notifications.h>

// C-Runtime
#include <stdio.h>

// Helpers
// #include <VersionHelpers.h> // We will use RtlGetVersion instead for more detail

// Local
#include "ToastNotifier.h"

#pragma comment(lib, "runtimeobject.lib")

const wchar_t* AppId = L"Roo.MemAlert";
const wchar_t* ToastTag = L"memalert_status";
const wchar_t* ToastGroup = L"memalert_group";

// Helper function to install the shortcut with AppUserModelID
bool InstallShortcut(PCWSTR shortcutPath) {
    wchar_t exePath[MAX_PATH];
    DWORD charCount = GetModuleFileNameW(NULL, exePath, ARRAYSIZE(exePath));
    if (charCount == 0) return false;

    Microsoft::WRL::ComPtr<IShellLinkW> shellLink;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&shellLink));
    if (FAILED(hr)) return false;

    hr = shellLink->SetPath(exePath);
    if (FAILED(hr)) return false;

    Microsoft::WRL::ComPtr<IPropertyStore> propertyStore;
    hr = shellLink.As(&propertyStore);
    if (FAILED(hr)) return false;

    PROPVARIANT appIdPropVar;
    hr = InitPropVariantFromString(AppId, &appIdPropVar);
    if (FAILED(hr)) return false;

    hr = propertyStore->SetValue(PKEY_AppUserModel_ID, appIdPropVar);
    PropVariantClear(&appIdPropVar); // Clear after use
    if (FAILED(hr)) return false;
    
    hr = propertyStore->Commit();
    if (FAILED(hr)) return false;

    Microsoft::WRL::ComPtr<IPersistFile> persistFile;
    hr = shellLink.As(&persistFile);
    if (FAILED(hr)) return false;

    hr = persistFile->Save(shortcutPath, TRUE);
    return SUCCEEDED(hr);
}


::ToastNotifier::ToastNotifier()
{
    if (SUCCEEDED(Windows::Foundation::Initialize(RO_INIT_MULTITHREADED)))
    {
        m_initialized = true;

        wchar_t shortcutPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_STARTMENU, NULL, 0, shortcutPath))) {
            wcscat_s(shortcutPath, L"\\Programs\\MemAlert.lnk");
            InstallShortcut(shortcutPath);
        }
    }
}

::ToastNotifier::~ToastNotifier()
{
    if (m_initialized)
    {
        Windows::Foundation::Uninitialize();
    }
}

bool IsModernNotificationSystemAvailable()
{
    using RtlGetVersionPtr = NTSTATUS(WINAPI*)(PRTL_OSVERSIONINFOW);

    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (hNtdll)
    {
        RtlGetVersionPtr pRtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hNtdll, "RtlGetVersion");
        if (pRtlGetVersion)
        {
            RTL_OSVERSIONINFOW rovi = { sizeof(rovi) };
            if (pRtlGetVersion(&rovi) == 0) // STATUS_SUCCESS
            {
                // Windows 10 is major version 10.
                // Windows 11 reports as Windows 10, but with a high build number (>= 22000).
                if (rovi.dwMajorVersion > 10) return true;
                if (rovi.dwMajorVersion == 10 && rovi.dwBuildNumber >= 14393) return true;
            }
        }
    }
    return false;
}

void ::ToastNotifier::ShowToast(int currentUsage)
{
    if (!m_initialized) return;

    Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotificationManagerStatics> toastStatics;
    HRESULT hr = Windows::Foundation::GetActivationFactory(Microsoft::WRL::Wrappers::HStringReference(RuntimeClass_Windows_UI_Notifications_ToastNotificationManager).Get(), &toastStatics);
    if (FAILED(hr)) return;

    // For older versions of Windows, the automatic replacement via Tag/Group might not work.
    // As a fallback, we manually remove the previous notification.
    if (!IsModernNotificationSystemAvailable())
    {
        Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotificationManagerStatics2> toastStatics2;
        if (SUCCEEDED(toastStatics.As(&toastStatics2)))
        {
            Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotificationHistory> history;
            if (SUCCEEDED(toastStatics2->get_History(&history)))
            {
                history->RemoveGroupedTagWithId(Microsoft::WRL::Wrappers::HStringReference(ToastTag).Get(), Microsoft::WRL::Wrappers::HStringReference(ToastGroup).Get(), Microsoft::WRL::Wrappers::HStringReference(AppId).Get());
            }
        }
    }

    Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotifier> notifier;
    hr = toastStatics->CreateToastNotifierWithId(Microsoft::WRL::Wrappers::HStringReference(AppId).Get(), &notifier);
    if (FAILED(hr)) return;

    Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlDocument> xml;
    hr = toastStatics->GetTemplateContent(ABI::Windows::UI::Notifications::ToastTemplateType_ToastText02, &xml);
    if (FAILED(hr)) return;

    Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNodeList> textNodes;
    hr = xml->GetElementsByTagName(Microsoft::WRL::Wrappers::HStringReference(L"text").Get(), &textNodes);
    if (FAILED(hr)) return;

    Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNode> textNode;
    textNodes->Item(0, &textNode);
    Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlText> text;
    Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNode> textChild;
    hr = xml->CreateTextNode(Microsoft::WRL::Wrappers::HStringReference(L"Memory Usage Alert").Get(), &text);
    if (SUCCEEDED(hr))
    {
        Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNode> nodeToAppend;
        if (SUCCEEDED(text.As(&nodeToAppend)))
        {
            textNode->AppendChild(nodeToAppend.Get(), &textChild);
        }
    }

    wchar_t usageText[100];
    swprintf_s(usageText, L"Commit charge has reached %d%%!", currentUsage);
    textNodes->Item(1, &textNode);
    xml->CreateTextNode(Microsoft::WRL::Wrappers::HStringReference(usageText).Get(), &text);
    if (SUCCEEDED(hr))
    {
        Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNode> nodeToAppend;
        if (SUCCEEDED(text.As(&nodeToAppend)))
        {
            textNode->AppendChild(nodeToAppend.Get(), &textChild);
        }
    }

    Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotificationFactory> toastFactory;
    hr = Windows::Foundation::GetActivationFactory(Microsoft::WRL::Wrappers::HStringReference(RuntimeClass_Windows_UI_Notifications_ToastNotification).Get(), &toastFactory);
    if (FAILED(hr)) return;

    Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotification> toast;
    hr = toastFactory->CreateToastNotification(xml.Get(), &toast);
    if (FAILED(hr)) return;

    // Set Tag and Group so we can find it later to remove it.
    Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotification2> toast2;
    if (SUCCEEDED(toast.As(&toast2)))
    {
        toast2->put_Tag(Microsoft::WRL::Wrappers::HStringReference(ToastTag).Get());
        toast2->put_Group(Microsoft::WRL::Wrappers::HStringReference(ToastGroup).Get());
    }

    notifier->Show(toast.Get());
}