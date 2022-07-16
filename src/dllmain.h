#pragma once

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

#define EnsureDirectory(D) (!CreateDirectory(D, NULL) ? ERROR_ALREADY_EXISTS == GetLastError() : true)
