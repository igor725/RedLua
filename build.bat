@echo off
setlocal enableextensions enabledelayedexpansion
SET RL_LUAJIT_SOURCE_DIR=.\src\thirdparty\luajit\src
SET RL_SCRIPTHOOK_SDK_DIR=.\src\thirdparty\ScriptHook
SET RL_OUT_PATH="D:\SteamLibrary\steamapps\common\Red Dead Redemption 2\RedLua.asi"
SET RL_CFLAGS=/DELPP_NO_DEFAULT_LOG_FILE /DELPP_DISABLE_LOG_FILE_FROM_ARG /W2 ^
/Isrc\ /EHsc /MP /MT /DLL /Foobjs\ /Fe%RL_OUT_PATH%
SET RL_LDFLAGS=/incremental /libpath:"%RL_LUAJIT_SOURCE_DIR%" /libpath:"%RL_SCRIPTHOOK_SDK_DIR%\lib" /dll
SET RL_LIBS=ScriptHookRDR2.lib user32.lib lua51.lib
SET RL_SOURCES=src\*.cpp src\thirdparty\*.cpp src\menus\*.cpp
CL %RL_CFLAGS% %RL_SOURCES% /link %RL_LDFLAGS% %RL_LIBS%
endlocal
