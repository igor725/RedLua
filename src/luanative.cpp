#include <map>
#include <string>
#include <fstream>
#include <stdlib.h>
#include "thirdparty\easyloggingpp.h"
#include "thirdparty\ScriptHook\inc\nativeCaller.h"
#include "thirdparty\ScriptHook\inc\types.h"
#include "thirdparty\json.hpp"
#include "luanative.h"

using json = nlohmann::json;
NativeNamespaces Natives;

typedef struct _RefMap {int ns, nc;} RefMap;
std::map<lua_State *, RefMap> ref_map {};
std::map<NativeType, std::map<int, int>> native_cache {};
std::map<NativeType, NativeTypeInfo> param_types {
	{-1, {-1, false, "UnknownType"}},
	{ 0, { 0, false, "void"}},
	{ 1, { 0, false, "int"}},
	{ 2, { 0,  true, "int*"}},
	{ 3, { 0, false, "float"}},
	{ 4, { 0,  true, "float*"}},
	{ 5, { 0, false, "BOOL"}},
	{ 6, { 0,  true, "BOOL*"}},
	{ 7, { 0,  true, "char*"}},
	{ 8, { 0,  true, "const char*"}},
	{ 9, { 0, false, "Any"}},
	{10, { 0,  true, "Any*"}},
	{11, { 0, false, "Blip"}},
	{12, { 0,  true, "Blip*"}},
	{13, { 0, false, "Cam"}},
	{14, { 0,  true, "Cam*"}},
	{15, { 0, false, "Entity"}},
	{16, { 0,  true, "Entity*"}},
	{17, { 0, false, "FireId"}},
	{18, { 0,  true, "FireId*"}},
	{19, { 0, false, "Hash"}},
	{20, { 0,  true, "Hash*"}},
	{21, { 0, false, "Interior"}},
	{22, { 0,  true, "Interior*"}},
	{23, { 0, false, "ItemSet"}},
	{24, { 0,  true, "ItemSet*"}},
	{25, { 0, false, "Object"}},
	{26, { 0,  true, "Object*"}},
	{27, {16, false, "Ped"}},
	{28, {17,  true, "Ped*"}},
	{29, { 0, false, "Pickup"}},
	{30, { 0,  true, "Pickup*"}},
	{31, { 0, false, "Player"}},
	{32, { 0,  true, "Player*"}},
	{33, { 0, false, "ScrHandle"}},
	{34, { 0,  true, "ScrHandle*"}},
	{35, {16, false, "Vector3"}},
	{36, {17,  true, "Vector3*"}},
	{37, { 0, false, "Vehicle"}},
	{38, { 0,  true, "Vehicle*"}},
	{39, { 0, false, "AnimScene"}},
	{40, { 0,  true, "AnimScene*"}},
	{41, { 0, false, "PersChar"}},
	{42, { 0,  true, "PersChar*"}},
	{43, { 0, false, "PopZone"}},
	{44, { 0,  true, "PopZone*"}},
	{45, { 0, false, "Prompt"}},
	{46, { 0,  true, "Prompt*"}},
	{47, { 0, false, "PropSet"}},
	{48, { 0,  true, "PropSet*"}},
	{49, { 0, false, "Volume"}},
	{50, { 0,  true, "Volume*"}}
};

static NativeTypeInfo& get_type_info(NativeType id) {
	return param_types[id];
}

static NativeType get_type(std::string& name) {
	for(auto& i : param_types) {
		if(i.second.name == name)
			return i.first;
	}

	return -1;
}

static int from_cache(NativeType type, int id) {
	if(native_cache.find(type) == native_cache.end())
		return 0;
	if(native_cache[type].find(id) == native_cache[type].end())
		return 0;
	
	return native_cache[type][id];
}

