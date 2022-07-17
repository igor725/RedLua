#pragma once

#include "thirdparty\luajit\src\lua.hpp"
#include "thirdparty\ScriptHook\inc\types.h"
#include "luanative.h"

typedef struct _NativeVector {
	NativeObjectHeader hdr;
	Vector3 data;
} NativeVector;

static void push_vector(lua_State *L, Vector3 *vec) {
	auto *nv = (NativeVector *)lua_newuserdata(L, sizeof(NativeVector));
	NATHDR_INIT(nv->hdr, NTYPE_VECTOR3, 1, false);
	luaL_setmetatable(L, LUANATIVE_OBJECT);
	if(!vec) nv->data.x = nv->data.y = nv->data.z = 0.0f;
	else nv->data = *vec;
}

static Vector3 *check_vector(lua_State *L, int idx) {
	auto *nv = (NativeVector *)luaL_checkudata(L, idx, LUANATIVE_OBJECT);
	luaL_argcheck(L, nv->hdr.type == NTYPE_VECTOR3, idx, "not a vector");
	return &nv->data;
}

static Vector3 *to_vector(lua_State *L, int idx) {
	auto *nv = (NativeVector *)luaL_checkudata(L, idx, LUANATIVE_OBJECT);
	if(nv->hdr.type == NTYPE_VECTOR3) return &nv->data;
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
