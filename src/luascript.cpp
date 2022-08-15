#include "luascript.hpp"
#include "luanative.hpp"
#include "luamisc.hpp"
#include "luamenu.hpp"
#include "lualang.hpp"
#include "lualog.hpp"

const luaL_Reg redlibs[] = {
	{"native", luaopen_native},
	{"misc", luaopen_misc},
	{"menu", luaopen_menu},
	{"lang", luaopen_lang},
	{"log", luaopen_log},

	{NULL, NULL}
};
