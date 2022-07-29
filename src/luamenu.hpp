#pragma once

#include "thirdparty\LuaJIT\src\lua.hpp"
#ifdef REDLUA_STANDALONE
#define luaopen_menu luaopen_RedLua_menu

extern "C" {
	__declspec(dllexport)
#endif
int luaopen_menu(lua_State *L);
#ifdef REDLUA_STANDALONE
}
#endif
