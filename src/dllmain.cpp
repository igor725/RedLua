#include "dllmain.hpp"
#include "base.hpp"
#include "constants.hpp"

#include "thirdparty\keyboard.h"
#include "thirdparty\ScriptHook\inc\main.h"

BOOL DllMain(HMODULE hInstance, DWORD dwReason, LPVOID lpReserved) {
#ifndef REDLUA_STANDALONE
	switch(dwReason) {
		case DLL_PROCESS_ATTACH:
			if(!EnsureDirectory(REDLUA_ROOT_DIR)
			|| !EnsureDirectory(REDLUA_SCRIPTS_DIR)
			|| !EnsureDirectory(REDLUA_LIBS_DIR)
			|| !EnsureDirectory(REDLUA_CLIBS_DIR))
				break;

			scriptRegister(hInstance, RedLuaMain);
			keyboardHandlerRegister(OnKeyboardMessage);
			break;
		case DLL_PROCESS_DETACH:
			scriptUnregister(hInstance);
			keyboardHandlerUnregister(OnKeyboardMessage);
			RedLuaFinish();
			break;
	}
#else
	switch(dwReason) {
		case DLL_PROCESS_ATTACH:
			RedLuaMain();
			break;
		case DLL_PROCESS_DETACH:
			RedLuaFinish();
			break;
	}
#endif

	return TRUE;
}
