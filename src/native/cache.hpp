#pragma once

#include "thirdparty\LuaJIT\src\lua.hpp"
#include "native\types.hpp"
#include <map>

typedef struct _RefMap {int ns, nc;} RefMap;
std::map<lua_State *, RefMap> ReferenceMap {};
std::map<int, std::map<NativeType, std::map<NativeCacheField, int>>> NativeCache {};

static int from_cache(int cache_ref, NativeType type, NativeCacheField id) {
	if(NativeCache.find(cache_ref) == NativeCache.end())
		return 0;
	if(NativeCache[cache_ref].find(type) == NativeCache[cache_ref].end())
		return 0;
	if(NativeCache[cache_ref][type].find(id) == NativeCache[cache_ref][type].end())
		return 0;

	return NativeCache[cache_ref][type][id];
}

static bool search_in_cache
(
	lua_State *L, int *cache_ref, NativeCacheField *cache_id,
	NativeType type, NativeData id, int *cached
) {
	if(*cache_ref > NATIVECACHE_DISABLE) { // Если cache_ref меньше или равен -2, значит кеширование выключено
		*cache_id = *cache_id == NATIVEDATA_INVAL ? id : *cache_id,
		*cache_ref = *cache_ref > 0 ? *cache_ref : ReferenceMap[L].nc;
		if(*cache_id == NATIVEDATA_INVAL || *cache_ref < 0)
			luaL_error(L, "Invalid cache request");
		lua_rawgeti(L, LUA_REGISTRYINDEX, *cache_ref);
		if((*cached = from_cache(*cache_ref, type, *cache_id)) > 0) {
			lua_rawgeti(L, -1, *cached);
			if(!lua_isnil(L, -1)) {
				lua_remove(L, -2);
				return true;
			}

			// Промах кеша, в мапе есть значение, а Lua его уже удалила
			// TODO: Переиндексация кеша, мейби?
			lua_pop(L, 1);
		}
	}

	return false;
}

static void save_to_cache(lua_State *L, int cache_ref, int cache_id, NativeType type, NativeData id, int cached) {
	if(cache_ref > NATIVECACHE_DISABLE) {
		// Сохраняем в кеш
		lua_pushvalue(L, -1);
		// Если cached установлена в ненулевое значение, значит был промах кеша,
		// заполняем дыру в Lua кеше новой юзердатой с теми же параметрами
		if(!cached) cached = (int)lua_objlen(L, -3) + 1;
		lua_rawseti(L, -3, cached);
		NativeCache[cache_ref][type][cache_id] = cached;
	}
}

static void create_luacache(lua_State *L) {
	lua_createtable(L, 0, 0);
	lua_createtable(L, 0, 1);
	lua_pushstring(L, "v");
	lua_setfield(L, -2, "__mode");
	lua_setmetatable(L, -2);
}
