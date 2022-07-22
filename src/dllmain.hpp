#pragma once

#include <windows.h>
#include "thirdparty\easyloggingpp.h"

INITIALIZE_EASYLOGGINGPP;
#define EnsureDirectory(D) (!CreateDirectory(D, NULL) ? ERROR_ALREADY_EXISTS == GetLastError() : true)
BOOL APIENTRY DllMain(HMODULE hInstance, DWORD dwReason, LPVOID lpReserved);
