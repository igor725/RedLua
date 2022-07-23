#pragma once

#include "thirdparty\ScriptHook\inc\types.h"
#include "thirdparty\ScriptHook\inc\nativeCaller.h"

namespace NATIVES {
	template<typename... Args>
	static const char* CREATE_STRING(int flags, const char* textTemplate, Args... args) { return invoke<char*>(0xFA925AC00EB830B9, flags, textTemplate, args...); }
	static Hash GET_HASH_KEY(char* string) { return invoke<Hash>(0xFD340785ADF8CFB7, string); }
	static void PLAY_SOUND_FRONTEND(char* audioName, char* audioRef, BOOL p3, BOOL p4) { invoke<Void>(0x67C540AA08E4A6F5, audioName, audioRef, p3, p4); }
	static void STOP_SOUND_FRONTEND(char* audioName, char* audioRef) { invoke<Void>(0x0F2A2175734926D8, audioName, audioRef); }
	static void DRAW_TEXT(const char* text, float x, float y) { invoke<Void>(0xD79334A4BB99BAD1, text, x, y); }
	static void SET_TEXT_SCALE(float p0, float scale) { invoke<Void>(0x4170B650590B3B00, p0, scale); }
	static void SET_TEXT_COLOR_RGBA(int r, int g, int b, int a) { invoke<Void>(0x50A41AD966910F03, r, g, b, a); }
	static void SET_TEXT_CENTRE(BOOL align) { invoke<Void>(0xBE5261939FBECB8C, align); }
	static void SET_TEXT_DROPSHADOW(int distance, int r, int g, int b, int a) { invoke<Void>(0x1BE39DBAA7263CA5, distance, r, g, b, a); }
	static void DRAW_RECT(float x, float y, float width, float height, int r, int g, int b, int a, BOOL p8, BOOL p9) { invoke<Void>(0x405224591DF02025, x, y, width, height, r, g, b, a, p8, p9); }

	static void NOTIFY(int type, int duration, const char *message) {
		struct {
			int val;
			int _padding[14];
		} dur = {duration};

		struct {
			void *padding;
			const char *msg;
		} dat = {{0}, CREATE_STRING(10, "LITERAL_STRING", message)};

		switch(type) {
			case 0 /*Tooltip*/:
				invoke<Void>(0x049D5C615BD38BAD, &dur, &dat, true);
				break;
			case 1/*Objective*/:
				invoke<Void>(0xCEDBF17EFCC0E4A4, &dur, &dat, true);
				break;
		}
	}
}
