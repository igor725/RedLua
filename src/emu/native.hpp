#pragma once

#include <Windows.h>

#define WAIT emu_scriptWait
#define nativeInit emu_nativeInit
#define nativePush64 emu_nativePush64
#define nativeCall emu_nativeCall

void emu_scriptWait(DWORD ms);
void emu_nativeInit(UINT64 hash);
void emu_nativePush64(UINT64 val);
PUINT64 emu_nativeCall(void);
