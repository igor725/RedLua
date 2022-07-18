#include "menus\about.hpp"
#include <shellapi.h>

class MenuItemLink : public MenuItemDefault {
	string m_link;

	virtual void OnSelect() {
		ShellExecute(0, 0, m_link.c_str(), 0, 0, SW_SHOW);
	}

public:
	MenuItemLink(string caption, string link)
		: MenuItemDefault(caption), m_link(link) {}
};

MenuBase *CreateAbout(MenuController *controller) {
	auto menu = new MenuBase(new MenuItemTitle("About RedLua"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemDefault("Creator: igor725"));
	menu->AddItem(new MenuItemLink("GitHub link", "https://github.com/igor725/RedLua"));
	menu->AddItem(new MenuItemDefault("Check for updates"));

	return menu;
}
