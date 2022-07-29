#pragma once

#include <Windows.h>

void emu_scriptWait(DWORD ms);
void emu_nativeInit(UINT64 hash);
void emu_nativePush64(UINT64 val);
PUINT64 emu_nativeCall(void);
