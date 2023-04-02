#pragma once

#include "scripthook.hpp"

namespace NATIVES {
#if defined(REDLUA_GTAV)
	static const char* CREATE_STRING(int flags, const char* textTemplate, const char *string) {(void)flags; (void)textTemplate; return string; }
	static Hash GET_HASH_KEY(char* string) { return invoke<Hash>(0xD24D37CC275948CC, string); }
	static void PLAY_SOUND_FRONTEND(char* audioName, char* audioRef, BOOL p3) { invoke<Void>(0x67C540AA08E4A6F5, -1, audioName, audioRef, p3); }
	static void DRAW_TEXT(const char* text, float x, float y) {
		invoke<Void>(0x25FBB336DF1804CB, "STRING");
		invoke<Void>(0x6C188BE134E074AA, text);
		invoke<Void>(0xCD015E5BB0D96A57, x, y);
	}
	static void SET_TEXT_SCALE(float p0, float scale) { invoke<Void>(0x07C837F9A01C34C9, p0, scale); }
	static void SET_TEXT_COLOR_RGBA(int r, int g, int b, int a) { invoke<Void>(0xBE6B23FFA53FB442, r, g, b, a); }
	static void SET_TEXT_CENTRE(BOOL align) { invoke<Void>(0xC02F4DBFB51D988B, align); }
	static void SET_TEXT_DROPSHADOW(int distance, int r, int g, int b, int a) { invoke<Void>(0x465C84BC39F1C351, distance, r, g, b, a); }
	static void DRAW_RECT(float x, float y, float width, float height, int r, int g, int b, int a, BOOL p8, BOOL p9) { (void)p8; (void)p9; invoke<Void>(0x3A618A217E5154F0, x, y, width, height, r, g, b, a); }
	static int GET_LOCALE(void) { return invoke<int>(0x2BDD44CC428A7EAE); }

	static void SHOW_KEYBOARD(int type, const char *title, const char *def, int maxLen) { invoke<Void>(0x00DC833F2568DBF6, type, title, "", def, "", "", "", maxLen); }
	static int UPDATE_KEYBOARD() { return invoke<int>(0x0CF2B696BBF945AE); }
	static void CANCEL_KEYBOARD() { invoke<Void>(0x58A39BE597CE99CD); }
	static const char *RESULT_KEYBOARD() { return invoke<const char *>(0x8362B09B91893647); }

	static void NOTIFY(int type, int duration, const char *message) {
		(void)type; (void)duration;
		invoke<Void>(0x202709F4C58A0424, "STRING");
		invoke<Void>(0x6C188BE134E074AA, message);
		invoke<Void>(0x2ED7843F8F801023, true, true);
	}
#elif defined(REDLUA_RDR3)
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
	static int GET_LOCALE(void) { return invoke<int>(0xDB917DA5C6835FCC); }

	static void SHOW_KEYBOARD(int type, const char *title, const char *def, int maxLen) { invoke<Void>(0x044131118D8DB3CD, type, title, "", def, "", "", "", maxLen); }
	static int UPDATE_KEYBOARD() { return invoke<int>(0x37DF360F235A3893); }
	static void CANCEL_KEYBOARD() { invoke<Void>(0x58A39BE597CE99CD); }
	static const char *RESULT_KEYBOARD() { return invoke<const char *>(0xAFB4CF58A4A292B1); }

	static void NOTIFY(int type, int duration, const char *message) {
		struct {
			int val;
			int _padding[14];
		} dur = {duration};

		struct {
			void *padding;
			const char *msg;
		} dat = {{0}, CREATE_STRING(10, "LITERAL_STRING", message)};

		switch (type) {
			case 0 /*Tooltip*/:
				invoke<Void>(0x049D5C615BD38BAD, &dur, &dat, true);
				break;
			case 1/*Objective*/:
				invoke<Void>(0xCEDBF17EFCC0E4A4, &dur, &dat, true);
				break;
		}
	}
#endif
}
