#pragma once

#include "thirdparty\LuaJIT\src\lua.hpp"

#ifdef REDLUA_STANDALONE
#define luaopen_lang luaopen_RedLua_lang

extern "C" {
	__declspec(dllexport)
#endif
int luaopen_lang(lua_State *L);
#ifdef REDLUA_STANDALONE
}
#endif
