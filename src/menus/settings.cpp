#include "thirdparty\scriptmenu.h"
#include "menus\settings.hpp"
#include "settingsctl.hpp"
#include "nativedb.hpp"
#include "base.hpp"

class MenuItemAutorun : public MenuItemSwitchable {
	virtual void OnSelect() {
		SetState(Settings.Switch("autorun", true));
	}

public:
	MenuItemAutorun(string caption, bool initial)
		: MenuItemSwitchable(caption, initial) {}
};

class MenuItemReloadDB : public MenuItemDefault {
	virtual void OnSelect() {
		NativeDB::Returns ret;
		if((ret = Natives.Load()) == NativeDB::Returns::NLOAD_OK)
			SetStatusText("Natives database has been reloaded");
		else
			SetStatusText("Failed to parse natives.json: " + std::to_string(ret));
	}

public:
	MenuItemReloadDB(string caption)
		: MenuItemDefault(caption) {}
};

class MenuItemToggleAll : public MenuItemDefault {
	bool m_state = true;

	virtual void OnSelect() {
		SetStatusText((m_state = !m_state) ? "All scripts were running" : "All scripts have been stopped");
		for(auto &s : Scripts)
			s.second->SetEnabled(m_state);
	}

public:
	MenuItemToggleAll(string caption)
		: MenuItemDefault(caption) {}
};

class MenuItemReloadAll : public MenuItemDefault {
	virtual void OnSelect() {
		SetStatusText("All scripts were reloaded");
		for(auto &s : Scripts)
			s.second->Load();
	}

public:
	MenuItemReloadAll(string caption)
		: MenuItemDefault(caption) {}
};

class MenuItemUnloadAll : public MenuItemDefault {
	virtual void OnSelect() {
		SetStatusText("All scripts were unloaded");
		for(std::map<std::string, LuaScript *>::iterator it = Scripts.begin(); it != Scripts.end();) {
			delete (*it).second;
			it = Scripts.erase(it);
		}
	}

public:
	MenuItemUnloadAll(string caption)
		: MenuItemDefault(caption) {}
};

MenuBase *CreateSettings(MenuController *controller) {
	auto menu = new MenuBase(new MenuItemTitle("RedLua Settings"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemAutorun("Autorun feature enabled", Settings.Read("autorun", true)));
	menu->AddItem(new MenuItemReloadDB("Reload native database"));
	menu->AddItem(new MenuItemToggleAll("Toggle all scripts"));
	menu->AddItem(new MenuItemReloadAll("Reload all scripts"));
	menu->AddItem(new MenuItemUnloadAll("Unload all scripts"));

	return menu;
}
