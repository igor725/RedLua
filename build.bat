@echo off
IF NOT "%VSCMD_ARG_TGT_ARCH%"=="x64" (
	ECHO Unsupported architecture
	EXIT /B 1
)
setlocal enableextensions enabledelayedexpansion
SET RL_LUAJIT_SOURCE_DIR=.\src\thirdparty\luajit\src
IF NOT EXIST "%RL_LUAJIT_SOURCE_DIR%\lua51.lib" (
	PUSHD %RL_LUAJIT_SOURCE_DIR%
	CALL .\msvcbuild.bat
	IF NOT "!ERRORLEVEL!"=="0" (
		ECHO Failed to compile LuaJIT
		EXIT /b 1
	)
	POPD
)
SET RL_SCRIPTHOOK_SDK_DIR=.\src\thirdparty\ScriptHook
SET RL_OUT_PATH="D:\SteamLibrary\steamapps\common\Red Dead Redemption 2"
MKDIR ".\objs\" 2> NUL
IF NOT EXIST !RL_OUT_PATH! (
	SET RL_OUT_PATH=".\objs\output"
	MKDIR !RL_OUT_PATH! 2> NUL
)
IF NOT EXIST "!RL_OUT_PATH!\lua51.dll" (
	COPY %RL_LUAJIT_SOURCE_DIR%\lua51.dll %RL_OUT_PATH% 2> NUL
)
SET RL_CFLAGS=/DELPP_NO_DEFAULT_LOG_FILE /DELPP_DISABLE_LOG_FILE_FROM_ARG /W2 ^
/Isrc\ /EHsc /MP /MT /DLL /Foobjs\ /Fe!RL_OUT_PATH!\RedLua.asi
SET RL_LDFLAGS=/INCREMENTAL /LIBPATH:"%RL_LUAJIT_SOURCE_DIR%" /LIBPATH:"%RL_SCRIPTHOOK_SDK_DIR%\lib" /dll
SET RL_LIBS=ScriptHookRDR2.lib user32.lib shell32.lib lua51.lib
SET RL_SOURCES=src\*.cpp src\thirdparty\*.cpp src\menus\*.cpp
CL %RL_CFLAGS% %RL_SOURCES% /link %RL_LDFLAGS% %RL_LIBS%
endlocal
