#pragma once
// Arguments, result collection and cross-thread queueing of the mission Lua
// named call. docs/MISSION_NAMED_CALL_ARGS.md carries the evidence; the shape of
// the call itself lives in bsp/mission_lua_host.hpp and is not repeated here.
//
// Native subjects: 00887750 BSP_MissionLuaHost_CallNamed (RET 1Ch, seven stack
// arguments), 00887B30 the forced wrapper, 00887E50 the thread-safe wrapper,
// 00887220 the result collector, 00886E00 the variant builder, 008874C0 the
// unreached pending-results destructor. Ghidra was read-only for this packet and
// every name is a hypothesis, not a recovered symbol. Nothing here is a drop-in
// binary replacement.
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/mission_lua_host.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Argument slots. 00887750's seven stack arguments, read from [ESP+2Ch+4N] (and
// [ESP+34h+4N] after PUSH EBP/EDI). NamedCallRequest in bsp/mission_lua_host.hpp
// already models them as a request; this table is the evidence that each field
// is the slot it claims to be, kept so a reader does not have to re-derive it.
// ---------------------------------------------------------------------------

struct NamedCallArgumentSlot {
    int index; // 1..7, first stack argument is 1
    const char* name;
    const char* slot; // offset after the SEH prologue and PUSH EBX/ESI
    const char* first_read; // the address that reads it
    bool optional; // a call site is allowed to pass zero
};

inline constexpr NamedCallArgumentSlot kNamedCallArgumentSlots[] = {
    {1, "self_key", "[ESP+30h]", "008878C7", true},
    {2, "name", "[ESP+34h]", "0088785A", false},
    {3, "args", "[ESP+38h]", "0088790F", true},
    {4, "stack_first", "[ESP+3Ch]", "008877E8", true},
    {5, "stack_last", "[ESP+40h]", "00887812", true},
    {6, "results", "[ESP+44h]", "008877C0", true},
    {7, "force", "[ESP+48h]", "00887782", false},
};
inline constexpr std::size_t kNamedCallArgumentSlotCount = 7;

// ---------------------------------------------------------------------------
// Stack forwarding, 008877E8..0088782A and 00887943..00887967.
// ---------------------------------------------------------------------------

// 00887808 and 00887822: a negative index becomes top + index + 1, so -1 is the
// current top. A non-negative index is already absolute and is left alone.
int normalise_lua_stack_index(int index, int top) noexcept;

struct MissionLuaStackRange {
    bool active{false}; // stack_first was non-zero
    int first{0}; // normalised
    int last{-1}; // normalised, and only normalised when active
    int count{0}; // last - first + 1, never negative
};

// The native gates the whole block on stack_first != 0 (008877F2), which makes 0
// the sentinel rather than a valid index, and normalises stack_last only inside
// that gate. An inverted range (first > last, 0088794B) pushes nothing but still
// counts as active. The dead lua_gettop at 008877F4 is not modelled.
MissionLuaStackRange resolve_named_call_stack_range(int stack_first, int stack_last, int top) noexcept;

// ---------------------------------------------------------------------------
// The nargs the native hands to lua_pcall, EBX in 00887750.
// ---------------------------------------------------------------------------

struct NamedCallArgumentCounts {
    int declared{0}; // what the native passes as nargs
    int pushed{0}; // values actually on the stack
};

// declared: 1 for a used self key (008878DC, and zero otherwise: EBX starts at 0
// at 00887780, so the count does NOT start at 1 without a self key), plus the
// record count verbatim (0088793C), plus the forwarded range (00887953).
// pushed: the same, except that a Skipped record pushes nothing. The two differ
// exactly when the argument vector holds a Skipped record.
NamedCallArgumentCounts named_call_argument_count(bool self_key_used,
    const std::vector<MissionLuaArgument>& arguments, const MissionLuaStackRange& range) noexcept;

// 008878CD CMP dword ptr [EDI],0: the self key is used when the pointer is
// non-null and the NativeString size word is non-zero, so an empty key is the
// same as no key.
bool named_call_self_key_used(const std::string& self_key) noexcept;

// ---------------------------------------------------------------------------
// Result collection. 00887220's third argument is not a mode: 00886E00 guards
// its body with depth < max_depth and recurses with depth+1, so the literal is
// the maximum table-expansion depth.
// ---------------------------------------------------------------------------

inline constexpr int kMissionLuaResultDepthChunk = 2; // 006B89F0
inline constexpr int kMissionLuaResultDepthNamedCall = 4; // 008879B3 and 008874C7

