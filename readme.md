[Read this in Chinese (简体中文)](./readme_chs.md)

---
# MemAlert

A lightweight, system-tray-based memory monitoring tool for Windows that alerts you when memory usage gets too high.

## What It Does

MemAlert runs silently in the background, monitoring your system's **Commit Charge** (the total amount of virtual memory that the system has committed). When the commit charge exceeds a user-defined threshold, it sends a toast notification to warn you.

This is particularly useful for preventing system slowdowns or crashes due to excessive memory allocation by applications.

## Features

- **System Tray Icon**: Lives in the system tray for unobtrusive background operation.
- **Memory Monitoring**: Periodically checks the system's memory commit charge.
- **Threshold Alerts**: Displays a Windows toast notification when usage surpasses a configured percentage.
- **GUI for Settings**: A simple settings dialog allows you to configure:
  - **Warning Threshold (%)**: The memory usage percentage that triggers an alert.
  - **Check Frequency (seconds)**: How often to check memory usage.
  - **Run on Startup**: Automatically start MemAlert when you log in to Windows.
- **Single Instance**: Ensures only one instance of the application can run at a time.
- **Portable**: Settings are stored in an `.ini` file located in `%APPDATA%\MemAlert`, leaving your registry untouched.

## Building from Source

### Prerequisites

- A C++ development environment for Windows, such as Visual Studio.
- The `nmake` build tool (available in the Developer Command Prompt for Visual Studio).

### Steps

1.  Open a Developer Command Prompt for Visual Studio.
2.  Navigate to the project's root directory.
3.  Run the build command:
    ```shell
    nmake
    ```
4.  The compiled executable (`memalert.exe`) will be placed in the `bin/` directory. Intermediate object files are stored in `build/`.

## How to Use

1.  Run `bin/memalert.exe`.
2.  An icon will appear in your system tray.
3.  **Right-click** the tray icon to open a context menu with "Settings" and "Exit" options.
4.  **Double-click** the tray icon to directly open the Settings dialog.
5.  Configure your desired threshold and check frequency in the settings window.

## Project Structure

```
.
├── bin/              # Contains the final compiled executable
├── build/            # Stores intermediate object files
├── doc/              # Project design documents
├── src/              # All source code, headers, and resource files
├── .gitignore
├── handover_notes.md
├── Makefile          # NMAKE-compatible makefile for building the project
└── readme.md