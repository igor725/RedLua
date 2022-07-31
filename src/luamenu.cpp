#include "luamenu.hpp"
#include "natives.hpp"
#include "thirdparty\scriptmenu.h"
#include "thirdparty\easyloggingpp.h"

class MenuLua : public MenuBase {
	MenuLua **m_self; lua_State *m_L;
	int m_depth;

public:
	MenuLua(MenuItemTitle *title, MenuLua **self, lua_State *L, int depth)
	: MenuBase(title), m_self(self), m_L(L), m_depth(depth) {}

	~MenuLua() {
		*m_self = nullptr;
		if (m_depth < 1 ) {
			lua_pushnil(m_L);
			lua_setfield(m_L, LUA_REGISTRYINDEX, "MY_MENU_CLASS");
		}
	}

	lua_State *GetLState(void) {
		return m_L;
	}
};

class MenuItemLua : public MenuItemDefault {

public:
	MenuItemLua(std::string caption)
	: MenuItemDefault(caption) {}

	lua_State *GetLState(void) {
		return ((MenuLua *)GetMenu())->GetLState();
	}
};

class MenuItemLuaMenu : public MenuItemLua {
	MenuBase *m_menu; int m_menuref;

	void OnSelect(void) {
		if (auto parentMenu = GetMenu())
			if (auto controller = parentMenu->GetController()) {
				controller->RegisterMenu(m_menu);
				controller->PushMenu(m_menu);
			}
	}

public:
	MenuItemLuaMenu(std::string title, lua_State *L, MenuLua *menu)
	: MenuItemLua(title), m_menu(menu), m_menuref(luaL_ref(L, LUA_REGISTRYINDEX)) {}

	~MenuItemLuaMenu(void) { luaL_unref(GetLState(), LUA_REGISTRYINDEX, m_menuref); }

	eMenuItemClass GetClass() { return eMenuItemClass::Menu; }
};

class MenuItemLuaButton : public MenuItemLua {
	int m_func;

	void OnSelect() {
		if (m_func != LUA_REFNIL) {
			lua_rawgeti(GetLState(), LUA_REGISTRYINDEX, m_func);
			if (lua_pcall(GetLState(), 0, 0, 0) != 0) {
				NATIVES::NOTIFY(1, 1500, lua_tostring(GetLState(), -1));
				lua_pop(GetLState(), 1);
			}
		}
	}

public:
	MenuItemLuaButton(std::string title, int func)
	: MenuItemLua(title), m_func(func) {}

	~MenuItemLuaButton() { luaL_unref(GetLState(), LUA_REGISTRYINDEX, m_func); }
};

class MenuItemLuaSwitchable : public MenuItemSwitchable {
	int m_func;

	lua_State *GetLState(void) {
		return ((MenuLua *)GetMenu())->GetLState();
	}

	void OnSelect() {
		if (m_func != LUA_REFNIL) {
			lua_rawgeti(GetLState(), LUA_REGISTRYINDEX, m_func);
			lua_pushboolean(GetLState(), GetState());
			if (lua_pcall(GetLState(), 1, 1, 0) == 0)
				SetState(lua_toboolean(GetLState(), -1));
			else
				NATIVES::NOTIFY(1, 1500, lua_tostring(GetLState(), -1));

			lua_pop(GetLState(), 1);
		}
	}

public:
	MenuItemLuaSwitchable(std::string title, int func, bool initial)
	: MenuItemSwitchable(title, initial), m_func(func) {}

	~MenuItemLuaSwitchable() { luaL_unref(GetLState(), LUA_REGISTRYINDEX, m_func); }
};

static int meta_gc(lua_State *L) {
	auto menu = (MenuBase **)luaL_checkudata(L, 1, "MenuBase");
	if (*menu) delete *menu;
	return 0;
}

static luaL_Reg menumeta[] = {
	{"__gc", meta_gc},

	{NULL, NULL}
};

static MenuLua *gen_menu(lua_State *L, int idx);