NativeReturn native_reload(void) {
	std::ifstream fnatives("RedLua\\natives.json");
	if(!fnatives.is_open()) return NLOAD_JSON_PARSE;
	json jnatives; fnatives >> jnatives;
	fnatives.close();
	if(!jnatives.is_object()) return NLOAD_NONOBJECT;

	for(auto& inspace : jnatives.items()) {
		json& nspace = inspace.value();
		if(nspace.is_object()) {
			NativeNamespace& _nmeths = Natives[inspace.key()];
			for(auto& inmeth : nspace.items()) {
				json& nmeth = inmeth.value();
				if(nmeth.is_object()) {
					if(!(nmeth["name"]).is_string())
						return NLOAD_METHOD_NONSTRING_NAME;
					std::string temp;
					NativeMeth& _meth = _nmeths[nmeth["name"].get_to(temp)];
					if((nmeth["return_type"]).is_string()) {
						_meth.ret = get_type((nmeth["return_type"]).get_to(temp));
						if(_meth.ret == -1) return NLOAD_METHOD_INVALID_RETURN_TYPE;
					} else return NLOAD_METHOD_NONSTRING_RETURN_TYPE;

					if((nmeth["params"]).is_array()) {
						NativeParams& _params = _meth.params;
						int argn = 0;
						for(auto& inmparam : nmeth["params"]) {
							if(inmparam.is_object()) {
								if((inmparam["type"]).get_to(temp).size() > 0) {
									NativeParam& _param = _params[argn++];
									_param.type = get_type(temp);
									if(_param.type == -1) return NLOAD_METHOD_PARAM_INVALID_TYPE;
									if((inmparam["name"]).is_string())
										(inmparam["name"]).get_to(_param.name);
									else return NLOAD_METHOD_PARAM_NONSTRING_TYPE;
								} else {
									_meth.isVararg = true;
									break;
								}
							} else return NLOAD_METHOD_PARAM_NONOBJECT;
						}
					} else return NLOAD_METHOD_PARAMS_NONARRAY;

					_meth.hash = std::stoull(inmeth.key().c_str(), nullptr, 16);
				}
			}
		}
	}

	return NLOAD_OK;
}

static void get_value(lua_State *L, int idx, int extype, PUINT64 val) {
	union {
		float f;
		UINT64 u64;
	} c;

	if(extype == -1) {
		float temp;
		switch(lua_type(L, idx)) {
			case LUA_TNUMBER: // Немного костыльно определяем, че передаём, целое число или же плавающее
				temp = (float)lua_tonumber(L, idx);
				extype = temp == (int)temp ? 1 : 3;
				break;
			case LUA_TBOOLEAN:
				extype = 5;
				break;
			case LUA_TSTRING:
				extype = 8;
				break;
			default:
				extype = 0;
				break;
		}
	}

	switch(extype) {
		case 1:
			*val = luaL_checkinteger(L, idx);
			return;
		case 3:
			c.f = (float)luaL_checknumber(L, idx);
			*val = c.u64;
			return;
		case 5:
			*val = lua_toboolean(L, idx);
			return;
		case 8:
			*val = (UINT64)luaL_checkstring(L, idx);
			return;
	}

	if(extype == 0) luaL_error(L, "Attempt to push unsupported value");
	auto *no = (NativeObject *)luaL_checkudata(L, idx, "NativeObject");

	if(extype != -1 && extype != no->hdr.type) {
		NativeTypeInfo &nti_obj = get_type_info(no->hdr.type),
		&nti_exp = get_type_info(extype);
		if(extype != nti_obj.superType)
			if(nti_exp.isPointer && (extype - no->hdr.type) != 1 && (extype - nti_obj.superType) != 1)
				luaL_error(L, "bad argument #%d (%s expected, got %s)", idx, nti_exp.name.c_str(), nti_obj.name.c_str());
		
		if(nti_exp.isPointer == nti_obj.isPointer)
			*val = no->hash.u64;
		else if(nti_exp.isPointer && !nti_obj.isPointer)
			*val = (UINT64)&no->hash.u64;
		else
			*val = *no->hash.pu64;
	} else
		*val = no->hash.u64;
}

