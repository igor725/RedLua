#include "thirdparty\scriptmenu.h"
#include "menus\settings.hpp"
#include "menus\position.hpp"
#include "menus\updates.hpp"
#include "menus\langlist.hpp"
#include "settingsctl.hpp"
#include "nativedb.hpp"
#include "base.hpp"
#include "lang.hpp"

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
		if (auto code = Natives.Load())
			SetStatusText(Lng.Get("core.setts.nfy.nrlfl", code));
		else
			SetStatusText(Lng.Get("core.setts.nfy.nrlsc"));
	}

public:
	MenuItemReloadDB(string caption)
		: MenuItemDefault(caption) {}
};

class MenuItemToggleAll : public MenuItemSwitchable {
	void OnSelect() {
		bool state = !GetState(); SetState(state);
		SetStatusText(state ? Lng.Get("core.setts.nfy.runall") : Lng.Get("core.setts.nfy.stpall"));
		for (auto &s : Scripts)
			s.second->SetEnabled(state);
	}

public:
	MenuItemToggleAll(string caption)
		: MenuItemSwitchable(caption, true) {}
};

class MenuItemReloadAll : public MenuItemDefault {
	void OnSelect() {
		SetStatusText(Lng.Get("core.setts.nfy.relall"));
		for (auto &s : Scripts)
			s.second->Load();
	}

public:
	MenuItemReloadAll(string caption)
		: MenuItemDefault(caption) {}
};

class MenuItemUnloadAll : public MenuItemDefault {
	void OnSelect() {
		SetStatusText(Lng.Get("core.setts.nfy.unlall"));
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
	auto menu = new MenuBase(new MenuItemTitle(Lng.Get("core.main.setts")));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemSSwitch(Lng.Get("core.setts.autorun"), "autorun", Settings.Read("autorun", true)));
	menu->AddItem(new MenuItemSSwitch(Lng.Get("core.setts.updater"), "auto_updates", Settings.Read("auto_updates", false)));
	menu->AddItem(new MenuItemMenu(Lng.Get("core.setts.chkupd"), CreateUpdatesMenu(controller)));
	menu->AddItem(new MenuItemMenu(Lng.Get("core.setts.langs"), CreateLangsMenu(controller)));
	menu->AddItem(new MenuItemReloadDB(Lng.Get("core.setts.rldndb")));
	menu->AddItem(new MenuItemMenu(Lng.Get("core.setts.pos"), CreatePositionMenu(controller)));
	menu->AddItem(new MenuItemToggleAll(Lng.Get("core.setts.toggall")));
	menu->AddItem(new MenuItemReloadAll(Lng.Get("core.setts.relall")));
	menu->AddItem(new MenuItemUnloadAll(Lng.Get("core.setts.unlall")));

	return menu;
}
