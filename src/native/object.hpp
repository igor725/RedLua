#pragma once

#include "native\types.hpp"
#include "native\cache.hpp"

#define LUANATIVE_OBJECT "NativeObject"

typedef struct _NativeVector {
	NativeObjectHeader hdr;
	Vector3 data;
} NativeVector;

static void push_native(lua_State *L, NativeType type, NativeData id, int cache_ref = -1, NativeData cache_id = -1) {
	cache_id = cache_id != NATIVEDATA_INVAL ? cache_id : id, cache_ref = cache_ref > 0 ? cache_ref : ReferenceMap[L].nc;
	int cached;
	if((cached = from_cache(cache_ref, type, cache_id)) > 0) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, cache_ref);
		lua_rawgeti(L, -1, cached);
		if(!lua_isnil(L, -1)) {
			lua_remove(L, -2);
			return;
		}

		lua_pop(L, 2);
	} else {
		lua_rawgeti(L, LUA_REGISTRYINDEX, cache_ref);
		cached = (int)lua_objlen(L, -1) + 1;
		lua_pop(L, 1);
	}

	auto no = (NativeObject *)lua_newuserdata(L, sizeof(NativeObject));
	NATHDR_INIT(no->hdr, type, 1, cache_ref == ReferenceMap[L].nc);
	luaL_setmetatable(L, LUANATIVE_OBJECT);
	no->hash.u64 = id;

	// Сохраняем в кеш
	NativeCache[cache_ref][type][cache_id] = cached;
	lua_rawgeti(L, LUA_REGISTRYINDEX, cache_ref);
	lua_pushvalue(L, -2);
	lua_rawseti(L, -2, cached);
	lua_pop(L, 1);
}

static void push_vector(lua_State *L, Vector3 *vec) {
	auto nv = (NativeVector *)lua_newuserdata(L, sizeof(NativeVector));
	NATHDR_INIT(nv->hdr, NTYPE_VECTOR3, 1, false);
	luaL_setmetatable(L, LUANATIVE_OBJECT);
	if(!vec) nv->data.x = nv->data.y = nv->data.z = 0.0f;
	else nv->data = *vec;
}

static int push_value(lua_State *L, NativeType type, PUINT64 val) {
	switch(type) {
		case NTYPE_VOID: return 0;

		case NTYPE_INT:
		case NTYPE_ANY:
		case NTYPE_HASH:
			lua_pushinteger(L, *val);
			break;
		case NTYPE_FLOAT:
			lua_pushnumber(L, *(float *)val);
			break;
		case NTYPE_BOOL:
			lua_pushboolean(L, (int)*val);
			break;
		case NTYPE_VECTOR3:
			push_vector(L, (Vector3 *)val);
			break;
		default:
			push_native(L, type, *val);
			break;
	}

	return 1;
}

static Vector3 *check_vector(lua_State *L, int idx) {
	auto nv = (NativeVector *)luaL_checkudata(L, idx, LUANATIVE_OBJECT);
	luaL_argcheck(L, nv->hdr.type == NTYPE_VECTOR3, idx, "not a vector");
	return &nv->data;
}

static Vector3 *to_vector(lua_State *L, int idx) {
	auto nv = (NativeVector *)luaL_testudata(L, idx, LUANATIVE_OBJECT);
	if(nv && nv->hdr.type == NTYPE_VECTOR3) return &nv->data;
	return nullptr;
}

static int vector_tostring(lua_State *L, NativeVector *nv) {
	lua_pushfstring(L, "Vector: %.3f, %.3f, %.3f",
		nv->data.x, nv->data.y, nv->data.z);
	return 1;
}

static int vector_newindex(lua_State *L, NativeVector *nv) {
	switch(*luaL_checkstring(L, 2)) {
		case 'x': case 'X':
			nv->data.x = (float)luaL_checknumber(L, 3);
			break;
		case 'y': case 'Y':
			nv->data.y = (float)luaL_checknumber(L, 3);
			break;
		case 'z': case 'Z':
			nv->data.z = (float)luaL_checknumber(L, 3);
			break;
	}

	return 0;
}

