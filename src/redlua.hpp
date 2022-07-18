#pragma once

#include <string>
#include "thirdparty\luajit\src\lua.hpp"
#include "thirdparty\easyloggingpp.h"

extern const luaL_Reg redlibs[];

static int log_print(lua_State *L) {
	std::string logstr;
	char pointer[20];
	lua_Number tempd;
	int tempi;

	for(int i = 1; i <= lua_gettop(L); i++) {
		switch(lua_type(L, i)) {
			case LUA_TNIL:
				logstr.append("nil");
				break;
			case LUA_TBOOLEAN:
				logstr.append(lua_toboolean(L, i) ? "true" : "false");
				break;
			case LUA_TNUMBER:
				tempd = lua_tonumber(L, i);
				tempi = (int)tempd;
				if(tempi == tempd)
					logstr.append(std::to_string(tempi));
				else
					logstr.append(std::to_string(tempd));
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
		bool enabled = false, haserror = false;
		int modref = LUA_REFNIL;

	public:
		LuaScript(std::string luafile) {
			L = luaL_newstate();
			luaL_openlibs(L);
			for(const luaL_Reg *lib = redlibs; lib->name; lib++) {
				lua_pushcfunction(L, lib->func);
				lua_pushstring(L, lib->name);
				lua_call(L, 1, 1);
				lua_setglobal(L, lib->name);
			}

			lua_getglobal(L, "package");
			if(!lua_isnil(L, -1)) {
				lua_pushstring(L, "RedLua\\Libs\\?.lua;RedLua\\Libs\\?\\init.lua");
				lua_setfield(L, -2, "path");
				lua_pushstring(L, "RedLua\\Libs\\C\\?.dll;RedLua\\Libs\\C\\?\\core.dll");
				lua_setfield(L, -2, "cpath");
			}
			lua_pop(L, 1);

			lua_pushcfunction(L, log_print);
			lua_setglobal(L, "print");

			path = "RedLua\\Scripts\\" + luafile;
		}

		~LuaScript(void) {
			if(LookForFunc("OnStop"))
				CallFunc(0, 0);

			lua_close(L);
		}

		void Load(void) {
			if(modref != LUA_REFNIL) luaL_unref(L, LUA_REGISTRYINDEX, modref);
			if((luaL_loadfile(L, path.c_str()) || lua_pcall(L, 0, 1, 0)) == 0) {
				modref = luaL_ref(L, LUA_REGISTRYINDEX);
				enabled = true, haserror = false;

				if(LookForFunc("OnLoad") && !CallFunc(0, 0))
					return;
			}

			enabled = false, haserror = true;
			lua_pop(L, 1);
			return;
		}

		bool Load(std::string& error) {
			if(modref != LUA_REFNIL) luaL_unref(L, LUA_REGISTRYINDEX, modref);
			if((luaL_loadfile(L, path.c_str()) || lua_pcall(L, 0, 1, 0)) == 0) {
				modref = luaL_ref(L, LUA_REGISTRYINDEX);
				enabled = true, haserror = false;

				if(LookForFunc("OnLoad") && !CallFunc(0, 0))
					return false;

				return true;
			}

			enabled = false, haserror = true;
			error = lua_tostring(L, -1);
			lua_pop(L, 1);
			return false;
		}

		bool LookForFunc(const char *name) {
			if(enabled && modref != -1) {
				lua_rawgeti(L, LUA_REGISTRYINDEX, modref);
				lua_getfield(L, -1, name);
				if(!lua_isfunction(L, -1)) {
					lua_pop(L, 2);
					return false;
				}

				lua_remove(L, -2);
				return true;
			}

			return false;
		}

		bool CallFunc(int argn, int retn, bool stop_on_error = true) {
			if(lua_pcall(L, argn, retn, 0) != 0) {
				if(stop_on_error) enabled = false, haserror = true;
				LOG(ERROR) << "Lua error occured (" << path << "): " << lua_tostring(L, -1);
				lua_pop(L, 1);
				lua_gc(L, LUA_GCCOLLECT, 0);
				return false;
			}

			return true;
		}

		void OnTick(void) {
			if(LookForFunc("OnTick"))
				CallFunc(0, 0);
		}

		float GetMemoryUsage(void) {
			return lua_gc(L, LUA_GCCOUNTB, 0) / 1024.0f;
		}

		void SetEnabled(bool en) {
			lua_gc(L, LUA_GCCOLLECT, 0);
			enabled = en;
		}

		bool IsEnabled(void) {
			return enabled;
		}

		bool HasError(void) {
			return haserror;
		}
};
