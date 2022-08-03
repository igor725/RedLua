#include "base.hpp"
#include "luascript.hpp"
#include "nativedb.hpp"
#include "settingsctl.hpp"
#include "updatesctl.hpp"
#include "menus\main.hpp"
#include "menus\updalert.hpp"
#include "constants.hpp"
#include "langctl.hpp"

#include "thirdparty\easyloggingpp.h"
#include <windows.h>
#include <map>

std::map <std::string, LuaScript *> Scripts {};
static BOOL HasConsole = false;

bool RedLuaScanLangs(void) {
	LOG(INFO) << "Searching for lng files...";
	WIN32_FIND_DATA findData;
	HANDLE hFind = FindFirstFile(REDLUA_LANGS_DIR "*.lng", &findData);
	if (hFind == INVALID_HANDLE_VALUE) return false;
	std::string currLang = "en";
	Settings.Read("menu_language", currLang);

	do {
		if (findData.dwFileAttributes & ~FILE_ATTRIBUTE_DIRECTORY) {
			std::string lngCode = findData.cFileName;
			auto len = lngCode.length();
			if (len > 4) {
				lngCode.erase(lngCode.length() - 4);
				Lng.Load(lngCode);
			}
		}
	} while (FindNextFile(hFind, &findData));

	Lng.Change(currLang);
	return true;
}

bool RedLuaScanScripts(void) {
	LOG(INFO) << "Searching for new scripts...";
	WIN32_FIND_DATA findData;
	std::string scpath = REDLUA_SCRIPTS_DIR;
	HANDLE hFind = FindFirstFile((scpath + "*.lua").c_str(), &findData);
	if (hFind == INVALID_HANDLE_VALUE) return false;
	bool autorun = Settings.Read("autorun", true);

	do {
		if (Scripts[findData.cFileName]) continue;
		LuaScript *script;
		if (findData.dwFileAttributes & ~FILE_ATTRIBUTE_DIRECTORY)
			script = new LuaScript(scpath, findData.cFileName);
		else
			script = new LuaScript(scpath + findData.cFileName, "main.lua", true);
		Scripts[findData.cFileName] = script;
		if (autorun) script->Load();
		else LOG(WARNING) << "Script " << script->GetPath() << " found but not loaded (autorun disabled)";
	} while (FindNextFile(hFind, &findData));

	FindClose(hFind);
	return true;
}

void RedLuaMain(void) {
	el::Configurations conf (REDLUA_LOGCONF_FILE);
	el::Loggers::reconfigureLogger("default", conf);
	el::base::TypedConfigurations *logger;
	logger = el::Loggers::getLogger("default")->typedConfigurations();
	if (logger->enabled(el::Level::Global) && logger->toStandardOutput(el::Level::Global)) {
		if (AttachConsole(ATTACH_PARENT_PROCESS) || (HasConsole = AllocConsole())) {
			if (HasConsole) SetConsoleTitle(REDLUA_NAME " debug console");
			freopen("CONOUT$", "w", stdout);
			freopen("CONOUT$", "w", stderr);
		}
	}

	LOG(INFO) << "Logger initialized";
	auto menuController = new MenuController();
	menuController->SetCurrentPosition(
		Settings.Read("menu_position", 0)
	);
	LOG(DEBUG) << REDLUA_NAME " menu initialized";
	if (Settings.Read("auto_updates", false)) {
		LOG(DEBUG) << "Starting updates checker...";
		std::string nevwer;
		if (auto code = UpdatesCtl.CheckRedLua(nevwer)) {
			if (code != UpdatesController::ERR_NO_UPDATES)
				LOG(ERROR) << "RedLua updater failed: " << code;
		} else
			CreateUpdateAlert(menuController, nevwer);

		if (auto code = UpdatesCtl.CheckNativeDB()) {
			if (code != UpdatesController::ERR_NO_UPDATES)
				LOG(ERROR) << "NativeDB updater failed: " << code;
		} else
			LOG(INFO) << "NativeDB updated successfully";
		LOG(DEBUG) << "Updates checker finished";
	}

	if (Natives.GetMethodCount() == 0)
		if (auto code = Natives.Load())
			LOG(ERROR) << "Failed to load " REDLUA_NATIVES_FILE ": " << code;

	RedLuaScanLangs();
	RedLuaScanScripts();
	MenuBase *mainMenu = nullptr;
	bool needRebuild;

	while (true) {
		if (needRebuild = menuController->IsRebuildRequested()) {
			menuController->UnregisterAll();
			mainMenu = CreateMainMenu(menuController);
		}

		if (MenuInput::MenuSwitchPressed()) {
			MenuInput::MenuInputBeep();
			if (menuController->HasActiveMenu())
				menuController->PopMenu(0);
			else
				menuController->PushMenu(mainMenu);
		}

		menuController->Update();
		for (auto &s : Scripts)
			s.second->OnTick(needRebuild);

		if (needRebuild)
			menuController->RebuildDone();

		WAIT(0);
	}
}

void RedLuaFinish(void) {
	for (auto it = Scripts.begin(); it != Scripts.end();) {
		delete it->second;
		it = Scripts.erase(it);
	}
	Settings.Save();
	UpdatesCtl.Stop();
	LOG(INFO) << "RedLua stopped";
	fclose(stderr); fclose(stdout);
	if (HasConsole && !FreeConsole())
		LOG(ERROR) << "FreeConsole() failed: " << GetLastError();
}
