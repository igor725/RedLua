#pragma once

#include <windows.h>

#define EnsureDirectory(D) (!CreateDirectory(D, NULL) ? ERROR_ALREADY_EXISTS == GetLastError() : true)
