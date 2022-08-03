/*
	THIS FILE IS A PART OF RDR 2 SCRIPT HOOK SDK
				http://dev-c.com
			(C) Alexander Blade 2019
*/

#include "keyboard.h"

const int KEYS_SIZE = 255;

struct {
	DWORD time;
	BOOL isWithAlt;
	BOOL wasDownBefore;
	BOOL isUpNow;
} keyStates[KEYS_SIZE];

const char *KeyNames[KEYS_SIZE] = {
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
	"VK_ZOOM", "VK_NONAME", "VK_PA1", "VK_OEM_CLEAR"
};

void OnKeyboardMessage(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow)
{
	if (key < KEYS_SIZE)
	{
		keyStates[key].time = GetTickCount();
		keyStates[key].isWithAlt = isWithAlt;
		keyStates[key].wasDownBefore = wasDownBefore;
		keyStates[key].isUpNow = isUpNow;
	}
}

const int NOW_PERIOD = 100, MAX_DOWN = 5000, MAX_DOWN_LONG = 30000; // ms

bool IsKeyDown(DWORD key)
{
	return (key < KEYS_SIZE) ? ((GetTickCount() < keyStates[key].time + MAX_DOWN) && !keyStates[key].isUpNow) : false;
}

bool IsKeyDownLong(DWORD key)
{
	return (key < KEYS_SIZE) ? ((GetTickCount() < keyStates[key].time + MAX_DOWN_LONG) && !keyStates[key].isUpNow) : false;
}

bool IsKeyJustUp(DWORD key, bool exclusive)
{
	bool b = (key < KEYS_SIZE) ? (GetTickCount() < keyStates[key].time + NOW_PERIOD && keyStates[key].isUpNow) : false;
	if (b && exclusive)
		ResetKeyState(key);
	return b;
}

void ResetKeyState(DWORD key)
{
	if (key < KEYS_SIZE)
		memset(&keyStates[key], 0, sizeof(keyStates[0]));
}
