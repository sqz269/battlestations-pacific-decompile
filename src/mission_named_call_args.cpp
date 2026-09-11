// Reconstruction of the mission Lua named call's argument marshalling, result
// collection and cross-thread queue. Evidence in docs/MISSION_NAMED_CALL_ARGS.md.
// Natives: 00887750, 00887B30, 00887E50, 00887220, 00886E00, 008874C0.
// Ghidra was read-only for this packet. Not ABI compatible, not game validated.
#include "bsp/mission_named_call_args.hpp"

#include <algorithm>
#include <utility>

namespace bsp {
namespace {

// game+1FE4h. 00887782 and 00887E73 both compare against this literal and refuse
// the call when it matches. The state's meaning is not established here.
constexpr int kMissionLuaRefusedLifecycleState = 2;

// LUA_MULTRET, pushed at 00887986.
constexpr int kNamedCallResultsAll = -1;

// The self key lookup at 008878D4..0088790A and the dotted-name walk at
// 0088787F..008878C5 both index their parent with -2 and then drop it.
constexpr int kParentTableIndex = -2;

bool is_skipped_record(const MissionLuaArgument& argument) noexcept
{
    // Tag 4 has no case in 00885DA0, so the record consumes no stack slot even
    // though 0088793C counted it.
    return argument.type == MissionLuaArgumentType::Skipped;
}

// Steps 3 to 13 of 00887750, shared with the inline path of 00887E50.
MissionNamedCallTrace run_named_call_body(
    MissionNamedCallHost& host, const NamedCallRequest& request, bool collect_results)
{
    MissionNamedCallTrace trace;

    const int saved_top = host.lua_gettop(); // 008877E3
    const MissionLuaStackRange range = resolve_named_call_stack_range(
        request.forward_stack_first, request.forward_stack_last, host.lua_gettop());

    // 0088782A: the error handler is pushed before anything else, so its index
    // survives the argument pushes. The native parks it in the force slot.
    host.lua_getglobal(kMissionLuaErrorHandler);
    trace.errfunc_index = host.lua_gettop();

    host.adjust_reentrancy_depth(1); // 00887844

    trace.resolved_path = split_lua_entry_point_name(request.name); // 0088786A
    if (!trace.resolved_path.empty()) {
        host.lua_getglobal(trace.resolved_path.front().c_str()); // 00887883
        for (std::size_t i = 1; i < trace.resolved_path.size(); ++i) {
            host.lua_pushstring(trace.resolved_path[i].c_str()); // 008878A5
            host.lua_gettable(kParentTableIndex); // 008878AF
            host.lua_remove(kParentTableIndex); // 008878B9
        }
    } else {
        // 0088787A substitutes the empty literal 00F87904 when a segment has no
        // data pointer; a name that splits to nothing still does one lookup.
        host.lua_getglobal("");
    }

    trace.self_key_used = named_call_self_key_used(request.self_key);
    if (trace.self_key_used) {
        host.lua_getglobal(kMissionLuaSelfTable); // 008878E1
        host.lua_pushstring(request.self_key.c_str()); // 008878F6
        host.lua_gettable(kParentTableIndex); // 00887900
        host.lua_remove(kParentTableIndex); // 0088790A
    }

    for (const MissionLuaArgument& argument : request.arguments) {
        host.push_argument(argument); // 0088792C, stride 14h
    }
    for (int index = range.first; range.active && index <= range.last; ++index) {
        host.lua_pushvalue(index); // 0088795B
    }

    trace.counts = named_call_argument_count(trace.self_key_used, request.arguments, range);

    const int marker = host.lua_gettop(); // 0088796C
    host.adjust_call_stack_marker(marker); // 0088797D
    trace.pcall_status = host.lua_pcall(trace.counts.declared, kNamedCallResultsAll, trace.errfunc_index);
    host.adjust_call_stack_marker(-marker); // 00887993

    trace.result_count = host.lua_gettop() - saved_top; // 008879A9
    if (collect_results && trace.result_count != 0) {
        trace.results = host.collect_results(trace.result_count, kMissionLuaResultDepthNamedCall);
    }

    host.lua_settop(saved_top); // 008879BF
    host.adjust_reentrancy_depth(-1); // 008879C4
    return trace;
}

} // namespace

int normalise_lua_stack_index(int index, int top) noexcept
{
    // 00887808 LEA EAX,[EAX+EBP+1] and 00887822 LEA ECX,[EDI+EAX+1].
    return index < 0 ? top + index + 1 : index;
}

MissionLuaStackRange resolve_named_call_stack_range(int stack_first, int stack_last, int top) noexcept
{
    MissionLuaStackRange range;
    if (stack_first == 0) {
        // 008877F2 and 00887945: zero is the sentinel, and stack_last is then
        // neither normalised nor read.
        range.last = stack_last;
        return range;
    }
    range.active = true;
    range.first = normalise_lua_stack_index(stack_first, top);
    range.last = normalise_lua_stack_index(stack_last, top);
    range.count = range.last >= range.first ? range.last - range.first + 1 : 0; // 0088794B
    return range;
}

bool named_call_self_key_used(const std::string& self_key) noexcept
{
    // 008878CB TEST EDI,EDI then 008878CF CMP dword ptr [EDI],0: a null pointer
    // and an empty string are the same thing here.
    return !self_key.empty();
}

NamedCallArgumentCounts named_call_argument_count(bool self_key_used,
    const std::vector<MissionLuaArgument>& arguments, const MissionLuaStackRange& range) noexcept
{
    NamedCallArgumentCounts counts;
    // EBX is cleared at 00887780 and only the self-key block sets it to 1
    // (008878DC), so without a self key the count starts at zero.
    const int self = self_key_used ? 1 : 0;
    counts.declared = self;
    counts.pushed = self;

    counts.declared += static_cast<int>(arguments.size()); // 0088793C
    for (const MissionLuaArgument& argument : arguments) {
        if (!is_skipped_record(argument)) {
            ++counts.pushed;
        }
    }

    counts.declared += range.count; // 00887953
    counts.pushed += range.count;
    return counts;
}

MissionLuaResultVariant build_named_call_result_variant(MissionNamedCallResultHost& host,
    int stack_index, int depth, int max_depth)
{
    MissionLuaResultVariant result;
    if (depth >= max_depth) {
        // 00886E1A: the guard is the whole body, so the record keeps the tag -1
        // that 00887220 constructed it with.
        return result;
    }

    result.filled = true;
    switch (host.lua_type(stack_index)) { // 006B7EC0 at 00886E1E
    case MissionLuaValueType::Nil:
    case MissionLuaValueType::Userdata:
        // Both land on the same arm and set tag 6.
        result.value.type = MissionLuaArgumentType::Nil;
        break;
    case MissionLuaValueType::Boolean:
        // 008869F0 converts with CVTSI2SS and writes tag 0, so a boolean result
        // is stored as the number 0.0 or 1.0 and cannot be told from one.
        result.value.type = MissionLuaArgumentType::Number;
        result.value.number = host.lua_toboolean(stack_index) ? 1.0 : 0.0;
        break;
    case MissionLuaValueType::LightUserdata:
        // 00886B30 writes tag 4, the one tag 00885DA0 cannot push back.
        result.value.type = MissionLuaArgumentType::Skipped;
        result.value.number = static_cast<double>(host.lua_touserdata(stack_index));
        break;
    case MissionLuaValueType::Number:
        result.value.type = MissionLuaArgumentType::Number;
        result.value.number = host.lua_tonumber(stack_index);
        break;
    case MissionLuaValueType::String:
        result.value.type = MissionLuaArgumentType::String;
        result.value.text = host.lua_tostring(stack_index);
        break;
    case MissionLuaValueType::Function:
        // 0088708E pushes the literal 00D0E78C into the same string setter.
        result.value.type = MissionLuaArgumentType::String;
        result.value.text = kMissionLuaFunctionResultText;
        break;
    case MissionLuaValueType::Table: {
        result.value.type = MissionLuaArgumentType::Table;
        const std::vector<MissionLuaTableEntry> entries =
            build_named_call_table_entries(host, stack_index, depth, max_depth);
        for (const MissionLuaTableEntry& entry : entries) {
            // MissionLuaArgument keeps members without their keys; the keys stay
            // in the entry list the caller can ask for separately.
            result.value.elements.push_back(entry.value.value);
        }
        break;
    }
    default:
        // LUA_TNONE and threads reach the assertion at 006B86B0 and never
        // produce a record.
        result.filled = false;
        break;
    }
    return result;
}

std::vector<MissionLuaTableEntry> build_named_call_table_entries(
    MissionNamedCallResultHost& host, int stack_index, int depth, int max_depth)
{
    std::vector<MissionLuaTableEntry> entries;
    host.lua_pushnil(); // 00886F1B
    while (host.lua_next(stack_index)) { // 0088702A
        MissionLuaTableEntry entry;
        if (host.key_is_string(kParentTableIndex)) { // 00886F44
            entry.key_text = host.lua_tostring(kParentTableIndex);
        } else {
            entry.key_is_number = true;
            entry.key_number = host.lua_tonumber(kParentTableIndex);
        }
        // 00886F8A reads the value's index with lua_gettop, then recurses one
        // level deeper with the same limit.
        entry.value = build_named_call_result_variant(host, host.lua_gettop(), depth + 1, max_depth);
        host.lua_pop(1); // 00886FA1
        entries.push_back(std::move(entry));
    }
    return entries;
}

std::vector<MissionLuaResultVariant> collect_named_call_results(
    MissionNamedCallResultHost& host, int count, int max_depth)
{
    std::vector<MissionLuaResultVariant> results;
    const int top = host.lua_gettop(); // 00887262
    const int first = count < 1 ? 1 : top - count + 1; // 0088726B..0088727C
    for (int index = first; index <= top; ++index) {
        results.push_back(build_named_call_result_variant(host, index, 0, max_depth));
    }
    // 00887220 pops nothing; 008879BF restores the top instead.
    return results;
}

void attach_result_sink_machine(MissionLuaResultSink& sink)
{
    // 008877C9: the items are discarded only when the sink already carried a
    // machine. A fresh sink keeps whatever it holds, which is how a reused sink
    // can accumulate two calls' results.
    if (sink.has_machine) {
        sink.items.clear(); // 008877D5, resize to zero
    }
    sink.has_machine = true; // 008877DD
}

MissionLuaDeferredCall queue_named_call(MissionLuaCallQueue& queue, const NamedCallRequest& request)
{
    MissionLuaDeferredCall call;
    call.self_key = request.self_key; // node+8h, 008881A0
    call.name = request.name; // node+10h, 008881B7 and 008881CB
    call.arguments = request.arguments; // node+18h, deep copied by 00887560
    call.stack_first = request.forward_stack_first; // node+24h, 008881E4
    call.stack_last = request.forward_stack_last; // node+28h, 008881DB
    queue.entries.push_back(call); // push_back, and the drain pops the front
    return call;
}

MissionNamedCallTrace run_named_call(MissionNamedCallHost& host, const NamedCallRequest& request)
{
    MissionNamedCallTrace trace;
    const int saved_block_depth = host.vfs_block_depth(); // 00887771
    if (!request.run_during_shutdown
        && host.game_lifecycle_state() == kMissionLuaRefusedLifecycleState) {
        // 008877A7 is the one path on which 00887750 releases its own scope
        // object; the normal path leaves it to the caller's frame.
        trace.refused = true;
        if (host.vfs_block_depth() > saved_block_depth) {
            host.vfs_leave_file_block();
            trace.vfs_block_unwound = true;
        }
        return trace;
    }
    return run_named_call_body(host, request, request.collect_results);
}

MissionNamedCallTrace run_named_call_forced(MissionNamedCallHost& host, const NamedCallRequest& request)
{
    // 00887B30 rebuilds the call with results = 0 and force = 1. Nothing else
    // about the dispatch changes.
    NamedCallRequest forced = request;
    forced.collect_results = false;
    forced.run_during_shutdown = true;
    return run_named_call(host, forced);
}

MissionNamedCallTrace run_named_call_threadsafe(MissionNamedCallHost& host, const NamedCallRequest& request)
{
    MissionNamedCallTrace trace;
    if (host.game_lifecycle_state() == kMissionLuaRefusedLifecycleState) { // 00887E73
        trace.refused = true;
        return trace;
    }
    if (host.on_frame_job_thread()) { // 00887E84 then the pool's virtual +10h
        MissionLuaDeferredCall call;
        call.self_key = request.self_key;
        call.name = request.name;
        call.arguments = request.arguments;
        call.stack_first = request.forward_stack_first;
        call.stack_last = request.forward_stack_last;
        host.queue_call(call);
        trace.queued = true;
        return trace;
    }

    const int saved_block_depth = host.vfs_block_depth(); // 00887F0B
    if (host.game_lifecycle_state() == kMissionLuaRefusedLifecycleState) { // 00887EC8, retested
        trace.refused = true;
    } else {
        // 00887E50 never passes a sink and never forces.
        NamedCallRequest inline_request = request;
        inline_request.collect_results = false;
        inline_request.run_during_shutdown = false;
        trace = run_named_call_body(host, inline_request, false);
    }
    if (host.vfs_block_depth() > saved_block_depth) {
        // 008880E7 -> 00885350: the scope repairs a virtual file system block
        // the script opened and did not close. It is not a mutex.
        host.vfs_leave_file_block();
        trace.vfs_block_unwound = true;
    }
    return trace;
}

std::vector<MissionNamedCallTrace> drain_named_call_queue(
    MissionNamedCallHost& host, MissionLuaCallQueue& queue)
{
    std::vector<MissionNamedCallTrace> traces;
    // 00888230 re-reads the size every iteration and pops the front, so a call
    // that queues another one during the drain is picked up in the same pass.
    while (!queue.entries.empty()) {
        const MissionLuaDeferredCall call = queue.entries.front();
        queue.entries.erase(queue.entries.begin());

        NamedCallRequest request;
        request.self_key = call.self_key;
        request.name = call.name;
        request.arguments = call.arguments;
        // Replayed verbatim: the indices were normalised against the queueing
        // thread's stack, not this one's.
        request.forward_stack_first = call.stack_first;
        request.forward_stack_last = call.stack_last;
        traces.push_back(run_named_call_threadsafe(host, request));
    }
    return traces;
}
}
