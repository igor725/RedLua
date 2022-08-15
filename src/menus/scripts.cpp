#include "base.hpp"
#include "luascript.hpp"
#include "menus\scripts.hpp"

class MenuScript : public MenuTemporary {
private:
	LuaScript *m_script;

public:
	MenuScript(MenuItemTitle *title, LuaScript *script)
		: MenuTemporary(title), m_script(script) {}

	LuaScript *GetScript(void) {
		return m_script;
	}
};

class MenuItemGenLua : public MenuItemDefault {
public:
	MenuItemGenLua(void) : MenuItemDefault("") {};
	MenuItemGenLua(std::string caption) : MenuItemDefault(caption) {};

	LuaScript *GetScript(void) {
		return dynamic_cast<MenuScript *>(GetMenu())->GetScript();
	}
};

class MenuItemStatus : public MenuItemGenLua {
	std::string GetCaption(void) {
		std::string state;
		auto scr = GetScript();
		if (scr->IsEnabled())
			state = Lng.Get("core.script.state1");
		else if (scr->HasError())
			state = Lng.Get("core.script.state2");
		else
			state = Lng.Get("core.script.state3");
		return Lng.Get("core.script.state", state.c_str());
	}

public:
	MenuItemStatus()
		: MenuItemGenLua() {}
};

class MenuItemUsage : public MenuItemGenLua {
	std::string m_usage = Lng.Get("core.script.rvlusage");

	std::string GetCaption(void) {
		return m_usage;
	}

	void OnSelect(void) {
		LuaScript *scr = ((MenuScript *)GetMenu())->GetScript();
		m_usage = Lng.Get("core.script.usage", scr->GetMemoryUsage());
	}

public:
	MenuItemUsage()
		: MenuItemGenLua() {}
};

class MenuItemReload : public MenuItemGenLua {
	void OnSelect(void) {
		if (GetScript()->Load())
			SetStatusText(Lng.Get("core.script.nfy.relsc"));
		else
			SetStatusText(Lng.Get("core.script.nfy.relfl"));
	}

public:
	MenuItemReload(std::string title)
		: MenuItemGenLua(title) {}
};

class MenuItemUnload : public MenuItemGenLua {
	void OnSelect(void) {
		auto scr = GetScript();
		for (auto it = Scripts.begin(); it != Scripts.end();) {
			if (it->second == scr) {
				it = Scripts.erase(it);
			} else {
				it++;
			}
		}
		delete scr;
		SetStatusText(Lng.Get("core.script.nfy.unlsucc"));
		GetMenu()->GetController()->PopMenu(2);
	}

public:
	MenuItemUnload(std::string title)
		: MenuItemGenLua(title) {}
};

class MenuItemToggle : public MenuItemGenLua {
	void OnSelect(void) {
		auto scr = GetScript();
		scr->SetEnabled(!scr->IsEnabled());
	}

public:
	MenuItemToggle(std::string title)
		: MenuItemGenLua(title) {}
};

class MenuItemScript : public MenuItemDefault {
	void OnSelect() {
		auto menu = new MenuScript(new MenuItemTitle(this->GetCaption()), script);
		auto controller = GetMenu()->GetController();
		controller->RegisterMenu(menu);

		menu->AddItem(new MenuItemStatus());
		menu->AddItem(new MenuItemUsage());

		auto scrmenu = script->GetMyMenu();
		if (scrmenu) {
			controller->RegisterMenu(scrmenu);
			menu->AddItem(new MenuItemMenu(Lng.Get("core.script.ownmenu"), scrmenu));
		}

		menu->AddItem(new MenuItemToggle(Lng.Get("core.script.tgl")));
		menu->AddItem(new MenuItemReload(Lng.Get("core.script.rel")));
		menu->AddItem(new MenuItemUnload(Lng.Get("core.script.unl")));

		controller->PushMenu(menu);
	}

private:
	LuaScript *script;

public:
	MenuItemScript(std::string name, LuaScript *script)
		: MenuItemDefault(name), script(script) {}
};

MenuTemporary *CreateScriptsList(MenuController *controller) {
	auto menu = new MenuTemporary(new MenuItemListTitle(Lng.Get("core.main.scripts")));
	controller->RegisterMenu(menu);
	for (auto &x : Scripts)
		menu->AddItem(new MenuItemScript(x.first, x.second));
	return menu;
}
