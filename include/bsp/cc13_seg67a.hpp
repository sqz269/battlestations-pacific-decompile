#pragma once

// cc13_seg67a: the plane `closetoship` bot task of segment 67 (kind 0, 554h
// bytes; Make 009A2F40, constructor 009A2730, approach block 009A2500 at
// task+3F8h). Window 009A1B70-009A2FFF (packet cc13-seg67a, docs/CC13_SEG67A.md;
// grant: cc9 gameplay lead 2026-10-17, standalone, nothing wired).
//
// Every name is a hypothesis, not a recovered symbol. These are new C++
// interfaces over the native records, not drop-in replacements of the original
// ABI; each function's comment gives the native ABI. Native objects are raw
// byte blocks touched only at the offsets the image reads or writes, and every
// call the image makes goes through a named Cc13Seg67aHost method at the same
// point with the same arguments. Nothing here is wired into the game.
// Build-tested only.
//
// Offsets. "task+N" is the whole task. "approach+N" is relative to the
// approach block at task+3F8h: +4 unit, +8 aircraft descriptor, +Ch squadron
// control block (unit+9D4h), +18h pilot plan, +24h a speed ratio the base
// approach constructor 009F9CE0 writes, +2Ch the target observer element (its
// +14h, approach+40h, is the observed ship), +44h dropOrdnance (byte), +48h the
// stay-close altitude, +4Ch beginDist, +50h the state registry, +64h the moveto
// state (task+45Ch), +A0h the follow state (task+498h), +138h the stayclose
// state (task+530h). task+310h is the current state, task+550h dropStarted.
//
// Precision: the image keeps x87 intermediates in the register and rounds at
// each float store. Every chain here is one operation per store. The three
// float-times-qword products (009A1D9D *1500.0, 009A2234 *1.2000000476837158,
// 009A23BC *50.0) have qword constants that are float-representable, so the
// exact product fits in 53 bits and the store rounds once: the result is the
// same under the 24-bit and the 53-bit control word (the pilot-bot tick runs in
// unit jobs, either mode; docs/CC13_FPU_PRECISION.md). Sums of two floats are
// written as double sums rounded at the store, also mode-independent.
//
// RNG: 00BD2F10 BSP_Random_UniformFloatRange, ECX = 1 (stream 1, the shared
// process-wide generator), is drawn at 009A1F54 (stayclose constructor,
// U(1, 2)) and at 009A20FA (after an ordnance release, U(1, 2)).

#include <cstddef>
#include <cstdint>

namespace bsp {

template <class T>
inline T& cc13_seg67a_at(void* base, std::size_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + offset);
}

// The eight-byte {type, value} record the image builds on the stack for the
// archive slots +0Ch (write) and +10h (read). Types seen here: 0 name string,
// 2 float, 3 byte, 4 object reference. For a read the payload is the address of
// the destination; for a write it is the value (a byte write sets only the low
// byte in the image; the upper three bytes are stack residue, zero here).
struct Cc13Seg67aArchiveValue {
    std::int32_t type = 0;
    std::uintptr_t payload = 0;
};

inline constexpr std::uint32_t kCloseToShipTaskSize = 0x554u;           // 009A2F58 PUSH 554h
inline constexpr std::uint32_t kCloseToShipCommandClass = 0x00E08FA8u;  // 009A28B0
inline constexpr std::uint32_t kCloseToShipRobotsRow = 0x00F8A320u;     // 009A2800
inline constexpr std::uint32_t kCloseToShipTaskVtable = 0x00D1F4E8u;
inline constexpr std::uint32_t kCloseToShipApproachVtable = 0x00D1F4DCu;
inline constexpr std::uint32_t kCloseToShipRegistryVtable = 0x00D1F4D8u;
inline constexpr std::uint32_t kCloseToShipApproachBlockVtable = 0x00D1F4D0u;
inline constexpr std::uint32_t kCloseToShipApproachBlockRegistryVtable = 0x00D1F4CCu;
inline constexpr std::uint32_t kCloseToShipApproachBaseVtable = 0x00D1F470u;
inline constexpr std::uint32_t kCloseToShipStayCloseVtable = 0x00D1F4A0u;
inline constexpr std::uint32_t kPointObserverElementVtable = 0x00CF5C20u;
inline constexpr std::uint32_t kShipObserverElementVtable = 0x00CF5C94u;