MenuLua *gen_menu(lua_State *L, int depth) {
	luaL_checktype(L, -1, LUA_TTABLE);
	lua_getfield(L, -1, "items");
	if (!lua_istable(L, -1)) goto error;
	lua_getfield(L, -2, "islist");
	lua_getfield(L, -3, "title");
	if (!lua_isstring(L, -1)) goto error;
	const char *errobj = nullptr; // Строка объекта с ошибкой
	MenuItemTitle *menutitleclass = nullptr;
	if (lua_toboolean(L, -2)) // islist
		menutitleclass = new MenuItemListTitle(lua_tostring(L, -1));
	else
		menutitleclass = new MenuItemTitle(lua_tostring(L, -1));
	lua_pop(L, 2); // Удаляем из стека тайтл и булево значение

	auto menu = (MenuLua **)lua_newuserdata(L, sizeof(MenuLua *));
	if (luaL_newmetatable(L, "MenuBase"))
		luaL_setfuncs(L, menumeta, 0);
	lua_setmetatable(L, -2);

	*menu = new MenuLua(menutitleclass, menu, L, depth);
	(*menu)->SetController(nullptr);

	int count = (int)lua_objlen(L, -2);
	for (int i = 1; i <= count; i++) {
		lua_rawgeti(L, -2, i);
		if (!lua_istable(L, -1)) goto error;
		lua_getfield(L, -1, "title");
		if (!lua_isstring(L, -1)) goto error;
		auto title = errobj = lua_tostring(L, -1);
		lua_getfield(L, -2, "type");
		int itype = lua_tointeger(L, -1);
		bool initial; // Для свитча
		switch(itype) {
			case 0: // Menu
				lua_getfield(L, -3, "menu");
				if (!lua_istable(L, -1)) goto error;
				(*menu)->AddItem(new MenuItemLuaMenu(title, L, gen_menu(L, depth + 1)));
				lua_pop(L, 1); // Удаляем из стека таблицу menu
				break;
			case 1: // Button
				lua_getfield(L, -3, "onclick");
				if (!lua_isfunction(L, -1) && !lua_isnil(L, -1)) goto error;
				(*menu)->AddItem(new MenuItemLuaButton(title, luaL_ref(L, LUA_REGISTRYINDEX)));
				break;
			case 2: // Switchable
				lua_getfield(L, -3, "initial");
				initial = lua_toboolean(L, -1);
				lua_getfield(L, -4, "onclick");
				if (!lua_isfunction(L, -1)) goto error;
				(*menu)->AddItem(new MenuItemLuaSwitchable(title, luaL_ref(L, LUA_REGISTRYINDEX), initial));
				lua_pop(L, 1); // Удаляем из стека initial
				break;

			default: goto error;
		}
		lua_pop(L, 3);
		errobj = nullptr;
	}

	lua_remove(L, -2); // Удаляем из стека таблицу итемов
	return *menu;

	error:
	luaL_error(L, "Menu creation error (depth: %d, menu: \"%s\", object: \"%s\")",
	depth, menutitleclass ? menutitleclass->GetCaption().c_str() : "[no title]",
	errobj ? errobj : "[no title]");
	return nullptr;
}

static int menu_set(lua_State *L) {
	int top = lua_gettop(L);
	(void)gen_menu(L, 0);
	lua_setfield(L, LUA_REGISTRYINDEX, "MY_MENU_CLASS");
	return 0;
}

static int menu_remove(lua_State *L) {
	lua_pushnil(L);
	lua_setfield(L, LUA_REGISTRYINDEX, "MY_MENU_CLASS");
	return 0;
}

static luaL_Reg menulib[] = {
	{"set", menu_set},
	{"remove", menu_remove},

	{NULL, NULL}
};

int luaopen_menu(lua_State *L) {
	lua_pushinteger(L, 0);
	lua_setglobal(L, "LUAMENU_ITEM_MENU");
	lua_pushinteger(L, 1);
	lua_setglobal(L, "LUAMENU_ITEM_BUTTON");
	lua_pushinteger(L, 2);
	lua_setglobal(L, "LUAMENU_ITEM_SWITCHABLE");

	luaL_newlib(L, menulib);
	return 1;
}
