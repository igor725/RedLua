#include "menus\about.hpp"
#include "constants.hpp"
#include <shellapi.h>

class MenuItemLink : public MenuItemDefault {
	string m_link;

	virtual void OnSelect() {
		ShellExecute(0, 0, m_link.c_str(), 0, 0, SW_SHOW);
		SetStatusText("Link will be opened in your default browser in a few moments...");
	}

public:
	MenuItemLink(string caption, string link)
		: MenuItemDefault(caption), m_link(link) {}
};

MenuBase *CreateAbout(MenuController *controller) {
	auto menu = new MenuBase(new MenuItemTitle("About " REDLUA_FULLNAME));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemLink("RedLua: igor725", "https://github.com/igor725/RedLua"));
	menu->AddItem(new MenuItemLink("LuaJIT: Mike Pall", "https://github.com/LuaJIT/LuaJIT"));
	menu->AddItem(new MenuItemLink("UI/ScriptHook: Alexander Blade", "https://www.dev-c.com/"));
	menu->AddItem(new MenuItemLink("NativeDB: alloc8or", "https://github.com/alloc8or/rdr3-nativedb-data"));
	menu->AddItem(new MenuItemLink("JSON parser: Niels Lohmann", "https://github.com/nlohmann/json"));
	menu->AddItem(new MenuItemLink("Logging: abumusamq", "https://github.com/amrayn/easyloggingpp"));

	return menu;
}
