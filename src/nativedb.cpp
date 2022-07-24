#include "nativedb.hpp"
#include "constants.hpp"

#include <fstream>
#include "thirdparty\json.hpp"
using json = nlohmann::json;

NativeDB Natives (REDLUA_NATIVES_FILE);

static inline bool jstring(json &obj, std::string &dst) {
	if(!obj.is_string()) return false;
	obj.get_to(dst);
	return true;
}

NativeDB::Returns NativeDB::Load(void) {
	std::ifstream fnatives(m_path);
	if(!fnatives.is_open()) return NLOAD_OPEN_FILE;
	json jnatives = json::parse(fnatives, nullptr, false);
	if(jnatives.is_discarded() || !jnatives.is_object())
		return NLOAD_MALFORMED_FILE;
	fnatives.close();

	for(auto &ijnspace : jnatives.items()) {
		json &jnspace = ijnspace.value();
		if(!jnspace.is_object()) return NLOAD_NAMESPACE_NONOBJECT;
		NativeNamespace &methods = m_db[ijnspace.key()];
		for(auto &ijnmeth : jnspace.items()) {
			json &jnmeth = ijnmeth.value();
			if(!jnmeth.is_object())
				return NLOAD_METHOD_NONOBJECT;
			if(!(jnmeth["name"]).is_string())
				return NLOAD_METHOD_NONSTRING_NAME;
			std::string temp;
			NativeMeth &meth = methods[jnmeth["name"].get_to(temp)];
			m_nummethods++;
			if(jstring(jnmeth["return_type"], temp)) {
				if(meth.isRetPtr = (temp.find("*") != std::string::npos))
					temp.pop_back();
				if(meth.isRetConst = (temp.find("const ") == 0))
					temp.erase(0, 6);
				if((meth.returns = get_type(temp)) == NTYPE_UNKNOWN)
					return NLOAD_METHOD_INVALID_RETURN_TYPE;
			} else return NLOAD_METHOD_NONSTRING_RETURN_TYPE;

			if(jstring(jnmeth["build"], temp))
				meth.firstSeen = std::stoul(temp);

			json &jnmethparams = jnmeth["params"];
			if(!jnmethparams.is_array())
				return NLOAD_METHOD_PARAMS_NONARRAY;
			int argn = 0;
			for(auto &inmparam : jnmeth["params"]) {
				if(!inmparam.is_object()) return NLOAD_METHOD_PARAM_NONOBJECT;
				if(!jstring(inmparam["type"], temp))
					return NLOAD_METHOD_PARAM_NONSTRING_TYPE;
				if(temp.size() == 0) {
					meth.isVararg = true;
					break;
				}
				NativeParam &methparam = meth.params[argn++];
				if(methparam.isPointer = (temp.find("*") != std::string::npos))
					temp.pop_back();
				if(temp.find("const ") == 0)
					temp.erase(0, 6);
				if((methparam.type = get_type(temp)) == NTYPE_UNKNOWN)
					return NLOAD_METHOD_PARAM_INVALID_TYPE;
				if(!jstring(inmparam["name"], methparam.name))
					return NLOAD_METHOD_PARAM_NONSTRING_NAME;
			}

			meth.hash = std::stoull(ijnmeth.key().c_str(), nullptr, 16);
		}
	}

	return NLOAD_OK;
}