static void push_native(lua_State *L, NativeType type, int id) {
	int cached;
	if((cached = from_cache(type, id)) > 0) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref_map[L].nc);
		lua_rawgeti(L, -1, cached);
		if(!lua_isnil(L, -1)) {
			lua_remove(L, -2);
			return;
		}

		lua_pop(L, 2);
	} else
		cached = (int)lua_objlen(L, -2) + 1;

	auto *no = (NativeObject *)lua_newuserdata(L, sizeof(NativeObject));	
	luaL_setmetatable(L, "NativeObject");
	native_cache[type][id] = cached;
	no->hash.i32 = id;
	no->hdr.type = type;
	no->hdr.size = 1;

	// Сохраняем в кеш
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref_map[L].nc);
	lua_pushvalue(L, -2);
	lua_rawseti(L, -2, cached);
	lua_pop(L, 1);
}

static int push_value(lua_State *L, NativeType type, PUINT64 val) {
	switch(type) {
		case 0: return 0;

		case 1:
		case 9:
			lua_pushinteger(L, *val);
			break;
		case 3:
			lua_pushnumber(L, *(float *)val);
			break;
		case 5:
			lua_pushboolean(L, (int)*val);
			break;
		case 8:
			lua_pushstring(L, *(const char **)val);
			break;

		default:
			push_native(L, type, *(int *)val);
			break;
	}

	return 1;
}

static NativeMeth *native_search(std::string nspace, std::string meth) {
	if(Natives.find(nspace) == Natives.end())
		return nullptr;
	if(Natives[nspace].find(meth) == Natives[nspace].end())
		return nullptr;

	return &Natives[nspace][meth];
}

static int native_prepare(lua_State *L, NativeMeth *meth, int nargs) {
	int exargs = (int)meth->params.size();
	if(exargs > nargs)
		luaL_error(L, "bad argument #%d (%s expected, got no value)",
			nargs + 1, get_type_info(meth->params[nargs].type).name);
	return meth->isVararg ? nargs : exargs;
}

static void native_mid(lua_State *L, NativeMeth *meth, int exargs) {
	int methargs = (int)meth->params.size();
	nativeInit(meth->hash);
	for(int i = 0; i < exargs; i++) {
		UINT64 value;
		get_value(L, i + 3, i < methargs ? meth->params[i].type : -1, &value);
		nativePush(value);
	}
}

static int native_call(lua_State *L) {
	if(Natives.size() == 0)
		luaL_error(L, "Natives database is empty or was loaded incorrectly");
	NativeMeth *_meth = native_search(
		luaL_checkstring(L, 1),
		luaL_checkstring(L, 2)
	);
	if(_meth == nullptr)
		luaL_error(L, "Method not found");
	
	int exargs = native_prepare(L, _meth, lua_gettop(L) - 2);
	native_mid(L, _meth, exargs);

	return push_value(L, _meth->ret, nativeCall());
}

static int native_calla(lua_State *L) {
	int top = lua_gettop(L);
	nativeInit(luaL_checkinteger(L, 1));
	for(int i = 2; i <= top; i++)
		nativePush(luaL_checkinteger(L, i));
	lua_pushinteger(L, *nativeCall());
	return 1;
}

static int native_pool(lua_State *L) {
	NativeType type_idx = get_type((std::string)luaL_checkstring(L, 1));
	NativeTypeInfo type = get_type_info(type_idx);
	if(type_idx < 2 || type.isPointer)
		luaL_error(L, "bad argument #1 (unsupported type passed)");

	uint size = (uint)luaL_checkinteger(L, 2);
	auto *hdr = (NativeObjectHeader *)lua_newuserdata(L, sizeof(NativeObjectHeader) + (size * 4));
	luaL_setmetatable(L, "NativeObject");
	hdr->type = type_idx + 1;
	hdr->size = size;
	return 1;
}

