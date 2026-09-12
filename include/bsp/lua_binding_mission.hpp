// The mission Lua bindings that decide usn_2_java.lua's flow.
//
// Packet `cc_lua_binding_audit`. Ghidra was read-only for this packet: no rename,
// comment, prototype or save was made. Every descriptive name here is a hypothesis,
// not a recovered symbol.
//
// docs/LUA_BINDING_MISSION.md audits every binding `Scripts/missions/usn/usn_2_java.lua`
// reaches and shows that the mission's whole control flow hangs on two groups:
//
//   the delayed-call scheduler  luaDelay -> CreateScript("luaDoTimeTable") ->
//   SetThink / SetWait / DeleteScript, re-entered by the think walk 00929460, and
//
//   four state queries the objective checker reads every pass: GetHpPercentage,
//   GetPosition (through luaGetDistance3D), GetMeasure and GameTime.
//
// The eight routines below are those two groups. The think walk itself is already
// reconstructed in `bsp/entity_think_dispatch.hpp` and is reused, not restated; the
// argument reader and result writer come from `bsp/lua_binding_core.hpp` for the same
// reason. `SetThink` (00897FB0) is `lua_binding_set_think` in that header.
//
// Shape shared with every other row of 00E0B7B8: `__fastcall(lua_State* in ECX)`
// returning the result count in EAX, with 00B66C00 / 00B679B0 around the body and
// 00B66400 / 00B669A0 after it. The per-binding `luakod` static-local log category is
// the compiler's guarded-initialisation pattern and has no per-call effect; it is not
// modelled, exactly as `bsp/lua_binding_core.hpp` says for its own ten.

#ifndef BSP_LUA_BINDING_MISSION_HPP
#define BSP_LUA_BINDING_MISSION_HPP

#include <cstddef>
#include <cstdint>
#include <string>

#include "bsp/lua_binding_core.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Constants recovered by this packet
// ---------------------------------------------------------------------------

// CreateScript 00898750. The script entity is its own allocation, not a scene
// instance: `00898834 PUSH 0x1E4` into 00BF55BE, zeroed by 00BF79F0 at 0089884F,
// then constructed by 00928630 at 0089886F.
inline constexpr std::size_t kScriptEntityAllocationSize = 0x1E4;  // 00898834

// `0089885D MOV EBX,0x3` then `00898892 MOV [ESI+0C4h],EBX`. EBX is not reused
// between those two sites (the listing's only other EBX writes are 00898771, the
// incoming lua_State, consumed by the PUSH at 0089880B).
inline constexpr std::size_t kScriptEntityKindFieldOffset = 0x0C4;  // 00898892
inline constexpr int kScriptEntityKindValue = 3;                    // 0089885D

// The four vtable-ish words the body writes before placing the entity, at
// 00898874 (+0h), 0089887A (+10h), 00898881 (+24h) and 00898888 (+170h). Their
// targets were not read, so they are carried as addresses and never as a verb.
inline constexpr std::uint32_t kScriptEntityVtableWord0 = 0x00D11138u;   // 00898874
inline constexpr std::uint32_t kScriptEntityVtableWord10 = 0x00D11120u;  // 0089887A
inline constexpr std::uint32_t kScriptEntityVtableWord24 = 0x00D11118u;  // 00898881
inline constexpr std::uint32_t kScriptEntityVtableWord170 = 0x00D11114u; // 00898888

// 00898940 asks the machine for the stack top and 00898945 compares it with 1:
// `JLE` leaves EDI at zero (0089893E XOR EDI,EDI), otherwise 0089894A sets 2. The
// value is the `stack_first` argument of the named call at 0089898B, whose
// `stack_last` is the literal -1 pushed at 00898978. So `CreateScript(name)` calls
// the global with no forwarded arguments and `CreateScript(name, a, b)` forwards
// Lua stack slots 2 upwards -- which is how `luaDelay`'s timer table and parameter
// table reach `luaDoTimeTable(this, timetable, paramTable)`.
inline constexpr int kCreateScriptStackForwardNone = 0;   // 0089893E
inline constexpr int kCreateScriptStackForwardFirst = 2;  // 0089894A
inline constexpr int kCreateScriptStackForwardLast = -1;  // 00898978