// The literal 00D0E78C that a function result is replaced with at 0088708E.
inline constexpr const char* kMissionLuaFunctionResultText = "<Function>";

// A record 00886E00 refused to fill because the depth limit was reached keeps
// the tag 00887220 constructed it with, -1, which no Lua value maps to. The
// reconstruction carries it as a flag because MissionLuaArgumentType has no such
// enumerator and adding one would change a type this packet does not own.
struct MissionLuaResultVariant {
    bool filled{false}; // false is the native tag -1
    MissionLuaArgument value{}; // meaningful only when filled
};

// Lua type codes 006B7EC0 returns, as the switch at 00886E1E reads them.
enum class MissionLuaValueType {
    Nil = 0,
    Boolean = 1,
    LightUserdata = 2,
    Number = 3,
    String = 4,
    Table = 5,
    Function = 6,
    Userdata = 7,
};

// One key/value pair of a table result, built by the lua_next walk at
// 00886F0C..00887037. A key is a number or a string; nothing else reaches the
// appenders.
struct MissionLuaTableEntry {
    bool key_is_number{false};
    double key_number{0.0};
    std::string key_text;
    MissionLuaResultVariant value;
};

// The stack a result is read from. One virtual method per native call site of
// 00886E00 and 00887220; no method has a default body.
struct MissionNamedCallResultHost {
    virtual ~MissionNamedCallResultHost() = default;
    virtual int lua_gettop() = 0; // 006B7E70, 00887262 and 00886F8A
    virtual MissionLuaValueType lua_type(int index) = 0; // 006B7EC0 at 00886E1E
    virtual bool lua_toboolean(int index) = 0; // 006B8000 at 00886E40
    virtual std::uintptr_t lua_touserdata(int index) = 0; // 006B80A0 at 0088707A
    virtual double lua_tonumber(int index) = 0; // 006B8020 at 00886E5E
    virtual std::string lua_tostring(int index) = 0; // 006B8060 at 00886E7B
    virtual void lua_pushnil() = 0; // 006B8000-family push at 00886F1B
    virtual bool lua_next(int index) = 0; // 006B85B0 at 0088702A
    virtual void lua_pop(int count) = 0; // 006B85F0 -> lua_settop(-1-n) at 00886FA1
    // 006B7F60 at 00886F44: is the key at the given index a string. The native
    // reads the key as a number when this is false.
    virtual bool key_is_string(int index) = 0;
};

// 00886E00, __thiscall(ResultSink*, LuaVariant* out, int stack_index, int depth,
// int max_depth). Booleans arrive as numbers (008869F0 converts with CVTSI2SS),
// a function becomes kMissionLuaFunctionResultText, and full userdata collapses
// onto Nil exactly like nil does. LUA_TNONE and threads reach the assertion at
// 006B86B0 and are reported here as an unfilled record.
MissionLuaResultVariant build_named_call_result_variant(MissionNamedCallResultHost& host,
    int stack_index, int depth, int max_depth);

// The table members 00886E00 collects for a Table result, exposed separately
// because MissionLuaArgument::elements is a flat list and loses the keys.
std::vector<MissionLuaTableEntry> build_named_call_table_entries(MissionNamedCallResultHost& host,
    int stack_index, int depth, int max_depth);

// 00887220, __thiscall(ResultSink*, int count, int max_depth), RET 8. Reads the
// top count values, or the whole stack when count < 1 (0088726B), appends each
// through 006EDF00 and pops nothing: the caller restores the top.
std::vector<MissionLuaResultVariant> collect_named_call_results(
    MissionNamedCallResultHost& host, int count, int max_depth);

// Projection of the 14h-byte native sink: vtable at +0h, machine at +4h and a
// {begin, size, capacity} vector of 14h-byte variants at +8h. 00887750 clears
// the items only when the sink already carried a machine (008877D5), so a sink
// reused without one keeps the previous call's results.
struct MissionLuaResultSink {
    bool has_machine{false}; // +4h non-null
    std::vector<MissionLuaResultVariant> items; // +8h..+10h
};
void attach_result_sink_machine(MissionLuaResultSink& sink);

// ---------------------------------------------------------------------------
// The deferred call, 00888101..00888225 and the drain 00888230.
// ---------------------------------------------------------------------------

// The list node payload. node+8h self key, node+10h name, node+18h arguments,
// node+24h and node+28h the two stack indices, confirmed by the drain's
// 00887E50(node+8h, node+10h, node+18h, node[9], node[10]) at 00888264.
struct MissionLuaDeferredCall {
    std::string self_key;
    std::string name;
    std::vector<MissionLuaArgument> arguments; // deep copied at queue time by 00887560
    int stack_first{0};
    int stack_last{-1};
};

