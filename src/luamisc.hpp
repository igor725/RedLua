#pragma once

#include "thirdparty\luajit\src\lua.hpp"

#ifdef REDLUA_STANDALONE
#define luaopen_misc luaopen_RedLua_misc

extern "C" {
	__declspec(dllexport)
#endif
int luaopen_misc(lua_State *L);
#ifdef REDLUA_STANDALONE
}
#endif
