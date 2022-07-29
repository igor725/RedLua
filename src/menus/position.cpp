#include "position.hpp"
#include "settingsctl.hpp"

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
	auto menu = new MenuBase(new MenuItemTitle("RedLua Settings"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemPosition("Left", 0));
	menu->AddItem(new MenuItemPosition("Center", 1));
	menu->AddItem(new MenuItemPosition("Right", 2));

	return menu;
}
