#include "base.hpp"
#include "thirdparty\easyloggingpp.h"
#include "redlua.hpp"
#include "menus\main.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <map>

#ifndef REDLUA_STANDALONE
std::map <std::string, LuaScript *> Scripts {};
static BOOL HasConsole = false;

bool ScanForNewScripts(void) {
	LOG(INFO) << "Searching for scripts...";
	WIN32_FIND_DATA findData;
	HANDLE hFind = FindFirstFile("RedLua\\Scripts\\*.lua", &findData);
	if(hFind == INVALID_HANDLE_VALUE) return false;

	do {
		if(!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			if(!Scripts[findData.cFileName]) {
				LuaScript *script = new LuaScript(findData.cFileName);
				std::string error;
				Scripts[findData.cFileName] = script;

				if(script->Load(error))
					LOG(INFO) << "Script " << findData.cFileName << " loaded";
				else
					LOG(ERROR) << "Failed to load script " << error;
			}
		}
	} while(FindNextFile(hFind, &findData));

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
			HasConsole = true;
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
				menuController->PopMenu(0);
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
	if(HasConsole && !FreeConsole())
		LOG(ERROR) << "Failed to free console";
}
#endif
