#pragma once

#include "thirdparty\ScriptHook\inc\types.h"
#include <string>
#include <map>

typedef enum _NativeType : int {
	NTYPE_UNKNOWN = -1,
	NTYPE_VOID,
	NTYPE_INT,
	NTYPE_FLOAT,
	NTYPE_BOOL,
	NTYPE_CHAR,
	NTYPE_ANY,
	NTYPE_BLIP,
	NTYPE_CAM,
	NTYPE_ENTITY,
	NTYPE_FIREID,
	NTYPE_HASH,
	NTYPE_INTERIOR,
	NTYPE_ITEMSET,
	NTYPE_OBJECT,
	NTYPE_PED,
	NTYPE_PICKUP,
	NTYPE_PLAYER,
	NTYPE_SCRHANDLE,
	NTYPE_VECTOR3,
	NTYPE_VEHICLE,
	NTYPE_ANIMSCENE,
	NTYPE_PERSCHAR,
	NTYPE_POPZONE,
	NTYPE_PROMPT,
	NTYPE_PROPSET,
	NTYPE_VOLUME
} NativeType;

typedef long long NativeCacheField;
typedef unsigned long long NativeData;
#define NATIVEDATA_INVAL ((NativeCacheField)-1)
#define NATIVECACHE_DISABLE ((int)-2)
#define NATIVECACHE_NODATA ((NativeData)-1)

typedef struct _NativeParam {
	NativeType type;
	std::string name;
	bool isPointer;
} NativeParam;

typedef std::map<int, NativeParam> NativeParams;

typedef struct _NativeMeth {
	UINT64 hash;
	NativeType returns;
	NativeParams params;
	long firstSeen;
	bool isVararg;
	bool isRetPtr;
	bool isRetConst;
} NativeMeth;

typedef struct _NativeTypeInfo {
	NativeType superType;
	uint size;
	std::string name;
} NativeTypeInfo;

typedef std::map<std::string, NativeMeth> NativeNamespace;
typedef std::map<std::string, NativeNamespace> NativeNamespaces;
typedef std::map<NativeType, NativeTypeInfo> NativeTypeMap;

extern NativeTypeMap NativeTypes;

static NativeTypeInfo &get_type_info(NativeType id) {
	if(NativeTypes.find(id) == NativeTypes.end())
		return NativeTypes[NTYPE_UNKNOWN];
	return NativeTypes[id];
}

static NativeType get_type(std::string &name) {
	for(auto &i : NativeTypes) {
		if(i.second.name == name)
			return i.first;
	}

	return NTYPE_UNKNOWN;
}

typedef struct _NativeObjectHeader {
	NativeType type;
	bool isPointer;
	bool isReadOnly;
	int ownCache;
	uint count;
} NativeObjectHeader;

typedef struct _NativeObject {
	NativeObjectHeader hdr;
	union _NativeContent {
		int i32;
		void *p;
		void **pp;
		NativeData nd;
	} content;
} NativeObject;

typedef struct _NativeVector {
	NativeObjectHeader hdr;
	Vector3 content;
} NativeVector;

#define NATIVEOBJECT_INIT(X, T, P, R, C, D) (X)->hdr.type = (T), \
	(X)->hdr.isPointer = (P), (X)->hdr.isReadOnly = (R), \
	(X)->hdr.count = (C), (X)->content.nd = (D), \
	(X)->hdr.ownCache = 0

#define NATIVEOBJECT_INITLIGHT(X, T, R, C, D) (X)->hdr.type = (T), \
	(X)->hdr.isPointer = true, (X)->hdr.isReadOnly = (R), \
	(X)->hdr.count = (C), (X)->content.nd = (D), \
	(X)->hdr.ownCache = 0

#define NATIVEOBJECT_GETPTR(X) ((X)->hdr.isPointer ? (X)->content.p : &(X)->content.i32)
#define NATIVEOBJECT_HDRSIZE (sizeof(NativeObject) - sizeof(NativeData))
#define IS_NATIVETYPES_EQU(AT, A, BT) ((AT) == (BT) || (A).superType == (BT))

#define NOBJCOUNT_UNKNOWN ((uint)-1)
