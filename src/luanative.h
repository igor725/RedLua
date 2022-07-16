#pragma once

#define LUANATIVE_OBJECT "NativeObject"
#include "thirdparty\luajit\src\lua.hpp"
#include <string>
#include <map>

typedef enum _NativeType {
	NTYPE_UNKNOWN = -1,
	NTYPE_VOID,
	NTYPE_INT,
	NTYPE_INTPTR,
	NTYPE_FLOAT,
	NTYPE_FLOATPTR,
	NTYPE_BOOL,
	NTYPE_BOOLPTR,
	NTYPE_CHARPTR,
	NTYPE_STRING,
	NTYPE_ANY,
	NTYPE_ANYPTR,
	NTYPE_BLIP,
	NTYPE_BLIPPTR,
	NTYPE_CAM,
	NTYPE_CAMPTR,
	NTYPE_ENTITY,
	NTYPE_ENTITYPTR,
	NTYPE_FIREID,
	NTYPE_FIREIDPTR,
	NTYPE_HASH,
	NTYPE_HASHPTR,
	NTYPE_INTERIOR,
	NTYPE_INTERIORPTR,
	NTYPE_ITEMSET,
	NTYPE_ITEMSETPTR,
	NTYPE_OBJECT,
	NTYPE_OBJECTPTR,
	NTYPE_PED,
	NTYPE_PEDPTR,
	NTYPE_PICKUP,
	NTYPE_PICKUPPTR,
	NTYPE_PLAYER,
	NTYPE_PLAYERPTR,
	NTYPE_SCRHANDLE,
	NTYPE_SCRHANDLEPTR,
	NTYPE_VECTOR3,
	NTYPE_VECTOR3PTR,
	NTYPE_VEHICLE,
	NTYPE_VEHICLEPTR,
	NTYPE_ANIMSCENE,
	NTYPE_ANIMSCENEPTR,
	NTYPE_PERSCHAR,
	NTYPE_PERSCHARPTR,
	NTYPE_POPZONE,
	NTYPE_POPZONEPTR,
	NTYPE_PROMPT,
	NTYPE_PROMPTPTR,
	NTYPE_PROPSET,
	NTYPE_PROPSETPTR,
	NTYPE_VOLUME,
	NTYPE_VOLUMEPTR
} NativeType;

typedef struct _NativeParam {
	NativeType type;
	std::string name;
} NativeParam;

typedef std::map<int, NativeParam> NativeParams;

typedef struct _NativeMeth {
	UINT64 hash;
	NativeType ret;
	NativeParams params;
	bool isVararg;
} NativeMeth;

typedef struct _NativeTypeInfo {
	NativeType superType;
	BOOL isPointer;
	std::string name;
} NativeTypeInfo;

typedef struct _NativeObjectHeader {
	NativeType type;
	UINT size;
} NativeObjectHeader;

typedef struct _NativeObject {
	NativeObjectHeader hdr;
	union {
		int i32;
		UINT64 u64;
		PUINT64 pu64;
		const void *p;
	} hash;
} NativeObject;

typedef std::map<std::string, NativeMeth> NativeNamespace;
typedef std::map<std::string, NativeNamespace> NativeNamespaces;

typedef enum _NativeReturn {
	NLOAD_OK,
	NLOAD_OPEN_FILE,
	NLOAD_NONOBJECT,
	NLOAD_JSON_PARSE,
	NLOAD_METHOD_NONSTRING_NAME,
	NLOAD_METHOD_INVALID_RETURN_TYPE,
	NLOAD_METHOD_NONSTRING_RETURN_TYPE,
	NLOAD_METHOD_PARAM_INVALID_TYPE,
	NLOAD_METHOD_PARAM_NONSTRING_TYPE,
	NLOAD_METHOD_PARAM_NONOBJECT,
	NLOAD_METHOD_PARAMS_NONARRAY,
} NativeReturn;

NativeReturn native_reload(void);
int luaopen_native(lua_State *L);
