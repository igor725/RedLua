#pragma once

#include "redlua.hpp"

#include <map>

extern std::map <std::string, LuaScript *> Scripts;

void ScriptMain(void);
void ScriptFinish(void);
bool ScanForNewScripts(void);