#define WORLDGETALL(T, TN) { \
	BYTE *ptr = (BYTE *)luaL_checkudata(L, 1, "NativeObject"); \
	auto *hdr = (NativeObjectHeader *)ptr; ptr += sizeof(NativeObjectHeader); \
	if(hdr->type != T) luaL_error(L, "Not a " #TN " pool"); \
	lua_pushinteger(L, worldGetAll##TN((int *)ptr, hdr->size)); \
	return 1; \
}

static int native_allobjs(lua_State *L) WORLDGETALL(27, Objects)
static int native_allpeds(lua_State *L) WORLDGETALL(29, Peds)
static int native_allpick(lua_State *L) WORLDGETALL(31, Pickups)
static int native_allvehs(lua_State *L) WORLDGETALL(39, Vehicles)

static const luaL_Reg nativelib[] = {
	{"call", native_call},
	{"calla", native_calla},
	{"pool", native_pool},

	{"allobjects", native_allobjs},
	{"allpeds", native_allpeds},
	{"allpickups", native_allpick},
	{"allvehicles", native_allvehs},

	{NULL, NULL}
};

static int native_tostring(lua_State *L) {
	auto *no = (NativeObject *)luaL_checkudata(L, 1, "NativeObject");
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
	auto *no = (NativeObject *)luaL_checkudata(L, 1, "NativeObject");
	NativeTypeInfo &ni = get_type_info(no->hdr.type);
	if(!ni.isPointer) return 0;
	uint idx = (uint)luaL_checkinteger(L, 2);
	if(idx > no->hdr.size) return 0;
	((int *)&no->hash.p)[idx] = (int)luaL_checkinteger(L, 3);
	return 0;
}

static int native_index(lua_State *L) {
	auto *no = (NativeObject *)luaL_checkudata(L, 1, "NativeObject");
	NativeTypeInfo &ni = get_type_info(no->hdr.type);
	uint idx = (uint)lua_tointeger(L, 2);
	if(!ni.isPointer || idx > no->hdr.size) return 0;

	switch(no->hdr.type) {
		case 1:
		case 9:
			lua_pushinteger(L, ((int *)&no->hash.p)[idx]);
			break;
		case 3:
			lua_pushnumber(L, ((float *)&no->hash.p)[idx]);
			break;
		case 5:
			lua_pushboolean(L, ((BOOL *)&no->hash.p)[idx]);
			break;
		case 7:
		case 8:
			lua_pushstring(L, ((const char **)&no->hash.p)[idx]);
			break;
		
		default:
			push_native(L, no->hdr.type - 1, ((int *)&no->hash.p)[idx]);
			break;
	}

	return 1;
}

static const luaL_Reg nativeobj[] = {
	{"__tostring", native_tostring},
	{"__newindex", native_newindex},
	{"__index", native_index},

	{NULL, NULL}
};

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
	int exargs = native_prepare(L, _meth, nargs - 2);
	native_mid(L, _meth, exargs);
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
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref_map[L].ns);
		lua_rawgeti(L, -1, 1);
		int mt_top = (int)lua_objlen(L, -1) + 1;
		lua_rawgeti(L, -2, 2);
		lua_pushvalue(L, 2);
		lua_rawseti(L, -2, mt_top);
		lua_pop(L, 2);
	}

	return 1;
}

int luaopen_native(lua_State *L) {
	if(Natives.size() == 0) {
		NativeReturn nret;
		if((nret = native_reload()) != NLOAD_OK)
			LOG(ERROR) << "Failed to load native.json: " << nret;
	}

	luaL_newmetatable(L, "NativeObject");
	lua_pushstring(L, "none of your business");
	lua_setfield(L, -2, "__metatable");
	luaL_setfuncs(L, nativeobj, 0);
	lua_pop(L, 1);

	lua_createtable(L, 0, 0);
	lua_createtable(L, 0, 1);
	lua_pushstring(L, "v");
	lua_setfield(L, -2, "__mode");
	lua_setmetatable(L, -2);
	ref_map[L].nc = luaL_ref(L, LUA_REGISTRYINDEX);

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
	ref_map[L].ns = luaL_ref(L, LUA_REGISTRYINDEX);

	lua_createtable(L, 0, 1);
	lua_pushcfunction(L, global_index);
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, LUA_GLOBALSINDEX);

	luaL_newlib(L, nativelib);
	return 1;
}
