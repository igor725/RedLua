#include "base.hpp"
#include "redlua.hpp"
#include "nativedb.hpp"
#include "settingsctl.hpp"
#include "updatesctl.hpp"
#include "menus\main.hpp"
#include "menus\updalert.hpp"
#include "constants.hpp"

#include "thirdparty\easyloggingpp.h"
#include <windows.h>
#include <map>

#ifndef REDLUA_STANDALONE
std::map <std::string, LuaScript *> Scripts {};
static BOOL HasConsole = false;

bool RedLuaScanScripts(void) {
	LOG(INFO) << "Searching for new scripts...";
	WIN32_FIND_DATA findData;
	std::string scpath = REDLUA_SCRIPTS_DIR;
	HANDLE hFind = FindFirstFile((scpath + "*.lua").c_str(), &findData);
	if(hFind == INVALID_HANDLE_VALUE) return false;
	bool autorun = Settings.Read("autorun", true);

	do {
		if(findData.dwFileAttributes & ~FILE_ATTRIBUTE_DIRECTORY) {
			if(!Scripts[findData.cFileName]) {
				LuaScript *script = new LuaScript(scpath + findData.cFileName);
				Scripts[findData.cFileName] = script;
				if(autorun) script->Load();
				else LOG(WARNING) << "Script " << script->GetPath() << " found but not loaded (autorun disabled)";
			}
		}
	} while(FindNextFile(hFind, &findData));

	FindClose(hFind);
	return true;
}

void RedLuaMain(void) {
	el::Configurations conf (REDLUA_LOGCONF_FILE);
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
	auto menuController = new MenuController();
	auto mainMenu = CreateMainMenu(menuController);
	menuController->SetCurrentPosition(
		Settings.Read("menu_position", 0)
	);
	LOG(DEBUG) << "RedLua menu initialized";

	if(Settings.Read("auto_updates", false)) {
		LOG(DEBUG) << "Starting updates checker...";
		std::string data;
		if(UpdatesCtl.CheckRedLua(data))
			CreateUpdateAlert(menuController, data);
		else
			LOG(DEBUG) << "RedLua updater: " << data;
		if(UpdatesCtl.CheckNativeDB(data))
			LOG(INFO) << "NativeDB updated successfully";
		else
			LOG(DEBUG) << "NativeDB updater: " << data;
		LOG(DEBUG) << "Updates checker finished";
	}

	if(Natives.GetMethodCount() == 0) {
		NativeDB::Returns ret;
		if((ret = Natives.Load()) != NativeDB::Returns::NLOAD_OK)
			LOG(ERROR) << "Failed to load " REDLUA_NATIVES_FILE ": " << ret;
	}

	RedLuaScanScripts();

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
		LOG(ERROR) << "Failed to free the console";
}
#endif
