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
SET RL_OUT_BIN=RedLua.asi
SET RL_OUT_PATH="D:\SteamLibrary\steamapps\common\Red Dead Redemption 2"
SET RL_LIBS=user32.lib shell32.lib lua51.lib
SET RL_SOURCES=src\*.cpp
SET RL_LDFLAGS=/dll /INCREMENTAL /LIBPATH:"%RL_LUAJIT_SOURCE_DIR%"
SET RL_CFLAGS=/DELPP_NO_DEFAULT_LOG_FILE /DELPP_DISABLE_LOG_FILE_FROM_ARG ^
/DWIN32_LEAN_AND_MEAN /D_CRT_SECURE_NO_WARNINGS /W2 /Isrc\ /EHsc /MP /DLL /Foobjs\

IF "%1"=="standalone" (
	SET RL_OUT_PATH=".\objs\output"
	SET RL_CFLAGS=/DREDLUA_STANDALONE /Zi /MTd /Fd!RL_OUT_PATH!\ ^
/DDEBUG /D_DEBUG /DEBUG !RL_CFLAGS!
	SET RL_SOURCES=!RL_SOURCES! src\thirdparty\easyloggingpp.cpp
	SET RL_OUT_BIN=RedLua.dll	
) ELSE (
	SET RL_CFLAGS=!RL_CFLAGS! /MT
	SET RL_LIBS=!RL_LIBS! ScriptHookRDR2.lib
	SET RL_SOURCES=!RL_SOURCES! src\thirdparty\*.cpp src\menus\*.cpp
	SET RL_LDFLAGS=!RL_LDFLAGS! /LIBPATH:"!RL_SCRIPTHOOK_SDK_DIR!\lib"
)
MKDIR ".\objs\" 2> NUL
IF NOT EXIST !RL_OUT_PATH! (
	SET RL_OUT_PATH=".\objs\output"
	MKDIR !RL_OUT_PATH! 2> NUL
)
IF NOT "%1"=="standalone" IF NOT EXIST "!RL_OUT_PATH!\lua51.dll" (
	COPY %RL_LUAJIT_SOURCE_DIR%\lua51.dll %RL_OUT_PATH% 2> NUL
)
CL !RL_CFLAGS! /Fe!RL_OUT_PATH!\!RL_OUT_BIN! !RL_SOURCES! /link !RL_LDFLAGS! !RL_LIBS!
IF NOT "%ERRORLEVEL%"=="0" (
	endlocal
	EXIT /b 1
)
endlocal
