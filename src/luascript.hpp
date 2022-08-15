#pragma once

#include "luanative.hpp"
#include "constants.hpp"

#include <string>
#include "thirdparty\LuaJIT\src\lua.hpp"
#include "thirdparty\easyloggingpp.h"
#include "thirdparty\scriptmenu.h"

extern const luaL_Reg redlibs[];

class LuaScript {
private:
	lua_State *L;
	std::string m_path;
	bool m_enabled = false, m_hasError = false,
	m_firstTick = false;
	int m_modRef = LUA_REFNIL;

public:
	LuaScript(std::string dir, std::string fname, bool dir_to_paths = false)
	: m_path(dir + "\\" + fname) {
		L = luaL_newstate();
		luaL_openlibs(L);
		for (auto lib = redlibs; lib->name; lib++) {
			lua_pushcfunction(L, lib->func);
			lua_pushstring(L, lib->name);
			lua_call(L, 1, 1);
			lua_setglobal(L, lib->name);
		}

		lua_getglobal(L, "package");
		if (!lua_isnil(L, -1)) {
			if (dir_to_paths)
				lua_pushfstring(L, "%s\\%s%s\\%s;", dir.c_str(),
					REDLUA_LPATH1, dir.c_str(), REDLUA_LPATH2);
			lua_pushstring(L, REDLUA_PATHS);
			if (dir_to_paths) lua_concat(L, 2);
			lua_setfield(L, -2, "path");
			if (dir_to_paths)
				lua_pushfstring(L, "%s\\%s%s\\%s;", dir.c_str(),
					REDLUA_CPATH1, dir.c_str(), REDLUA_CPATH2);
			lua_pushstring(L, REDLUA_CPATHS);
			if (dir_to_paths) lua_concat(L, 2);
			lua_setfield(L, -2, "cpath");
		}
		lua_pop(L, 1);
	}

	~LuaScript(void) {
		if (LookForFunc("OnStop"))
			CallFunc(0, 0);

		lua_close(L);
	}

	bool Load(void) {
		if (m_modRef != LUA_REFNIL) {
			if (LookForFunc("OnReload") && !CallFunc(0, 0, false))
				return false;

			luaL_unref(L, LUA_REGISTRYINDEX, m_modRef);
		}

		if (luaL_loadfile(L, m_path.c_str()) != 0)
			return (LogLuaError(), false);
		if (CallFunc(0, 1)) {
			m_modRef = luaL_ref(L, LUA_REGISTRYINDEX);
			if (m_modRef == LUA_REFNIL) {
				m_enabled = false, m_hasError = true;
				LOG(ERROR) << m_path << " does not return the table";
				return false;
			}
			m_enabled = true, m_hasError = false, m_firstTick = true;

			if (LookForFunc("OnLoad") && !CallFunc(0, 0))
				return false;

			LOG(INFO) << m_path << " loaded";
			return true;
		}

		return false;
	}

	bool LookForFunc(const char *name) {
		if (m_enabled && m_modRef != -1) {
			lua_rawgeti(L, LUA_REGISTRYINDEX, m_modRef);
			lua_getfield(L, -1, name);
			if (!lua_isfunction(L, -1)) {
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
		if (lua_pcall(L, argn, retn, 0) == 0) return true;
		if (stop_on_error) m_enabled = false, m_hasError = true;
		LogLuaError();
		return false;
	}

	void OnTick(bool rebuild_menu) {
		if (LookForFunc("OnTick")) {
			lua_pushboolean(L, m_firstTick || rebuild_menu);
			m_firstTick = false;
			CallFunc(1, 0);
		}
	}

	MenuBase *GetMyMenu(void) {
		lua_getfield(L, LUA_REGISTRYINDEX, "MY_MENU_CLASS");
		auto menu = (MenuBase **)lua_topointer(L, -1);
		lua_pop(L, 1);
		return menu ? *menu : nullptr;
	}

	float GetMemoryUsage(void) {
		return lua_gc(L, LUA_GCCOUNTB, 0) / 1024.0f;
	}

	void SetEnabled(bool en) {
		lua_gc(L, LUA_GCCOLLECT, 0);
		if (en && m_modRef == LUA_REFNIL && !Load())
			return;
		m_enabled = en;
	}

	bool IsEnabled(void) {
		return m_enabled;
	}

	bool HasError(void) {
		return m_hasError;
	}

	std::string GetPath(void) {
		return m_path;
	}
};
