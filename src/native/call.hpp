#pragma once

#include "scripthook.hpp"

#include "nativedb.hpp"
#include "native\cache.hpp"
#include "native\object.hpp"

static int RedLua_NameSpace = -1;

static int native_prepare_arg_error(lua_State *L, NativeParam *param, int idx) {
	static std::string ptr[] = {"", "*"};
	auto tname = get_type_info(param ? param->type : NTYPE_UNKNOWN).name;
	auto msg = lua_pushfstring(L,
		"%s expected, got %s",
		(tname + ptr[param->isPointer]).c_str(),
		luaL_typename(L, idx)
	);

	return luaL_argerror(L, idx - 1, msg);
}

static int native_prepare_nobj(lua_State *L, NativeParam *param, bool vector_allowed, int idx) {
	auto no = (NativeObject *)luaL_checkudata(L, idx, LUANATIVE_OBJECT);

	if (vector_allowed && no->hdr.type == NTYPE_VECTOR3) {
		auto vec = (float *)NATIVEOBJECT_GETPTR(no);
		for (int i = 0; i < 3; i++)
			nativePush(vec[i * 2]);
		return 3;
	}

	NativeTypeInfo &nti_obj = get_type_info(no->hdr.type),
	&nti_param = get_type_info(param->type);

	if (!param || IS_NATIVETYPES_EQU(param->type, nti_param, no->hdr.type, nti_obj)) {
		if (param ? (param->isPointer == no->hdr.isPointer) : no->hdr.isPointer)
			return (nativePush(no->content.nd), 1);
		else if (!param || param->isPointer)
			return (nativePush(&no->content.nd), 1);
		else if (no->hdr.isPointer)
			return (nativePush(*no->content.pp), 1);
	}

	return native_prepare_arg_error(L, param, idx);
}

static int native_prepare_arg(lua_State *L, NativeParam *param, bool vector_allowed, int idx) {
	float temp;

	switch (lua_type(L, idx)) {
		case LUA_TNIL:
			if (!param || param->isPointer)
				return (nativePush(nullptr), 1);
			break;
		case LUA_TBOOLEAN:
			if (!param || (param->type == NTYPE_BOOL && !param->isPointer))
				return (nativePush(lua_toboolean(L, idx)), 1);

			break;
		case LUA_TNUMBER:
			if (!param || !param->isPointer) {
				temp = (float)lua_tonumber(L, idx);
				if (param->type == NTYPE_FLOAT)
					return (nativePush(temp), 1);
				else if (param->type == NTYPE_INT || param->type == NTYPE_HASH)
					return (nativePush((int)temp), 1);
			}
			break;
		case LUA_TSTRING:
			if ((!param || param->type == NTYPE_CHAR && param->isPointer /*&&param->isConst*/))
				return (nativePush(lua_tostring(L, idx)), 1);
			break;
		case LUA_TUSERDATA:
			return native_prepare_nobj(L, param, vector_allowed, idx);
		case 10/*LUA_TCDATA*/:
			if (!param || (param->type == NTYPE_ANY && param->isPointer))
				return (nativePush(lua_topointer(L, idx)), 1);
			break;
	}

	return native_prepare_arg_error(L, param, idx);
}

static void native_prepare(lua_State *L, NativeMeth *meth, int nargs) {
	int methargs = (int)meth->params.size(), idx = 3;
	int iend = (meth->isVararg && (nargs > methargs)) ? nargs : methargs;
	nativeInit(meth->hash);
	for (int i = 0; i < iend; idx++) {
		if ((idx - nargs) == 3) luaL_error(L, "insufficient arguments (%d expected, got %d)", iend, i);
		bool vector_allowed = (methargs - i >= 3) && meth->params[i].type == NTYPE_FLOAT &&
			meth->params[i + 1].type == NTYPE_FLOAT && meth->params[i + 2].type == NTYPE_FLOAT;
		i += native_prepare_arg(L, (i < methargs ? &meth->params[i] : nullptr), vector_allowed, idx);
	}
}

static int native_perform(lua_State *L, NativeMeth *meth) {
	PUINT64 ret = nativeCall();
	if (meth->returns == NTYPE_VOID || meth->returns == NTYPE_UNKNOWN)
		return 0;

	if (!meth->isRetPtr) {
		switch (meth->returns) {
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
	luaL_error(L, "attempt to modify readonly object");
	return 0;
}

static int meth_call(lua_State *L) {
	int nargs = lua_gettop(L);
	lua_rawgeti(L, LUA_REGISTRYINDEX, RedLua_NameSpace);
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
	if (lua_type(L, 2) != LUA_TSTRING)
		luaL_error(L, "index must be a string");
	lua_rawgeti(L, 1, 1); // Снова вытаскиваем таблицу методов
	int mt_top = (int)lua_objlen(L, -1) + 1;
	lua_rawgeti(L, 1, 2); // А теперь и стек
	lua_rawgeti(L, -1, mt_top); // Получаем из стека неймспейсов тот, что нужен для текущего метода
	auto meth = Natives.GetMethod(
		(NativeNamespace *)lua_touserdata(L, -1),
		lua_tostring(L, 2)
	);
	if (meth == nullptr)
		luaL_error(L, "unknown method");
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
	if (lua_type(L, 2) != LUA_TSTRING)
		return 0;

	auto nspace = Natives.GetNamespace(lua_tostring(L, 2));
	if (nspace == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	lua_rawgeti(L, LUA_REGISTRYINDEX, RedLua_NameSpace);
	lua_rawgeti(L, -1, 1); // Вытаскиваем из таблицы неймспейса таблицу метода
	int mt_top = (int)lua_objlen(L, -1) + 1; // Свободная ячейка стека неймспейсов
	lua_rawgeti(L, -2, 2); // Получаем стек неймспейсов
	lua_pushlightuserdata(L, nspace);
	lua_rawseti(L, -2, mt_top); // Сохраняем указатель на неймспейс в свободную ячейку стека
	lua_pop(L, 2);
	return 1;
}

static void call_init(lua_State *L) {
	lua_createtable(L, 1, 0); // Создаём таблицу для неймспейса
	lua_createtable(L, 0, 0); // Создаём стек неймспейсов
	lua_rawseti(L, -2, 2); // Сохраняем стек неймспейса в таблицу под индексом 2
	lua_createtable(L, 0, 0); // Создаём таблицу для нативного метода
	lua_createtable(L, 0, 3); // Создаём метатаблицу для нативного метода
	lua_pushstring(L, "method");
	lua_setfield(L, -2, "__metatable");
	luaL_setfuncs(L, methmeta, 0);
	lua_setmetatable(L, -2); // Присваиваем метатаблицу таблице нативного метода
	lua_rawseti(L, -2, 1); // Сохраняем таблицу нативного метода в таблицу неймспейса под индексом 1
	lua_createtable(L, 0, 3); // Создаём метатаблицу для неймспейса
	lua_pushstring(L, "namespace");
	lua_setfield(L, -2, "__metatable");
	luaL_setfuncs(L, nspacemeta, 0);
	lua_setmetatable(L, -2); // Присваиваем метатаблицу таблице для нейспейса
	RedLua_NameSpace = luaL_ref(L, LUA_REGISTRYINDEX);

	/*
		Дальше идёт весёлый хак, который к
		глобальной таблице цепляет метатаблицу.
		Эта штука нужна для красивых обращений к
		нативным функциям через псевдоглобальные
		переменные.
	*/
	lua_createtable(L, 0, 1);
	lua_pushcfunction(L, global_index);
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, LUA_GLOBALSINDEX);
}