class Cc13Seg67aHost {
public:
    virtual ~Cc13Seg67aHost() = default;

    // ---- CRT ----
    // 00BF681B operator new (cdecl, size).
    virtual void* operator_new_00bf681b(std::uint32_t size) = 0;

    // ---- observers ----
    // 00694A60 BSP_Observer_RegisterPair: __fastcall(ECX = observed, EDX = element).
    virtual void observer_register_pair_00694a60(void* observed, void* element) = 0;
    // 006952A0 BSP_Observer_UnregisterPair: __fastcall(ECX = observed, EDX = element).
    virtual void observer_unregister_pair_006952a0(void* observed, void* element) = 0;
    // 0064A610: __thiscall(ECX = element), the observer element destructor.
    virtual void observer_element_destruct_0064a610(void* element) = 0;

    // ---- geometry and math ----
    // 00414DB0 BSP_EntityPose_RefreshWorld: __thiscall(ECX = entity); called only
    // while entity+C8h is zero.
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    // 00414C60: __thiscall(ECX = float[2]), float in ST0.
    virtual float vector2_length_00414c60(const float* v) = 0;
    // 00414EB0 BSP_Geometry_HeadingAngle: __thiscall(ECX = float[2]), RET, ST0.
    virtual float heading_angle_00414eb0(const float* v) = 0;
    // 00438B10 BSP_Math_SubtractWrappedAngle: __stdcall(left, right), RET 8.
    virtual float wrapped_angle_subtract_00438b10(float left, float right) = 0;
    // 00419010 BSP_Math_InterpolateClamped: (x0, y0, x1, y1, x), float in ST0.
    virtual float interpolate_clamped_00419010(float x0, float y0, float x1, float y1,
                                               float x) = 0;
    // 004192E0 BSP_Vector2f_Normalize: __fastcall(ECX = out float[2], EDX = in
    // float[2]); returns the float[2] the caller reads (EAX).
    virtual const float* vector2_normalize_004192e0(float* out, const float* in) = 0;
    // 00BD2F10 BSP_Random_UniformFloatRange: __thiscall(ECX = stream, low, high).
    virtual float random_uniform_00bd2f10(std::uint32_t stream, float low, float high) = 0;

    // ---- units, planes, game tuning ----
    // unit->vtable[+50h](): the unit's heading, float in ST0.
    virtual float unit_vcall_50_heading(void* unit) = 0;
    // 007B8AD0: __thiscall(ECX = unit), AL = (unit+9D8h == 0), the flight leader.
    virtual bool unit_is_flight_leader_007b8ad0(void* unit) = 0;
    // 007BBBA0 BSP_Unit_RequestOrdnanceRelease: __thiscall(ECX = unit).
    virtual void unit_request_ordnance_release_007bbba0(void* unit) = 0;
    // 007B9140 BSP_Unit_CanDropOrdnance: __thiscall(ECX = unit, value), RET 4, AL.
    virtual bool unit_can_drop_ordnance_007b9140(void* unit, std::uint32_t value) = 0;
    // 007C1D80 BSP_Plane_ReloadBombPlatforms: __thiscall(ECX = unit).
    virtual void plane_reload_bomb_platforms_007c1d80(void* unit) = 0;
    // 007C47F0 BSP_PlaneClass_LevelFlightSpeed: __thiscall(ECX = descriptor), ST0.
    virtual float plane_class_level_flight_speed_007c47f0(void* descriptor) = 0;
    // 0042E740 BSP_GameTuning_GetSingleton: returns the tuning block (EAX).
    virtual void* game_tuning_singleton_0042e740() = 0;
    // 00521EA0: __thiscall(ECX = object), EAX; resolves the object the control
    // block's slot +178h hands back (compared against the target ship).
    virtual void* resolve_object_00521ea0(void* object) = 0;
    // ctl->vtable[+174h](): the command class (compared with 00E08FA8).
    virtual std::uint32_t control_vcall_174_command_class(void* ctl) = 0;
    // ctl->vtable[+178h](&scratch), RET 4: returns a pointer (EAX). `scratch` is
    // the caller's 14h-byte stack area at ESP+4 of 009A2920.
    virtual void* control_vcall_178(void* ctl, void* scratch) = 0;

