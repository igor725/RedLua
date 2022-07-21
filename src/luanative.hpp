#pragma once

#include "thirdparty\luajit\src\lua.hpp"

#ifdef REDLUA_STANDALONE
#define luaopen_native luaopen_RedLua_native

extern "C" {
	__declspec(dllexport)
#endif
int luaopen_native(lua_State *L);
#ifdef REDLUA_STANDALONE
}
#endif
