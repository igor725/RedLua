#include "redlua.h"
#include "luanative.h"

const luaL_Reg redlibs[] = {
	{"native", luaopen_native},

	{NULL, NULL}
};


