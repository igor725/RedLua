#include "menus\updates.hpp"
#include "menus\updalert.hpp"
#include "menus\helpers.hpp"
#include "updatesctl.hpp"
#include "constants.hpp"
#include "nativedb.hpp"
#include "thirdparty\keyboard.h"

class MenuItemUpdateRL : public MenuItemDefault {
	void OnSelect(void) {
		std::string newver;
		if (auto code = UpdatesCtl.CheckRedLua(newver))
			if (code == UpdatesController::Returns::ERR_NO_UPDATES)
				SetStatusText(Lng.Get("core.chkupd.nfy.noup"));
			else
				SetStatusText(Lng.Get("core.chkupd.nfy.upfl", code));
		else
			CreateUpdateAlert(GetMenu()->GetController(), newver);
	}

public:
	MenuItemUpdateRL(std::string title)
		: MenuItemDefault(title) {}
};

class MenuItemUpdateDB : public MenuItemDefault {
	void OnSelect(void) {
		if (auto code = UpdatesCtl.CheckNativeDB(IsKeyDown(VK_CONTROL))) {
			if (code == UpdatesController::Returns::ERR_NO_UPDATES)
				SetStatusText(Lng.Get("core.chkupd.nfy.noup"));
			else
				SetStatusText(Lng.Get("core.chkupd.nfy.upfl", code));
		} else {
			if (auto ndbcode = Natives.Load())
				SetStatusText(Lng.Get("core.setts.nfy.nrlfl", ndbcode));
			else
				SetStatusText(Lng.Get("core.setts.nfy.nrlsc"));
		}
	}

public:
	MenuItemUpdateDB(std::string title)
		: MenuItemDefault(title) {}
};

MenuBase *CreateUpdatesMenu(MenuController *controller) {
	auto menu = new MenuBase(new MenuItemTitle(Lng.Get("core.setts.chkupd")));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemUpdateRL(Lng.Get("core.chkupd.rl")));
	menu->AddItem(new MenuItemUpdateDB(Lng.Get("core.chkupd.ndb")));

	return menu;
}
