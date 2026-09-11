#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// Host projection of the mission Lua host that the game object owns at
// game+1A08h. Recovered read-only from 00884be0 (bring-up), 006b8740 (state
// creation), 006b8610 (global registration), 006b89f0 (chunk load and run),
// 00885110 / 00885fb0 / 008860b0 (file load), 00887750 / 00887b30 / 00887e50
// (named entry-point dispatch), 0045f440 / 0045f520 (guarded entry points) and
// the inlined game+644h counter at 004b6b00..004b6b3c. Evidence and native ABI:
// docs/MISSION_LUA_HOST.md.
//
// The embedded interpreter is Lua 5.1.1 compiled with the register convention
// (lua_State* in ECX, first integer argument in EDX). Nothing here rebuilds any
// part of that library; MissionLuaHostServices is the seam where the matched
// library attaches, with one method per native Lua API call site.
namespace bsp {

// ---------------------------------------------------------------------------
// Standard libraries, opened by 006b8740 from the pair table at 00cf8350.
// ---------------------------------------------------------------------------

struct LuaStandardLibrary {
    const char* name; // Lua global the library is installed under.
    std::uint32_t open_function; // luaopen_* address in the matched library.
};

// Seven entries, in the order 006b8740 walks them. luaopen_package (00c2ff90)
// is absent, so the mission state has no require, no package and no loadlib.
inline constexpr LuaStandardLibrary kMissionLuaStandardLibraries[] = {
    {"", 0x00A67210U}, // luaopen_base; name string 00ce3a0c is empty
    {"table", 0x00A66010U},
    {"io", 0x00A652B0U},
    {"os", 0x00A64190U},
    {"string", 0x00A63910U},
    {"math", 0x00A61C50U},
    {"debug", 0x00A61620U},
};
inline constexpr std::size_t kMissionLuaStandardLibraryCount = 7;

// lua_gc(L, LUA_GCSETPAUSE, 100) at 006b8768: EDX = 6, stack argument 100. A
// pause of 100 means the collector restarts as soon as a cycle finishes.
inline constexpr int kLuaGcSetPause = 6;
inline constexpr int kMissionLuaGcPause = 100;

// Pseudo-indices and sentinels the native call sites use literally.
inline constexpr int kLuaGlobalsIndex = -10002; // EDX = 0FFFFD8EEh at 006b8463
inline constexpr int kLuaMultRet = -1; // 006b7e50-style nresults, 00887986
inline constexpr int kLuaNoErrorHandler = 0; // errfunc pushed at 006b8a1c

// Panic function installed by lua_atpanic at 006b8759.
inline constexpr std::uint32_t kMissionLuaPanicFunction = 0x006B8720U;

// ---------------------------------------------------------------------------
// The registration table at 00e0b7b8: {const char* name, lua_CFunction fn}
// pairs, terminated by a null function pointer at 00e0c938. 00884be0 installs
// every row as a plain global through 006b8610, which is
// lua_pushcclosure(L, fn, 0) followed by lua_setfield(L, LUA_GLOBALSINDEX,
// name). There is no namespace table: all 560 bindings are globals.
// ---------------------------------------------------------------------------

struct MissionLuaBinding {
    const char* name; // Lua global name.
    std::uint32_t address; // Native lua_CFunction, __fastcall(lua_State* in ECX).
};

// The table in table order. Addresses are the recovered call targets; argument
// conventions of the individual bodies are not reconstructed here, only the
// registration contract they all share.
const MissionLuaBinding* mission_lua_bindings() noexcept;
std::size_t mission_lua_binding_count() noexcept;

// Linear lookup over the table, matching the native's own exact-name semantics
// (lua_setfield installs the key verbatim). Returns nullptr when absent.
const MissionLuaBinding* find_mission_lua_binding(const char* name) noexcept;

// 00884be0 runs this chunk through 006b8ad0 before the table is installed.
inline constexpr const char* kMissionLuaPlatformChunk = "PC=true"; // 00d0e714

// 00884be0 then loads this chunk name; the bytes come from the singleton at
// 00884770 rather than the virtual file system, so the name is a chunk label.
inline constexpr const char* kMissionLuaFundamentalsChunk = "Scripts\\fundamentals.lua"; // 00d0e6f8

// The global function 00887750 pushes as the pcall error handler before every
// named call, and the dotted name that handler resolves in turn.
inline constexpr const char* kMissionLuaErrorHandler = "debugtrap"; // 00d0e79c
inline constexpr const char* kMissionLuaTracebackName = "debug.traceback"; // 008c8390

// Global table 00887750 indexes when a call carries a self key, and the byte
// the dotted-path splitter uses.
inline constexpr const char* kMissionLuaSelfTable = "thisTable"; // 00ce7494
inline constexpr char kMissionLuaNameSeparator = '.'; // DL = 2Eh at 00887863

// Script file paths come from mission_script_path in bsp/mission_scene_load.hpp,
// which already models the "Scripts/missions/" + name + ".lua" build at 008860b0.

// ---------------------------------------------------------------------------
// The game+644h re-entry guard. 004b6b20 increments the counter, 004b6b30
// decrements it and 004b6b00 adds +1 or -1 from a bool; all three are inlined
// at their use sites, including 004e0a83 and 004e0c52 inside the mission scene
// load. 008890f0 is the predicate (depth > 0). While the depth is positive the
// Loading_* bindings 008cc850 and 008c77f0 skip their bodies, so a script that
// is being run as part of an outer load cannot drive the loading screen.
// ---------------------------------------------------------------------------

class ScriptLoadGuard {
public:
    // 004b6b20: [00e188a8]+644h += 1.
    void enter() noexcept { depth_ += 1; }
    // 004b6b30: [00e188a8]+644h += -1. The native does not clamp.
    void leave() noexcept { depth_ -= 1; }
    // 004b6b00: adds +1 when the byte argument is non-zero, -1 otherwise.
    void adjust(bool entering) noexcept { depth_ += entering ? 1 : -1; }
    int depth() const noexcept { return depth_; }
    // 008890f0: xor eax,eax / cmp [ecx+644h],eax / setg al.
    bool active() const noexcept { return depth_ > 0; }

private:
    int depth_{0};
};

// 008cc93a and 008c78d4 both compare against 1 with a signed test and skip the
// body when the depth is at least 1, which is the same rule as 008890f0.
bool loading_screen_calls_suppressed(int script_load_depth) noexcept;

// ---------------------------------------------------------------------------
// Chunk and call outcomes.
// ---------------------------------------------------------------------------

enum class LuaChunkStatus {
    Ok, // luaL_loadbuffer and lua_pcall both returned 0
    LoadFailed, // luaL_loadbuffer returned non-zero; no pcall was attempted
    RunFailed, // lua_pcall returned non-zero
};

// 006b89f0 collapses both failures into the single byte it returns.
inline bool lua_chunk_failed(LuaChunkStatus status) noexcept
{
    return status != LuaChunkStatus::Ok;
}

struct LuaChunkResult {
    LuaChunkStatus status{LuaChunkStatus::Ok};
    // Copied from lua_tolstring(L, -1, nullptr) only when the caller supplied a
    // sink. 00885110 supplies a stack local and then releases it without
    // reading it, so a failing mission script reports nothing.
    std::string error_message;
    bool error_message_captured{false};
    // Number of values 006b89f0 handed to the result reader 00887220 on
    // success. Zero when the caller passed no sink, as 00885110 does.
    int results_collected{0};
};

// ---------------------------------------------------------------------------
// Call arguments. 00887750 walks a vector of twenty-byte records and pushes
// each through 00885da0, which switches on the tag at record+4.
// ---------------------------------------------------------------------------

enum class MissionLuaArgumentType {
    Number = 0, // 006b80e0 -> lua_pushnumber
    String = 1, // 006b8120 -> lua_pushstring, from the native string at +8
    Boolean = 2, // 006b80c0 -> lua_pushboolean, byte at +8
    EntityId = 3, // thisTable[format("%d", short at +8)], or nil when zero
    Skipped = 4, // pushes nothing and consumes no stack slot
    Table = 5, // 006b84b0 -> lua_createtable, then the container is walked
    Nil = 6, // 006b8130 -> lua_pushnil
};

struct MissionLuaArgument {
    MissionLuaArgumentType type{MissionLuaArgumentType::Nil};
    double number{0.0};
    std::string text;
    bool boolean{false};
    std::int16_t entity_id{0};
    std::vector<MissionLuaArgument> elements; // Table members, in order.
};

// Format string 00ce3a34 that tag 3 uses to build the thisTable key.
inline constexpr const char* kMissionLuaEntityKeyFormat = "%d";

// A tag-4 record pushes nothing, so it does not count towards nargs. Every
// other tag contributes exactly one value.
int mission_lua_pushed_value_count(const std::vector<MissionLuaArgument>& arguments) noexcept;

// ---------------------------------------------------------------------------
// Dotted entry-point names. 00887750 splits the name on '.' with 00bd20a0,
// which drops empty segments, resolves the first segment with
// lua_getfield(L, LUA_GLOBALSINDEX, seg) and every later one with
// lua_pushstring / lua_gettable(-2) / lua_remove(-2).
// ---------------------------------------------------------------------------

std::vector<std::string> split_lua_entry_point_name(const std::string& name);

// ---------------------------------------------------------------------------
// The integration seam. Each method is one native call site. Nothing has a
// default body: none of these stand in for unrecovered game behaviour.
// ---------------------------------------------------------------------------

struct MissionLuaHostServices {
    virtual ~MissionLuaHostServices() = default;

