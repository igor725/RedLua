#include "lualang.hpp"
#include "lang.hpp"

static int lang_install(lua_State *L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	LangsMap &map = Lng.GetMap();

	lua_pushnil(L);
	while (lua_next(L, 1)) {
		if (lua_istable(L, -1) && lua_isstring(L, -2)) {
			auto ln = map.find(lua_tostring(L, -2));
			if (ln != map.end()) {
				lua_pushnil(L);
				while (lua_next(L, -2)) {
					if (lua_isstring(L, -1) && lua_isstring(L, -2))
						ln->second.f_map[lua_tostring(L, -2)] = lua_tostring(L, -1);

					lua_pop(L, 1);
				}
			}
		}

		lua_pop(L, 1);
	}

	return 0;
}

static int lang_get(lua_State *L) {
	lua_pushstring(L, Lng.Get(luaL_checkstring(L, 1)).c_str());
	return 1;
}

static const luaL_Reg langlib[] = {
	{"install", lang_install},
	{"get", lang_get},

	{NULL, NULL}
};

int luaopen_lang(lua_State *L) {
	luaL_newlib(L, langlib);
	return 1;
}
