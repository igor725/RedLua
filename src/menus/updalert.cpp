#include "menus\helpers.hpp"
#include "menus\updalert.hpp"
#include "thirdparty\scriptmenu.h"
#include "constants.hpp"
#include "lang.hpp"

void CreateUpdateAlert(MenuController *controller, std::string &version) {
	auto menu = new MenuTemporary(new MenuItemTitle(Lng.Get("core.updalert.nfn", version.c_str())));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemLink(Lng.Get("core.updalert.btn"), REDLUA_RELS_URL + version));
	controller->PushMenu(menu);
}
