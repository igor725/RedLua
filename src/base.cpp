#include "base.h"
#include "thirdparty\easyloggingpp.h"
#include "redlua.h"
#include "menus\main.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <map>

std::map <std::string, LuaScript *> Scripts {};
BOOL hasConsole = false;

bool ScanForNewScripts(void) {
	LOG(INFO) << "Searching for scripts...";
	WIN32_FIND_DATA FindData;
	HANDLE hFind = FindFirstFile("RedLua\\Scripts\\*.lua", &FindData);
	if(hFind == INVALID_HANDLE_VALUE) return false;

	do {
		if(!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			if(!Scripts[FindData.cFileName]) {
				LuaScript *script = new LuaScript(FindData.cFileName);
				std::string error;
				Scripts[FindData.cFileName] = script;

				if(script->Load(error))
					LOG(INFO) << "Script " << FindData.cFileName << " loaded";
				else
					LOG(ERROR) << "Failed to load script " << error;
			}
		}
	} while(FindNextFile(hFind, &FindData));

	FindClose(hFind);
	return true;
}

void ScriptMain(void) {
	el::Configurations conf("RedLua\\log.conf");
	el::Loggers::reconfigureLogger("default", conf);
	if(el::Loggers::getLogger("default")->typedConfigurations()->toStandardOutput(el::Level::Global)) {
		bool alloced = false;
		if(AttachConsole(ATTACH_PARENT_PROCESS) || (alloced = AllocConsole()) == true) {
			if(alloced) SetConsoleTitle("RedLua debug console");
			freopen("CONOUT$", "w", stdout);
			freopen("CONOUT$", "w", stderr);
			hasConsole = true;
		}
	}

	LOG(INFO) << "Logger initialized";
	srand(GetTickCount());
	ScanForNewScripts();

	auto menuController = new MenuController();
	auto mainMenu = CreateMainMenu(menuController);

	while(true) {
		if(MenuInput::MenuSwitchPressed()) {
			MenuInput::MenuInputBeep();
			if(menuController->HasActiveMenu())
				menuController->CloseAttempt();
			else
				menuController->PushMenu(mainMenu);
		}

		menuController->Update();
		for(auto &s : Scripts)
			s.second->OnTick();

		WAIT(0);
	}
}

void ScriptFinish(void) {
	for(auto const & x : Scripts)
		delete x.second;

	Scripts.clear();
	LOG(INFO) << "RedLua stopped";
	fclose(stderr); fclose(stdout);
	if(hasConsole && !FreeConsole())
		LOG(ERROR) << "Failed to free console";
}
