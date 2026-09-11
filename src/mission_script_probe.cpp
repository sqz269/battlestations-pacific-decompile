// Mission Lua machine probe.
//
// Builds a real Lua 5.1.1 state the way 006b8740 builds the mission machine,
// installs the 560 mission bindings from the table at 00e0b7b8 as stubs that
// log their name and argument count, then runs one installed mission script
// through the 006b89f0 chunk rule and calls the four entry points through the
// 00887750 named-call rule.
//
// The point is to check the recovered host contract against a real script: a
// contract that is wrong shows up here as a nil-call error on a name the table
// does not carry, or as a chunk that never loads. Nothing in this file is a
// reconstruction of a native body; the native rules it follows all live in
// bsp/mission_lua_host.hpp and are cited per step.
//
// Usage: bsp_mission_script_probe [game-root] [mission-name]
//            [--stub-dofile] [--skip-global-folders] [--skip-lobby-settings]
// Defaults to the installed copy and "usn/usn_2_java".

#include "bsp/mission_lua_host.hpp"
#include "bsp/mission_lua_machine.hpp"
#include "bsp/mission_scene_load.hpp"

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace {

// config/target.json's installed copy. Overridden by argv[1].
constexpr const char kDefaultGameRoot[]
    = "I:/SteamLibrary/steamapps/common/Battlestations Pacific";

// The name as the scene record's script table at +928h carries it, which is
// what 008860b0 concatenates. The subdirectory is part of the name: see the
// resolution report this probe prints.
constexpr const char kDefaultMissionName[] = "usn/usn_2_java";

// ---------------------------------------------------------------------------
// Call log. One entry per native binding the script actually reached.
// ---------------------------------------------------------------------------

struct NativeCall {
    std::string phase;
    std::string name;
    int argc;
};

struct ProbeState {
    std::string game_root;
    std::string phase;
    bool follow_dofile{true};
    bool run_global_folders{true};
    bool create_lobby_settings{true};
    std::vector<NativeCall> calls;
    std::map<std::string, int> call_counts;
    std::vector<std::string> dofile_paths;
    std::vector<std::string> dofile_missing;
    std::string first_error;
    std::string first_error_phase;
};

ProbeState g_probe;

// Every binding is installed as this one C function plus an index upvalue.
// 006b8610 uses nup = 0 because each native row is a distinct function; the
// probe needs identity at call time, so it carries the row index instead. That
// is the only divergence from the native registration, and it is invisible to
// the script: the global is still a plain C closure on a plain global name.
int binding_stub(lua_State* L)
{
    const int index = static_cast<int>(lua_tointeger(L, lua_upvalueindex(1)));
    const int argc = lua_gettop(L);
    const bsp::MissionLuaBinding& row = bsp::mission_lua_bindings()[static_cast<std::size_t>(index)];
    g_probe.calls.push_back(NativeCall{g_probe.phase, row.name, argc});
    g_probe.call_counts[row.name] += 1;
    return 0;
}

// 006b8720 is the machine's panic function; its body was not read by this
// packet, so the probe installs its own rather than guess. Reaching it means
// an unprotected error escaped, which is a probe bug, not a finding.
int probe_panic(lua_State* L)
{
    const char* message = lua_tostring(L, -1);
    std::cerr << "lua panic: " << (message != nullptr ? message : "(no message)") << "\n";
    return 0;
}

bool read_file(const std::string& path, std::vector<char>& output)
{
    std::ifstream input(path.c_str(), std::ios::binary);
    if (!input) {
        return false;
    }
    input.seekg(0, std::ios::end);
    const std::streamoff size = input.tellg();
    if (size < 0) {
        return false;
    }
    input.seekg(0, std::ios::beg);
    output.assign(static_cast<std::size_t>(size), '\0');
    if (size > 0) {
        input.read(output.data(), size);
    }
    return static_cast<bool>(input);
}

// 006b89f0. top = lua_gettop; luaL_loadbuffer; on success lua_pcall(0,
// LUA_MULTRET, 0) with errfunc 0, so a chunk gets no traceback; on failure
// lua_tolstring the message even when nothing will read it; then restore the
// entry top. Returns true when the chunk failed.
bool run_chunk(lua_State* L, const std::vector<char>& bytes, const char* chunk_name,
    std::string& error_out)
{
    const int top = lua_gettop(L);
    int status = luaL_loadbuffer(L, bytes.data(), bytes.size(), chunk_name);
    if (status == 0) {
        status = lua_pcall(L, 0, LUA_MULTRET, bsp::kLuaNoErrorHandler);
    }
    const bool failed = status != 0;
    if (lua_gettop(L) > top) {
        if (failed) {
            const char* message = lua_tolstring(L, -1, nullptr);
            error_out = message != nullptr ? message : "(no message)";
        }
        lua_settop(L, top);
    }
    return failed;
}

void note_error(const std::string& message)
{
    if (g_probe.first_error.empty() && !message.empty()) {
        g_probe.first_error = message;
        g_probe.first_error_phase = g_probe.phase;
    }
}

bool run_script_file(lua_State* L, const std::string& vfs_path)
{
    std::vector<char> bytes;
    const std::string disk_path = g_probe.game_root + "/" + vfs_path;
    if (!read_file(disk_path, bytes)) {
        return false;
    }
    std::string error;
    if (run_chunk(L, bytes, vfs_path.c_str(), error)) {
        note_error(error);
        std::cout << "    chunk error in " << vfs_path << ": " << error << "\n";
        if (error.find("nesting of [[") != std::string::npos) {
            // Stock lua-5.1.1 sets LUA_COMPAT_LSTR to 1, which turns a nested
            // long bracket into a lex error. The executable's copy of the same
            // lexer function (00a71350) carries that function's other two
            // messages and not this one, so the game built with the value 2.
            std::cout << "      the matched library must be built with "
                         "LUA_COMPAT_LSTR=2; see cmake/startup.cmake\n";
        }
    }
    return true;
}

// DoFile is NOT one of the 560 rows. 00b6a303 installs it as a global from the
// LuaStateOwner layer with callback 00b69e00, which runs the named script
// through 00b69d40. The probe follows it so the script reaches its real depth;
// --stub-dofile turns it back into a logging stub to show the difference.
int dofile_stub(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* path = lua_tolstring(L, 1, nullptr);
    const std::string vfs_path = path != nullptr ? path : "";
    g_probe.calls.push_back(NativeCall{g_probe.phase, "DoFile", argc});
    g_probe.call_counts["DoFile"] += 1;
    g_probe.dofile_paths.push_back(vfs_path);
    if (!g_probe.follow_dofile || vfs_path.empty()) {
        return 0;
    }
    const std::string saved_phase = g_probe.phase;
    g_probe.phase = "DoFile " + vfs_path;
    if (!run_script_file(L, vfs_path)) {
        g_probe.dofile_missing.push_back(vfs_path);
        std::cout << "    DoFile could not open " << vfs_path << "\n";
    }
    g_probe.phase = saved_phase;
    return 0;
}

// 006b8740 step 4. The pair table at 00cf8350 is walked in order and each row
// is run as lua_pushcclosure(open, 0); lua_pushstring(name); lua_call(1, 0).
// luaopen_package is absent, so the state has no require and no loadlib.
void open_standard_libraries(lua_State* L)
{
    const lua_CFunction openers[] = {
        luaopen_base, luaopen_table, luaopen_io, luaopen_os,
        luaopen_string, luaopen_math, luaopen_debug,
    };
    const std::size_t opener_count = sizeof(openers) / sizeof(openers[0]);
    if (opener_count != bsp::kMissionLuaStandardLibraryCount) {
        std::cerr << "library table size disagrees with the header\n";
        return;
    }
    for (std::size_t i = 0; i < opener_count; ++i) {
        lua_pushcclosure(L, openers[i], 0);
        lua_pushstring(L, bsp::kMissionLuaStandardLibraries[i].name);
        lua_call(L, 1, 0);
    }
}

// 00884be0 step 3 through 006b8610: lua_pushcclosure(fn, 0) then
// lua_setfield(LUA_GLOBALSINDEX, name). Every binding is a plain global; there
// is no namespace table and no metatable.
void register_bindings(lua_State* L)
{
    const bsp::MissionLuaBinding* rows = bsp::mission_lua_bindings();
    const std::size_t count = bsp::mission_lua_binding_count();
    for (std::size_t i = 0; i < count; ++i) {
        lua_pushinteger(L, static_cast<lua_Integer>(i));
        lua_pushcclosure(L, binding_stub, 1);
        lua_setfield(L, LUA_GLOBALSINDEX, rows[i].name);
    }
}

// 00887750, reduced to the no-argument, no-self-key case the four mission
// entry points use. debugtrap is fetched as the error handler before the name
// is resolved, the name is split on '.', and the stack is restored afterwards.
//
// use_error_handler = false reruns the same call with errfunc 0. The native
// always passes the handler; the probe needs the bare form only to show what
// the handler discards, because 008c8390 returns 0 results and Lua 5.1 then
// replaces the error object with nil.
bool call_entry_point(lua_State* L, const std::string& name, std::string& error_out,
    bool use_error_handler)
{
    const int saved_top = lua_gettop(L);
    int error_handler = bsp::kLuaNoErrorHandler;
    if (use_error_handler) {
        lua_getfield(L, LUA_GLOBALSINDEX, bsp::kMissionLuaErrorHandler);
        error_handler = lua_gettop(L);
    }

    const std::vector<std::string> segments = bsp::split_lua_entry_point_name(name);
    if (segments.empty()) {
        lua_settop(L, saved_top);
        return false;
    }
    lua_getfield(L, LUA_GLOBALSINDEX, segments[0].c_str());
    for (std::size_t i = 1; i < segments.size(); ++i) {
        lua_pushstring(L, segments[i].c_str());
        lua_gettable(L, -2);
        lua_remove(L, -2);
    }
    if (lua_isnil(L, -1)) {
        lua_settop(L, saved_top);
        return false;
    }
    const int status = lua_pcall(L, 0, LUA_MULTRET, error_handler);
    if (status != 0) {
        const char* message = lua_tolstring(L, -1, nullptr);
        error_out = message != nullptr ? message : "(no message)";
    }
    lua_settop(L, saved_top);
    return status == 0;
}

bool file_exists(const std::string& path)
{
    std::ifstream input(path.c_str(), std::ios::binary);
    return static_cast<bool>(input);
}

// 00886370, reduced to the disk case. The native enumerates .luab first and
// then runs a .lua only when its computed compiled name did not appear in the
// first pass. The install ships no .luab under either folder, so every .lua
// runs; the probe keeps the rule anyway so the ordering stays honest. The
// native's enumeration order comes from the VFS provider list, which this
// probe cannot reproduce, so it sorts by name instead. That divergence only
// matters for scripts that redefine each other's globals.
std::size_t run_script_folder(lua_State* L, const std::string& directory)
{
    namespace fs = std::filesystem;
    const fs::path root = fs::path(g_probe.game_root) / directory;
    std::error_code ec;
    if (!fs::is_directory(root, ec)) {
        return 0;
    }
    std::vector<std::string> compiled;
    std::vector<std::string> source;
    for (const fs::directory_entry& entry : fs::directory_iterator(root, ec)) {
        if (!entry.is_regular_file(ec)) {
            continue;
        }
        const std::string stem = entry.path().stem().string();
        const std::string extension = entry.path().extension().string();
        if (extension == ".luab") {
            compiled.push_back(stem);
        } else if (extension == ".lua") {
            source.push_back(stem);
        }
    }
    std::sort(compiled.begin(), compiled.end());
    std::sort(source.begin(), source.end());

    std::size_t ran = 0;
    for (const std::string& stem : compiled) {
        if (run_script_file(L, directory + "/" + stem + ".luab")) {
            ++ran;
        }
    }
    for (const std::string& stem : source) {
        if (std::find(compiled.begin(), compiled.end(), stem) != compiled.end()) {
            continue; // shadowed by the compiled pass, even when that load failed
        }
        if (run_script_file(L, directory + "/" + stem + ".lua")) {
            ++ran;
        }
    }
    return ran;
}

} // namespace

