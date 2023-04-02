#include "dllmain.hpp"
#include "base.hpp"
#include "constants.hpp"

#include "thirdparty\keyboard.h"
#include "scripthook.hpp"
BOOL registred = FALSE;

BOOL DllMain(HMODULE hInstance, DWORD dwReason, LPVOID lpReserved) {
#ifndef REDLUA_STANDALONE
	switch (dwReason) {
		case DLL_PROCESS_ATTACH:
			tryagain:
			if (!EnsureDirectory(REDLUA_ROOT_DIR)
			|| !EnsureDirectory(REDLUA_SCRIPTS_DIR)
			|| !EnsureDirectory(REDLUA_LANGS_DIR)
			|| !EnsureDirectory(REDLUA_LIBS_DIR)
			|| !EnsureDirectory(REDLUA_CLIBS_DIR)) {
				switch (MessageBox(NULL, "Failed to create RedLua "
					"directory, please set write permissions "
					"to the root folder of the game then press "
					"\"Continue\".", REDLUA_FULLNAME,
					MB_ICONERROR | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
				)) {
					case IDCANCEL:
						ExitProcess(ERROR_ACCESS_DENIED);
						return FALSE;
					case IDTRYAGAIN:
						goto tryagain;
					case IDCONTINUE:
						return FALSE;
				}
			}

			registred = TRUE;
			scriptRegister(hInstance, RedLuaMain);
			keyboardHandlerRegister(OnKeyboardMessage);
			break;
		case DLL_PROCESS_DETACH:
			if (registred) {
				scriptUnregister(hInstance);
				keyboardHandlerUnregister(OnKeyboardMessage);
				RedLuaFinish();
			}
			break;
	}
#else
	switch (dwReason) {
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
