#include "thirdparty\scriptmenu.h"
#include "menus\settings.hpp"
#include "menus\position.hpp"
#include "menus\updates.hpp"
#include "settingsctl.hpp"
#include "nativedb.hpp"
#include "base.hpp"

class MenuItemSSwitch : public MenuItemSwitchable {
	std::string m_field;
	bool m_initial;

	void OnSelect() {
		SetState(Settings.Switch(m_field, m_initial));
	}

public:
	MenuItemSSwitch(string caption, std::string field, bool initial)
		: MenuItemSwitchable(caption, initial),
		  m_field(field), m_initial(initial) {}
};

class MenuItemReloadDB : public MenuItemDefault {
	void OnSelect() {
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

	void OnSelect() {
		SetStatusText((m_state = !m_state) ? "All scripts were running" : "All scripts have been stopped");
		for(auto &s : Scripts)
			s.second->SetEnabled(m_state);
	}

public:
	MenuItemToggleAll(string caption)
		: MenuItemDefault(caption) {}
};

class MenuItemReloadAll : public MenuItemDefault {
	void OnSelect() {
		SetStatusText("All scripts were reloaded");
		for(auto &s : Scripts)
			s.second->Load();
	}

public:
	MenuItemReloadAll(string caption)
		: MenuItemDefault(caption) {}
};

class MenuItemUnloadAll : public MenuItemDefault {
	void OnSelect() {
		SetStatusText("All scripts were unloaded");
		for (auto it = Scripts.begin(); it != Scripts.end();) {
			delete it->second;
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

	menu->AddItem(new MenuItemSSwitch("Autorun feature enabled", "autorun", Settings.Read("autorun", true)));
	menu->AddItem(new MenuItemSSwitch("Check for updates at startup", "auto_updates", Settings.Read("auto_updates", false)));
	menu->AddItem(new MenuItemReloadDB("Reload NativeDB"));
	menu->AddItem(new MenuItemMenu("Change menu position", CreatePositionMenu(controller)));
	menu->AddItem(new MenuItemToggleAll("Toggle all scripts"));
	menu->AddItem(new MenuItemReloadAll("Reload all scripts"));
	menu->AddItem(new MenuItemUnloadAll("Unload all scripts"));
	menu->AddItem(new MenuItemMenu("Check for updates", CreateUpdatesMenu(controller)));

	return menu;
}
