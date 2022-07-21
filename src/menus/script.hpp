#pragma once

#ifndef REDLUA_STANDALONE
#include "thirdparty\scriptmenu.h"

class MenuScripts : public MenuBase {
	virtual void OnPop(void) {
		delete this;
	}

public:
	MenuScripts(MenuItemTitle *title)
		: MenuBase(title) {}
};

MenuScripts *CreateScriptsList(MenuController *controller);
#endif