    // ---- pilot-bot approach and states (other packets) ----
    // 009FB800 BSP_PilotBot_CommandPitchFromAltitude: __thiscall(ECX = state+4,
    // altitude, gain), RET 8.
    virtual void pilot_command_pitch_from_altitude_009fb800(void* state_approach_slot,
                                                            float altitude, float gain) = 0;
    // 009F9D90 BSP_BotApproach_SteerToPoint: __thiscall(ECX = state+4, float[2]
    // {x, z}), RET 4.
    virtual void bot_approach_steer_to_point_009f9d90(void* state_approach_slot,
                                                      const float* point) = 0;
    // 009F9CE0 BSP_BotApproach_ConstructSpeedReference: __thiscall(ECX = approach,
    // owner, speed), RET 8.
    virtual void bot_approach_construct_009f9ce0(void* approach, void* owner, float speed) = 0;
    // 009F9980: __thiscall(ECX = approach, task), RET 4.
    virtual void bot_approach_attach_task_009f9980(void* approach, void* task) = 0;
    // 00411E20: __thiscall(ECX = registry), the state-registry constructor.
    virtual void state_registry_construct_00411e20(void* registry) = 0;
    // 00411E70 BSP_BotStateRegistry_Add: __thiscall(ECX = registry, name, state), RET 8.
    virtual void state_registry_add_00411e70(void* registry, const char* name, void* state) = 0;
    // 00411170: __thiscall(ECX = registry, task+30Ch, archive), RET 8 (load).
    virtual void state_registry_load_00411170(void* registry, void* slot, void* archive) = 0;
    // 00411450: __thiscall(ECX = registry, task+30Ch, archive), RET 8 (save).
    virtual void state_registry_save_00411450(void* registry, void* slot, void* archive) = 0;
    // 004116D0: __thiscall(ECX = registry or null), the registry destructor.
    virtual void state_registry_destruct_004116d0(void* registry) = 0;
    // 009C2AC0: __thiscall(ECX = state, approach, target, a, b, c), RET 14h: the
    // moveto state constructor (excluded cc9 range; called only).
    virtual void bot_state_moveto_construct_009c2ac0(void* state, void* approach, void* target,
                                                     float a, float b, float c) = 0;
    // 009C2980: __thiscall(ECX = state, approach, value), RET 8: the follow state
    // constructor (excluded cc9 range; called only).
    virtual void bot_state_follow_construct_009c2980(void* state, void* approach,
                                                     float value) = 0;
    // 009BDE80 BSP_BotStateMoveTo_SetRanges: __thiscall(ECX = state, a, b, c), RET 0Ch.
    virtual void bot_state_moveto_set_ranges_009bde80(void* state, float a, float b,
                                                      float c) = 0;
    // 009BE3E0 BSP_BotStateFollow_WaitValue: __fastcall(ECX = follow state), ST0.
    virtual float bot_state_follow_wait_value_009be3e0(void* state) = 0;
    // 0099B720: the task base slot +4Ch body (FLD [00D7A260] = -1.0; RET).
    virtual float bot_task_base_wait_value_0099b720(void* task) = 0;
    // 007B45F0: __thiscall(ECX = state), the state base destructor.
    virtual void bot_state_base_destruct_007b45f0(void* state) = 0;
    // 006CDD70: __thiscall(ECX = member), the destructor of follow state+18h.
    virtual void member_destruct_006cdd70(void* member) = 0;
    // 007B65E0: __thiscall(ECX = state), the moveto state destructor.
    virtual void bot_state_moveto_destruct_007b65e0(void* state) = 0;
    // 0099C6F0 BSP_BotTask_ConstructBase: __thiscall(ECX = task, bot, kind), RET 8.
    virtual void bot_task_construct_base_0099c6f0(void* task, void* bot, std::uint32_t kind) = 0;
    // 0099C8B0 BSP_BotTask_ConstructBaseFromStream: __thiscall(ECX = task, bot,
    // archive, 0), RET 0Ch.
    virtual void bot_task_construct_base_from_stream_0099c8b0(void* task, void* bot,
                                                              void* archive,
                                                              std::uint32_t zero) = 0;
    // 0099D060 BSP_BotTask_SaveBase: __thiscall(ECX = task, archive), RET 4.
    virtual void bot_task_save_base_0099d060(void* task, void* archive) = 0;
    // 007B4030: __thiscall(ECX = task), the task base destructor.
    virtual void bot_task_destruct_base_007b4030(void* task) = 0;
    // 009A2810 BSP_BotTaskCloseToShip_AttackModeCountdown: __thiscall(ECX = task,
    // dt), RET 4 (reconstructed in src/torpedo_release_orders.cpp).
    virtual void close_to_ship_attack_mode_countdown_009a2810(void* task, float dt) = 0;

