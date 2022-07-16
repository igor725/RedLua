#include "dllmain.h"
#include "base.h"
#include "thirdparty\keyboard.h"
#include "thirdparty\easyloggingpp.h"

INITIALIZE_EASYLOGGINGPP

BOOL APIENTRY DllMain(HMODULE hInstance, DWORD reason, LPVOID lpReserved) {
	switch(reason) {
		case DLL_PROCESS_ATTACH:
			if(!EnsureDirectory("RedLua")
			|| !EnsureDirectory("RedLua\\Scripts")
			|| !EnsureDirectory("RedLua\\Logs")
			|| !EnsureDirectory("RedLua\\Libs")
			|| !EnsureDirectory("RedLua\\Libs\\C"))
				break;

			scriptRegister(hInstance, ScriptMain);
			keyboardHandlerRegister(OnKeyboardMessage);
			break;
		case DLL_PROCESS_DETACH:
			scriptUnregister(hInstance);
			keyboardHandlerUnregister(OnKeyboardMessage);
			ScriptFinish();
			break;
	}

	return TRUE;
}