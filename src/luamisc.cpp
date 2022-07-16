#include "redlua.h"
#include "thirdparty\keyboard.h"
#include "thirdparty\ScriptHook\inc\main.h"

static const char *keynames[] = {
	NULL, "VK_LBUTTON", "VK_RBUTTON", "VK_CANCEL", "VK_MBUTTON",
	"VK_XBUTTON1", "VK_XBUTTON2", NULL, "VK_BACK", "VK_TAB",
	NULL, NULL, "VK_CLEAR", "VK_RETURN", NULL, NULL,
	"VK_SHIFT", "VK_CONTROL", "VK_MENU", "VK_PAUSE", "VK_CAPITAL",
	"VK_KANA", NULL, "VK_JUNJA", "VK_FINAL", "VK_KANJI", NULL,
	"VK_ESCAPE", "VK_CONVERT", "VK_NONCONVERT", "VK_ACCEPT",
	"VK_MODECHANGE", "VK_SPACE", "VK_PRIOR", "VK_NEXT", "VK_END",
	"VK_HOME", "VK_LEFT", "VK_UP", "VK_RIGHT", "VK_DOWN",
	"VK_SELECT", "VK_PRINT", "VK_EXECUTE", "VK_SNAPSHOT",
	"VK_INSERT", "VK_DELETE", "VK_HELP", "VK_0", "VK_1", "VK_2",
	"VK_3", "VK_4", "VK_5", "VK_6", "VK_7", "VK_8", "VK_9",
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, "VK_A", "VK_B",
	"VK_C", "VK_D", "VK_E", "VK_F", "VK_G", "VK_H", "VK_I",
	"VK_J", "VK_K", "VK_L", "VK_M", "VK_N", "VK_O", "VK_P",
	"VK_Q", "VK_R", "VK_S", "VK_T", "VK_U", "VK_V", "VK_W",
	"VK_X", "VK_Y", "VK_Z", "VK_LWIN", "VK_RWIN", "VK_APPS",
	NULL, "VK_SLEEP", "VK_NUMPAD0", "VK_NUMPAD1", "VK_NUMPAD2",
	"VK_NUMPAD3", "VK_NUMPAD4", "VK_NUMPAD5", "VK_NUMPAD6",
	"VK_NUMPAD7", "VK_NUMPAD8", "VK_NUMPAD9", "VK_MULTIPLY",
	"VK_ADD", "VK_SEPARATOR", "VK_SUBTRACT", "VK_DECIMAL",
	"VK_DIVIDE", "VK_F1", "VK_F2", "VK_F3", "VK_F4", "VK_F5",
	"VK_F6", "VK_F7", "VK_F8", "VK_F9", "VK_F10", "VK_F11",
	"VK_F12", "VK_F13", "VK_F14", "VK_F15", "VK_F16", "VK_F17",
	"VK_F18", "VK_F19", "VK_F20", "VK_F21", "VK_F22", "VK_F23",
	"VK_F24", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	"VK_NUMLOCK", "VK_SCROLL", NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	"VK_LSHIFT", "VK_RSHIFT", "VK_LCONTROL", "VK_RCONTROL",
	"VK_LMENU", "VK_RMENU", "VK_BROWSER_BACK", "VK_BROWSER_FORWARD",
	"VK_BROWSER_REFRESH", "VK_BROWSER_STOP", "VK_BROWSER_SEARCH",
	"VK_BROWSER_FAVORITES", "VK_BROWSER_HOME", "VK_VOLUME_MUTE",
	"VK_VOLUME_DOWN", "VK_VOLUME_UP", "VK_MEDIA_NEXT_TRACK",
	"VK_MEDIA_PREV_TRACK", "VK_MEDIA_STOP", "VK_MEDIA_PLAY_PAUSE",
	"VK_LAUNCH_MAIL", "VK_MEDIA_SELECT", "VK_LAUNCH_APP1",
	"VK_LAUNCH_APP2", NULL, NULL, "VK_OEM_1", "VK_OEM_PLUS",
	"VK_OEM_COMMA", "VK_OEM_MINUS", "VK_OEM_PERIOD", "VK_OEM_2",
	"VK_OEM_3", "VK_ABNT_C1", "VK_ABNT_C2", NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "VK_OEM_4",
	"VK_OEM_5", "VK_OEM_6", "VK_OEM_7", "VK_OEM_8", NULL, NULL,
	"VK_OEM_102", NULL, NULL, "VK_PROCESSKEY", NULL, "VK_PACKET", NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, "VK_ATTN", "VK_CRSEL", "VK_EXSEL", "VK_EREOF", "VK_PLAY",
	"VK_ZOOM", "VK_NONAME", "VK_PA1", "VK_OEM_CLEAR", NULL
};

static int misc_iskeydown(lua_State *L) {
	lua_pushboolean(L, IsKeyDown((DWORD)luaL_checkinteger(L, 1)));
	return 1;
}

static int misc_iskeydownlong(lua_State *L) {
	lua_pushboolean(L, IsKeyDownLong((DWORD)luaL_checkinteger(L, 1)));
	return 1;
}

static int misc_iskeyjustup(lua_State *L) {
	lua_pushboolean(L, IsKeyJustUp((DWORD)luaL_checkinteger(L, 1), lua_toboolean(L, 2)));
	return 1;
}

static int misc_resetkey(lua_State *L) {
	ResetKeyState((DWORD)luaL_checkinteger(L, 1));
	return 0;
}

static int misc_gamever(lua_State *L) {
	const char *ver;

	switch(getGameVersion()) {
		case VER_AUTO: ver = "VER_AUTO";
		case VER_1_0_1207_60_RGS: ver = "VER_1_0_1207_60_RGS";
		case VER_1_0_1207_69_RGS: ver = "VER_1_0_1207_69_RGS";
		case VER_UNK: default: ver = "VER_UNK";
	}

	lua_pushstring(L, ver);
	return 1;
}

static luaL_Reg misclib[] = {
	{"iskeydown", misc_iskeydown},
	{"iskeydownlong", misc_iskeydownlong},
	{"iskeyjustup", misc_iskeyjustup},
	{"resetkey", misc_resetkey},

	{"gamever", misc_gamever},

	{NULL, NULL}
};

int luaopen_misc(lua_State *L) {
	for(int i = 0; i < 256; i++) {
		if(keynames[i]) {
			lua_pushinteger(L, i);
			lua_setglobal(L, keynames[i]);
		}
	}

	luaL_newlib(L, misclib);
	return 1;
}
