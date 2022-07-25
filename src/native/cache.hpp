#pragma once

#include "thirdparty\LuaJIT\src\lua.hpp"
#include "thirdparty\easyloggingpp.h"
#include "native\types.hpp"
#include <map>

#define LUANATIVE_CACHE "NativeCache"
#define GLOBAL_NATIVECACHE "REDLUA_CACHE"

typedef std::map<NativeType, std::map<NativeCacheField, int>> NativeCacheMap;

typedef struct _NativeCache {
	int ref;
	NativeCacheMap *map;
} NativeCache;

static NativeCache *get_native_cache(lua_State *L, int cache_ref) {
	if(cache_ref == -2) return nullptr;
	if(cache_ref == -1) lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_NATIVECACHE);
	else lua_rawgeti(L, LUA_REGISTRYINDEX, cache_ref);
	auto nc = (NativeCache *)luaL_checkudata(L, -1, LUANATIVE_CACHE);
	lua_pop(L, 2);
	return nc;
}

static int from_cache(NativeCacheMap &map, NativeType type, NativeCacheField id) {
	if(map.find(type) == map.end())
		return 0;
	if(map[type].find(id) == map[type].end())
		return 0;

	return map[type][id];
}

static bool search_in_cache
(
	lua_State *L, NativeCache *cache_obj, NativeCacheField *cache_id,
	NativeType type, NativeData id, int *cached
) {
	if(cache_obj) {
		*cache_id = *cache_id == NATIVEDATA_INVAL ? id : *cache_id;
		lua_rawgeti(L, LUA_REGISTRYINDEX, cache_obj->ref);
		if((*cached = from_cache(*cache_obj->map, type, *cache_id)) > 0) {
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

static void save_to_cache(lua_State *L, NativeCache *cache_obj, int cache_id, NativeType type, NativeData id, int cached) {
	if(cache_obj) {
		// Сохраняем в кеш
		lua_pushvalue(L, -1);
		// Если cached установлена в ненулевое значение, значит был промах кеша,
		// заполняем дыру в Lua кеше новой юзердатой с теми же параметрами
		if(!cached) cached = (int)lua_objlen(L, -3) + 1;
		lua_rawseti(L, -3, cached);
		(*cache_obj->map)[type][cache_id] = cached;
	}
}

static int ncache_gc(lua_State *L) {
	auto nc = (NativeCache *)luaL_checkudata(L, 1, LUANATIVE_CACHE);
	if(nc->map) delete nc->map;
	return 0;
}

static void create_luacache(lua_State *L) {
	lua_createtable(L, 0, 0);
	lua_createtable(L, 0, 1);
	lua_pushstring(L, "v");
	lua_setfield(L, -2, "__mode");
	lua_setmetatable(L, -2);

	int ref = luaL_ref(L, LUA_REGISTRYINDEX);
	auto nc = (NativeCache *)lua_newuserdata(L, sizeof(NativeCache));
	if(luaL_newmetatable(L, LUANATIVE_CACHE)) {
		lua_pushcfunction(L, ncache_gc);
		lua_setfield(L, -2, "__gc");
	}

	lua_setmetatable(L, -2);
	nc->map = new NativeCacheMap();
	nc->ref = ref;
}
