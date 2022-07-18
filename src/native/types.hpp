#pragma once

#include "thirdparty/ScriptHook/inc/types.h"
#include <string>
#include <map>

typedef enum _NativeType : int {
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
	long firstSeen;
	bool isVararg;
} NativeMeth;

typedef struct _NativeTypeInfo {
	NativeType superType;
	bool isPointer;
	std::string name;
} NativeTypeInfo;

typedef struct _NativeObjectHeader {
	NativeType type;
	UINT size;
	int owncache, readonly;
} NativeObjectHeader;

#define NATHDR_INIT(H, T, S, R) ((H).type = (NativeType)(T), (H).size = (UINT)(S),\
	(H).owncache = 0, (H).readonly = R)
#define NATHDR_SZ sizeof(NativeObjectHeader)

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
typedef std::map<NativeType, NativeTypeInfo> NativeTypeMap;

extern NativeTypeMap NativeTypes;

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
