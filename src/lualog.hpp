#pragma once

#include "thirdparty\LuaJIT\src\lua.hpp"
#ifdef REDLUA_STANDALONE
#define luaopen_log luaopen_RedLua_log

extern "C" {
	__declspec(dllexport)
#endif
int luaopen_log(lua_State *L);
#ifdef REDLUA_STANDALONE
}
#endif
