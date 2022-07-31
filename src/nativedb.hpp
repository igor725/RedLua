#pragma once

#include "native\types.hpp"

class NativeDB {
	std::string m_path;
	NativeNamespaces m_db;
	int m_nummethods = 0;

public:
	enum Returns {
		OK,
		ERR_OPEN_FILE,
		ERR_MALFORMED_FILE,
		ERR_NAMESPACE_NONOBJECT,
		ERR_METHOD_NONOBJECT,
		ERR_METHOD_NONSTRING_NAME,
		ERR_METHOD_INVALID_RETURN_TYPE,
		ERR_METHOD_NONSTRING_RETURN_TYPE,
		ERR_METHOD_PARAM_INVALID_TYPE,
		ERR_METHOD_PARAM_NONSTRING_TYPE,
		ERR_METHOD_PARAM_NONSTRING_NAME,
		ERR_METHOD_PARAM_NONOBJECT,
		ERR_METHOD_PARAMS_NONARRAY,
	};

	virtual Returns Load(void);

	NativeNamespace *GetNamespace(std::string nspace) {
		for (auto &it : m_db)
			if (it.first == nspace) return &it.second;
		return nullptr;
	}

	NativeMeth *GetMethod(NativeNamespace *nspace, std::string method) {
		for (auto &it : (*nspace))
			if (it.first == method) return &it.second;
		return nullptr;
	}

	uint GetMethodCount(void) {
		return m_nummethods;
	}

	NativeDB(std::string path) : m_path(path) {}
};


extern NativeDB Natives;
