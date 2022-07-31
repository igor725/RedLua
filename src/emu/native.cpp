#include "emu\native.hpp"
#include "thirdparty\easyloggingpp.h"
#include "thirdparty\keyboard.h"
#include "vector"

struct KeyEvent {
	DWORD key;
	WORD repeats;
	BYTE scanCode;
	BOOL isExtended;
	BOOL isWithAlt;
	BOOL wasDownBefore;
	BOOL isUpNow;
};

static std::vector<struct KeyEvent> VirtualPress {};

void emu_scriptWait(DWORD ms) {
	Sleep(100);
	if (VirtualPress.size() > 0) {
		struct KeyEvent &ke = VirtualPress.back();
		OnKeyboardMessage(ke.key, ke.repeats, ke.scanCode,
			ke.isExtended, ke.isWithAlt, ke.wasDownBefore, ke.isUpNow);
		VirtualPress.pop_back();
	}
}

void emu_nativeInit(UINT64 hash) {
	LOG(DEBUG) << "[NC] START " << (void *)hash;
}

void emu_nativePush64(UINT64 val) {
	LOG(DEBUG) << "\t [NC] Argument: " << (void *)val;
}

static struct _Ret {
	int _pad[6];
} ret;

PUINT64 emu_nativeCall(void) {
	LOG(DEBUG) << "[NC] END";
	return (PUINT64)&ret;
}
