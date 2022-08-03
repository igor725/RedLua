#include "menus\updates.hpp"
#include "menus\updalert.hpp"
#include "menus\helpers.hpp"
#include "updatesctl.hpp"
#include "constants.hpp"
#include "nativedb.hpp"
#include "thirdparty\keyboard.h"

MenuBase *CreateUpdatesMenu(MenuController *controller) {
	auto menu = new MenuBase(new MenuItemTitle(Lng.Get("core.setts.chkupd")));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemButton(Lng.Get("core.chkupd.rl"), [](auto ctl) {
		std::string newver;
		if (auto code = UpdatesCtl.CheckRedLua(newver))
			if (code == UpdatesController::ERR_NO_UPDATES)
				ctl->SetStatusText(Lng.Get("core.chkupd.nfy.noup"));
			else
				ctl->SetStatusText(Lng.Get("core.chkupd.nfy.upfl", code));
		else
			CreateUpdateAlert(ctl, newver);
	}));
	menu->AddItem(new MenuItemButton(Lng.Get("core.chkupd.ndb"), [](auto ctl) {
		if (auto code = UpdatesCtl.CheckNativeDB(IsKeyDown(VK_CONTROL))) {
			if (code == UpdatesController::ERR_NO_UPDATES)
				ctl->SetStatusText(Lng.Get("core.chkupd.nfy.noup"));
			else
				ctl->SetStatusText(Lng.Get("core.chkupd.nfy.upfl", code));
		} else {
			if (auto ndbcode = Natives.Load())
				ctl->SetStatusText(Lng.Get("core.setts.nfy.nrlfl", ndbcode));
			else
				ctl->SetStatusText(Lng.Get("core.setts.nfy.nrlsc"));
		}
	}));

	return menu;
}