int main(int argc, char** argv)
{
    std::string mission_name = kDefaultMissionName;
    g_probe.game_root = kDefaultGameRoot;
    int positional = 0;
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--stub-dofile") {
            g_probe.follow_dofile = false;
        } else if (argument == "--skip-global-folders") {
            g_probe.run_global_folders = false;
        } else if (argument == "--skip-lobby-settings") {
            g_probe.create_lobby_settings = false;
        } else if (positional == 0) {
            g_probe.game_root = argument;
            positional = 1;
        } else if (positional == 1) {
            mission_name = argument;
            positional = 2;
        }
    }

    std::cout << "game root      : " << g_probe.game_root << "\n";
    std::cout << "mission name   : " << mission_name << "\n";
    std::cout << "DoFile         : " << (g_probe.follow_dofile ? "followed" : "stubbed") << "\n\n";

    // -- the machine, 006b8740 ----------------------------------------------
    lua_State* L = luaL_newstate();
    if (L == nullptr) {
        std::cerr << "luaL_newstate failed\n";
        return 1;
    }
    lua_atpanic(L, probe_panic);
    lua_gc(L, bsp::kLuaGcSetPause, bsp::kMissionLuaGcPause);
    open_standard_libraries(L);

    std::cout << "libraries      : " << bsp::kMissionLuaStandardLibraryCount
              << " opened, package/require absent\n";
    lua_getfield(L, LUA_GLOBALSINDEX, "require");
    const bool has_require = !lua_isnil(L, -1);
    lua_pop(L, 1);
    std::cout << "require global : " << (has_require ? "PRESENT (unexpected)" : "absent") << "\n";

    // -- the bindings, 00884be0 step 3 --------------------------------------
    register_bindings(L);
    const std::size_t binding_count = bsp::mission_lua_binding_count();
    std::cout << "bindings       : " << binding_count << " installed as plain globals, first \""
              << bsp::mission_lua_bindings()[0].name << "\" last \""
              << bsp::mission_lua_bindings()[binding_count - 1].name << "\"\n";

    // DoFile, from the LuaStateOwner layer at 00b6a303, not from the table.
    lua_pushcclosure(L, dofile_stub, 0);
    lua_setfield(L, LUA_GLOBALSINDEX, bsp::kMissionLuaStateOwnerGlobal);
    std::cout << "DoFile global  : installed from the owner layer (00b6a303), not from the table\n\n";

    // -- 00884be0 step 2, the platform chunk --------------------------------
    g_probe.phase = "platform chunk";
    {
        const char* source = bsp::kMissionLuaPlatformChunk;
        const std::vector<char> bytes(source, source + std::strlen(source));
        std::string error;
        if (run_chunk(L, bytes, source, error)) {
            note_error(error);
            std::cout << "platform chunk error: " << error << "\n";
        }
    }

    // -- 00884be0 step 4, fundamentals --------------------------------------
    g_probe.phase = "fundamentals";
    const bool fundamentals_ok = run_script_file(L, "Scripts/fundamentals.lua");
    std::cout << "fundamentals   : " << (fundamentals_ok ? "ran" : "MISSING on disk") << "\n";

    // -- 005e2f00, the LobbySettings table ----------------------------------
    // The native reads and writes the global table named by 00cf1f28 through
    // the LuaObject API, one slot per row of the pointer array at 00e08908.
    // The probe only creates the table and gives every recovered field a
    // value, because the values come from game state this probe has none of.
    // Without it the multiplayer mission scripts index a nil global at their
    // first line, which is what puts 005e2f00 before the entry points.
    if (g_probe.create_lobby_settings) {
        lua_createtable(L, 0, static_cast<int>(bsp::kLobbySettingsFieldCount));
        for (std::size_t slot = 0; slot < bsp::kLobbySettingsSlotCount; ++slot) {
            const char* field = bsp::lobby_settings_field_name(slot);
            if (field == nullptr) {
                continue; // slot 0Dh, the null pointer at 00e08940
            }
            lua_pushinteger(L, 0);
            lua_setfield(L, -2, field);
        }
        lua_setfield(L, LUA_GLOBALSINDEX, bsp::kLobbySettingsTable);
        std::cout << "LobbySettings  : created with " << bsp::kLobbySettingsFieldCount
                  << " fields (005e2f00)\n";
    }

    // -- 00886900, the global script folders --------------------------------
    // Not part of 00884be0: this runs earlier, from 004dc6a0 and 004e3aa0. It
    // is here because the mission chunk's entry points call globals only these
    // folders define. --skip-global-folders shows what breaks without it.
    if (g_probe.run_global_folders) {
        g_probe.phase = "Scripts/global/";
        const std::size_t global_count = run_script_folder(L, "Scripts/global");
        g_probe.phase = "Scripts/datatables/autoload/";
        const std::size_t autoload_count = run_script_folder(L, "Scripts/datatables/autoload");
        std::cout << "global folders : " << global_count << " from Scripts/global/, "
                  << autoload_count << " from Scripts/datatables/autoload/\n";
    } else {
        std::cout << "global folders : SKIPPED (--skip-global-folders)\n";
    }

    // -- the mission script path, 008860b0 ----------------------------------
    const std::string script_path = bsp::mission_script_path(mission_name);
    const bool script_present = file_exists(g_probe.game_root + "/" + script_path);
    std::cout << "script path    : \"" << script_path << "\" "
              << (script_present ? "present" : "ABSENT") << "\n";
    {
        // The bare short name derive_scene_short_name produces has no
        // subdirectory. Showing both settles which one the record must carry.
        const std::string bare = mission_name.substr(mission_name.find_last_of('/') + 1);
        const std::string bare_path = bsp::mission_script_path(bare);
        std::cout << "bare-name path : \"" << bare_path << "\" "
                  << (file_exists(g_probe.game_root + "/" + bare_path) ? "present" : "ABSENT")
                  << "\n\n";
    }

    // -- the mission chunk, 00885110 through 006b89f0 -----------------------
    g_probe.phase = "mission chunk";
    const std::size_t calls_before_chunk = g_probe.calls.size();
    const bool chunk_ran = run_script_file(L, script_path);
    if (!chunk_ran) {
        std::cout << "mission chunk could not be opened; stopping\n";
        lua_close(L);
        return 1;
    }
    std::cout << "load-time native calls (" << (g_probe.calls.size() - calls_before_chunk) << "):\n";
    for (std::size_t i = calls_before_chunk; i < g_probe.calls.size(); ++i) {
        std::cout << "  " << g_probe.calls[i].name << "  argc=" << g_probe.calls[i].argc
                  << "  [" << g_probe.calls[i].phase << "]\n";
    }
    if (!g_probe.dofile_paths.empty()) {
        std::cout << "DoFile targets:\n";
        for (const std::string& path : g_probe.dofile_paths) {
            std::cout << "  " << path << " "
                      << (file_exists(g_probe.game_root + "/" + path) ? "(resolved at the VFS root)"
                                                                      : "(NOT at the VFS root)")
                      << "\n";
        }
    }

    // -- the four entry points, 0045f440 / 0045f520 -------------------------
    std::cout << "\nentry points:\n";
    for (std::size_t i = 0; i < bsp::kMissionLuaEntryPointCount; ++i) {
        const bsp::MissionLuaEntryPoint& entry = bsp::kMissionLuaEntryPoints[i];
        g_probe.phase = entry.name;
        const std::size_t before = g_probe.calls.size();
        std::string error;
        lua_getfield(L, LUA_GLOBALSINDEX, entry.name);
        const bool defined = !lua_isnil(L, -1);
        lua_pop(L, 1);
        if (!defined) {
            std::cout << "  " << entry.name << ": not defined, skipped (the 0045f440/0045f520 guard)\n";
            continue;
        }
        const bool ok = call_entry_point(L, entry.name, error, true);
        std::cout << "  " << entry.name << ": " << (ok ? "ok" : "FAILED")
                  << ", " << (g_probe.calls.size() - before) << " native calls"
                  << (entry.threadsafe ? "  [0045f440]" : "  [0045f520]") << "\n";
        for (std::size_t c = before; c < g_probe.calls.size(); ++c) {
            std::cout << "      called " << g_probe.calls[c].name
                      << "  argc=" << g_probe.calls[c].argc << "\n";
        }
        if (!ok) {
            std::cout << "      through debugtrap: \"" << error << "\"\n";
            std::string bare_error;
            call_entry_point(L, entry.name, bare_error, false);
            std::cout << "      with errfunc 0   : \"" << bare_error << "\"\n";
            note_error(bare_error.empty() ? error : bare_error);
        }
    }

    // -- summary ------------------------------------------------------------
    std::cout << "\ndistinct natives called: " << g_probe.call_counts.size() << "\n";
    std::size_t unresolved = 0;
    for (const std::pair<const std::string, int>& entry : g_probe.call_counts) {
        if (bsp::find_mission_lua_binding(entry.first.c_str()) == nullptr) {
            ++unresolved;
            std::cout << "  not in the 00e0b7b8 table: " << entry.first << "\n";
        }
    }
    std::cout << "natives outside the table: " << unresolved << "\n";
    if (g_probe.first_error.empty()) {
        std::cout << "first error: none\n";
    } else {
        std::cout << "first error [" << g_probe.first_error_phase << "]: " << g_probe.first_error << "\n";
    }

    lua_close(L);
    return 0;
}