// DeleteScript 00898AC0. `00898BA3 MOV EBX,0x2` (EBX held the lua_State until the
// PUSH at 00898B7A) and `00898C04 PUSH EBX` ahead of 00926D90, whose second
// parameter docs/UNIT_DAMAGE_AND_DEATH.md names the kill cause.
inline constexpr int kDeleteScriptKillCause = 2;  // 00898BA3 / 00898C04

// GetHpPercentage 0088E9D0 -> 00923BE0 (already reconstructed, see that record).
// The ceiling the health is clamped to and the slot the clamped value is cached in.
inline constexpr float kUnitHealthCeiling = 1.0f;             // 00D7A24C, 00923C1E
inline constexpr std::size_t kUnitHealthCacheOffset = 0x164;  // 00923C16, param_1[0x59]
inline constexpr std::size_t kUnitHealthVtableSlot = 0x110;   // 00923BF6

// GetPosition 008A7B00. The three keys 0088BA30 writes into the fresh table, from
// 00CEB488, 00D045F8 and 00CFD718; the source is the world matrix translation row,
// `008A7C3C LEA EDX,[ESI+0FCh]`. The pose offsets themselves are already declared
// as kEntityPoseStaleOffset / kEntityPoseXOffset in bsp/mission_entity_lua_attach.hpp
// and are reused rather than restated.
inline constexpr const char* kPositionTableKeyX = "x";  // 00CEB488
inline constexpr const char* kPositionTableKeyY = "y";  // 00D045F8
inline constexpr const char* kPositionTableKeyZ = "z";  // 00CFD718

// GetMeasure 0088D8E0. `0088D9BD CMP byte ptr [00F88988],0x0`, `JZ` to the metric
// arm. Both arms push the *value at a dotted globals path*, through 00B672B0, so
// the binding answers the localised unit name the globals table carries, not a
// literal of its own.
inline constexpr const char* kMeasureMetricGlobalPath = "globals.kilometer";    // 0088D9DA arm
inline constexpr const char* kMeasureImperialGlobalPath = "globals.mile";       // 00CF5914
inline constexpr std::uint32_t kMeasureImperialFlagAddress = 0x00F88988u;       // 0088D9BD

// GameTime 008A9320. `008A93FE FLD float ptr [00F876A4]`, the mission clock the
// fixed-step driver writes at 0087464C.
inline constexpr std::uint32_t kGameClockSecondsAddress = 0x00F876A4u;  // 008A93FE

// random 0088C160. Three arms on the argument count (0088C251, 0088C25A,
// 0088C263), each one `trunc(00BD2F10(1, minimum, maximum))` pushed as a number.
// The ceiling of the no-argument arm is the float at 00D11318, 0x46FFFE00.
inline constexpr float kRandomDefaultCeiling = 32767.0f;  // 00D11318, 0088C360
inline constexpr int kRandomStreamSelector = 1;           // 0088C36D MOV ECX,1

// The pair of floats one arm of `random` hands 00BD2F10, in the stack order the
// body pushes them: [ESP] is the minimum, [ESP+4] the maximum. Both `n` arms add
// one to the upper argument (0088C321, 0088C2A4), so the result is inclusive.
struct RandomBindingRange {
    float minimum{0.0f};
    float maximum{0.0f};
};

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// 00923BE0's value rule, expressed over its inputs so a caller that has a health
// source can run it without the entity. `gate_5d` is the byte at +5Dh tested at
// 00923BE4; `raw` is what the virtual at +110h answered. Negative answers are
// floored to zero (00923C0C) and the ceiling is applied at 00923C1E; the result is
// written back to the cache slot in both arms (00923C16, 00923C2C).
float unit_health_value_00923be0(bool gate_5d, float raw, float ceiling,
    float& cache) noexcept;

// 00898940-0089894A. The `stack_first` CreateScript hands the named call.
int create_script_stack_first_00898945(int argument_count) noexcept;