    // -- state bring-up, 006b8740 and 00884be0 --------------------------------
    virtual void create_state() = 0; // luaL_newstate 00a6a260
    virtual void set_panic_function(std::uint32_t function) = 0; // lua_atpanic 00a67390
    virtual void set_gc_pause(int what, int pause) = 0; // lua_gc 00a68280
    // lua_pushcclosure(L, luaopen_x, 0), lua_pushstring(L, name), lua_call(L, 1, 0).
    virtual void open_standard_library(const LuaStandardLibrary& library) = 0;
    // 006b8610: lua_pushcclosure(L, fn, 0) then lua_setfield(L, -10002, name).
    virtual void register_global_function(const MissionLuaBinding& binding) = 0;

    // -- virtual file system, 00885110 ---------------------------------------
    // [0109ceec] virtual +4h with mode 2. Returns false when the stream's
    // virtual +18h reports the file is not open.
    virtual bool open_script(const std::string& path) = 0;
    virtual int script_size() = 0; // stream virtual +30h
    virtual void read_script(char* buffer, int size) = 0; // stream virtual +24h
    virtual void close_script() = 0; // refcount at stream+4, then virtual +0h
    // 00bdef90 on the virtual file system singleton: the content-variant names
    // of a path. 00885fb0 runs every one of them after the base file.
    virtual std::vector<std::string> script_variant_names(const std::string& path) = 0;

