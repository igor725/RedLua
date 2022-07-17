#include <map>
#include <string>
#include <fstream>
#include <stdlib.h>
#include "thirdparty\easyloggingpp.h"
#include "thirdparty\ScriptHook\inc\nativeCaller.h"
#include "thirdparty\ScriptHook\inc\types.h"
#include "thirdparty\json.hpp"
#include "luanative.h"
#include "luavectorfuncs.h"

using json = nlohmann::json;

NativeNamespaces Natives;
typedef struct _RefMap {int ns, nc;} RefMap;
std::map<lua_State *, RefMap> ReferenceMap {};
std::map<int, std::map<NativeType, std::map<int, int>>> NativeCache {};
std::map<NativeType, NativeTypeInfo> NativeTypes {
	{     NTYPE_UNKNOWN, {  NTYPE_UNKNOWN, false, "UnknownType"}},
	{        NTYPE_VOID, {     NTYPE_VOID, false, "void"       }},
	{         NTYPE_INT, {     NTYPE_VOID, false, "int"        }},
	{      NTYPE_INTPTR, {     NTYPE_VOID,  true, "int*"       }},
	{       NTYPE_FLOAT, {     NTYPE_VOID, false, "float"      }},
	{    NTYPE_FLOATPTR, {     NTYPE_VOID,  true, "float*"     }},
	{        NTYPE_BOOL, {     NTYPE_VOID, false, "BOOL"       }},
	{     NTYPE_BOOLPTR, {     NTYPE_VOID,  true, "BOOL*"      }},
	{     NTYPE_CHARPTR, {     NTYPE_VOID,  true, "char*"      }},
	{      NTYPE_STRING, {     NTYPE_VOID,  true, "const char*"}},
	{         NTYPE_ANY, {     NTYPE_VOID, false, "Any"        }},
	{      NTYPE_ANYPTR, {     NTYPE_VOID,  true, "Any*"       }},
	{        NTYPE_BLIP, {     NTYPE_VOID, false, "Blip"       }},
	{     NTYPE_BLIPPTR, {     NTYPE_VOID,  true, "Blip*"      }},
	{         NTYPE_CAM, {     NTYPE_VOID, false, "Cam"        }},
	{      NTYPE_CAMPTR, {     NTYPE_VOID,  true, "Cam*"       }},
	{      NTYPE_ENTITY, {     NTYPE_VOID, false, "Entity"     }},
	{   NTYPE_ENTITYPTR, {     NTYPE_VOID,  true, "Entity*"    }},
	{      NTYPE_FIREID, {     NTYPE_VOID, false, "FireId"     }},
	{   NTYPE_FIREIDPTR, {     NTYPE_VOID,  true, "FireId*"    }},
	{        NTYPE_HASH, {     NTYPE_VOID, false, "Hash"       }},
	{     NTYPE_HASHPTR, {     NTYPE_VOID,  true, "Hash*"      }},
	{    NTYPE_INTERIOR, {     NTYPE_VOID, false, "Interior"   }},
	{ NTYPE_INTERIORPTR, {     NTYPE_VOID,  true, "Interior*"  }},
	{     NTYPE_ITEMSET, {     NTYPE_VOID, false, "ItemSet"    }},
	{  NTYPE_ITEMSETPTR, {     NTYPE_VOID,  true, "ItemSet*"   }},
	{      NTYPE_OBJECT, {   NTYPE_ENTITY, false, "Object"     }},
	{   NTYPE_OBJECTPTR, {NTYPE_ENTITYPTR,  true, "Object*"    }},
	{         NTYPE_PED, {   NTYPE_ENTITY, false, "Ped"        }},
	{      NTYPE_PEDPTR, {NTYPE_ENTITYPTR,  true, "Ped*"       }},
	{      NTYPE_PICKUP, {     NTYPE_VOID, false, "Pickup"     }},
	{   NTYPE_PICKUPPTR, {     NTYPE_VOID,  true, "Pickup*"    }},
	{      NTYPE_PLAYER, {     NTYPE_VOID, false, "Player"     }},
	{   NTYPE_PLAYERPTR, {     NTYPE_VOID,  true, "Player*"    }},
	{   NTYPE_SCRHANDLE, {     NTYPE_VOID, false, "ScrHandle"  }},
	{NTYPE_SCRHANDLEPTR, {     NTYPE_VOID,  true, "ScrHandle*" }},
	{     NTYPE_VECTOR3, {     NTYPE_VOID, false, "Vector3"    }},
	{  NTYPE_VECTOR3PTR, {     NTYPE_VOID,  true, "Vector3*"   }},
	{     NTYPE_VEHICLE, {   NTYPE_ENTITY, false, "Vehicle"    }},
	{  NTYPE_VEHICLEPTR, {NTYPE_ENTITYPTR,  true, "Vehicle*"   }},
	{   NTYPE_ANIMSCENE, {     NTYPE_VOID, false, "AnimScene"  }},
	{NTYPE_ANIMSCENEPTR, {     NTYPE_VOID,  true, "AnimScene*" }},
	{    NTYPE_PERSCHAR, {     NTYPE_VOID, false, "PersChar"   }},
	{ NTYPE_PERSCHARPTR, {     NTYPE_VOID,  true, "PersChar*"  }},
	{     NTYPE_POPZONE, {     NTYPE_VOID, false, "PopZone"    }},
	{  NTYPE_POPZONEPTR, {     NTYPE_VOID,  true, "PopZone*"   }},
	{      NTYPE_PROMPT, {     NTYPE_VOID, false, "Prompt"     }},
	{   NTYPE_PROMPTPTR, {     NTYPE_VOID,  true, "Prompt*"    }},
	{     NTYPE_PROPSET, {     NTYPE_VOID, false, "PropSet"    }},
	{  NTYPE_PROPSETPTR, {     NTYPE_VOID,  true, "PropSet*"   }},
	{      NTYPE_VOLUME, {     NTYPE_VOID, false, "Volume"     }},
	{   NTYPE_VOLUMEPTR, {     NTYPE_VOID,  true, "Volume*"    }}
};

