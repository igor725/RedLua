#include "base.hpp"
#include "redlua.hpp"
#include "menus\script.hpp"

class MenuScript : public MenuBase {
	virtual void OnPop() {
		delete this;
	}

private:
	LuaScript *_script;

public:
	MenuScript(MenuItemTitle *title, LuaScript *script)
		: MenuBase(title) { _script = script; }

	virtual LuaScript *GetScript(void) {
		return _script;
	}
};

class MenuItemStatus : public MenuItemDefault {
	virtual std::string GetCaption(void) {
		LuaScript *scr = ((MenuScript *)GetMenu())->GetScript();
		return MenuItemDefault::GetCaption() +
		std::string(scr->IsEnabled() ? "enabled," : "disabled,") +
		std::string(scr->HasError() ? " with errors" : " no errors");
	}

public:
	MenuItemStatus(std::string title)
		: MenuItemDefault(title) {}
};

class MenuItemUsage : public MenuItemDefault {
	virtual std::string GetCaption(void) {
		return MenuItemDefault::GetCaption() + _usage + " KB";
	}

	virtual void OnSelect(void) {
		LuaScript *scr = ((MenuScript *)GetMenu())->GetScript();
		_usage = std::to_string(scr->GetMemoryUsage());
	}

private:
	std::string _usage = "[press to update]";

public:
	MenuItemUsage(std::string title)
		: MenuItemDefault(title) {}
};

class MenuItemReload : public MenuItemDefault {
	virtual void OnSelect(void) {
		LuaScript *script = ((MenuScript *)GetMenu())->GetScript();
		std::string error;
		if(script->Load(error))
			SetStatusText("Script successfully reloaded");
		else {
			SetStatusText("Failed to reload script, see logs");
			LOG(ERROR) << "Failed to reload script " + error;
		}
	}

public:
	MenuItemReload(std::string title)
		: MenuItemDefault(title) {}
};

class MenuItemUnload : public MenuItemDefault {
	virtual void OnSelect(void) {
		LuaScript *script = ((MenuScript *)GetMenu())->GetScript();
		for(std::map<std::string, LuaScript *>::iterator it = Scripts.begin(); it != Scripts.end();) {
			if(it->second == script) {
				it = Scripts.erase(it);
			} else {
				it++;
			}
		}
		delete script;
		SetStatusText("Script successfully unloaded");
		GetMenu()->GetController()->PopMenu(2);
	}

public:
	MenuItemUnload(std::string title)
		: MenuItemDefault(title) {}
};

class MenuItemToggle : public MenuItemDefault {
	virtual void OnSelect(void) {
		LuaScript *script = ((MenuScript *)GetMenu())->GetScript();
		script->SetEnabled(!script->IsEnabled());
	}

public:
	MenuItemToggle(std::string title)
		: MenuItemDefault(title) {}
};

class MenuItemScript : public MenuItemDefault {
	virtual void OnSelect() {
		auto menu = new MenuScript(new MenuItemTitle(this->GetCaption()), _script);
		auto controller = GetMenu()->GetController();
		controller->RegisterMenu(menu);

		menu->AddItem(new MenuItemStatus("Status: "));
		menu->AddItem(new MenuItemUsage("Usage: "));
		menu->AddItem(new MenuItemToggle("Toggle"));
		menu->AddItem(new MenuItemReload("Reload"));
		menu->AddItem(new MenuItemUnload("Unload"));

		controller->PushMenu(menu);
	}

private:
	LuaScript *_script;

public:
	MenuItemScript(std::string name, LuaScript *script)
		: MenuItemDefault(name) { _script = script; }
};

MenuScripts *CreateScriptsList(MenuController *controller) {
	auto menu = new MenuScripts(new MenuItemTitle("Scripts list"));
	controller->RegisterMenu(menu);

	if(Scripts.size() > 0) {
		for(auto const& x : Scripts)
			menu->AddItem(new MenuItemScript(x.first, x.second));
	} else
		menu->AddItem(new MenuItemDefault("No scripts found"));


	return menu;
}
