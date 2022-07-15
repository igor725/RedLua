#include "thirdparty\scriptmenu.h"
#include "menus\settings.h"

class MenuItemAutorun : public MenuItemSwitchable {
	virtual void OnSelect() {
		bool newState = !GetState();
		// TODO: Переключение авторана
		SetStatusText("Not ready yet");
		SetState(newState);
	}

	virtual void OnFrame() {}

public:
	MenuItemAutorun(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemReloadDB : public MenuItemDefault {
	virtual void OnSelect() {
		NativeReturn ret;
		if((ret = native_reload()) == NLOAD_OK)
			SetStatusText("Natives database has been reloaded");
		else
			SetStatusText("Failed to parse natives.json: " + std::to_string(ret));
	}

public:
	MenuItemReloadDB(string caption)
		: MenuItemDefault(caption) {}
};

class MenuItemReloadAll : public MenuItemDefault {
	virtual void OnSelect() {
		// Перезагрузка Lua скриптов
		SetStatusText("Not ready yet");
	}

public:
	MenuItemReloadAll(string caption)
		: MenuItemDefault(caption) {}
};

class MenuItemUnloadAll : public MenuItemDefault {
	virtual void OnSelect() {
		// Выгрузка Lua скриптов
		SetStatusText("Not ready yet");
	}

public:
	MenuItemUnloadAll(string caption)
		: MenuItemDefault(caption) {}
};

MenuBase *CreateSettings(MenuController *controller) {
	auto menu = new MenuBase(new MenuItemTitle("RedLua Settings"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemAutorun("Autorun feature enabled"));
	menu->AddItem(new MenuItemReloadDB("Reload native database"));
	menu->AddItem(new MenuItemReloadAll("Reload all scripts"));
	menu->AddItem(new MenuItemUnloadAll("Unload all scripts"));

	return menu;
}