// 0088C251-0088C2C7. Which bounds `random` asks for, given its argument count and
// the two integers it may have read. An argument count above two takes no arm at
// all (0088C263 JNZ straight to the epilogue), which the `pushes` flag reports.
RandomBindingRange random_binding_range_0088c160(int argument_count, int argument0,
    int argument1, bool& pushes) noexcept;

// ---------------------------------------------------------------------------
// The integration boundary
// ---------------------------------------------------------------------------

// One pure virtual per native call site of the eight bodies. The comment on each
// carries the call site inside the binding and the callee it reaches; a method
// named by slot is one whose callee body this packet did not read, and it never
// carries a verb.
struct LuaBindingMissionHost {
    virtual ~LuaBindingMissionHost() = default;

    // --- shared tail: the float push 00B66480 -----------------------------
    // 0088EAF5, 008A9414. The number push that takes a float32, distinct from
    // 00B664B0, which bsp::LuaBindingResultWriter::push_number already carries.
    virtual void push_number_float_00b66480(float value) = 0;

    // --- GetHpPercentage 0088E9D0 -----------------------------------------
    // 0088EAE8 / 00923BE0. Split into the three reads 00923BE0 makes so a host
    // that has only some of them can say which one it is missing.
    virtual bool unit_health_gate_5d_00923be4(void* entity) = 0;
    virtual float unit_health_vtable_110_00923bf6(void* entity) = 0;
    virtual void unit_health_cache_store_00923c16(void* entity, float value) = 0;

    // --- GetPosition 008A7B00 ---------------------------------------------
    virtual bool entity_pose_stale_008a7c24(void* entity) = 0;
    // 008A7C37 / 00414DB0, already reconstructed as BSP_EntityPose_RefreshWorld.
    virtual void entity_pose_refresh_00414db0(void* entity) = 0;
    // 008A7C3C, the world matrix translation row at entity+0FCh.
    virtual bool entity_pose_translation_008a7c3c(void* entity, float out[3]) = 0;
    // 008A7C1F / 00B67930 then 008A7C46 / 0088BA30: a fresh table with x, y and z.
    virtual void push_vector3_table_0088ba30(const float xyz[3]) = 0;

    // --- GetMeasure 0088D8E0 ----------------------------------------------
    virtual bool measure_is_imperial_0088d9bd() = 0;
    // 0088D9EA / 00B672B0: push the value the globals table holds at a dotted path.
    virtual void push_global_path_value_00b672b0(const char* dotted_path) = 0;

    // --- GameTime 008A9320 -------------------------------------------------
    virtual float game_clock_seconds_008a93fe() = 0;

    // --- random 0088C160 ----------------------------------------------------
    // 0088C377 / 0088C339 / 0088C2C7, all three with ECX = 1. 00BD2F10 is already
    // reconstructed as BSP_Random_UniformFloatRange; this is the call, not a
    // second copy of it.
    virtual float random_uniform_00bd2f10(float minimum, float maximum) = 0;

    // --- CreateScript 00898750 ---------------------------------------------
    // 00898841 / 00BF55BE, 0089884F / 00BF79F0 and 0089886F / 00928630, plus the
    // four word writes and [+0C4h] = 3. A null return is the native's 0089889A
    // arm, which continues with a null entity.
    virtual void* script_entity_create_00898841() = 0;
    // 0089892C, the virtual at the entity's vtable +98h, with the 16-float matrix
    // the body stages at 008988BA-00898914 and [game+19CCh]. The concrete vtable
    // was not resolved, so this is named by slot.
    virtual void script_entity_vcall_98_0089892c(void* entity) = 0;
    // 00898932 / 00927610, with DL = 0 (0089892E XOR DL,DL). Body not read.
    virtual void script_entity_call_00927610_00898932(void* entity) = 0;
    // 00898940 / 00B65EB0, the machine's stack top, which decides stack_first.
    virtual int lua_stack_top_00b65eb0() = 0;
    // 0089898B / 009290A0, the named call scoped by the entity's self key at
    // +178h (docs/MISSION_NAMED_CALL_ARGS.md). `args` is the null the site pushes.
    virtual void entity_call_named_009290a0(void* entity, const std::string& name,
        int stack_first, int stack_last) = 0;
    // 008989C7 / 004260B0 then 008989F6 / 00B67910 and the sets that follow: the
    // binding's return value is thisTable[<the entity's +174h id as decimal>].
    virtual bool push_self_table_slot_008989f6(void* entity) = 0;