static int vector_index(lua_State *L, NativeVector *nv) {
	switch(*luaL_checkstring(L, 2)) {
		case 'x': case 'X':
			lua_pushnumber(L, nv->data.x);
			return 1;
		case 'y': case 'Y':
			lua_pushnumber(L, nv->data.y);
			return 1;
		case 'z': case 'Z':
			lua_pushnumber(L, nv->data.z);
			return 1;
	}

	return 0;
}

static int native_topointer(lua_State *L) {
	auto no = (NativeObject *)luaL_checkudata(L, 1, LUANATIVE_OBJECT);
	lua_pushlightuserdata(L, get_type_info(
		no->hdr.type).isPointer ? no->hash.p : &no->hash);
	return 1;
}

static int native_tostring(lua_State *L) {
	auto no = (NativeObject *)luaL_checkudata(L, 1, LUANATIVE_OBJECT);
	if(no->hdr.type == NTYPE_VECTOR3) return vector_tostring(L, (NativeVector *)no);
	NativeTypeInfo &nti_no = get_type_info(no->hdr.type);
	std::string &type = nti_no.name;
	if(nti_no.isPointer) {
		if(no->hdr.size > 1)
			lua_pushfstring(L, "%s[%d]: 0x%p", type.substr(0, type.length() - 1).c_str(), no->hdr.size, no);
		else
			lua_pushfstring(L, "%s: 0x%p", type.c_str(), no);
	} else
		lua_pushfstring(L, "%s: %d", type.c_str(), no->hash.i32);
	return 1;
}

static int native_newindex(lua_State *L) {
	auto no = (NativeObject *)luaL_checkudata(L, 1, LUANATIVE_OBJECT);
	luaL_argcheck(L, !no->hdr.readonly, 1, "readonly object");
	luaL_argcheck(L, no->hdr.type != NTYPE_ANY, 1, "Any is not indexable");
	if(no->hdr.type == NTYPE_VECTOR3) return vector_newindex(L, (NativeVector *)no);
	NativeTypeInfo &ni = get_type_info(no->hdr.type);
	
	if(!ni.isPointer) return 0;
	uint idx = (uint)luaL_checkinteger(L, 2);
	luaL_argcheck(L, idx < no->hdr.size, 2, "out of bounds");
	((int *)&no->hash.p)[idx] = (int)luaL_checkinteger(L, 3);
	return 0;
}

static int native_index(lua_State *L) {
	if(lua_isstring(L, 2))
		return luaL_getmetafield(L, 1, lua_tostring(L, 2));

	uint idx = (uint)luaL_checkinteger(L, 2);
	auto no = (NativeObject *)luaL_checkudata(L, 1, LUANATIVE_OBJECT);
	if(no->hdr.type == NTYPE_VECTOR3) return vector_index(L, (NativeVector *)no);
	NativeTypeInfo &ni = get_type_info(no->hdr.type);
	luaL_argcheck(L, ni.isPointer, 1, "not a pointer");
	luaL_argcheck(L, idx < no->hdr.size, 2, "out of bounds");

	switch(no->hdr.type) {
		case NTYPE_ANY:
			luaL_argerror(L, 1, "Any is not indexable");
			return 0;
		case NTYPE_INT:
		case NTYPE_HASH:
			lua_pushinteger(L, ((int *)&no->hash.p)[idx]);
			break;
		case NTYPE_FLOAT:
			lua_pushnumber(L, ((float *)&no->hash.p)[idx]);
			break;
		case NTYPE_BOOL:
			lua_pushboolean(L, ((int *)&no->hash.p)[idx]);
			break;
		default:
			if(no->hdr.owncache == 0) {
				// Создаём собственный кеш для объекта
				create_luacache(L);
				no->hdr.owncache = luaL_ref(L, LUA_REGISTRYINDEX);
			}
			push_native(L, (NativeType)(no->hdr.type - 1),
				((char *)&no->hash.p)[idx * ni.size], no->hdr.owncache, idx);
			break;
	}

	return 1;
}

static int native_gc(lua_State *L) {
	// Удаляем собственный кеш объекта, если он был
	auto no = (NativeObject *)luaL_checkudata(L, 1, LUANATIVE_OBJECT);
	if(no->hdr.owncache) luaL_unref(L, LUA_REGISTRYINDEX, no->hdr.owncache);
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
