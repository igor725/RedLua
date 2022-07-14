@echo off
setlocal enableextensions enabledelayedexpansion
SET MY_CFLAGS=/DELPP_NO_DEFAULT_LOG_FILE /DELPP_DISABLE_LOG_FILE_FROM_ARG ^
/EHsc /MP /MT /DLL /Foobjs\ /Fe"D:\SteamLibrary\steamapps\common\Red Dead Redemption 2\RedLua.asi"
SET MY_LDFLAGS=/incremental /libpath:luajit\src /libpath:ScriptHook\lib /dll
SET MY_LIBS=ScriptHookRDR2.lib user32.lib lua51.lib
CL %MY_CFLAGS% *.cpp /link %MY_LDFLAGS% %MY_LIBS%
