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
		std::string m_path;
		bool m_enabled = false, m_haserror = false;
		int m_modref = LUA_REFNIL;

	public:
		LuaScript(std::string luafile)
		: m_path(luafile) {
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
		}

		~LuaScript(void) {
			if(LookForFunc("OnStop"))
				CallFunc(0, 0);

			lua_close(L);
		}

		bool Load(void) {
			if(m_modref != LUA_REFNIL) luaL_unref(L, LUA_REGISTRYINDEX, m_modref);
			if(luaL_loadfile(L, m_path.c_str())) {
				LogLuaError();
				return false;
			}

			if(CallFunc(0, 1)) {
				m_modref = luaL_ref(L, LUA_REGISTRYINDEX);
				if(m_modref == LUA_REFNIL) {
					m_enabled = false, m_haserror = true;
					LOG(ERROR) << m_path << " does not return the table";
					return false;
				}
				m_enabled = true, m_haserror = false;

				if(LookForFunc("OnLoad") && !CallFunc(0, 0))
					return false;

				LOG(INFO) << m_path << " loaded";
				return true;
			}

			return false;
		}

		bool LookForFunc(const char *name) {
			if(m_enabled && m_modref != -1) {
				lua_rawgeti(L, LUA_REGISTRYINDEX, m_modref);
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

		void LogLuaError(void) {
			LOG(ERROR) << "lua_State(" << L << ") error: " << lua_tostring(L, -1);
			lua_pop(L, 1);
			lua_gc(L, LUA_GCCOLLECT, 0);
		}

		bool CallFunc(int argn, int retn, bool stop_on_error = true) {
			if(lua_pcall(L, argn, retn, 0) == 0) return true;
			if(stop_on_error) m_enabled = false, m_haserror = true;
			LogLuaError();
			return false;
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
			if(en && m_modref == LUA_REFNIL && !Load())
				return;
			m_enabled = en;
		}

		bool IsEnabled(void) {
			return m_enabled;
		}

		bool HasError(void) {
			return m_haserror;
		}

		std::string GetPath(void) {
			return m_path;
		}
};
