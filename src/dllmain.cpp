#include "dllmain.hpp"
#include "base.hpp"
#include "thirdparty\keyboard.h"
#include "thirdparty\ScriptHook\inc\main.h"

BOOL DllMain(HMODULE hInstance, DWORD dwReason, LPVOID lpReserved) {
#ifndef REDLUA_STANDALONE
	switch(dwReason) {
		case DLL_PROCESS_ATTACH:
			if(!EnsureDirectory("RedLua")
			|| !EnsureDirectory("RedLua\\Scripts")
			|| !EnsureDirectory("RedLua\\Logs")
			|| !EnsureDirectory("RedLua\\Libs")
			|| !EnsureDirectory("RedLua\\Libs\\C"))
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
#endif

	return TRUE;
}
