#pragma once

#include "thirdparty\luajit\src\lua.hpp"
#include "native\types.hpp"
#include <map>

typedef struct _RefMap {int ns, nc;} RefMap;
std::map<lua_State *, RefMap> ReferenceMap {};
std::map<int, std::map<NativeType, std::map<NativeData, int>>> NativeCache {};

static int from_cache(int cache_ref, NativeType type, NativeData id) {
	if(NativeCache.find(cache_ref) == NativeCache.end())
		return 0;
	if(NativeCache[cache_ref].find(type) == NativeCache[cache_ref].end())
		return 0;
	if(NativeCache[cache_ref][type].find(id) == NativeCache[cache_ref][type].end())
		return 0;

	return NativeCache[cache_ref][type][id];
}

static void create_luacache(lua_State *L) {
	lua_createtable(L, 0, 0);
	lua_createtable(L, 0, 1);
	lua_pushstring(L, "v");
	lua_setfield(L, -2, "__mode");
	lua_setmetatable(L, -2);
}
