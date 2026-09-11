# Stock Lua matches the version label in the executable, not its private ABI.
include(FetchContent)
FetchContent_Declare(lua511
  URL https://www.lua.org/ftp/lua-5.1.1.tar.gz
  URL_HASH SHA256=c5daeed0a75d8e4dd2328b7c7a69888247868154acbda69110e97d4a6e17d1f0
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(lua511)

# The executable's copy of the 5.1.1 lexer (00a71350) carries that function's
# other two messages, "unfinished long comment" and "unfinished long string",
# but not "nesting of [[...]] is deprecated", which stock 5.1.1 emits from the
# same function under LUA_COMPAT_LSTR == 1. Two shipped scripts need the old
# behaviour: Scripts/global/commandhelpers.lua nests --[[ inside --[[ at line
# 17625 and Scripts/datatables/autoload/vehicleclasses.lua at line 190011. So
# the game built with the value 2. luaconf.h defines the macro unconditionally,
# which beats a command-line -D, so the vendored header is rewritten instead.
# Idempotent. Evidence: docs/MISSION_LUA_MACHINE.md.
set(lua511_conf "${lua511_SOURCE_DIR}/src/luaconf.h")
file(READ "${lua511_conf}" lua511_conf_text)
string(REPLACE "#define LUA_COMPAT_LSTR\t\t1" "#define LUA_COMPAT_LSTR\t\t2"
  lua511_conf_patched "${lua511_conf_text}")
if(NOT lua511_conf_patched STREQUAL lua511_conf_text)
  file(WRITE "${lua511_conf}" "${lua511_conf_patched}")
endif()
set(lua511_units lapi lcode ldebug ldo ldump lfunc lgc llex lmem lobject lopcodes
  lparser lstate lstring ltable ltm lundump lvm lzio lauxlib lbaselib
  loadlib ltablib liolib loslib lstrlib lmathlib ldblib)
set(lua511_sources)
foreach(unit IN LISTS lua511_units)
  list(APPEND lua511_sources "${lua511_SOURCE_DIR}/src/${unit}.c")
endforeach()
add_library(bsp_lua511 STATIC ${lua511_sources})
target_include_directories(bsp_lua511 PUBLIC "${lua511_SOURCE_DIR}/src")
target_compile_definitions(bsp_lua511 PRIVATE _CRT_SECURE_NO_WARNINGS)
target_compile_options(bsp_lua511 PRIVATE /W0)
