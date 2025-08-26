# Compiler and tools
CC = cl.exe
RC = rc.exe
LINK = link.exe

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin
TARGET = $(BIN_DIR)/memalert.exe

# Source and object files (explicitly listed for NMAKE)
SRCS = $(SRC_DIR)/main.cpp \
       $(SRC_DIR)/Settings.cpp \
       $(SRC_DIR)/SettingsDialog.cpp \
       $(SRC_DIR)/StartupManager.cpp \
       $(SRC_DIR)/ToastNotifier.cpp

OBJS = $(BUILD_DIR)/main.obj \
       $(BUILD_DIR)/Settings.obj \
       $(BUILD_DIR)/SettingsDialog.obj \
       $(BUILD_DIR)/StartupManager.obj \
       $(BUILD_DIR)/ToastNotifier.obj

RES = $(BUILD_DIR)/memalert.res

# Compiler and Linker Flags
CFLAGS = /W4 /nologo /EHsc /D_UNICODE /DUNICODE /D_WIN32_WINNT=0x0A00
LIBS = user32.lib shell32.lib runtimeobject.lib gdi32.lib
LDFLAGS = /nologo /SUBSYSTEM:WINDOWS /OUT:$(TARGET)

# --- Targets ---

all: $(TARGET)

$(TARGET): $(OBJS) $(RES)
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	@echo Linking $@...
	$(LINK) $(LDFLAGS) $(OBJS) $(RES) $(LIBS)

# Inference rule for NMAKE: how to build .obj files from .cpp files
{$(SRC_DIR)}.cpp{$(BUILD_DIR)}.obj:
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	@echo Compiling $<...
	$(CC) $(CFLAGS) /c /Fo$@ $<

$(RES): $(SRC_DIR)/memalert.rc $(SRC_DIR)/resource.h $(SRC_DIR)/app.ico
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	@echo Compiling resources...
	$(RC) /fo$@ $(SRC_DIR)/memalert.rc

clean:
	@if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	@if exist $(BIN_DIR) rmdir /s /q $(BIN_DIR)
	@echo Cleaned build and bin directories.

.PHONY: all clean