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

	NativeMeth *meth = native_search(
		luaL_checkstring(L, 1),
		luaL_checkstring(L, 2)
	);
	if(meth == nullptr)
		luaL_error(L, "Method not found");

	native_prepare(L, meth, lua_gettop(L) - 2);
	return native_perform(L, meth);
}

static int native_info(lua_State *L) {
	auto meth = native_search(
		luaL_checkstring(L, 1),
		luaL_checkstring(L, 2)
	);

	lua_createtable(L, 0, 4);
	lua_pushfstring(L, "%p", meth->hash);
	lua_setfield(L, -2, "hash");
	lua_pushnumber(L, meth->firstSeen);
	lua_setfield(L, -2, "build");
	lua_pushstring(L, get_type_info(meth->ret).name.c_str());
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
	NativeType type_idx = get_type((std::string)luaL_checkstring(L, 1));
	NativeTypeInfo &type = get_type_info(type_idx);
	luaL_argcheck(L, type_idx > NTYPE_VOID && !type.isPointer, 1, "unsupported type passed");

	uint count = (uint)luaL_checkinteger(L, 2);
	uint objsize = (count * type.size);
	auto hdr = (NativeObjectHeader *)lua_newuserdata(L, NATHDR_SZ + objsize);
	NATHDR_INIT(*hdr, type_idx + 1, count, false);
	luaL_setmetatable(L, LUANATIVE_OBJECT);
	memset(&(hdr[1]), 0, objsize);
	return 1;
}

#define VTN(idx) (float)lua_tonumber(L, idx)
static int native_vector(lua_State *L) {
	Vector3 pos {VTN(1), VTN(2), VTN(3)};
	push_vector(L, &pos);
	return 1;
}

#define WORLDGETALL(T, TN) { \
	auto ptr = (char *)luaL_checkudata(L, 1, LUANATIVE_OBJECT); \
	auto hdr = (NativeObjectHeader *)ptr; ptr += sizeof(NativeObjectHeader); \
	luaL_argcheck(L, hdr->type != T, 1, "not a" #TN " pool"); \
	lua_pushinteger(L, worldGetAll##TN((int *)ptr, hdr->size)); \
	return 1; \
}
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
