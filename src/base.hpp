#pragma once

#include "redlua.hpp"

#include <map>

extern std::map <std::string, LuaScript *> Scripts;

void RedLuaMain(void);
void RedLuaFinish(void);
bool RedLuaScanScripts(void);
