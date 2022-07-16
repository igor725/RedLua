#pragma once

#include "thirdparty\luajit\src\lua.hpp"

typedef int NativeType;

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