    // ---- virtual slots (ECX = the object) ----
    virtual void state_vcall_04_enter(void* state) = 0;
    virtual void state_vcall_08_exit(void* state) = 0;
    virtual void state_vcall_0c_tick(void* state, float dt) = 0;
    // approach->vtable[+0](float out[3]), RET 4, EAX = the float[3] read back.
    virtual const float* approach_vcall_00_target_point(void* approach, float* out) = 0;
    // archive->vtable[+0Ch](name, value) and [+10h](name, value): two eight-byte
    // records passed by value, RET 10h.
    virtual void archive_vcall_0c_write(void* archive, Cc13Seg67aArchiveValue name,
                                        Cc13Seg67aArchiveValue value) = 0;
    virtual void archive_vcall_10_read(void* archive, Cc13Seg67aArchiveValue name,
                                       Cc13Seg67aArchiveValue value) = 0;

    // ---- manoeuvre tasks (second batch) ----
    // 009BAFC0: __thiscall(ECX = task, bot, condition), RET 8: the pullup /
    // conditioned-manoeuvre constructor (excluded cc9 range; called only).
    virtual void bot_task_pullup_construct_009bafc0(void* task, void* bot, void* condition) = 0;
    // 009BA830: __thiscall(ECX = task, bot, archive), RET 8: its stream twin.
    virtual void bot_task_conditioned_construct_from_stream_009ba830(void* task, void* bot,
                                                                     void* archive) = 0;
    // 007B5E20: __thiscall(ECX = condition, a, b, distance), RET 0Ch, EAX: the
    // distance condition constructor.
    virtual void* distance_condition_construct_007b5e20(void* condition, void* a, void* b,
                                                        float distance) = 0;
    // The float at 00F876A4 (BSS; read as the clock the timed condition adds to).
    virtual float global_float_00f876a4() = 0;
    // 0099B690 BSP_BotTask_Retire: __thiscall(ECX = task).
    virtual void bot_task_retire_0099b690(void* task) = 0;
    // condition->vtable[+0](), AL.
    virtual bool condition_vcall_00(void* condition) = 0;
    // (unit+72Ch)->vtable[+38h]() with ECX = unit+72Ch (an embedded object), AL.
    virtual bool unit_embedded_72c_vcall_38(void* object) = 0;
    // unit->vtable[+38h](), float in ST0 (a speed; 009BBB56).
    virtual float unit_vcall_38_speed(void* unit) = 0;
};

// 009A1B70: __thiscall(ECX = approach block), RET. Unreferenced copy of the three
// registrations 009A2500 makes (no xref; kept for completeness).
void close_to_ship_register_states_009a1b70(Cc13Seg67aHost& host, void* approach);
// 009A1BB0: task slot (follow state for the unit's role), __thiscall(ECX = task):
// task+45Ch (moveto) for a flight leader, task+498h (follow) otherwise.
void* close_to_ship_follow_state_for_role_009a1bb0(Cc13Seg67aHost& host, void* task);
// 009A1C30: approach vtable[0] (Ghidra labels it Unwind@009A1C30), __thiscall(ECX =
// approach, float out[3]), RET 4, returns out: the target ship's world position,
// or the unit's when no ship is observed.
float* close_to_ship_target_point_009a1c30(Cc13Seg67aHost& host, void* approach, float* out);
// 009A1CA0: __thiscall(ECX = element, observed), RET 4, returns element.
void* point_observer_element_construct_009a1ca0(Cc13Seg67aHost& host, void* element,
                                                void* observed);
