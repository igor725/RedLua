#pragma once

#define REDLUA_NAME "RedLua"
#define REDLUA_VERSION "v0.2.2"
#define REDLUA_VERSION_NUM 022
#define REDLUA_FULLNAME REDLUA_NAME " " REDLUA_VERSION

#define REDLUA_TAGS_URL "https://api.github.com/repos/igor725/RedLua/tags"
#define REDLUA_RELS_URL "https://github.com/igor725/RedLua/releases/tag/"
#define REDLUA_NATIVEDB_URL "https://raw.githubusercontent.com/alloc8or/rdr3-nativedb-data/master/natives.json"

#define REDLUA_ROOT_DIR ".\\RedLua\\"
#define REDLUA_SCRIPTS_DIR REDLUA_ROOT_DIR "Scripts\\"
#define REDLUA_LIBS_DIR REDLUA_ROOT_DIR "Libs\\"
#define REDLUA_CLIBS_DIR REDLUA_ROOT_DIR "Libs\\C\\"

#define REDLUA_NATIVES_FILE REDLUA_ROOT_DIR "Natives.json"
#define REDLUA_SETTINGS_FILE REDLUA_ROOT_DIR "Settings.json"
#define REDLUA_LOGCONF_FILE REDLUA_ROOT_DIR "Log.conf"

#define REDLUA_PATHS REDLUA_LIBS_DIR "?.lua;" REDLUA_LIBS_DIR "\\?\\init.lua"
#define REDLUA_CPATHS REDLUA_CLIBS_DIR "?.dll;" REDLUA_CLIBS_DIR "\\?\\core.dll"
