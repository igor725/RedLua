#include "redlua.h"
#include "keyboard.h"

static int misc_iskeydown(lua_State *L) {
	lua_pushboolean(L, IsKeyDown(luaL_checkinteger(L, 1)));
	return 1;
}

static int misc_iskeydownlong(lua_State *L) {
	lua_pushboolean(L, IsKeyDownLong(luaL_checkinteger(L, 1)));
	return 1;
}

static int misc_isjustup(lua_State *L) {
	lua_pushboolean(L, IsKeyJustUp(luaL_checkinteger(L, 1), lua_toboolean(L, 2)));
	return 1;
}

static int misc_resetkey(lua_State *L) {
	ResetKeyState(luaL_checkinteger(L, 1));
	return 0;
}

static luaL_Reg misclib[] = {
	{"iskeydown", misc_iskeydown},
	{"iskeydownlong", misc_iskeydownlong},
	{"isjustup", misc_isjustup},
	{"resetkey", misc_resetkey},

	{NULL, NULL}
};

int luaopen_misc(lua_State *L) {
	luaL_newlib(L, misclib);
	return 1;
}
