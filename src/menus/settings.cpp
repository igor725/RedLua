#include "thirdparty\scriptmenu.h"
#include "thirdparty\keyboard.h"
#include "menus\settings.hpp"
#include "menus\position.hpp"
#include "menus\updates.hpp"
#include "menus\langlist.hpp"
#include "menus\helpers.hpp"
#include "settingsctl.hpp"
#include "nativedb.hpp"
#include "natives.hpp"
#include "base.hpp"
#include "langctl.hpp"

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

MenuBase *CreateSettings(MenuController *controller) {
	auto menu = new MenuBase(new MenuItemTitle(Lng.Get("core.main.setts")));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemButton(Lng.Get("core.setts.hotkey"), [](auto ctl) {
		int kcode = Settings.Read("menu_hotkey", REDLUA_HOTKEY_DEFAULT);
		if (kcode < 0 || kcode >= 255) kcode = REDLUA_HOTKEY_DEFAULT;
		auto name = KeyNames[kcode]; if (!name) name = "";
		NATIVES::SHOW_KEYBOARD(
#			ifdef REDLUA_GTAV
				6, "FMMC_KEY_TIP8S",
#			else
				5, "STABLE_RENAME_MOUNT_PROMPT",
#			endif
			KeyNames[kcode], 10
		);
		int status;
		while ((status = NATIVES::UPDATE_KEYBOARD()) == 0) WAIT(0);
		auto out = NATIVES::RESULT_KEYBOARD();
		if (!out) return;
		if (auto outi = std::atoi(out)) {
			if (outi < 255 && KeyNames[outi]) {
				Settings.Write("menu_hotkey", outi);
				ctl->SetStatusText(Lng.Get("core.setts.nfy.hksc", KeyNames[outi]));
			} else
				ctl->SetStatusText(Lng.Get("core.setts.nfy.hkin", outi));
			return;
		}
		for (int i = 0; i < 255; i++) {
			if (auto key = KeyNames[i]) {
				auto iof = std::strstr(key, out) - key;
				auto lendiff = std::strlen(key) - std::strlen(out);
				if ((iof == 0 && lendiff == 0) || (iof == 3 && lendiff == 3)) {
					Settings.Write("menu_hotkey", i);
					ctl->SetStatusText(Lng.Get("core.setts.nfy.hksc", key));
					return;
				}
			}
		}
		ctl->SetStatusText(Lng.Get("core.setts.nfy.hkfl"));
	}));
	menu->AddItem(new MenuItemSSwitch(Lng.Get("core.setts.autorun"), "autorun", Settings.Read("autorun", true)));
	menu->AddItem(new MenuItemSSwitch(Lng.Get("core.setts.updater"), "auto_updates", Settings.Read("auto_updates", false)));
	menu->AddItem(new MenuItemMenu(Lng.Get("core.setts.chkupd"), CreateUpdatesMenu(controller)));
	menu->AddItem(new MenuItemMenu(Lng.Get("core.setts.langs"), CreateLangsMenu(controller)));
	menu->AddItem(new MenuItemButton(Lng.Get("core.setts.rldndb"), [](auto ctl) {
		if (auto code = Natives.Load())
			ctl->SetStatusText(Lng.Get("core.setts.nfy.nrlfl", code));
		else
			ctl->SetStatusText(Lng.Get("core.setts.nfy.nrlsc"));
	}));
	menu->AddItem(new MenuItemMenu(Lng.Get("core.setts.pos"), CreatePositionMenu(controller)));
	menu->AddItem(new MenuItemToggleAll(Lng.Get("core.setts.toggall")));
	menu->AddItem(new MenuItemButton(Lng.Get("core.setts.relall"), [](auto ctl) {
		ctl->SetStatusText(Lng.Get("core.setts.nfy.relall"));
		for (auto &s : Scripts)
			s.second->Load();
	}));
	menu->AddItem(new MenuItemButton(Lng.Get("core.setts.unlall"), [](auto ctl) {
		ctl->SetStatusText(Lng.Get("core.setts.nfy.unlall"));
		for (auto it = Scripts.begin(); it != Scripts.end();) {
			delete it->second;
			it = Scripts.erase(it);
		}
	}));

	return menu;
}