static NativeTypeInfo& get_type_info(NativeType id) {
	if(NativeTypes.find(id) == NativeTypes.end())
		return NativeTypes[NTYPE_UNKNOWN];
	return NativeTypes[id];
}

static NativeType get_type(std::string& name) {
	for(auto& i : NativeTypes) {
		if(i.second.name == name)
			return i.first;
	}

	return NTYPE_UNKNOWN;
}

static int from_cache(int cache_ref, NativeType type, int id) {
	if(NativeCache.find(cache_ref) == NativeCache.end())
		return 0;
	if(NativeCache[cache_ref].find(type) == NativeCache[cache_ref].end())
		return 0;
	if(NativeCache[cache_ref][type].find(id) == NativeCache[cache_ref][type].end())
		return 0;

	return NativeCache[cache_ref][type][id];
}

static void init_cache(lua_State *L) {
	lua_createtable(L, 0, 0);
	lua_createtable(L, 0, 1);
	lua_pushstring(L, "v");
	lua_setfield(L, -2, "__mode");
	lua_setmetatable(L, -2);
}

static inline bool get_jstring(json &obj, std::string &dst) {
	if(!obj.is_string()) return false;
	obj.get_to(dst);
	return true;
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
					if(get_jstring(nmeth["return_type"], temp)) {
						if((_meth.ret = get_type(temp)) == NTYPE_UNKNOWN)
							return NLOAD_METHOD_INVALID_RETURN_TYPE;
					} else return NLOAD_METHOD_NONSTRING_RETURN_TYPE;

					if(get_jstring(nmeth["build"], temp))
						_meth.firstSeen = std::stoul(temp);

					if((nmeth["params"]).is_array()) {
						NativeParams& _params = _meth.params;
						int argn = 0;
						for(auto& inmparam : nmeth["params"]) {
							if(!inmparam.is_object()) return NLOAD_METHOD_PARAM_NONOBJECT;
							if(!get_jstring(inmparam["type"], temp))
								return NLOAD_METHOD_PARAM_NONSTRING_TYPE;
							if(temp.size() == 0) {
								_meth.isVararg = true;
								break;
							}
							NativeParam& _param = _params[argn++];
							_param.type = get_type(temp);
							if(_param.type == -1) return NLOAD_METHOD_PARAM_INVALID_TYPE;
							if(!get_jstring(inmparam["name"], _param.name))
								return NLOAD_METHOD_PARAM_NONSTRING_NAME;
						}
					} else return NLOAD_METHOD_PARAMS_NONARRAY;

					_meth.hash = std::stoull(inmeth.key().c_str(), nullptr, 16);
				}
			}
		}
	}

	return NLOAD_OK;
}

