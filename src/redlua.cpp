#include "redlua.h"
#include "luanative.h"
#include "luamisc.h"

const luaL_Reg redlibs[] = {
	{"native", luaopen_native},
	{"misc", luaopen_misc},

	{NULL, NULL}
};


