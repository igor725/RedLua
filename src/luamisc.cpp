#include "luascript.hpp"

#ifndef REDLUA_STANDALONE
#include "luamisc.hpp"
#include "constants.hpp"

#include "thirdparty\keyboard.h"
#include "scripthook.hpp"

static int misc_iskeydown(lua_State *L) {
	lua_pushboolean(L, IsKeyDown((DWORD)luaL_checkinteger(L, 1)));
	return 1;
}

static int misc_iskeydownlong(lua_State *L) {
	lua_pushboolean(L, IsKeyDownLong((DWORD)luaL_checkinteger(L, 1)));
	return 1;
}

static int misc_iskeyjustup(lua_State *L) {
	lua_pushboolean(L, IsKeyJustUp((DWORD)luaL_checkinteger(L, 1), lua_toboolean(L, 2)));
	return 1;
}

static int misc_resetkey(lua_State *L) {
	ResetKeyState((DWORD)luaL_checkinteger(L, 1));
	return 0;
}

static int misc_gamever(lua_State *L) {
	lua_pushinteger(L, getGameVersion());
	lua_pushstring(L, REDLUA_GAMECODE);
	return 2;
}

static int misc_libver(lua_State *L) {
	lua_pushinteger(L, REDLUA_VERSION_NUM);
	return 1;
}

static luaL_Reg misclib[] = {
	{"iskeydown", misc_iskeydown},
	{"iskeydownlong", misc_iskeydownlong},
	{"iskeyjustup", misc_iskeyjustup},
	{"resetkey", misc_resetkey},

	{"gamever", misc_gamever},
	{"libver", misc_libver},

	{NULL, NULL}
};

int luaopen_misc(lua_State *L) {
	for (int i = 0; i < 255; i++) {
		if (KeyNames[i]) {
			lua_pushinteger(L, i);
			lua_setglobal(L, KeyNames[i]);
		}
	}

	luaL_newlib(L, misclib);
	return 1;
}
#else
// nullsub
int luaopen_misc(lua_State *L) {return 0;}
#endif
