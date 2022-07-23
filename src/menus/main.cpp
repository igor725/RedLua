#include "thirdparty\scriptmenu.h"
#include "menus\settings.hpp"
#include "menus\scripts.hpp"
#include "menus\about.hpp"
#include "constants.hpp"
#include "base.hpp"

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
		if(RedLuaScanScripts())
			SetStatusText("Scripts list updated");
		else
			SetStatusText("Failed to iterate scripts directory");
	}

public:
	MenuItemUpdates(string caption)
		: MenuItemDefault(caption) {}
};

MenuBase *CreateMainMenu(MenuController *controller) {
	auto menu = new MenuBase(new MenuItemTitle(REDLUA_FULLNAME));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemScripts("Scripts"));
	menu->AddItem(new MenuItemUpdates("Refresh scripts"));
	menu->AddItem(new MenuItemMenu("Settings", CreateSettings(controller)));
	menu->AddItem(new MenuItemMenu("About", CreateAbout(controller)));

	return menu;
}
