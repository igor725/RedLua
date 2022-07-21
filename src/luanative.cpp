#include "thirdparty\easyloggingpp.h"

#include "luanative.hpp"

#include "native\object.hpp"
#include "native\types.hpp"
#include "native\typemap.hpp"
#include "native\call.hpp"
#include "native\db.hpp"

NativeNamespaces Natives;

static int native_call(lua_State *L) {
	if(Natives.size() == 0)
		luaL_error(L, "Natives database is empty or was loaded incorrectly");
	auto nspace = native_namespace(luaL_checkstring(L, 1));
	luaL_argcheck(L, nspace != nullptr, 1, "Unknown namespace");
	auto meth = native_method(nspace, luaL_checkstring(L, 2));
	luaL_argcheck(L, meth != nullptr, 2, "Unknown method");

	native_prepare(L, meth, lua_gettop(L) - 2);
	return native_perform(L, meth);
}

static int native_info(lua_State *L) {
	auto nspace = native_namespace(luaL_checkstring(L, 1));
	if(nspace == nullptr) return 0;
	auto meth = native_method(nspace, luaL_checkstring(L, 2));
	if(meth == nullptr) return 0;

	lua_createtable(L, 0, 4);
	lua_pushfstring(L, "%p", meth->hash);
	lua_setfield(L, -2, "hash");
	lua_pushnumber(L, meth->firstSeen);
	lua_setfield(L, -2, "build");
	lua_pushstring(L, get_type_info(meth->returns).name.c_str());
	lua_setfield(L, -2, "returns");

	int argc = (int)meth->params.size();
	if(argc > 0) {
		lua_createtable(L, argc, 0);
		for(int i = 0; i < argc; i++) {
			NativeParam &param = meth->params[i];
			lua_createtable(L, 0, 2);
			lua_pushstring(L, get_type_info(param.type).name.c_str());
			lua_setfield(L, -2, "type");
			lua_pushstring(L, param.name.c_str());
			lua_setfield(L, -2, "name");
			lua_rawseti(L, -2, i + 1);
		}
		lua_setfield(L, -2, "params");
	}

	return 1;
}

static int native_new(lua_State *L) {
	std::string stype = luaL_checkstring(L, 1);
	uint count = (uint)luaL_checkinteger(L, 2);
	NativeType type = get_type(stype);
	NativeTypeInfo nti = get_type_info(type);

	// Считаем размер структуры NativeObjectHeader вместе с выравниванием
	size_t objsize = (sizeof(NativeObject) - sizeof(NativeData));
	objsize += nti.size * count;
	auto no = (NativeObject *)lua_newuserdata(L, objsize);
	NATIVEOBJECT_INIT(no, type, false, false, count, 0);
	luaL_setmetatable(L, LUANATIVE_OBJECT);
	memset(&no->content, 0, nti.size * count);
	return 1;
}

#define VTN(idx) (float)lua_tonumber(L, idx)
static int native_vector(lua_State *L) {
	Vector3 pos {VTN(1), VTN(2), VTN(3)};

	return 0;
}

#ifndef REDLUA_STANDALONE
#define WORLDGETALL(T, TN) { \
	auto no = (NativeObject *)luaL_checkudata(L, 1, LUANATIVE_OBJECT); \
	luaL_argcheck(L, no->hdr.type != T, 1, "not a" #TN " pool"); \
	lua_pushinteger(L, worldGetAll##TN((int *)NATIVEOBJECT_GETPTR(no), no->hdr.count)); \
	return 1; \
}
#else
#define WORLDGETALL(T, TN) {lua_pushinteger(L, 0); return 1;}
#endif

static int native_allobjs(lua_State *L) WORLDGETALL(NTYPE_OBJECT, Objects)
static int native_allpeds(lua_State *L) WORLDGETALL(NTYPE_PED, Peds)
static int native_allpick(lua_State *L) WORLDGETALL(NTYPE_PICKUP, Pickups)
static int native_allvehs(lua_State *L) WORLDGETALL(NTYPE_VEHICLE, Vehicles)

static const luaL_Reg nativelib[] = {
	{"call", native_call},
	{"info", native_info},

	{"new", native_new},
	{"vector", native_vector},

	{"allobjects", native_allobjs},
	{"allpeds", native_allpeds},
	{"allpickups", native_allpick},
	{"allvehicles", native_allvehs},

	{NULL, NULL}
};

int luaopen_native(lua_State *L) {
	if(Natives.size() == 0) {
		NativeReturn nret;
		if((nret = native_reload()) != NLOAD_OK)
			LOG(ERROR) << "Failed to load native.json: " << nret;
	}

	call_init(L);
	nativeobj_init(L);
	luaL_newlib(L, nativelib);
	return 1;
}
