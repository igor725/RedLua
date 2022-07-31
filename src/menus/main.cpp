#include "thirdparty\scriptmenu.h"
#include "menus\settings.hpp"
#include "menus\scripts.hpp"
#include "menus\about.hpp"
#include "constants.hpp"
#include "base.hpp"
#include "lang.hpp"

class MenuItemScripts : public MenuItemDefault {
	void OnSelect() {
		if (auto menu = GetMenu())
			if (auto controller = menu->GetController()) {
				auto scrmenu = CreateScriptsList(controller);
				controller->PushMenu(scrmenu);
			}
	}

public:
	MenuItemScripts(string caption)
		: MenuItemDefault(caption) {}
};

class MenuItemUpdates : public MenuItemDefault {
	void OnSelect() {
		if (RedLuaScanScripts())
			SetStatusText(Lng.Get("core.setts.nfy.srlsc"));
		else
			SetStatusText(Lng.Get("core.setts.nfy.srlfl"));
	}

public:
	MenuItemUpdates(string caption)
		: MenuItemDefault(caption) {}
};

MenuBase *CreateMainMenu(MenuController *controller) {
	auto menu = new MenuBase(new MenuItemTitle(REDLUA_FULLNAME));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemScripts(Lng.Get("core.main.scripts")));
	menu->AddItem(new MenuItemUpdates(Lng.Get("core.main.refr")));
	menu->AddItem(new MenuItemMenu(Lng.Get("core.main.setts"), CreateSettings(controller)));
	menu->AddItem(new MenuItemMenu(Lng.Get("core.main.about"), CreateAbout(controller)));

	return menu;
}
