#include "menus\helpers.hpp"
#include "menus\updalert.hpp"
#include "thirdparty\scriptmenu.h"
#include "constants.hpp"

void CreateUpdateAlert(MenuController *controller, std::string &version) {
	auto menu = new MenuTemporary(new MenuItemTitle("RedLua update " + version + " found!"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemLink("Open download page", (std::string)REDLUA_RELS_URL + version));
	controller->PushMenu(menu);
}
