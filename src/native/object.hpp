#pragma once

#include "native\types.hpp"
#include "native\cache.hpp"

#define LUANATIVE_OBJECT "NativeObject"

static NativeObject *push_uncached_lightobjectcopy
(
	lua_State *L,
	NativeType type, NativeData *ptr,
	uint count = NOBJCOUNT_UNKNOWN
) {
	auto no = (NativeObject *)lua_newuserdata(L, sizeof(NativeObject));
	NATIVEOBJECT_INITLIGHT(no, type, false, count, *ptr); // TODO: Поддержка больших типов (н.р векторов)
	luaL_setmetatable(L, LUANATIVE_OBJECT);

	return no;
}

static NativeObject *push_uncached_fullcopy
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
	
	return no;
}

static NativeObject *push_cached_lightobjectlink
(
	lua_State *L,
	NativeType type, NativeData *ptr,
	int cache_ref = -1, NativeCacheField cache_id = NATIVEDATA_INVAL
) {
	int cached = 0;
	NativeCache *cache = get_native_cache(L, cache_ref);
	if(search_in_cache(L, cache, &cache_id, type, -1, &cached))
		return (NativeObject *)lua_touserdata(L, -1);
	
	auto no = (NativeObject *)lua_newuserdata(L, sizeof(NativeObject));
	NATIVEOBJECT_INITLIGHT(no, type, false, 1, *ptr);
	luaL_setmetatable(L, LUANATIVE_OBJECT);

	save_to_cache(L, cache, cache_id, type, -1, cached);
	return no;
}

static NativeObject *push_cached_fullobject
(
	lua_State *L,
	NativeType type, NativeData id,
	int cache_ref = -1, NativeCacheField cache_id = NATIVEDATA_INVAL
) {
	int cached = 0;
	NativeCache *cache = get_native_cache(L, cache_ref);
	if(search_in_cache(L, cache, &cache_id, type, id, &cached))
		return (NativeObject *)lua_touserdata(L, -1);

	auto no = (NativeObject *)lua_newuserdata(L, sizeof(NativeObject));
	NATIVEOBJECT_INIT(no, type, false, cache_ref == -1, 1, id);
	luaL_setmetatable(L, LUANATIVE_OBJECT);

	save_to_cache(L, cache, cache_id, type, id, cached);
	return no;
}

static NativeObject *native_check(lua_State *L, int idx, NativeType type) {
	auto no = (NativeObject *)luaL_checkudata(L, idx, LUANATIVE_OBJECT);
	if(type != NTYPE_UNKNOWN) {
		NativeTypeInfo &nti_got = get_type_info(no->hdr.type),
		&nti_exp = get_type_info(type);
		if(!IS_NATIVETYPES_EQU(type, nti_exp, no->hdr.type, nti_got)) {
			auto msg = lua_pushfstring(L, "%s expected, got %s",
			nti_exp.name.c_str(), nti_got.name.c_str());
			luaL_argerror(L, idx, msg);
			return nullptr;
		}
	}

	return no;
}

static int vector_tostring(lua_State *L, NativeObject *no) {
	auto nv = (Vector3 *)NATIVEOBJECT_GETPTR(no);
	lua_pushfstring(L, "Vector3: %.3f, %.3f, %.3f",
		nv->x, nv->y, nv->z);
	return 1;
}

