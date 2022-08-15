#include "thirdparty\scriptmenu.h"
#include "menus\settings.hpp"
#include "menus\helpers.hpp"
#include "menus\scripts.hpp"
#include "menus\about.hpp"
#include "constants.hpp"
#include "base.hpp"
#include "langctl.hpp"

MenuBase *CreateMainMenu(MenuController *controller) {
	auto menu = new MenuBase(new MenuItemTitle(REDLUA_FULLNAME));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemButton(Lng.Get("core.main.scripts"), [](auto ctl) {
		if(Scripts.size() > 0)
			ctl->PushMenu(CreateScriptsList(ctl));
		else
			ctl->SetStatusText(Lng.Get("core.scripts.nf"));
	}));
	menu->AddItem(new MenuItemButton(Lng.Get("core.main.refr"), [](auto ctl) {
		if (RedLuaScanScripts())
			ctl->SetStatusText(Lng.Get("core.setts.nfy.srlsc"));
		else
			ctl->SetStatusText(Lng.Get("core.setts.nfy.srlfl"));
	}));
	menu->AddItem(new MenuItemMenu(Lng.Get("core.main.setts"), CreateSettings(controller)));
	menu->AddItem(new MenuItemMenu(Lng.Get("core.main.about"), CreateAbout(controller)));

	return menu;
}
