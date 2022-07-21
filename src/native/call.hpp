#pragma once

#ifndef REDLUA_STANDALONE
#include "thirdparty\ScriptHook\inc\nativeCaller.h"
#else
#include "thirdparty\easyloggingpp.h"

static void nativeInit(UINT64 hash) {
	LOG(INFO) << "[NC] START " << (void *)hash;
}

template <typename T>
static inline void nativePush(T val) {
	UINT64 val64 = 0;
	if (sizeof(T) > sizeof(UINT64)) {
		throw "error, value size > 64 bit";
	}
	*reinterpret_cast<T *>(&val64) = val;
	LOG(INFO) << "\t [NC] Argument: " << (void *)val64;
}

static PUINT64 nativeCall() {
	static UINT64 natret = 0;
	LOG(INFO) << "[NC] END";
	return &natret;
}
#endif

#include "thirdparty\ScriptHook\inc\types.h"

#include "native\db.hpp"
#include "native\cache.hpp"
#include "native\object.hpp"

static int native_prepare_arg_error(lua_State *L, NativeParam *param, int idx) {
	static std::string ptr[] = {"*", ""};
	return luaL_typerror(L, idx,
		(get_type_info(param->type).name + ptr[param->isPointer]).c_str());
}

#define IS_TYPES_EQU(AT, A, BT) ((AT) == (BT) || (A).superType == (BT))

static int native_prepare_nobj(lua_State *L, NativeParam *param, bool vector_allowed, int idx) {
	auto no = (NativeObject *)luaL_checkudata(L, idx, LUANATIVE_OBJECT);
	auto nti_no = get_type_info(no->hdr.type);

	if(IS_TYPES_EQU(no->hdr.type, nti_no, param->type)) {
		if(param->isPointer == no->hdr.isPointer)
			return (nativePush(no->content.nd), 1);
		else if(param->isPointer)
			return (nativePush(&no->content.nd), 1);
		else
			return native_prepare_arg_error(L, param, idx);
	}

	return 0;
}

static int native_prepare_arg(lua_State *L, NativeParam *param, bool vector_allowed, int idx) {
	float temp;

	switch(lua_type(L, idx)) {
		case LUA_TNIL:
			if(param->isPointer)
				return (nativePush(nullptr), 1);
			break;
		case LUA_TBOOLEAN:
			if(param->type == NTYPE_BOOL && !param->isPointer)
				return (nativePush(lua_toboolean(L, idx)), 1);

			break;
		case LUA_TNUMBER:
			if(!param->isPointer) {
				temp = (float)lua_tonumber(L, idx);
				if(temp == (int)temp && param->type == NTYPE_FLOAT)
					return (nativePush(temp), 1);
				else if(param->type == NTYPE_INT)
					return (nativePush((int)temp), 1);
			}
			break;
		case LUA_TSTRING:
			if(param->type == NTYPE_CHAR && param->isPointer /*&&param->isConst*/)
				return (nativePush(lua_tostring(L, idx)), 1);
			break;
		case LUA_TUSERDATA:
			return native_prepare_nobj(L, param, vector_allowed, idx);
		case 10/*LUA_TCDATA*/:
			if(param->type == NTYPE_ANY && param->isPointer)
				return (nativePush(lua_topointer(L, idx)), 1);
			break;
	}

	return native_prepare_arg_error(L, param, idx);
}

static void native_prepare(lua_State *L, NativeMeth *meth, int nargs) {
	int methargs = (int)meth->params.size(), idx = 3;
	int iend = (meth->isVararg && (nargs > methargs)) ? nargs : methargs;
	nativeInit(meth->hash);
	for(int i = 0; i < iend; idx++) {
		if(i >= nargs) luaL_error(L, "insufficient arguments (%d expected, got %d)", iend, i);
		bool vector_allowed = (methargs - i >= 3) && meth->params[i].type == NTYPE_FLOAT &&
			meth->params[i + 1].type == NTYPE_FLOAT && meth->params[i + 2].type == NTYPE_FLOAT;
		i += native_prepare_arg(L, (i < methargs ? &meth->params[i] : nullptr), vector_allowed, idx);
	}
}

static int native_perform(lua_State *L, NativeMeth *meth) {
	PUINT64 ret = nativeCall();
	if(meth->returns == NTYPE_VOID || meth->returns == NTYPE_UNKNOWN)
		return 0;

	if(!meth->isRetPtr) {
		switch(meth->returns) {
			case NTYPE_INT:
			case NTYPE_ANY:
			case NTYPE_HASH:
				lua_pushinteger(L, *ret);
				break;
			
			case NTYPE_FLOAT:
				lua_pushnumber(L, *(float *)ret);
				break;
			
			case NTYPE_BOOL:
				lua_pushboolean(L, *(int *)ret);
				break;
			
			case NTYPE_CHAR:
				lua_pushlstring(L, (char *)ret, 1);
				break;
			
			default:
				push_cached_fullobject(L, meth->returns, (NativeData)*ret);
				break;
		}
	} else
		push_uncached_lightobjectcopy(L, meth->returns, ret);

	return 1;
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
	auto meth = (NativeMeth *)lua_touserdata(L, -1);
	native_prepare(L, meth, nargs - 2);
	return native_perform(L, meth);
}

static const luaL_Reg methmeta[] = {
	{"__newindex", generic_newindex},
	{"__call", meth_call},

	{NULL, NULL}
};

static int nspace_index(lua_State *L) {
	luaL_checktype(L, 2, LUA_TSTRING);
	lua_rawgeti(L, 1, 1);
	int mt_top = (int)lua_objlen(L, -1) + 1;
	lua_rawgeti(L, 1, 2);
	lua_rawgeti(L, -1, mt_top);
	auto meth = native_method(
		(NativeNamespace *)lua_touserdata(L, -1),
		lua_tostring(L, 2)
	);
	if(meth == nullptr) {
		lua_pushnil(L);
		return 1;
	}
	lua_pushlightuserdata(L, meth);
	lua_rawseti(L, -4, mt_top);
	lua_pop(L, 2);
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
		auto nspace = native_namespace(lua_tostring(L, 2));
		if(nspace == nullptr) return 1; // Возвращаем тот nil, что нам дал rawget выше

		lua_pop(L, 1);
		lua_rawgeti(L, LUA_REGISTRYINDEX, ReferenceMap[L].ns);
		lua_rawgeti(L, -1, 1);
		int mt_top = (int)lua_objlen(L, -1) + 1;
		lua_rawgeti(L, -2, 2);
		lua_pushlightuserdata(L, nspace);
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