static int vector_newindex(lua_State *L, NativeObject *no, char idx) {
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

static int native_topointer(lua_State *L) {
	auto no = (NativeObject *)luaL_checkudata(L, 1, LUANATIVE_OBJECT);
	lua_pushlightuserdata(L, NATIVEOBJECT_GETPTR(no));
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

static int native_newindex(lua_State *L) {
	auto no = (NativeObject *)luaL_checkudata(L, 1, LUANATIVE_OBJECT);
	luaL_argcheck(L, !no->hdr.isReadOnly, 1, "readonly object");
	if(lua_type(L, 2) == LUA_TSTRING && no->hdr.type == NTYPE_VECTOR3)
		return vector_newindex(L, no, *lua_tostring(L, 2));
	uint idx = (uint)luaL_checkinteger(L, 2);
	luaL_argcheck(L, idx < no->hdr.count, 2, "out of bounds");
	NativeTypeInfo &nti = get_type_info(no->hdr.type);
	auto ptr = &(((char *)(&no->content))[idx * nti.size]);

	switch(no->hdr.type) {
		case NTYPE_INT:
		case NTYPE_HASH:
		case NTYPE_ANY:
			*(int *)ptr = (int)luaL_checkinteger(L, 3);
			break;

		default:
			switch(lua_type(L, 3)) {
				case LUA_TUSERDATA:
					memcpy(ptr, NATIVEOBJECT_GETPTR(native_check(L, 3, no->hdr.type)), nti.size);
					break;
				case 10/*LUA_TCDATA*/:
					memcpy(ptr, lua_topointer(L, 3), nti.size);
					break;
				default:
					luaL_typerror(L, 3, (nti.name + "/cdata").c_str());
					break;
			}
			break;
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

static int vector_unm(lua_State *L, NativeObject *no) {
	auto nv = (Vector3 *)NATIVEOBJECT_GETPTR(push_uncached_fullcopy(
		L, NTYPE_VECTOR3, &no->content.nd, 1
	));
	nv->x = -nv->x, nv->y = -nv->y, nv->z = -nv->z;
	return 1;
}

static inline int vector_mul_vector(Vector3 *dst, Vector3 *v1, Vector3 *v2) {
	dst->x = v1->x * v2->x, dst->y = v1->y * v2->y, dst->z = v1->z * v2->z;
	return 1;
}

static inline int vector_mul_num(Vector3 *v, float num) {
	v->x *= num, v->y *= num, v->z *= num;
	return 1;
}

static int vector_mul(lua_State *L, NativeObject *no) {
	switch(lua_type(L, 2)) {
		case LUA_TUSERDATA:
			return vector_mul_vector(
				(Vector3 *)NATIVEOBJECT_GETPTR(push_uncached_fullcopy(
					L, NTYPE_VECTOR3, &no->content.nd, 1
				)),
				(Vector3 *)NATIVEOBJECT_GETPTR(no),
				(Vector3 *)NATIVEOBJECT_GETPTR(native_check(L, 2, NTYPE_VECTOR3))
			);
		case LUA_TNUMBER:
			return vector_mul_num(
				(Vector3 *)NATIVEOBJECT_GETPTR(push_uncached_fullcopy(
					L, NTYPE_VECTOR3, &no->content.nd, 1
				)),
				(float)lua_tonumber(L, 2));
		default:
			return luaL_typerror(L, 2, "NativeObject/number");
	}

	return 0;
}

static int native_arith_error(lua_State *L, NativeType type) {
	return luaL_error(L, "attempt to perform arithmetic on a %s value",
		get_type_info(type).name.c_str());
}

static int native_mul(lua_State *L) {
	auto no = (NativeObject *)luaL_checkudata(L, 1, LUANATIVE_OBJECT);
	switch(no->hdr.type) {
		case NTYPE_VECTOR3:
			return vector_mul(L, no);
		default:
			return native_arith_error(L, no->hdr.type);
	}

	return 0;
}

static int native_unm(lua_State *L) {
	auto no = (NativeObject *)luaL_checkudata(L, 1, LUANATIVE_OBJECT);
	switch(no->hdr.type) {
		case NTYPE_VECTOR3:
			return vector_unm(L, no);
		default:
			return native_arith_error(L, no->hdr.type);
	}

	return 0;
}

static int native_eq(lua_State *L) {
	auto no1 = (NativeObject *)luaL_checkudata(L, 1, LUANATIVE_OBJECT);
	auto no2 = (NativeObject *)luaL_testudata(L, 2, LUANATIVE_OBJECT);
	NativeTypeInfo &nti1 = get_type_info(no1->hdr.type),
	&nti2 = get_type_info(no2->hdr.type);
	lua_pushboolean(L, IS_NATIVETYPES_EQU(
			no1->hdr.type, nti1, no2->hdr.type, nti2
		) &&
		memcmp(
			no1->hdr.isPointer ? no1->content.p : &no1->content,
			no2->hdr.isPointer ? no2->content.p : &no2->content,
			get_type_info(no1->hdr.type).size
		) == 0
	);
	return 1;
}

static const luaL_Reg nativeobj[] = {
	{"topointer", native_topointer},

	{"__tostring", native_tostring},
	{"__newindex", native_newindex},
	{"__index",    native_index},
	{"__mul",      native_mul},
	{"__unm",      native_unm},
	{"__eq",       native_eq},

	{NULL, NULL}
};

static void nativeobj_init(lua_State *L) {
	luaL_newmetatable(L, LUANATIVE_OBJECT);
	lua_pushstring(L, "none of your business");
	lua_setfield(L, -2, "__metatable");
	luaL_setfuncs(L, nativeobj, 0);

	create_luacache(L);
	lua_setfield(L, LUA_REGISTRYINDEX, GLOBAL_NATIVECACHE);
}
