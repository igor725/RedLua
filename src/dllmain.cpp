#include "dllmain.h"
#include "base.h"
#include "thirdparty\keyboard.h"
#include "thirdparty\easyloggingpp.h"

INITIALIZE_EASYLOGGINGPP
el::Configurations conf("RedLua\\log.conf");
BOOL hasConsole = false;

BOOL APIENTRY DllMain(HMODULE hInstance, DWORD reason, LPVOID lpReserved) {
	switch(reason) {
		case DLL_PROCESS_ATTACH:
			if(!EnsureDirectory("RedLua")
			|| !EnsureDirectory("RedLua\\Scripts")
			|| !EnsureDirectory("RedLua\\Logs"))
				break;

			el::Loggers::reconfigureLogger("default", conf);
			if(el::Loggers::getLogger("default")->typedConfigurations()->toStandardOutput(el::Level::Global)) {
				if(AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole()) {
					freopen("CONOUT$", "w", stdout);
					freopen("CONOUT$", "w", stderr);
					hasConsole = true;
				}
			}
			LOG(INFO) << "Logger initialized";

			scriptRegister(hInstance, ScriptMain);
			keyboardHandlerRegister(OnKeyboardMessage);
			break;
		case DLL_PROCESS_DETACH:
			scriptUnregister(hInstance);
			keyboardHandlerUnregister(OnKeyboardMessage);
			ScriptFinish();
			fclose(stderr); fclose(stdout);
			if(hasConsole && !FreeConsole())
				LOG(ERROR) << "Failed to free console";
			break;
	}

	return TRUE;
}