// this+8h is the list, this+Ch its head and this+10h its size. push_back under
// the critical section of the 004C1570 singleton, front-pop in the drain, so the
// queue is FIFO.
struct MissionLuaCallQueue {
    std::vector<MissionLuaDeferredCall> entries;
};

MissionLuaDeferredCall queue_named_call(MissionLuaCallQueue& queue, const NamedCallRequest& request);

// ---------------------------------------------------------------------------
// The dispatcher sequence. One virtual method per native call site of 00887750
// and 00887E50, in body order. No method has a default body: nothing here
// stands in for unrecovered game behaviour.
// ---------------------------------------------------------------------------

struct MissionNamedCallHost {
    virtual ~MissionNamedCallHost() = default;

    // -- gates -----------------------------------------------------------
    virtual int game_lifecycle_state() = 0; // game+1FE4h, 00887792 and 00887E73
    virtual bool on_frame_job_thread() = 0; // 004C1130 at 00887E84, then virtual +10h
    virtual int vfs_block_depth() = 0; // [0109CEEC]+14h at 00887771 and in 00885350
    virtual void vfs_leave_file_block() = 0; // 00BDC9B0 with the empty string 00CE3A0C

    // -- stack ------------------------------------------------------------
    virtual int lua_gettop() = 0; // 006B7E70
    virtual void lua_settop(int index) = 0; // 006B7E80 at 008879BF
    virtual void lua_getglobal(const char* name) = 0; // 006B8460
    virtual void lua_pushstring(const char* text) = 0; // 006B8120
    virtual void lua_gettable(int index) = 0; // 006B8470 with -2
    virtual void lua_remove(int index) = 0; // 006B7EA0 with -2
    virtual void lua_pushvalue(int index) = 0; // 006B7E90 at 0088795B
    virtual void push_argument(const MissionLuaArgument& argument) = 0; // 00885DA0
    virtual int lua_pcall(int nargs, int nresults, int errfunc_index) = 0; // 006B8530

    // -- counters ---------------------------------------------------------
    virtual void adjust_reentrancy_depth(int delta) = 0; // 00F87900
    virtual void adjust_call_stack_marker(int delta) = 0; // game+1A18h

    // -- results and queue -------------------------------------------------
    // 00887220 with the named-call depth limit 4 at 008879B3.
    virtual std::vector<MissionLuaResultVariant> collect_results(int count, int max_depth) = 0;
    // 00887C30 and the field writes at 008881A0..008881E4, under the 004C1570
    // critical section.
    virtual void queue_call(const MissionLuaDeferredCall& call) = 0;
};

// What one dispatch did, for a caller that cannot see the Lua stack.
struct MissionNamedCallTrace {
    bool refused{false}; // the lifecycle gate turned the call away
    bool queued{false}; // handed to the main thread instead
    bool self_key_used{false};
    std::vector<std::string> resolved_path; // the '.' split, first segment first
    NamedCallArgumentCounts counts{};
    int errfunc_index{0}; // the debugtrap index, which occupies the force slot
    int pcall_status{0};
    int result_count{0}; // lua_gettop delta across the pcall
    std::vector<MissionLuaResultVariant> results;
    bool vfs_block_unwound{false}; // 00885350 had to leave a file block
};

// 00887750, the whole body, with `force` and `collect_results` taken from the
// request. Restores the saved top on every path that reaches the pcall.
MissionNamedCallTrace run_named_call(MissionNamedCallHost& host, const NamedCallRequest& request);

// 00887B30: 00887750 with results disabled and force set. The only thing force
// buys is the 00887782 gate.
MissionNamedCallTrace run_named_call_forced(MissionNamedCallHost& host, const NamedCallRequest& request);

// 00887E50: refuse at lifecycle state 2, queue when the frame job pool says this
// is a worker thread, otherwise run the 00887750 body inside the virtual file
// system block scope. Never collects results and never forces.
MissionNamedCallTrace run_named_call_threadsafe(MissionNamedCallHost& host, const NamedCallRequest& request);

// 00888230: pop the front and dispatch it through 00887E50 until the size is
// zero. The two stack indices are replayed verbatim on the draining thread.
std::vector<MissionNamedCallTrace> drain_named_call_queue(
    MissionNamedCallHost& host, MissionLuaCallQueue& queue);
}
