#pragma once

class ToastNotifier
{
public:
    ToastNotifier();
    ~ToastNotifier();
    void ShowToast(int currentUsage);

private:
    bool m_initialized = false;
};