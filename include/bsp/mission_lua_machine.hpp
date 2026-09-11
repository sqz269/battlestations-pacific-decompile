#pragma once
// The mission Lua machine as an assembly: where the mission state's globals
// come from, which host call places each of them, and the order the mission
// load walks the Lua hosts in.
//
// This header does not restate bsp/mission_lua_host.hpp. That header carries
// the machine construction (006b8740), the 560-row binding table at 00e0b7b8,
// the chunk and named-call rules, and the MissionLuaHostServices seam. What is
// here is the layer above: the mission state is NOT fully described by the
// binding table, and the mission load runs five different Lua hosts in a fixed
// order. Both facts are needed to bring the state up correctly and neither is
// derivable from the binding table alone.
//
// Every name below is a hypothesis, not a recovered symbol.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bsp {

// ---------------------------------------------------------------------------
// Where the mission state's globals come from.
//
// 00884be0 installs the 560 rows of 00e0b7b8 and runs two chunks. That is not
// the whole global namespace the installed mission scripts rely on: the very
// first statement of Scripts/missions/usn/usn_2_java.lua is DoFile(...), and
// DoFile is not one of the 560 rows. It is installed from the LuaStateOwner
// layer at 00b6a303 with callback 00b69e00, over the same lua_State: game+1A0Ch
// is an inline LuaStateOwner constructed at 004dd641 from [[game+1A08h]+4h]+4h.
//
// A rebuild that installs only the binding table produces a state on which
// every stock mission script fails at its first line. The probe
// src/mission_script_probe.cpp demonstrates both halves.
// ---------------------------------------------------------------------------

enum class MissionLuaGlobalOrigin {
    // 006b8610 walking 00e0b7b8, from 00884be0.
    BindingTable,
    // 006b8ad0 on a literal, from 00884be0.
    PlatformChunk,
    // 00b6a303, the LuaStateOwner over the same state.
    StateOwnerCallback,
    // 006b89f0 on the 00884770 singleton bytes, from 00884be0.
    FundamentalsChunk,
    // 00886900 running Scripts/global/ then Scripts/datatables/autoload/.
    GlobalScriptFolders,
};

struct MissionLuaGlobalSource {
    MissionLuaGlobalOrigin origin;
    const char* description;
    std::uint32_t install_site; // native address that places these globals
};

// In the order the game reaches them. The binding table and the platform chunk
// are both inside 00884be0 and the table is walked after the chunk runs, so a
// binding may not be assumed present while the platform chunk executes.
inline constexpr MissionLuaGlobalSource kMissionLuaGlobalSources[] = {
    {MissionLuaGlobalOrigin::PlatformChunk, "PC=true", 0x00884BE0U},
    {MissionLuaGlobalOrigin::BindingTable, "560 rows of 00e0b7b8", 0x006B8610U},
    {MissionLuaGlobalOrigin::FundamentalsChunk, "Scripts\\fundamentals.lua", 0x006B89F0U},
    {MissionLuaGlobalOrigin::StateOwnerCallback, "DoFile", 0x00B6A303U},
    {MissionLuaGlobalOrigin::GlobalScriptFolders, "Scripts/global/, Scripts/datatables/autoload/",
        0x00886900U},
};
inline constexpr std::size_t kMissionLuaGlobalSourceCount = 5;

// The one global the mission state needs that the binding table does not
// carry. Callback 00b69e00, which runs the named script through 00b69d40.
// Installed-file-checked: both arguments the stock mission script passes it
// ("Scripts/datatables/Inputs.lua" and "shipnames.lua") resolve against the
// install root, so the callback takes a VFS-root-relative path and applies no
// directory of its own.
inline constexpr const char* kMissionLuaStateOwnerGlobal = "DoFile"; // 00b6a303

// True when a name the script called has to come from somewhere other than the
// 560-row table. Pure; no state.
bool mission_lua_global_needs_owner_layer(const char* name) noexcept;

// ---------------------------------------------------------------------------
// The Lua hosts the mission load walks, in call order.
//
// All five run inside BSP_Game_LoadMissionScene (004dfb70) except 00886900,
// which runs earlier from 004dc6a0 / 004e3aa0 and is listed because the mission
// chunk depends on the globals it leaves behind. 004dfb70 raises the game+644h
// script-load depth around the entry-point group, which is what makes the
// Loading_* bindings inert inside a load.
// ---------------------------------------------------------------------------

