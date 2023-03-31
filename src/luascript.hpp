#pragma once

#include <string>
#include "thirdparty\LuaJIT\src\lua.hpp"
#include "thirdparty\scriptmenu.h"

class LuaScript {
private:
	lua_State *L;
	std::string m_path;
	bool m_enabled = false, m_hasError = false,
	m_firstTick = false;
	int m_modRef = LUA_REFNIL;

public:
	LuaScript(std::string dir, std::string fname, bool dir_to_paths = false);

	~LuaScript(void);

	bool Load(void);

	bool LookForFunc(const char *name);

	void LogLuaError(void);

	bool CallFunc(int argn, int retn, bool stop_on_error = true);

	void OnTick(bool rebuild_menu);

	MenuBase *GetMyMenu(void);

	float GetMemoryUsage(void);

	void SetEnabled(bool en);

	bool IsEnabled(void);

	bool HasError(void);

	std::string GetPath(void);
};
