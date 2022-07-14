#include "script.h"
#include "scriptmenu.h"
#include "redlua.h"
#include "luanative.h"
#include "easyloggingpp.h"
#include <map>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

INITIALIZE_EASYLOGGINGPP

static std::map <std::string, class LuaScript *> Scripts;

static bool ScanForNewScripts(void) {
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

class MenuItemAutorun : public MenuItemSwitchable {
	virtual void OnSelect() {
		bool newState = !GetState();
		// TODO: Переключение авторана
		SetStatusText("Not ready yet");
		SetState(newState);
	}

	virtual void OnFrame() {}

public:
	MenuItemAutorun(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemReloadDB : public MenuItemDefault {
	virtual void OnSelect() {
		NativeReturn ret;
		if((ret = native_reload()) == NLOAD_OK)
			SetStatusText("Natives database has been reloaded");
		else
			SetStatusText("Failed to parse natives.json: " + std::to_string(ret));
	}

public:
	MenuItemReloadDB(string caption)
		: MenuItemDefault(caption) {}
};

class MenuItemReloadAll : public MenuItemDefault {
	virtual void OnSelect() {
		// Перезагрузка Lua скриптов
		SetStatusText("Not ready yet");
	}

public:
	MenuItemReloadAll(string caption)
		: MenuItemDefault(caption) {}
};

class MenuItemUnloadAll : public MenuItemDefault {
	virtual void OnSelect() {
		// Выгрузка Lua скриптов
		SetStatusText("Not ready yet");
	}

public:
	MenuItemUnloadAll(string caption)
		: MenuItemDefault(caption) {}
};

class MenuScripts : public MenuBase {
	virtual void OnPop() {
		delete this;
	}

public:
	MenuScripts(MenuItemTitle *title)
		: MenuBase(title) {}
};

class MenuScript : public MenuBase {
	virtual void OnPop() {
		delete this;
	}

private:
	LuaScript *_script;

public:
	MenuScript(MenuItemTitle *title, LuaScript *script)
		: MenuBase(title) { _script = script; }

	virtual LuaScript *GetScript(void) {
		return _script;
	}
};

class MenuItemStatus : public MenuItemDefault {
	virtual std::string GetCaption(void) {
		LuaScript *scr = ((MenuScript *)GetMenu())->GetScript();
		return MenuItemDefault::GetCaption() +
		std::string(scr->IsLoaded() ? "loaded," : "not loaded,") +
		std::string(scr->HasError() ? " with errors" : " no errors");
	}

public:
	MenuItemStatus(std::string title)
		: MenuItemDefault(title) {}
};

class MenuItemUsage : public MenuItemDefault {
	virtual std::string GetCaption(void) {
		return MenuItemDefault::GetCaption() + _usage + " KB";
	}

	virtual void OnSelect(void) {
		LuaScript *scr = ((MenuScript *)GetMenu())->GetScript();
		_usage = std::to_string(scr->GetMemoryUsage());
	}

private:
	std::string _usage = "[press to update]";

public:
	MenuItemUsage(std::string title)
		: MenuItemDefault(title) {}
};

class MenuItemReload : public MenuItemDefault {
	virtual void OnSelect(void) {
		LuaScript *script = ((MenuScript *)GetMenu())->GetScript();
		std::string error;
		if(script->Load(error))
			SetStatusText("Script successfully reloaded");
		else {
			SetStatusText("Failed to reload script, see logs");
			LOG(ERROR) << "Failed to reload script " + error;
		}
	}

public:
	MenuItemReload(std::string title)
		: MenuItemDefault(title) {}
};

class MenuItemUnload : public MenuItemDefault {
	virtual void OnSelect(void) {
		LuaScript *script = ((MenuScript *)GetMenu())->GetScript();
		for(std::map<std::string, LuaScript *>::iterator it = Scripts.begin(); it != Scripts.end();) {
			if(it->second == script) {
				it = Scripts.erase(it);
			} else {
				it++;
			}
		}
		delete script;
		SetStatusText("Script successfully unloaded");
		GetMenu()->GetController()->PopMenu(2);
	}

public:
	MenuItemUnload(std::string title)
		: MenuItemDefault(title) {}
};

class MenuItemScript : public MenuItemDefault {
	virtual void OnSelect() {
		auto menu = new MenuScript(new MenuItemTitle(this->GetCaption()), _script);
		auto controller = GetMenu()->GetController();
		controller->RegisterMenu(menu);

		menu->AddItem(new MenuItemStatus("Status: "));
		menu->AddItem(new MenuItemUsage("Usage: "));
		menu->AddItem(new MenuItemReload("Reload this script"));
		menu->AddItem(new MenuItemUnload("Unload this script"));

		controller->PushMenu(menu);
	}

private:
	LuaScript *_script;

public:
	MenuItemScript(std::string name, LuaScript *script)
		: MenuItemDefault(name) { _script = script; }
};

static MenuScripts *CreateScriptsList(MenuController *controller) {
	auto menu = new MenuScripts(new MenuItemTitle("Scripts list"));
	controller->RegisterMenu(menu);

	if(Scripts.size() > 0) {
		for(auto const& x : Scripts)
			menu->AddItem(new MenuItemScript(x.first, x.second));
	} else
		menu->AddItem(new MenuItemDefault("No scripts found"));


	return menu;
}

class MenuItemScripts : public MenuItemDefault {
	virtual void OnSelect() {
		if(auto menu = GetMenu())
			if(auto controller = menu->GetController()) {
				auto scrmenu = CreateScriptsList(controller);
				controller->PushMenu(scrmenu);
			}
	}

public:
	MenuItemScripts(string caption)
		: MenuItemDefault(caption) {}
};

class MenuItemUpdates : public MenuItemDefault {
	virtual void OnSelect() {
		if(ScanForNewScripts())
			SetStatusText("Scripts list updated");
		else
			SetStatusText("Failed to iterate scripts directory");
	}

public:
	MenuItemUpdates(string caption)
		: MenuItemDefault(caption) {}
};

MenuBase *CreateSettings(MenuController *controller) {
	auto menu = new MenuBase(new MenuItemTitle("RedLua Settings"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemAutorun("Autorun feature enabled"));
	menu->AddItem(new MenuItemReloadDB("Reload native database"));
	menu->AddItem(new MenuItemReloadAll("Reload all scripts"));
	menu->AddItem(new MenuItemUnloadAll("Unload all scripts"));

	return menu;
}

MenuBase *CreateMainMenu(MenuController *controller) {
	auto menu = new MenuBase(new MenuItemTitle("RedLua"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemScripts("Scripts"));
	menu->AddItem(new MenuItemUpdates("Load new scripts"));
	menu->AddItem(new MenuItemMenu("Settings", CreateSettings(controller)));

	return menu;
}

void main(void) {
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

void ScriptMain(void) {
	srand(GetTickCount());
	main();
}

void ScriptFinish(void) {
	for(auto const & x : Scripts)
		delete x.second;
	
	Scripts.clear();
	LOG(INFO) << "RedLua stopped";
}