struct MissionLoadLuaStep {
    std::uint32_t address;
    const char* name; // hypothesis, not a recovered symbol
    const char* effect;
    bool inside_load_depth; // true when game+644h is raised across the step
};

inline constexpr MissionLoadLuaStep kMissionLoadLuaSteps[] = {
    {0x00886900U, "BSP_MissionLua_RunGlobalScriptFolders",
        "Scripts/global/ then Scripts/datatables/autoload/; runs from 004dc6a0 and 004e3aa0, "
        "before the scene load", false},
    {0x005E2F00U, "BSP_Game_SyncLobbySettingsFromLua",
        "reads and writes the LobbySettings global table; sets the three mode flags and the "
        "effective game mode", false},
    {0x004D30F0U, "BSP_Game_RefillScriptedNameList",
        "rebuilds the name set at game+1934h by iterating a Lua global table", false},
    {0x008860B0U, "BSP_MissionScript_RunFile",
        "\"Scripts/missions/\" + name + \".lua\" through 00885fb0 with variants enabled", true},
    {0x0045F520U, "BSP_Game_CallLuaEntryPointForced",
        "luaPrecacheUnits then luaStageInitMulti, on the calling thread", true},
    {0x0045F440U, "BSP_Game_CallLuaEntryPointThreadSafe",
        "luaStageInit, and luaEngineMovieInit for slot 9", true},
};
inline constexpr std::size_t kMissionLoadLuaStepCount = 6;

// ---------------------------------------------------------------------------
// The LobbySettings table, 005e2f00.
//
// The pointer array at 00e08908 holds fourteen slots; the fourteenth is null,
// which is exactly why the loop at 005e2f6f skips index 0Dh. Slots 0 and 2 are
// also special-cased: 005e2f8b excludes them from the "read the stored value
// back" branch, so PlayerCount and GameMode are always written from the game
// side and never taken from the script.
// ---------------------------------------------------------------------------

inline constexpr const char* kLobbySettingsTable = "LobbySettings"; // 00cf1f28

inline constexpr const char* kLobbySettingsFields[] = {
    "PlayerCount",    // 00cf1f1c
    "Map",            // 00cea3f8
    "GameMode",       // 00cf1f10
    "MapSize",        // 00cf1f08
    "UnitType",       // 00cf1efc
    "ResourceLimit",  // 00cf1eec
    "TimeLimit",      // 00cf1ee0
    "TimeLimit_IC",   // 00cf1ed0
    "PointLimit",     // 00cf1ec4
    "RoundLimit",     // 00cf1eb8
    "EnablePowerups", // 00cf1ea8
    "EnableMap",      // 00cf1e9c
    "ReloadPayload",  // 00cf1e8c
};
inline constexpr std::size_t kLobbySettingsFieldCount = 13;

// Slot count the native loop walks, including the null slot it skips.
inline constexpr std::size_t kLobbySettingsSlotCount = 14; // 005e2f6f, iVar5 < 0Eh

// Null for the skipped slot 0Dh and for any index past the array. Pure.
const char* lobby_settings_field_name(std::size_t slot) noexcept;

// 005e2f8b: slots 0 and 2 never read the script's stored value back.
bool lobby_settings_field_is_game_owned(std::size_t slot) noexcept;

// ---------------------------------------------------------------------------
// The mission script name.
//
// 008860b0 concatenates "Scripts/missions/" + name + ".lua" with no directory
// walk. Installed-file-checked: Scripts/missions/ holds no loose .lua at all,
// only eight subdirectories, so the name the scene record carries at +928h must
// include the subdirectory ("usn/usn_2_java", not "usn_2_java"). The short name
// derive_scene_short_name (004cd7f0) produces is therefore NOT what reaches
// 008860b0; the two are different fields of the record.
// ---------------------------------------------------------------------------

// True when the name would resolve to a path directly under Scripts/missions/,
// which no installed mission script occupies. Pure; a name that fails this is
// a record field taken from the wrong place.
bool mission_script_name_carries_subdirectory(const std::string& script_name) noexcept;

// The eight mission subdirectories the install ships, for the check above.
std::vector<std::string> installed_mission_subdirectories();

} // namespace bsp
