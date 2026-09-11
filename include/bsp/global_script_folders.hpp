#pragma once

#include "bsp/language_catalog.hpp"
#include "bsp/mission_lua_host.hpp"
#include "bsp/native_string.hpp"

namespace bsp {
struct GlobalScriptFolderContext {
    NativeStringStorage& strings;
    // The live resource-enumeration adapter for0109CEEC, reloaded for each
    // extension pass. VfsLocaleRuntime implements this existing interface.
    LanguageCatalogSource*& resources_0109ceec;
};

//00886370: ECX=captured mission Lua host, stack directory/flags, RET8.
// Enumerates/runs .luab first, then .lua unless its computed compiled name
// appeared in the first enumeration (even when that compiled load failed).
// Reuses00885110 run_script_file and existing00886280 VFS enumeration.
// Native list/vector allocation and iterator ABIs remain library projections.
void load_global_script_folder_00886370(MissionLuaHostServices& captured_owner,
    const char* directory, std::uint32_t flags, GlobalScriptFolderContext&);

//00886900: ECX=captured mission Lua host, RET. Same owner for both folders.
void load_global_script_folders_00886900(MissionLuaHostServices& captured_owner,
    GlobalScriptFolderContext&);
} // namespace bsp
