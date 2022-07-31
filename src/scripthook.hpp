#pragma once

#ifdef REDLUA_GTAV
#include "thirdparty\ScriptHookV\inc\main.h"
#ifdef REDLUA_STANDALONE
#	include "emu\native.hpp"
#endif
#include "thirdparty\ScriptHookV\inc\nativeCaller.h"
#include "thirdparty\ScriptHookV\inc\types.h"
#else
#include "thirdparty\ScriptHookRDR2\inc\main.h"
#ifdef REDLUA_STANDALONE
#	include "emu\native.hpp"
#endif
#include "thirdparty\ScriptHookRDR2\inc\nativeCaller.h"
#include "thirdparty\ScriptHookRDR2\inc\types.h"
#endif
