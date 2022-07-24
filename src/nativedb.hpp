#pragma once

#include "native\types.hpp"

class NativeDB {
	std::string m_path;
	NativeNamespaces m_db;
	int m_nummethods = 0;

public:
	enum Returns {
		NLOAD_OK,
		NLOAD_OPEN_FILE,
		NLOAD_MALFORMED_FILE,
		NLOAD_NAMESPACE_NONOBJECT,
		NLOAD_METHOD_NONOBJECT,
		NLOAD_METHOD_NONSTRING_NAME,
		NLOAD_METHOD_INVALID_RETURN_TYPE,
		NLOAD_METHOD_NONSTRING_RETURN_TYPE,
		NLOAD_METHOD_PARAM_INVALID_TYPE,
		NLOAD_METHOD_PARAM_NONSTRING_TYPE,
		NLOAD_METHOD_PARAM_NONSTRING_NAME,
		NLOAD_METHOD_PARAM_NONOBJECT,
		NLOAD_METHOD_PARAMS_NONARRAY,
	};

	virtual Returns Load(void);

	NativeNamespace *GetNamespace(std::string nspace) {
		if(m_db.find(nspace) == m_db.end())
			return nullptr;
		return &m_db[nspace];
	}

	NativeMeth *GetMethod(NativeNamespace *nspace, std::string method) {
		if((*nspace).find(method) == (*nspace).end())
			return nullptr;
		return &(*nspace)[method];
	}

	uint GetMethodCount(void) {
		return m_nummethods;
	}

	NativeDB(std::string path) : m_path(path) {}
};


extern NativeDB Natives;
