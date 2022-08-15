#include "lualog.hpp"

#include "thirdparty\easyloggingpp.h"
#include <string>

static int tocppstring(lua_State *L, std::string &logstr) {
	char pointer[20];
	lua_Number tempd;
	int tempi;

	for (int i = 1; i <= lua_gettop(L); i++) {
		switch (lua_type(L, i)) {
			case LUA_TNIL:
				logstr.append("nil");
				break;
			case LUA_TBOOLEAN:
				logstr.append(lua_toboolean(L, i) ? "true" : "false");
				break;
			case LUA_TNUMBER:
				tempd = lua_tonumber(L, i);
				tempi = (int)tempd;
				if (tempi == tempd)
					logstr.append(std::to_string(tempi));
				else
					logstr.append(std::to_string(tempd));
				break;
			case LUA_TSTRING:
				logstr.append(lua_tostring(L, i));
				break;
			default:
				if (luaL_callmeta(L, i, "__tostring")) {
					if (!lua_isstring(L, -1))
						luaL_error(L, "'__tostring' must return a string");
					logstr.append(lua_tostring(L, -1));
					lua_pop(L, 1);
				} else {
					logstr.append(luaL_typename(L, i));
					std::snprintf(pointer, 20, ": %p", lua_topointer(L, i));
					logstr.append(pointer);
				}
				break;
		}

		logstr.append(", ");
	}

	if (!logstr.empty())
		(logstr.pop_back(), logstr.pop_back());

	return 0;
}

static int log_info(lua_State *L) {
	std::string logstr;
	tocppstring(L, logstr);
	LOG(INFO) << logstr;
	return 1;
}

static int log_warn(lua_State *L) {
	std::string logstr;
	tocppstring(L, logstr);
	LOG(WARNING) << logstr;
	return 1;
}

static int log_debug(lua_State *L) {
	std::string logstr;
	tocppstring(L, logstr);
	LOG(DEBUG) << logstr;
	return 1;
}

static int log_error(lua_State *L) {
	std::string logstr;
	tocppstring(L, logstr);
	LOG(ERROR) << logstr;
	return 1;
}

const luaL_Reg loglib[] = {
	{"info", log_info},
	{"warn", log_warn},
	{"debug", log_debug},
	{"error", log_error},

	{NULL, NULL}
};

int luaopen_log(lua_State *L) {
	luaL_newlib(L, loglib);
	lua_pushcfunction(L, log_info);
	lua_setglobal(L, "print");
	return 1;
}
