#pragma once

#include "thirdparty\json.hpp"
#include "native\types.hpp"
#include <fstream>

using json = nlohmann::json;
extern NativeNamespaces Natives;

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
	NLOAD_METHOD_PARAM_NONSTRING_NAME,
	NLOAD_METHOD_PARAM_NONOBJECT,
	NLOAD_METHOD_PARAMS_NONARRAY,
} NativeReturn;

static inline bool get_jstring(json &obj, std::string &dst) {
	if(!obj.is_string()) return false;
	obj.get_to(dst);
	return true;
}

static NativeReturn native_reload(void) {
	std::ifstream fnatives("RedLua\\natives.json");
	if(!fnatives.is_open()) return NLOAD_JSON_PARSE;
	json jnatives = json::parse(fnatives);
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

static NativeMeth *native_search(std::string nspace, std::string meth) {
	if(Natives.find(nspace) == Natives.end())
		return nullptr;
	if(Natives[nspace].find(meth) == Natives[nspace].end())
		return nullptr;

	return &Natives[nspace][meth];
}
