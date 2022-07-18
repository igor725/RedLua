#pragma once

#include "thirdparty\ScriptHook\inc\nativeCaller.h"
#include "thirdparty\ScriptHook\inc\types.h"

#include "native/db.hpp"
#include "native/cache.hpp"
#include "native/object.hpp"

#define IS_PTR_OF(t1, ti1, t2, ti2) (ti1.isPointer && (((t1) - (t2)) == 1 || ((t1) - (ti2).superType) == 1))

static void get_value(lua_State *L, int idx, NativeType extype, PUINT64 val) {
	union {
		float f;
		UINT64 u64;
	} c;

	if(extype == NTYPE_UNKNOWN || extype == NTYPE_ANY) {
		float temp;
		switch(lua_type(L, idx)) {
			case LUA_TNUMBER: // Немного костыльно определяем, че передаём, целое число или же плавающее
				temp = (float)lua_tonumber(L, idx);
				extype = temp == (int)temp ? NTYPE_INT : NTYPE_FLOAT;
				break;
			case LUA_TBOOLEAN:
				extype = NTYPE_BOOL;
				break;
			case LUA_TSTRING:
				extype = NTYPE_STRING;
				break;
			case LUA_TUSERDATA:
				extype = NTYPE_ANY;
				break;
			default:
				extype = NTYPE_VOID;
				break;
		}
	}

	switch(extype) {
		case NTYPE_INT:
			*val = luaL_checkinteger(L, idx);
			return;
		case NTYPE_FLOAT:
			c.f = (float)luaL_checknumber(L, idx);
			*val = c.u64;
			return;
		case NTYPE_BOOL:
			*val = lua_toboolean(L, idx);
			return;
		case NTYPE_STRING:
			*val = (UINT64)luaL_checkstring(L, idx);
			return;
	}

	auto *no = (NativeObject *)luaL_checkudata(L, idx, LUANATIVE_OBJECT);
	NativeTypeInfo &nti_obj = get_type_info(no->hdr.type),
	&nti_exp = get_type_info(extype);
	luaL_argcheck(L, !nti_exp.isPointer || !no->hdr.readonly, idx,
		"attempt to push an readonly object as a pointer");

	if(extype != no->hdr.type && (extype < NTYPE_ANY || extype > NTYPE_ANYPTR)) {
		if(extype != nti_obj.superType && nti_exp.superType != nti_obj.superType
		&& !IS_PTR_OF(extype, nti_exp, no->hdr.type, nti_obj))
			luaL_error(L, "bad argument #%d (%s expected, got %s)",
				idx, nti_exp.name.c_str(), nti_obj.name.c_str());

		if(nti_exp.isPointer)
			*val = (UINT64)&no->hash.u64;
		else *val = no->hash.u64;
	} else if(nti_exp.isPointer)
		*val = (UINT64)&no->hash.u64;
	else *val = no->hash.u64;
}

static void native_prepare(lua_State *L, NativeMeth *meth, int nargs) {
	int methargs = (int)meth->params.size(), idx = 3;
	int iend = (meth->isVararg && (nargs > methargs)) ? nargs : methargs;
	nativeInit(meth->hash);
	for(int i = 0; i < iend; i++, idx++) {
		if(i >= nargs) luaL_error(L, "insufficient arguments (%d expected, got %d)", iend, i);
		NativeType extype = (i < methargs) ? meth->params[i].type : NTYPE_UNKNOWN;
		// Осуществляем подмену трёх float значения на вектор, если нужно
		if(extype == NTYPE_FLOAT && methargs - i >= 3) {
			if(meth->params[i + 1].type == NTYPE_FLOAT
			&& meth->params[i + 2].type == NTYPE_FLOAT) {
				auto *vec = (PUINT64)to_vector(L, idx);
				if(vec != nullptr) {
					for(int j = 0; j < 3; j++, i++)
						nativePush(vec[j]);
					continue;
				}
			}
		}

		if(extype != NTYPE_VECTOR3) {
			UINT64 value;
			get_value(L, idx, extype, &value);
			nativePush(value);
		} else {
			auto *vec = (PUINT64)check_vector(L, idx);
			for(int j = 0; j < 3; j++)
				nativePush(vec[j]);
		}
	}
}

static int generic_newindex(lua_State *L) {
	luaL_error(L, "Attempt to modify readonly object");
	return 0;
}

static int meth_call(lua_State *L) {
	int nargs = lua_gettop(L);
	int mt_top = (int)lua_objlen(L, 1);
	lua_rawgeti(L, 1, mt_top);
	lua_pushnil(L);
	lua_rawseti(L, 1, mt_top);
	lua_rawgeti(L, 2, 2);
	lua_rawgeti(L, -1, mt_top);
	lua_pushnil(L);
	lua_rawseti(L, -3, mt_top);
	NativeMeth *_meth = native_search(lua_tostring(L, -1), lua_tostring(L, -3));
	if(!_meth) luaL_error(L, "Method not found");
	native_prepare(L, _meth, nargs - 2);
	return push_value(L, _meth->ret, nativeCall());
}

static const luaL_Reg methmeta[] = {
	{"__newindex", generic_newindex},
	{"__call", meth_call},

	{NULL, NULL}
};

static int nspace_index(lua_State *L) {
	luaL_checktype(L, 2, LUA_TSTRING);
	lua_rawgeti(L, 1, 1);
	lua_pushvalue(L, 2);
	lua_rawseti(L, -2, (int)lua_objlen(L, -2) + 1);
	return 1;
}

static const luaL_Reg nspacemeta[] = {
	{"__newindex", generic_newindex},
	{"__index", nspace_index},

	{NULL, NULL}
};

static int global_index(lua_State *L) {
	lua_pushvalue(L, 2);
	lua_rawget(L, LUA_GLOBALSINDEX);
	if(lua_isnil(L, -1) && lua_type(L, 2) == LUA_TSTRING) {
		lua_pop(L, 1);
		lua_rawgeti(L, LUA_REGISTRYINDEX, ReferenceMap[L].ns);
		lua_rawgeti(L, -1, 1);
		int mt_top = (int)lua_objlen(L, -1) + 1;
		lua_rawgeti(L, -2, 2);
		lua_pushvalue(L, 2);
		lua_rawseti(L, -2, mt_top);
		lua_pop(L, 2);
	}

	return 1;
}

static void call_init(lua_State *L) {
	lua_createtable(L, 1, 0);
	lua_createtable(L, 0, 0);
	lua_rawseti(L, -2, 2);
	lua_createtable(L, 0, 0);
	lua_createtable(L, 0, 3);
	lua_pushstring(L, "method");
	lua_setfield(L, -2, "__metatable");
	luaL_setfuncs(L, methmeta, 0);
	lua_setmetatable(L, -2);
	lua_rawseti(L, -2, 1);
	lua_createtable(L, 0, 3);
	lua_pushstring(L, "namespace");
	lua_setfield(L, -2, "__metatable");
	luaL_setfuncs(L, nspacemeta, 0);
	lua_setmetatable(L, -2);
	ReferenceMap[L].ns = luaL_ref(L, LUA_REGISTRYINDEX);

	lua_createtable(L, 0, 1);
	lua_pushcfunction(L, global_index);
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, LUA_GLOBALSINDEX);
}