// 009A1D60: __thiscall(ECX = approach, owner, ship), RET 8, returns approach.
void* close_to_ship_approach_construct_009a1d60(Cc13Seg67aHost& host, void* approach,
                                                void* owner, void* ship);
// 009A1DC0: __thiscall(ECX = approach, owner, archive), RET 8, returns approach.
void* close_to_ship_approach_construct_from_stream_009a1dc0(Cc13Seg67aHost& host,
                                                            void* approach, void* owner,
                                                            void* archive);
// 009A1F00: __thiscall(ECX = state, approach), RET 4, returns state.
void* close_to_ship_stay_close_construct_009a1f00(Cc13Seg67aHost& host, void* state,
                                                  void* approach);
// 009A1FC0: __thiscall(ECX = state, distance, dt), RET 8, AL.
bool close_to_ship_try_drop_ordnance_009a1fc0(Cc13Seg67aHost& host, void* state,
                                              float distance, float dt);
// 009A2170: stayclose vtable[+0Ch], __thiscall(ECX = state, dt), RET 4.
void close_to_ship_stay_close_tick_009a2170(Cc13Seg67aHost& host, void* state, float dt);
// 009A2440: __thiscall(ECX = approach block), the approach block destructor body.
void close_to_ship_approach_destruct_009a2440(Cc13Seg67aHost& host, void* approach);
// 009A2500: __thiscall(ECX = approach, owner, ship), RET 8, returns approach.
void* close_to_ship_approach_block_construct_009a2500(Cc13Seg67aHost& host, void* approach,
                                                      void* owner, void* ship);
// 009A2610: __thiscall(ECX = approach, owner, archive), RET 8, returns approach.
void* close_to_ship_approach_block_construct_from_stream_009a2610(Cc13Seg67aHost& host,
                                                                  void* approach, void* owner,
                                                                  void* archive);
// 009A2730: __thiscall(ECX = task, bot, ship), RET 8, returns task.
void* close_to_ship_task_construct_009a2730(Cc13Seg67aHost& host, void* task, void* bot,
                                            void* ship);
// 009A2800 (slot +5Ch), 009A2890 (slot +10h), 009A28B0 (slot +3Ch), 009A28C0
// (slot +30h): constant returns, __thiscall(ECX = task), RET.
std::uint32_t close_to_ship_robots_row_009a2800() noexcept;
std::int32_t close_to_ship_slot10_009a2890() noexcept;
std::uint32_t close_to_ship_command_class_009a28b0() noexcept;
bool close_to_ship_slot30_009a28c0() noexcept;
// 009A2920: slot +40h, __thiscall(ECX = task), EAX: 2 when the control block is
// missing or reports no command, 0 when the command class is not 00E08FA8 or
// the commanded object is not the target ship, 1 otherwise.
std::int32_t close_to_ship_command_still_valid_009a2920(Cc13Seg67aHost& host, void* task);
// 009A29B0: __thiscall(ECX = task, state), RET 4.
void close_to_ship_set_state_009a29b0(Cc13Seg67aHost& host, void* task, void* state);
// 009A29F0: __thiscall(ECX = task, dt), RET 4 (dt unread).
void close_to_ship_select_state_009a29f0(Cc13Seg67aHost& host, void* task, float dt);
// 009A2B00: slot +64h, the per-tick update, __thiscall(ECX = task, dt), RET 4.
void close_to_ship_task_tick_009a2b00(Cc13Seg67aHost& host, void* task, float dt);
// 009A2B90: slot +0Ch, __thiscall(ECX = task, out), RET 4, returns out: out+0..+8
// the approach's target point, out+0Ch = 1.
void* close_to_ship_target_record_009a2b90(Cc13Seg67aHost& host, void* task, void* out);
// 009A2C00: slot +4Ch, __thiscall(ECX = task), float in ST0.
float close_to_ship_wait_value_009a2c00(Cc13Seg67aHost& host, void* task);
// 009A2C50: __thiscall(ECX = approach, archive), RET 4.
void close_to_ship_approach_save_009a2c50(Cc13Seg67aHost& host, void* approach, void* archive);
// 009A2CF0: slot +60h, __thiscall(ECX = task, archive), RET 4.
void close_to_ship_task_save_009a2cf0(Cc13Seg67aHost& host, void* task, void* archive);
// 009A2DF0: __thiscall(ECX = task), the task destructor body.
void close_to_ship_task_destruct_009a2df0(Cc13Seg67aHost& host, void* task);
// 009A2E50: __thiscall(ECX = task, bot, archive), RET 8, returns task.
void* close_to_ship_task_construct_from_stream_009a2e50(Cc13Seg67aHost& host, void* task,
                                                       void* bot, void* archive);
