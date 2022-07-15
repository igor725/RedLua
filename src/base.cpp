#include "base.h"
#include "thirdparty\easyloggingpp.h"
#include "redlua.h"
#include "menus\main.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <map>

std::map <std::string, class LuaScript *> Scripts {};

bool ScanForNewScripts(void) {
	LOG(INFO) << "Searching for scripts...";
	WIN32_FIND_DATA FindData;
	HANDLE hFind = FindFirstFile("RedLua\\Scripts\\*.lua", &FindData);
	if(hFind == INVALID_HANDLE_VALUE) return false;

	do {
		if(!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			if(!Scripts[FindData.cFileName]) {
				class LuaScript *script = new LuaScript(FindData.cFileName);
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
	srand(GetTickCount());
	ScanForNewScripts();

	auto menuController = new MenuController();
	auto mainMenu = CreateMainMenu(menuController);

	while(true) {
		if (!menuController->HasActiveMenu() && MenuInput::MenuSwitchPressed()) {
			MenuInput::MenuInputBeep();
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
}
