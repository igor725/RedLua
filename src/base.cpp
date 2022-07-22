#include "base.hpp"
#include "thirdparty\easyloggingpp.h"
#include "redlua.hpp"
#include "settingsctl.hpp"
#include "menus\main.hpp"
#include <windows.h>
#include <map>

#ifndef REDLUA_STANDALONE
std::map <std::string, LuaScript *> Scripts {};
static BOOL HasConsole = false;

bool RedLuaScanScripts(void) {
	LOG(INFO) << "Searching for scripts...";
	WIN32_FIND_DATA findData;
	std::string scripts = "RedLua\\Scripts\\";
	HANDLE hFind = FindFirstFile((scripts + "*.lua").c_str(), &findData);
	if(hFind == INVALID_HANDLE_VALUE) return false;
	bool autorun = Settings.Read("autorun", true);

	do {
		if(findData.dwFileAttributes & ~FILE_ATTRIBUTE_DIRECTORY) {
			if(!Scripts[findData.cFileName]) {
				LuaScript *script = new LuaScript(scripts + findData.cFileName);
				Scripts[findData.cFileName] = script;
				if(autorun) script->Load();
				else LOG(INFO) << "Script " << script->GetPath() << " found but not loaded";
			}
		}
	} while(FindNextFile(hFind, &findData));

	FindClose(hFind);
	return true;
}

void RedLuaMain(void) {
	el::Configurations conf ("RedLua\\Log.conf");
	el::Loggers::reconfigureLogger("default", conf);
	el::base::TypedConfigurations *logger;
	logger = el::Loggers::getLogger("default")->typedConfigurations();
	if(logger->enabled(el::Level::Global) && logger->toStandardOutput(el::Level::Global)) {
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
	RedLuaScanScripts();

	auto menuController = new MenuController();
	auto mainMenu = CreateMainMenu(menuController);
	menuController->SetCurrentPosition(
		Settings.Read("menu_position", 0)
	);

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

void RedLuaFinish(void) {
	for (auto it = Scripts.begin(); it != Scripts.end();) {
		delete it->second;
		it = Scripts.erase(it);
	}
	Settings.Save();
	LOG(INFO) << "RedLua stopped";
	fclose(stderr); fclose(stdout);
	if(HasConsole && !FreeConsole())
		LOG(ERROR) << "Failed to free console";
}
#endif
