#pragma once

#include "thirdparty\scriptmenu.h"

class MenuScripts : public MenuBase {
	virtual void OnPop() {
		delete this;
	}

public:
	MenuScripts(MenuItemTitle *title)
		: MenuBase(title) {}
};

MenuScripts *CreateScriptsList(MenuController *controller);