    // -- chunk execution, 006b89f0 -------------------------------------------
    virtual int lua_gettop() = 0; // 006b7e70 -> 00a673d0
    virtual void lua_settop(int index) = 0; // 006b7e80 -> 00a673e0
    // 00a6a160. Returns the Lua status code; zero is success.
    virtual int luaL_loadbuffer(const char* buffer, int size, const char* chunk_name) = 0;
    // 00a680e0. 006b8530 forwards nargs in EDX and the rest on the stack.
    virtual int lua_pcall(int nargs, int nresults, int errfunc_index) = 0;
    // 00a67810 with index -1 and a null length pointer.
    virtual std::string lua_tolstring_at_top() = 0;
    // 00887220: pop the top values into the caller's result vector. mode is the
    // literal the native passes, 2 for chunks and 4 for named calls.
    virtual int collect_results(int count, int mode) = 0;

    // -- named dispatch, 00887750 --------------------------------------------
    virtual void lua_getglobal(const char* name) = 0; // 006b8460 -> lua_getfield
    virtual void lua_pushstring(const char* text) = 0; // 006b8120 -> 00a67a50
    virtual void lua_gettable(int index) = 0; // 006b8470 -> 00a67c20
    virtual void lua_remove(int index) = 0; // 006b7ea0 -> 00a67430
    virtual void lua_pushvalue(int index) = 0; // 006b7e90 -> 00a67570
    // One 00885da0 record. The implementation chooses the push by tag.
    virtual void push_argument(const MissionLuaArgument& argument) = 0;
    // 00b67980 then 00b67800 then 00b66200: does the global exist and is it
    // not nil. 0045f440 and 0045f520 skip the call entirely when it is false.
    virtual bool global_is_defined(const char* name) = 0;
    // game+1FE4h. Value 2 is the state 00887750 and 00887e50 refuse to run in.
    virtual int game_lifecycle_state() = 0;
    // game+1A18h, adjusted by the current stack top across the pcall at
    // 0088797d and 00887993. Purpose not established.
    virtual void adjust_call_stack_marker(int delta) = 0;
    // 00f87900, incremented at 00887844 and decremented at 008879c4.
    virtual void adjust_reentrancy_depth(int delta) = 0;
    // 004c1130 then the pool's virtual +10h. When true, 00887e50 queues the
    // call through 00887c30 instead of running it on this thread.
    virtual bool on_frame_job_thread() = 0;
    virtual void queue_named_call_for_main_thread(const std::string& name) = 0;
};

// ---------------------------------------------------------------------------
// Recovered sequences.
// ---------------------------------------------------------------------------

// 00884be0, __fastcall(this), RET. Creates the state, opens the seven standard
// libraries, runs "PC=true", installs every row of the registration table and
// finally runs the embedded fundamentals chunk. Returns how many globals were
// installed.
std::size_t initialise_mission_lua_host(MissionLuaHostServices& host);

// 006b89f0, __thiscall, RET 18h. this is the Lua machine, L is at +4h.
// capture_error selects the native's arg5; capture_results selects arg4.
LuaChunkResult run_lua_chunk(MissionLuaHostServices& host, const char* buffer, int size,
    const char* chunk_name, bool capture_error, bool capture_results, int result_mode);

// 00885110, __thiscall(const char* path), RET 4. Opens the path through the
// virtual file system with mode 2, reads the whole file into a heap buffer,
// runs it with the path as the chunk name, frees the buffer and releases the
// stream. A file that does not open is a silent no-op. The error string the
// chunk produces is allocated and released without being read.
LuaChunkResult run_script_file(MissionLuaHostServices& host, const std::string& path);

// 00885fb0, __thiscall(const char* path, char run_variants), RET 8. Runs the
// base file, then, when the flag is set, every content variant the virtual file
// system reports for that path. Returns one result per file actually run,
// base first.
std::vector<LuaChunkResult> run_script_with_variants(
    MissionLuaHostServices& host, const std::string& path, bool run_variants);

// 008860b0, __thiscall(const NativeString* scene_name), RET 4. Builds the
// mission script path and calls 00885fb0 with the flag set.
std::vector<LuaChunkResult> run_mission_script(
    MissionLuaHostServices& host, const std::string& scene_name);

struct NamedCallRequest {
    std::string self_key; // arg1, empty when the native pointer is null or the string is empty
    std::string name; // arg2, the dotted entry-point name
    std::vector<MissionLuaArgument> arguments; // arg3, may be empty
    int forward_stack_first{0}; // arg4, 0 disables forwarding
    int forward_stack_last{-1}; // arg5, negative values are relative to the top
    bool collect_results{false}; // arg6 in 00887750; 00887e50 has no such argument
    bool run_during_shutdown{false}; // arg7 in 00887750; 00887b30 passes 1
};

struct NamedCallOutcome {
    bool dispatched{false}; // false when the shutdown or lifecycle gate refused
    bool queued{false}; // 00887e50 handed the call to the main thread instead
    int arguments_pushed{0}; // the nargs 006b8530 received
    int results{0}; // lua_gettop delta after the pcall
    int pcall_status{0}; // the value 00a680e0 returned
    std::vector<std::string> resolved_path; // the split name, first segment first
};

// 00887750, __thiscall, RET 1Ch. Pushes debugtrap as the error handler,
// resolves the dotted name from the globals, optionally pushes
// thisTable[self_key] as the first argument, pushes the argument records and
// the forwarded stack range, then calls with LUA_MULTRET and the error handler
// index. Restores the stack top on every path.
NamedCallOutcome call_named_entry_point(MissionLuaHostServices& host, const NamedCallRequest& request);

// 00887b30, __thiscall, RET 14h: 00887750 with collect_results false and
// run_during_shutdown true.
NamedCallOutcome call_named_entry_point_forced(
    MissionLuaHostServices& host, const std::string& name, std::vector<MissionLuaArgument> arguments);

// 00887e50, __thiscall, RET 14h. Refuses outright when the lifecycle state is
// 2, queues the call when the frame job pool says this is a worker thread, and
// otherwise runs the same body as 00887750 under the virtual file system lock.
NamedCallOutcome call_named_entry_point_threadsafe(
    MissionLuaHostServices& host, const std::string& name, std::vector<MissionLuaArgument> arguments);

// 0045f440 and 0045f520, both __thiscall(const char* name, args), RET 8. They
// test the global first and call nothing when it is missing; 0045f440 goes
// through 00887e50 and 0045f520 through 00887b30.
NamedCallOutcome call_entry_point_if_defined(MissionLuaHostServices& host, const std::string& name,
    std::vector<MissionLuaArgument> arguments, bool threadsafe);

// The four names the mission scene load invokes, and which wrapper each uses.
// 004dfb70 raises the game+644h depth around the whole group.
struct MissionLuaEntryPoint {
    const char* name;
    bool threadsafe; // true selects 0045f440, false selects 0045f520
};
inline constexpr MissionLuaEntryPoint kMissionLuaEntryPoints[] = {
    {"luaPrecacheUnits", false}, // 0045f520 at 004e0a96 (docs/GAME_EXECUTABLE.md, milestone 2f correction 2: xrefs give 004e0a96 and 004e0c2e; 004e0aa5 is inside the 0095ca70 precache block)
    {"luaStageInitMulti", false}, // 0045f520
    {"luaStageInit", true}, // 0045f440
    {"luaEngineMovieInit", true}, // 0045f440, slot 9 only
};
inline constexpr std::size_t kMissionLuaEntryPointCount = 4;

} // namespace bsp