// 009A2F40 BSP_BotTask_MakeCloseToShip: __fastcall(ECX = bot, EDX = ship), EAX.
void* close_to_ship_make_009a2f40(Cc13Seg67aHost& host, void* bot, void* ship);
// 009A2FB0: the factory's create slot, __thiscall(ECX = factory, bot, archive),
// RET 8, EAX. The factory pointer is unread.
void* close_to_ship_factory_create_009a2fb0(Cc13Seg67aHost& host, void* bot, void* archive);

// ---------------------------------------------------------------------------
// Second batch: the manoeuvre tasks of window 009BB150-009BC5C0 (pullup,
// tightturn, leavealone, barrelroll, flikflak, split s). Their ticks and the
// shared constructors 009BAFC0 / 009BA830 lie in the excluded 009B4000-009BAFFF
// range and are host calls. Task offsets: +2F4h the unit, +2F8h the aircraft
// descriptor, +3F8h the end condition (leavealone, barrelroll; split s keeps
// "stage" there). 009BC030 (tightturn make) and 009BC0A0 / 009BB910 (flikflak)
// are hosted by game_hosts_units.cpp (ledger only, docs/CC13_SEG67A.md).
//
// RNG: 00BD2F10 stream 1 at 009BB7ED (barrelroll, U(0.5, 1)) and 009BBB1E
// (split s, U(0, 1)).
// ---------------------------------------------------------------------------

// Constant slots, __thiscall(ECX = task), RET.
const char* pullup_name_009bb150() noexcept;          // "pullup" 00D205D4, slot +14h
std::int32_t manoeuvre_slot10_009bb160() noexcept;    // 10, slot +10h (pullup/tightturn/barrelroll)
std::uint32_t tightturn_robots_row_009bb2c0() noexcept;  // 00F8A4B8, slot +5Ch
const char* tightturn_name_009bb2d0() noexcept;       // "tightturn" 00D2064C
std::uint32_t leavealone_robots_row_009bb6a0() noexcept; // 00F8A4A8
bool leavealone_slot34_009bb6b0() noexcept;           // false
bool leavealone_slot30_009bb6d0() noexcept;           // false
std::int32_t leavealone_slot10_009bb6e0() noexcept;   // 3
const char* leavealone_name_009bb6f0() noexcept;      // "leavealone" 00D206C0
std::uint32_t barrelroll_robots_row_009bb820() noexcept; // 00F8A488
const char* barrelroll_name_009bb830() noexcept;      // "barrelroll" 00D2073C
std::uint32_t flikflak_robots_row_009bb9d0() noexcept;   // 00F8A4C8
std::int32_t flikflak_slot10_009bb9e0() noexcept;     // 10
const char* flikflak_name_009bb9f0() noexcept;        // "flikflak" 00D207B0
std::uint32_t splits_robots_row_009bbbb0() noexcept;  // 00F8A498
bool splits_slot34_009bbbc0() noexcept;               // true
std::int32_t splits_slot10_009bbbd0() noexcept;       // 10
const char* splits_name_009bbbe0() noexcept;          // "split s" 00D20828