    // --- SetWait 00898150 ---------------------------------------------------
    // 008982C9 MOVSS [ESI+1E0h] and 008982D1 MOV byte [ESI+1DCh],1, one step.
    virtual void entity_arm_think_delay_008982c9(void* entity, float seconds) = 0;

    // --- ClearThink 00898490 ------------------------------------------------
    // 008985A6 reads +1D8h, 008985B1 frees it when non-null, 008985B9 stores the
    // null (EBP, zero since 008984B3) and 008985C3 clears +1DCh.
    virtual void entity_clear_think_name_008985a6(void* entity) = 0;

    // --- DeleteScript 00898AC0 ----------------------------------------------
    virtual bool entity_flag_5e_00898bd9(void* entity) = 0;
    // 00898C07 / 00926D90, already reconstructed as BSP_MissionEntity_Kill.
    virtual void entity_kill_00926d90(void* entity, int cause) = 0;
};

// ---------------------------------------------------------------------------
// The eight bodies
// ---------------------------------------------------------------------------

// 0088E9D0. Reads the entity at argument 0 (0088EACF / 00888AA0), runs 00923BE0 over
// it and pushes the result as a float. Returns 1.
int lua_binding_get_hp_percentage(LuaBindingArgumentReader& args,
    LuaBindingMissionHost& host);

// 008A7B00. Entity at argument 0, a fresh table, the pose refresh when +0C8h is
// clear, then x/y/z from +0FCh. Returns 1. A null entity cannot happen in the
// native, which would fault; here it pushes the zero vector and says so through the
// translation reader's false return.
int lua_binding_get_position(LuaBindingArgumentReader& args,
    LuaBindingMissionHost& host);

// 0088D8E0. No argument. Pushes globals.mile or globals.kilometer. Returns 1.
int lua_binding_get_measure(LuaBindingMissionHost& host);

// 008A9320. No argument. Pushes the clock at 00F876A4. Returns 1.
int lua_binding_game_time(LuaBindingMissionHost& host);

// 0088C160. `random()`, `random(n)` or `random(a, b)`; the result is
// `trunc(uniform(minimum, maximum))` pushed through 00B664B0. Returns 1, or 0 when
// the argument count is above two, which takes no arm.
int lua_binding_random(LuaBindingArgumentReader& args, LuaBindingResultWriter& results,
    LuaBindingMissionHost& host);

// 00898750. Argument 0 is the global's name; arguments 1.. are forwarded to it by
// stack position. Creates the script entity, places it, calls the named global with
// the entity as its `this`, then pushes that entity's self table. Returns 1.
int lua_binding_create_script(LuaBindingArgumentReader& args,
    LuaBindingMissionHost& host);

// 00898150. Entity at 0, seconds at 1, clamped up to 0.5 by
// bsp::clamp_think_delay. Returns 0.
int lua_binding_set_wait(LuaBindingArgumentReader& args, LuaBindingMissionHost& host);

// 00898490. Entity at 0. Returns 0.
int lua_binding_clear_think(LuaBindingArgumentReader& args,
    LuaBindingMissionHost& host);

// 00898AC0. Entity at 0. Kills it unless +5Eh is already set, in which case the
// body returns without a result at 00898C00. Returns 0 either way; the two arms
// differ only in whether the kill happened, which the bool reports.
int lua_binding_delete_script(LuaBindingArgumentReader& args,
    LuaBindingMissionHost& host, bool* killed = nullptr);

}  // namespace bsp

#endif  // BSP_LUA_BINDING_MISSION_HPP
