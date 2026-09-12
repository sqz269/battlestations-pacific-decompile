// The mission Lua bindings that decide usn_2_java.lua's flow.
//
// See include/bsp/lua_binding_mission.hpp for the address list, the evidence per
// constant and what this packet read rather than assumed. Packet
// `cc_lua_binding_audit`; Ghidra was read-only.

#include "bsp/lua_binding_mission.hpp"

#include "bsp/entity_think_dispatch.hpp"

#include <algorithm>

namespace bsp {

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

float unit_health_value_00923be0(bool gate_5d, float raw, float ceiling,
    float& cache) noexcept {
    // 00923BE4: the gate returns 0.0f without touching the cache slot.
    if (gate_5d) return 0.0f;
    // 00923C0C: a negative answer writes an integer zero into the cache and
    // returns zero. The native's store is `param_1[0x59] = 0`, an int store of the
    // literal, which for IEEE-754 is the same bit pattern as +0.0f.
    if (raw < 0.0f) {
        cache = 0.0f;
        return 0.0f;
    }
    // 00923C1E: COMISS against 00D7A24C, the larger loses.
    const float clamped = std::min(raw, ceiling);
    // 00923C16/00923C2C: the native stores `(int)value` here, so the cache slot is
    // the truncated health and not the float. The returned value is the float.
    cache = clamped;
    return clamped;
}

RandomBindingRange random_binding_range_0088c160(int argument_count, int argument0,
    int argument1, bool& pushes) noexcept {
    RandomBindingRange range;
    pushes = true;
    if (argument_count == 0) {
        // 0088C360: FLD [00D11318] into the upper slot, FLDZ into the lower.
        range.minimum = 0.0f;
        range.maximum = kRandomDefaultCeiling;
        return range;
    }
    if (argument_count == 1) {
        // 0088C301: FLDZ lower, (float)(arg0 + 1) upper.
        range.minimum = 0.0f;
        range.maximum = static_cast<float>(argument0 + 1);
        return range;
    }
    if (argument_count == 2) {
        // 0088C269: argument 1 read first, +1, as the upper; argument 0 as the
        // lower. Both through 00B66290, which truncates toward zero.
        range.minimum = static_cast<float>(argument0);
        range.maximum = static_cast<float>(argument1 + 1);
        return range;
    }
    // 0088C263 JNZ 0088C38B: three or more arguments fall straight through to the
    // result count with nothing pushed.
    pushes = false;
    return range;
}

int create_script_stack_first_00898945(int argument_count) noexcept {
    // 00898945 CMP EAX,0x1 / JLE leaves EDI zero; 0089894A sets 2.
    return argument_count > 1 ? kCreateScriptStackForwardFirst
                              : kCreateScriptStackForwardNone;
}

// ---------------------------------------------------------------------------
// The eight bodies
// ---------------------------------------------------------------------------

int lua_binding_get_hp_percentage(LuaBindingArgumentReader& args,
    LuaBindingMissionHost& host) {
    // 0088EAC0 / 00B677E0 then 0088EACF / 00888AA0.
    void* entity = args.entity_at(0);
    float health = 0.0f;
    if (entity != nullptr) {
        // 0088EAE8 / 00923BE0, whose three reads the host splits.
        const bool gate = host.unit_health_gate_5d_00923be4(entity);
        const float raw = gate ? 0.0f : host.unit_health_vtable_110_00923bf6(entity);
        float cache = 0.0f;
        health = unit_health_value_00923be0(gate, raw, kUnitHealthCeiling, cache);
        if (!gate) host.unit_health_cache_store_00923c16(entity, cache);
    }
    // 0088EAF5 / 00B66480, then the result count at 0088EAFE.
    host.push_number_float_00b66480(health);
    return 1;
}

int lua_binding_get_position(LuaBindingArgumentReader& args,
    LuaBindingMissionHost& host) {
    // 008A7BFF / 00888AA0.
    void* entity = args.entity_at(0);
    float xyz[3] = {0.0f, 0.0f, 0.0f};
    if (entity != nullptr) {
        // 008A7C24 CMP byte ptr [ESI+0C8h],0 / JNZ past the refresh: the refresh
        // runs only while the flag is clear, which is what 00414DB0 tests again.
        if (host.entity_pose_stale_008a7c24(entity)) {
            host.entity_pose_refresh_00414db0(entity);
        }
        // 008A7C3C LEA EDX,[ESI+0FCh], the world matrix translation row.
        host.entity_pose_translation_008a7c3c(entity, xyz);
    }
    // 008A7C1F / 00B67930 created the table before the refresh; the fill at
    // 008A7C46 / 0088BA30 writes x, y and z into it. Order does not matter to the
    // observable result, so the host is handed the finished vector once.
    host.push_vector3_table_0088ba30(xyz);
    return 1;
}

int lua_binding_get_measure(LuaBindingMissionHost& host) {
    // 0088D9BD CMP byte ptr [00F88988],0 / JZ to the kilometre arm.
    const bool imperial = host.measure_is_imperial_0088d9bd();
    host.push_global_path_value_00b672b0(
        imperial ? kMeasureImperialGlobalPath : kMeasureMetricGlobalPath);
    return 1;
}

int lua_binding_game_time(LuaBindingMissionHost& host) {
    // 008A93FE FLD float ptr [00F876A4], 008A9414 / 00B66480.
    host.push_number_float_00b66480(host.game_clock_seconds_008a93fe());
    return 1;
}

int lua_binding_random(LuaBindingArgumentReader& args, LuaBindingResultWriter& results,
    LuaBindingMissionHost& host) {
    // 0088C24A / 00B663F0, the frame's own argument count.
    const int argument_count = args.count();
    const int argument0 = argument_count >= 1 ? args.get_integer(0) : 0;
    const int argument1 = argument_count >= 2 ? args.get_integer(1) : 0;
    bool pushes = false;
    const RandomBindingRange range = random_binding_range_0088c160(argument_count,
        argument0, argument1, pushes);
    if (!pushes) return 0;
    const float sample = host.random_uniform_00bd2f10(range.minimum, range.maximum);
    // 0088C37C / 00BF7420, the CRT float-to-long that truncates toward zero. Both
    // bounds are non-negative in every shipped call, so this is also the floor.
    results.push_number(static_cast<int>(sample));
    return 1;
}

int lua_binding_create_script(LuaBindingArgumentReader& args,
    LuaBindingMissionHost& host) {
    // 00898841/0089884F/0089886F, then the four word writes and [+0C4h] = 3.
    void* entity = host.script_entity_create_00898841();
    if (entity != nullptr) {
        // 0089892C, the vtable +98h placement, and 00898932 / 00927610.
        host.script_entity_vcall_98_0089892c(entity);
        host.script_entity_call_00927610_00898932(entity);
    }
    // 00898940 / 00B65EB0 and the compare at 00898945.
    const int stack_first = create_script_stack_first_00898945(
        host.lua_stack_top_00b65eb0());
    // 00898959 / 00B677E0 then 00898969 / 00B662B0: the name is a borrowed string,
    // copied into the NativeString at 00898973 before the call.
    const std::string name = args.get_string(0);
    if (entity != nullptr) {
        host.entity_call_named_009290a0(entity, name, stack_first,
            kCreateScriptStackForwardLast);
        // 008989C7 / 004260B0, 008989F6 / 00B67910, 00898A0C / 00B678E0 and the
        // push at 00898A1B: thisTable[<id>] for the entity just created.
        if (host.push_self_table_slot_008989f6(entity)) return 1;
    }
    // The native always leaves the pushed value on the stack, so a host that
    // could not resolve the slot returns no result rather than a wrong one.
    return 0;
}

int lua_binding_set_wait(LuaBindingArgumentReader& args,
    LuaBindingMissionHost& host) {
    // 0089824F / 00888AA0 and 00898280 / 00B66270. The number is narrowed to
    // float32 by the FSTP at 00898285 before the compare.
    void* entity = args.entity_at(0);
    const float requested = static_cast<float>(args.get_number(1));
    // 00898289-008982A9, the same clamp bsp::clamp_think_delay carries.
    const float delay = clamp_think_delay(requested);
    if (entity != nullptr) host.entity_arm_think_delay_008982c9(entity, delay);
    return 0;
}

int lua_binding_clear_think(LuaBindingArgumentReader& args,
    LuaBindingMissionHost& host) {
    // 0089858F / 00888AA0, then the free/null/disarm at 008985A6-008985C3.
    void* entity = args.entity_at(0);
    if (entity != nullptr) host.entity_clear_think_name_008985a6(entity);
    return 0;
}

int lua_binding_delete_script(LuaBindingArgumentReader& args,
    LuaBindingMissionHost& host, bool* killed) {
    if (killed != nullptr) *killed = false;
    // 00898BC2 / 00888AA0.
    void* entity = args.entity_at(0);
    if (entity == nullptr) return 0;
    // 00898BD9 CMP byte ptr [ESI+5Eh],0 / JZ to the kill arm: a set byte takes the
    // 00898C00 arm, which returns the literal zero without calling the result
    // count. Both arms return zero results.
    if (host.entity_flag_5e_00898bd9(entity)) return 0;
    host.entity_kill_00926d90(entity, kDeleteScriptKillCause);
    if (killed != nullptr) *killed = true;
    return 0;
}

}  // namespace bsp
