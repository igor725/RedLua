#pragma once

#include <string>
#include "luajit\src\lua.hpp"
#include "easyloggingpp.h"
extern const luaL_Reg redlibs[];

static int log_print(lua_State *L) {
	std::string logstr;
	char pointer[20];

	for(int i = 1; i <= lua_gettop(L); i++) {
		switch(lua_type(L, i)) {
			case LUA_TNONE:
				continue;
			case LUA_TNIL:
				logstr.append("nil");
				break;
			case LUA_TBOOLEAN:
				logstr.append(lua_toboolean(L, i) ? "true" : "false");
				break;
			case LUA_TNUMBER:
				logstr.append(std::to_string(lua_tonumber(L, i)));
				break;
			case LUA_TSTRING:
				logstr.append(lua_tostring(L, i));
				break;

			default:
				if(luaL_callmeta(L, i, "__tostring")) {
					if(!lua_isstring(L, -1))
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

	if(logstr.length() > 0) {
		(logstr.pop_back(), logstr.pop_back());
		LOG(INFO) << logstr;
	}
	return 0;
}

class LuaScript {
	private:
		lua_State *L;
		std::string path;

	public:
		bool loaded, haserror;
		LuaScript(std::string luafile) {
			L = luaL_newstate();
			luaL_openlibs(L);
			for(const luaL_Reg *lib = redlibs; lib->name; lib++) {
#				if LUA_VERSION_NUM < 502
					lua_pushcfunction(L, lib->func);
					lua_pushstring(L, lib->name);
					lua_call(L, 1, 1);
					lua_setglobal(L, lib->name);
#				else
					luaL_requiref(L, lib->name, lib->func, 1);
					lua_pop(L, 1);
#				endif
			}

			lua_pushcfunction(L, log_print);
			lua_setglobal(L, "print");

			path = "RedLua\\Scripts\\" + luafile;
			loaded = false, haserror = false;
		}

		~LuaScript() {
			LookupFunc("OnStop");
			CallFunc(0, 0);
			lua_close(L);
		}

		virtual bool Load(std::string& error) {
			if(luaL_dofile(L, path.c_str()) == 0) {
				loaded = true, haserror = false;
				return true;
			}

			loaded = false, haserror = true;
			error = lua_tostring(L, -1);
			return false;
		}

		virtual bool LookupFunc(std::string name) {
			lua_getglobal(L, name.c_str());
			if(!lua_isfunction(L, -1)) {
				lua_pop(L, 1);
				return false;
			}
			
			return true;
		}

		virtual bool CallFunc(int nargs, int nres) {
			return lua_pcall(L, nargs, nres, 0) == 0;
		}

		virtual float GetMemoryUsage(void) {
			return lua_gc(L, LUA_GCCOUNTB, 0) / 1024.0f;
		}

		virtual lua_State *GetState(void) {
			return L;
		}

		virtual bool IsLoaded(void) {
			return loaded;
		}
};
