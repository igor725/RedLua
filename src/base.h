#pragma once

#include "thirdparty\ScriptHook\inc\natives.h"
#include "thirdparty\ScriptHook\inc\types.h"
#include "thirdparty\ScriptHook\inc\enums.h"

#include "thirdparty\ScriptHook\inc\main.h"

#include <map>
#include "redlua.h"

extern std::map <std::string, LuaScript *> Scripts;

void ScriptMain(void);
void ScriptFinish(void);
bool ScanForNewScripts(void);
