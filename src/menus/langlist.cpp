#include "menus\langlist.hpp"
#include "lang.hpp"

class MenuItemLang : public MenuItemDefault {
	std::string m_langCode;

	void OnSelect() {
		Lng.Change(m_langCode);
		Settings.Write("menu_language", m_langCode);
		SetStatusText(Lng.Get("core.langs.nfy.chsc"));
		GetMenu()->GetController()->RequestRebuild();
	}

public:
	MenuItemLang(std::string name, std::string code)
	: MenuItemDefault(name), m_langCode(code) {}
};

MenuBase *CreateLangsMenu(MenuController *controller) {
	auto menu = new MenuBase(new MenuItemTitle(Lng.Get("core.setts.langs")));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemLang(Lng.Get("core.langs.ingame"), "ingame"));

	for (auto &it : Lng.GetMap())
		menu->AddItem(new MenuItemLang(it.second.f_locName, it.first));

	return menu;
}