#define IS_PTR_OF(t1, ti1, t2, ti2) (ti1.isPointer && (((t1) - (t2)) == 1 || ((t1) - (ti2).superType) == 1))

static void get_value(lua_State *L, int idx, NativeType extype, PUINT64 val) {
	union {
		float f;
		UINT64 u64;
	} c;

	if(extype == -1) {
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

	if(extype < NTYPE_INT) luaL_error(L, "Something is wrong with NativeDB");
	auto *no = (NativeObject *)luaL_checkudata(L, idx, LUANATIVE_OBJECT);
	NativeTypeInfo &nti_obj = get_type_info(no->hdr.type),
	&nti_exp = get_type_info(extype);

	if(extype != no->hdr.type) {
		if(extype != nti_obj.superType && nti_exp.superType != nti_obj.superType
		&& !IS_PTR_OF(extype, nti_exp, no->hdr.type, nti_obj))
			luaL_error(L, "bad argument #%d (%s expected, got %s)",
				idx, nti_exp.name.c_str(), nti_obj.name.c_str());

		if(nti_exp.isPointer) {
			luaL_argcheck(L, !no->hdr.readonly, idx,
				"attempt to push an readonly object as a pointer");
			*val = (UINT64)&no->hash.u64;
		} else *val = no->hash.u64;
	} else if(nti_exp.isPointer)
		*val = (UINT64)&no->hash.u64;
	else *val = no->hash.u64;
}

static void push_native(lua_State *L, NativeType type, int id, int cache_ref = -1, int cache_id = -1) {
	cache_id = cache_id >= 0 ? cache_id : id, cache_ref = cache_ref > 0 ? cache_ref : ReferenceMap[L].nc;
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

	auto *no = (NativeObject *)lua_newuserdata(L, sizeof(NativeObject));
	NATHDR_INIT(no->hdr, type, 1, cache_ref == ReferenceMap[L].nc);
	luaL_setmetatable(L, LUANATIVE_OBJECT);
	no->hash.i32 = id;

	// Сохраняем в кеш
	NativeCache[cache_ref][type][cache_id] = cached;
	lua_rawgeti(L, LUA_REGISTRYINDEX, cache_ref);
	lua_pushvalue(L, -2);
	lua_rawseti(L, -2, cached);
	lua_pop(L, 1);
}

static int push_value(lua_State *L, NativeType type, PUINT64 val) {
	switch(type) {
		case NTYPE_VOID: return 0;

		case NTYPE_INT:
		case NTYPE_ANY:
			lua_pushinteger(L, *val);
			break;
		case NTYPE_FLOAT:
			lua_pushnumber(L, *(float *)val);
			break;
		case NTYPE_BOOL:
			lua_pushboolean(L, (int)*val);
			break;
		case NTYPE_STRING:
			lua_pushstring(L, *(const char **)val);
			break;
		case NTYPE_VECTOR3:
			push_vector(L, (Vector3 *)val);
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

static int native_call(lua_State *L) {
	if(Natives.size() == 0)
		luaL_error(L, "Natives database is empty or was loaded incorrectly");
	NativeMeth *_meth = native_search(
		luaL_checkstring(L, 1),
		luaL_checkstring(L, 2)
	);
	if(_meth == nullptr)
		luaL_error(L, "Method not found");

	native_prepare(L, _meth, lua_gettop(L) - 2);
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

static int native_version(lua_State *L) {
	auto *meth = native_search(
		luaL_checkstring(L, 1),
		luaL_checkstring(L, 2)
	);
	lua_pushinteger(L, meth->firstSeen);
	return 1;
}

static int native_new(lua_State *L) {
	NativeType type_idx = get_type((std::string)luaL_checkstring(L, 1));
	NativeTypeInfo type = get_type_info(type_idx);
	luaL_argcheck(L, type_idx > NTYPE_VOID && !type.isPointer, 1, "unsupported type passed");

	uint size = (uint)luaL_checkinteger(L, 2);
	auto *hdr = (NativeObjectHeader *)lua_newuserdata(L, NATHDR_SZ + (size * 4));
	NATHDR_INIT(*hdr, type_idx + 1, size, false);
	luaL_setmetatable(L, LUANATIVE_OBJECT);
	memset(&(hdr[1]), 0, size);
	return 1;
}

#define VTN(idx) (float)lua_tonumber(L, idx)
static int native_newvec(lua_State *L) {
	Vector3 pos {VTN(1), VTN(2), VTN(3)};
	push_vector(L, &pos);
	return 1;
}

#define WORLDGETALL(T, TN) { \
	BYTE *ptr = (BYTE *)luaL_checkudata(L, 1, LUANATIVE_OBJECT); \
	auto *hdr = (NativeObjectHeader *)ptr; ptr += sizeof(NativeObjectHeader); \
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
	{"calla", native_calla},

	{"version", native_version},

	{"new", native_new},
	{"newvector", native_newvec},

	{"allobjects", native_allobjs},
	{"allpeds", native_allpeds},
	{"allpickups", native_allpick},
	{"allvehicles", native_allvehs},

	{NULL, NULL}
};

static int native_tostring(lua_State *L) {
	auto *no = (NativeObject *)luaL_checkudata(L, 1, LUANATIVE_OBJECT);
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
	auto *no = (NativeObject *)luaL_checkudata(L, 1, LUANATIVE_OBJECT);
	luaL_argcheck(L, !no->hdr.readonly, 1, "readonly object");
	if(no->hdr.type == NTYPE_VECTOR3) return vector_newindex(L, (NativeVector *)no);
	NativeTypeInfo &ni = get_type_info(no->hdr.type);
	
	if(!ni.isPointer) return 0;
	uint idx = (uint)luaL_checkinteger(L, 2);
	luaL_argcheck(L, idx < no->hdr.size, 2, "out of bounds");
	((int *)&no->hash.p)[idx] = (int)luaL_checkinteger(L, 3);
	return 0;
}

static int native_index(lua_State *L) {
	auto *no = (NativeObject *)luaL_checkudata(L, 1, LUANATIVE_OBJECT);
	if(no->hdr.type == NTYPE_VECTOR3) return vector_index(L, (NativeVector *)no);
	NativeTypeInfo &ni = get_type_info(no->hdr.type);
	luaL_argcheck(L, ni.isPointer, 1, "not a pointer");
	uint idx = (uint)lua_tointeger(L, 2);
	luaL_argcheck(L, idx < no->hdr.size, 2, "out of bounds");

	switch(no->hdr.type) {
		case NTYPE_INT:
		case NTYPE_ANY:
			lua_pushinteger(L, ((int *)&no->hash.p)[idx]);
			break;
		case NTYPE_FLOAT:
			lua_pushnumber(L, ((float *)&no->hash.p)[idx]);
			break;
		case NTYPE_BOOL:
			lua_pushboolean(L, ((BOOL *)&no->hash.p)[idx]);
			break;
		case NTYPE_CHARPTR:
		case NTYPE_STRING:
			lua_pushstring(L, ((const char **)&no->hash.p)[idx]);
			break;
		default:
			if(no->hdr.owncache == 0) {
				// Создаём собственный кеш для объекта
				init_cache(L);
				no->hdr.owncache = luaL_ref(L, LUA_REGISTRYINDEX);
			}
			push_native(L, (NativeType)(no->hdr.type - 1),
				((int *)&no->hash.p)[idx], no->hdr.owncache, idx);
			break;
	}

	return 1;
}

static int native_gc(lua_State *L) {
	// Удаляем собственный кеш объекта, если он был
	auto *no = (NativeObject *)luaL_checkudata(L, 1, LUANATIVE_OBJECT);
	if(no->hdr.owncache) luaL_unref(L, LUA_REGISTRYINDEX, no->hdr.owncache);
	return 0;
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

int luaopen_native(lua_State *L) {
	if(Natives.size() == 0) {
		NativeReturn nret;
		if((nret = native_reload()) != NLOAD_OK)
			LOG(ERROR) << "Failed to load native.json: " << nret;
	}

	luaL_newmetatable(L, LUANATIVE_OBJECT);
	lua_pushstring(L, "none of your business");
	lua_setfield(L, -2, "__metatable");
	luaL_setfuncs(L, nativeobj, 0);

	init_cache(L);
	ReferenceMap[L].nc = luaL_ref(L, LUA_REGISTRYINDEX);

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

	luaL_newlib(L, nativelib);
	return 1;
}
