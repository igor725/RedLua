#include "position.hpp"
#include "settingsctl.hpp"
#include "lang.hpp"

class MenuItemPosition : public MenuItemDefault {
	void OnSelect() {
		GetMenu()->GetController()->SetCurrentPosition(
			Settings.Write("menu_position", m_position)
		);
	}

private:
	int m_position;

public:
	MenuItemPosition(std::string name, int pos)
		: MenuItemDefault(name), m_position(pos) {}
};

MenuBase *CreatePositionMenu(MenuController *controller) {
	auto menu = new MenuBase(new MenuItemTitle(Lng.Get("core.setts.pos")));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemPosition(Lng.Get("core.pos.left"), 0));
	menu->AddItem(new MenuItemPosition(Lng.Get("core.pos.center"), 1));
	menu->AddItem(new MenuItemPosition(Lng.Get("core.pos.right"), 2));

	return menu;
}