// 009BB180: __thiscall(ECX = task, bot, archive), RET 8: pullup stream
// constructor (009BA830, vtable 00D20568, safePitch +400h, refPitch +3FCh,
// started +404h).
void* pullup_construct_from_stream_009bb180(Cc13Seg67aHost& host, void* task, void* bot,
                                            void* archive);
// 009BB280 (no Ghidra function): __thiscall(ECX = task, bot, condition, byte
// left), RET 0Ch: tightturn constructor.
void* tightturn_construct_009bb280(Cc13Seg67aHost& host, void* task, void* bot, void* condition,
                                   std::uint8_t left);
// 009BB2F0: __thiscall(ECX = task, bot, archive), RET 8.
void* tightturn_construct_from_stream_009bb2f0(Cc13Seg67aHost& host, void* task, void* bot,
                                               void* archive);
// 009BB640: __thiscall(ECX = task, bot, condition), RET 8.
void* leavealone_construct_009bb640(Cc13Seg67aHost& host, void* task, void* bot,
                                    void* condition);
// 009BB6C0 (no Ghidra function), slot +38h: AL = (word task+414h > 10), signed.
bool leavealone_slot38_009bb6c0(void* task) noexcept;
// 009BB710: __thiscall(ECX = task, bot, archive), RET 8.
void* leavealone_construct_from_stream_009bb710(Cc13Seg67aHost& host, void* task, void* bot,
                                                void* archive);
// 009BB770: __thiscall(ECX = task, bot, condition), RET 8.
void* barrelroll_construct_009bb770(Cc13Seg67aHost& host, void* task, void* bot,
                                    void* condition);
// 009BB850: __thiscall(ECX = task, bot, archive), RET 8.
void* barrelroll_construct_from_stream_009bb850(Cc13Seg67aHost& host, void* task, void* bot,
                                                void* archive);
// 009BBA10: __thiscall(ECX = task, bot, archive), RET 8.
void* flikflak_construct_from_stream_009bba10(Cc13Seg67aHost& host, void* task, void* bot,
                                              void* archive);
// 009BBAD0: __thiscall(ECX = task, bot), RET 4.
void* splits_construct_009bbad0(Cc13Seg67aHost& host, void* task, void* bot);
// 009BBC00: __thiscall(ECX = task, bot, archive), RET 8.
void* splits_construct_from_stream_009bbc00(Cc13Seg67aHost& host, void* task, void* bot,
                                            void* archive);
// 009BBCF0 (no Ghidra function), slot +60h: __thiscall(ECX = task, archive), RET 4.
void splits_save_009bbcf0(Cc13Seg67aHost& host, void* task, void* archive);
// 009BBE20: __fastcall(ECX = bot, EDX = target, distance, seconds), RET 8, EAX.
void* barrelroll_make_until_far_009bbe20(Cc13Seg67aHost& host, void* bot, void* target,
                                         float distance, float seconds);
// 009BBEF0 / 009BBF60 / 009BBFC0: __fastcall makers, EAX.
void* barrelroll_create_009bbef0(Cc13Seg67aHost& host, void* bot, void* condition);
void* splits_create_009bbf60(Cc13Seg67aHost& host, void* bot);
void* leavealone_create_009bbfc0(Cc13Seg67aHost& host, void* bot, void* condition);
// Factory create slots, __thiscall(ECX = factory, bot, archive), RET 8, EAX.
void* barrelroll_factory_create_009bc110(Cc13Seg67aHost& host, void* bot, void* archive);
void* splits_factory_create_009bc180(Cc13Seg67aHost& host, void* bot, void* archive);
void* leavealone_factory_create_009bc1f0(Cc13Seg67aHost& host, void* bot, void* archive);
void* tightturn_factory_create_009bc260(Cc13Seg67aHost& host, void* bot, void* archive);
void* flikflak_factory_create_009bc2d0(Cc13Seg67aHost& host, void* bot, void* archive);
// 009BC3A0: leavealone slot +64h, __thiscall(ECX = task, dt), RET 4 (dt unread).
void leavealone_tick_009bc3a0(Cc13Seg67aHost& host, void* task, float dt);

} // namespace bsp
