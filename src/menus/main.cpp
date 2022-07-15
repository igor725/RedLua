#include "thirdparty\scriptmenu.h"
#include "menus\settings.h"
#include "menus\script.h"

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

MenuBase *CreateMainMenu(MenuController *controller) {
	auto menu = new MenuBase(new MenuItemTitle("RedLua"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemScripts("Scripts"));
	menu->AddItem(new MenuItemUpdates("Load new scripts"));
	menu->AddItem(new MenuItemMenu("Settings", CreateSettings(controller)));

	return menu;
}
