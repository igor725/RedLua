#include "menus\updates.hpp"
#include "menus\updalert.hpp"
#include "menus\helpers.hpp"
#include "updatesctl.hpp"
#include "constants.hpp"
#include "nativedb.hpp"

class MenuItemUpdateRL : public MenuItemDefault {
	virtual void OnSelect(void) {
		std::string data;
		if(UpdatesCtl.CheckRedLua(data))
			CreateUpdateAlert(GetMenu()->GetController(), data);
		else
			SetStatusText(data);
	}

public:
	MenuItemUpdateRL(std::string title)
		: MenuItemDefault(title) {}
};

class MenuItemUpdateDB : public MenuItemDefault {
	virtual void OnSelect(void) {
		std::string err;
		if(UpdatesCtl.CheckNativeDB(err)) {
			NativeDB::Returns ret;
			if((ret = Natives.Load()) == NativeDB::Returns::NLOAD_OK)
				SetStatusText("NativeDB updated and reloaded successfully!");
			else
				SetStatusText("Failed to reload NativeDB, error code: " + std::to_string(ret));
		} else
			SetStatusText(err);
	}

public:
	MenuItemUpdateDB(std::string title)
		: MenuItemDefault(title) {}
};

MenuBase *CreateUpdatesMenu(MenuController *controller) {
	auto menu = new MenuBase(new MenuItemTitle("Version manager"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemUpdateRL("Check for RedLua updates"));
	menu->AddItem(new MenuItemUpdateDB("Check for NativeDB updates"));

	return menu;
}
