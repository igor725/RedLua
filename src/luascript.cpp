#include "luascript.hpp"
#include "luanative.hpp"
#include "luamisc.hpp"

const luaL_Reg redlibs[] = {
	{"native", luaopen_native},
	{"misc", luaopen_misc},

	{NULL, NULL}
};
