#pragma once

#include "native\types.hpp"
#include "native\cache.hpp"

#define LUANATIVE_OBJECT "NativeObject"

static void push_uncached_lightobjectcopy
(
	lua_State *L,
	NativeType type, NativeData *ptr,
	uint count = NOBJCOUNT_UNKNOWN
) {
	auto no = (NativeObject *)lua_newuserdata(L, sizeof(NativeObject));
	NATIVEOBJECT_INITLIGHT(no, type, false, count, *ptr); // TODO: Поддержка больших типов (н.р векторов)
	luaL_setmetatable(L, LUANATIVE_OBJECT);
}

static void push_uncached_fullcopy
(
	lua_State *L,
	NativeType type, NativeData *ptr,
	uint count = 1
) {
	NativeTypeInfo nti = get_type_info(type);
	// Считаем размер структуры NativeObjectHeader вместе с выравниванием
	auto no = (NativeObject *)lua_newuserdata(L, NATIVEOBJECT_HDRSIZE + nti.size * count);
	NATIVEOBJECT_INIT(no, type, false, false, count, 0);
	luaL_setmetatable(L, LUANATIVE_OBJECT);
	if(ptr)
		memcpy(&no->content, ptr, nti.size * count);
	else
		memset(&no->content, 0, nti.size * count);
}

static void push_cached_lightobjectlink
(
	lua_State *L,
	NativeType type, NativeData *ptr,
	int cache_ref = -1, NativeCacheField cache_id = NATIVEDATA_INVAL
) {
	int cached = 0;
	if(search_in_cache(L, &cache_ref, &cache_id, type, -1, &cached))
		return;
	
	auto no = (NativeObject *)lua_newuserdata(L, sizeof(NativeObject));
	NATIVEOBJECT_INITLIGHT(no, type, false, 1, *ptr);
	luaL_setmetatable(L, LUANATIVE_OBJECT);

	save_to_cache(L, cache_ref, cache_id, type, -1, cached);
}

static void push_cached_fullobject
(
	lua_State *L,
	NativeType type, NativeData id,
	int cache_ref = -1, NativeCacheField cache_id = NATIVEDATA_INVAL
) {
	int cached = 0;
	if(search_in_cache(L, &cache_ref, &cache_id, type, id, &cached))
		return;

	auto no = (NativeObject *)lua_newuserdata(L, sizeof(NativeObject));
	NATIVEOBJECT_INIT(no, type, false, ReferenceMap[L].nc == cache_ref, 1, id);
	luaL_setmetatable(L, LUANATIVE_OBJECT);

	save_to_cache(L, cache_ref, cache_id, type, id, cached);
}

static int native_topointer(lua_State *L) {
	auto no = (NativeObject *)luaL_checkudata(L, 1, LUANATIVE_OBJECT);
	lua_pushlightuserdata(L, NATIVEOBJECT_GETPTR(no));
	return 1;
}

static int vector_tostring(lua_State *L, NativeObject *no) {
	auto nv = (Vector3 *)NATIVEOBJECT_GETPTR(no);
	lua_pushfstring(L, "Vector3: %.3f, %.3f, %.3f",
		nv->x, nv->y, nv->z);
	return 1;
}

static int native_tostring(lua_State *L) {
	auto no = (NativeObject *)luaL_checkudata(L, 1, LUANATIVE_OBJECT);
	if(no->hdr.type == NTYPE_VECTOR3  && no->hdr.count == 1)
		return vector_tostring(L, no);
	NativeTypeInfo &nti = get_type_info(no->hdr.type);

	if(no->hdr.count != NOBJCOUNT_UNKNOWN && no->hdr.count > 1)
		lua_pushfstring(L, "%s[%d]: %p", nti.name.c_str(), no->hdr.count, &no->content);
	else if(no->hdr.isPointer)
		lua_pushfstring(L, "%s*: %p", nti.name.c_str(), no->content.p);
	else
		lua_pushfstring(L, "%s: %d", nti.name.c_str(), no->content.i32);

	return 1;
}

static int vector_newindex(lua_State *L, NativeObject *no, char idx) {
	if(no->hdr.type != NTYPE_VECTOR3) return 0;
	auto nv = (Vector3 *)NATIVEOBJECT_GETPTR(no);

	switch(idx) {
		case 'x':
		case 'X':
			nv->x = (float)luaL_checknumber(L, 3);
			break;
		
		case 'y':
		case 'Y':
			nv->y = (float)luaL_checknumber(L, 3);
			break;

		case 'z':
		case 'Z':
			nv->z = (float)luaL_checknumber(L, 3);
			break;
	}

	return 0;
}

static int native_newindex(lua_State *L) {
	auto no = (NativeObject *)luaL_checkudata(L, 1, LUANATIVE_OBJECT);
	if(lua_type(L, 2) == LUA_TSTRING)
		return vector_newindex(L, no, *lua_tostring(L, 2));

	return 0;
}

static int vector_index(lua_State *L, NativeObject *no, char idx) {
	if(no->hdr.type != NTYPE_VECTOR3) return 0;
	auto nv = (Vector3 *)NATIVEOBJECT_GETPTR(no);

	switch(idx) {
		case 'x':
		case 'X':
			lua_pushnumber(L, nv->x);
			return 1;
		
		case 'y':
		case 'Y':
			lua_pushnumber(L, nv->y);
			return 1;

		case 'z':
		case 'Z':
			lua_pushnumber(L, nv->z);
			return 1;
	}

	return 0;
}

static int native_index(lua_State *L) {
	auto no = (NativeObject *)luaL_checkudata(L, 1, LUANATIVE_OBJECT);
	if(lua_type(L, 2) == LUA_TSTRING) {
		const char *str = lua_tostring(L, 2);
		return vector_index(L, no, *str) || luaL_getmetafield(L, 1, lua_tostring(L, 2));
	}
	uint idx = (uint)luaL_checkinteger(L, 2);
	luaL_argcheck(L, idx < no->hdr.count, 2, "out of bounds");
	NativeTypeInfo &nti = get_type_info(no->hdr.type);
	auto ptr = (NativeData)&(((char *)(&no->content))[idx * nti.size]);
	switch(no->hdr.type) {
		case NTYPE_INT:
		case NTYPE_HASH:
		case NTYPE_ANY:
			lua_pushinteger(L, *(int *)ptr);
			return 1;
		case NTYPE_FLOAT:
			lua_pushnumber(L, *(float *)ptr);
			return 1;
		case NTYPE_BOOL:
			lua_toboolean(L, *(int *)ptr);
			return 1;
		case NTYPE_CHAR:
			lua_pushlstring(L, (char *)ptr, 1);
			return 1;
		default:
			if(no->hdr.ownCache == 0) {
				create_luacache(L);
				no->hdr.ownCache = luaL_ref(L, LUA_REGISTRYINDEX);
			}
			push_cached_lightobjectlink(L, no->hdr.type, &ptr,
				no->hdr.ownCache, idx);
			return 1;
	}

	return 0;
}

static const luaL_Reg nativeobj[] = {
	{"topointer", native_topointer},

	{"__tostring", native_tostring},
	{"__newindex", native_newindex},
	{"__index", native_index},

	{NULL, NULL}
};

static void nativeobj_init(lua_State *L) {
	luaL_newmetatable(L, LUANATIVE_OBJECT);
	lua_pushstring(L, "none of your business");
	lua_setfield(L, -2, "__metatable");
	luaL_setfuncs(L, nativeobj, 0);

	create_luacache(L);
	ReferenceMap[L].nc = luaL_ref(L, LUA_REGISTRYINDEX);
}
