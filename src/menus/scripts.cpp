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

class MenuItemStatus : public MenuItemDefault {
	std::string GetCaption(void) {
		LuaScript *scr = ((MenuScript *)GetMenu())->GetScript();
		std::string state;
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
		: MenuItemDefault("") {}
};

class MenuItemUsage : public MenuItemDefault {
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
		: MenuItemDefault("") {}
};

class MenuItemReload : public MenuItemDefault {
	void OnSelect(void) {
		LuaScript *script = ((MenuScript *)GetMenu())->GetScript();
		if (script->Load())
			SetStatusText(Lng.Get("core.script.nfy.relsc"));
		else
			SetStatusText(Lng.Get("core.script.nfy.relfl"));
	}

public:
	MenuItemReload(std::string title)
		: MenuItemDefault(title) {}
};

class MenuItemUnload : public MenuItemDefault {
	void OnSelect(void) {
		LuaScript *script = ((MenuScript *)GetMenu())->GetScript();
		for (auto it = Scripts.begin(); it != Scripts.end();) {
			if (it->second == script) {
				it = Scripts.erase(it);
			} else {
				it++;
			}
		}
		delete script;
		SetStatusText(Lng.Get("core.script.nfy.unlsucc"));
		GetMenu()->GetController()->PopMenu(2);
	}

public:
	MenuItemUnload(std::string title)
		: MenuItemDefault(title) {}
};

class MenuItemToggle : public MenuItemDefault {
	void OnSelect(void) {
		LuaScript *script = ((MenuScript *)GetMenu())->GetScript();
		script->SetEnabled(!script->IsEnabled());
	}

public:
	MenuItemToggle(std::string title)
		: MenuItemDefault(title) {}
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

	if (Scripts.size() > 0) {
		for (auto &x : Scripts)
			menu->AddItem(new MenuItemScript(x.first, x.second));
	} else
		menu->AddItem(new MenuItemDefault(Lng.Get("core.scripts.nf")));

	return menu;
}
