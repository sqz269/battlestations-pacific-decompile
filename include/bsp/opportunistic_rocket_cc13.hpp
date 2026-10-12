#pragma once

// The pilot bot's opportunistic rocket sub-object (task+38Ch, 5Ch bytes): ctor
// 009FCEF0(task+38Ch, task) from the bot task base ctor (0099C79C), target
// setter 007B7870, per-tick 009FD0E0 (__thiscall(this = task+38Ch, float dt),
// RET 4, body 009FD0E0-009FD54E) with its range-slot helper 009F99B0. Packet
// cc13-oprocket1; evidence, the host glue and the missions in
// docs/CC13_OPPORTUNISTIC_ROCKET.md. Every name here is a hypothesis, not a
// recovered symbol. New C++ interfaces, not binary replacements; build-tested
// only, not wired into any host.
//
// What it does: while a dogfight (009A7650) or strafe (009CA780) task has handed
// it a target, it decides every RocketCheckTime seconds whether to fire a salvo
// of up to NumRockets small rockets (no ignition delay: HVAR, R4M), chosen by
// the target's class (Rocket_SmallPlane / LargePlane / Ship / Landfort rows of
// robots.lua PilotBot), and fires each one through 0080AA20 (homing) and
// 007BBBA0 (release) once the range, flight-time and angle gates pass.
//
// Precision: x87 intermediates are carried in double and rounded at each float
// store; qword constants stay double, dword constants float (the main thread's
// 24-bit control word, docs/CC13_FPU_PRECISION.md, can move the last bit).
//
// RNG: two 00BD2F10 draws with ECX = 1 (stream 1, never stream 0), routed
// through RocketRandom with the keys kOpportunisticRocketCheckKey and
// kOpportunisticRocketChanceKey; the host prefixes "#or" to a per-unit stream.

#include <array>
#include <cstddef>
#include <cstdint>

#include "bsp/rocket_task_cc13.hpp"

namespace bsp {

// For the units host (cc9). True: every AI pilot bot ticks 009FD0E0 after its
// task arm (0099998C) and the dogfight and strafe tasks feed it a target. It
// needs the rocket ordnance feed (007ED970 / BSP_Unit_FindAimRocket 007C1EB0)
// and the homing glue (rocket_homing_cc13.hpp), so it lands with them.
inline constexpr bool kOpportunisticRocketBound = false;   // OFF: docs/CC13_OPPORTUNISTIC_ROCKET.md 7

namespace opportunistic_rocket {
inline constexpr float kTimerSeed = -1.0f;              // 00D7A260, ctor +10h/+14h
inline constexpr float kChanceLow = 0.0f;               // 009FD1D8 FLDZ
inline constexpr float kChanceHigh = 1.0f;              // 009FD1CA FLD1
inline constexpr float kMinForward = 1.0f;              // 00D7A24C, body z must exceed
inline constexpr double kMinRangeScale = 0.4000000059604645;  // 00CE65D0, FMUL qword
inline constexpr float kMinFlightTime = 0.25f;          // 00CE3868
inline constexpr float kSalvoScaleLast = 0.66f;         // 00D21CE4 (0x3F28F5C3)
inline constexpr float kLateralFloor = 5.0f;            // 00CE3850
inline constexpr float kMinRocketDelay = 0.08f;         // 00D05B50 (0x3DA3D70A)
inline constexpr std::uint32_t kDescCheckTime = 0x134;  // desc = row+0Ch: row+140h/+144h
inline constexpr std::uint32_t kDescClassRows = 0x13C;  // row+148h, stride 34h
inline constexpr std::uint32_t kClassRowStride = 0x34;
}   // namespace opportunistic_rocket

// RocketRandom keys (the host builds `unit name + "#or" + key`).
inline constexpr const char* kOpportunisticRocketCheckKey = "chk";     // 009FD1B3 U(CheckTime)
inline constexpr const char* kOpportunisticRocketChanceKey = "chance"; // 009FD1DD U(0, 1)

// *this at +0: the class row the last decision chose.
enum class OpportunisticRocketClass : int {
    kNone = -1,
    kSmallPlane = 0,   // kind 0Fh and 0047B880 true (not kind 10h/16h)
    kLargePlane = 1,   // kind 0Fh, kind 10h or 16h
    kShip = 2,         // kind 6
    kLandfort = 3,     // kind 1Bh, 45h or 46h
};

// One Rocket_* block, row+148h + class * 34h (desc+13Ch). attack_dist is one
// six-slot array indexed by 009F99B0; robots.lua fills only the slots of its own
// class (small 0, large 1, ship 2-4, landfort 5, include/bsp/robot_config.hpp).
struct OpportunisticRocketClassRow {
    float rocket_chance_00{0.0f};
    std::array<float, 6> attack_dist_04{};
    float attack_time_1c{0.0f};
    std::int32_t num_rockets_20{0};
    float rocket_delay_24{0.0f};
    float attack_angle_28{0.0f};   // radians
    float homing_angle_2c{0.0f};   // radians per second, 0080AA20's angle
};

// The fields 009FD0E0 reads from one 248h PilotBot robots row.
struct OpportunisticRocketRow {
    float check_time_lo_140{0.0f};   // RocketCheckTime[1], desc+134h
    float check_time_hi_144{0.0f};   // RocketCheckTime[2], desc+138h
    std::array<OpportunisticRocketClassRow, 4> classes{};
};

// This installation's robots.lua PilotBot rows (mtime 2025-06-01), by the
// 00901610 level index: Stun 0, SPNormal 1, SPVeteran 2, MPNormal 3,
// MPVeteran 4, Elite 5. Slots robots.lua does not author are 0.0 (PilotBotConfig
// is zero-filled BSS at 00F8A30C); docs/CC13_OPPORTUNISTIC_ROCKET.md 2 has lines.
extern const std::array<OpportunisticRocketRow, 6> kOpportunisticRocketRows;

// task+38Ch. Pointers become one-based unit indices; the image keeps observer
// nodes at +24h (target, vtable 00CE75CC, pointer +38h) and +44h (rocket,
// vtable 00D21CCC, pointer +58h), which a host must clear when the referent dies.
struct OpportunisticRocketState {
    OpportunisticRocketClass class_00{OpportunisticRocketClass::kNone};
    // +04h task, +08h the plane (task+2F4h) and +40h the cached desc are the host's.
    std::int32_t salvo_left_0c{0};
    float check_timer_10{opportunistic_rocket::kTimerSeed};
    float delay_timer_14{opportunistic_rocket::kTimerSeed};
    std::array<float, 3> aim_point_18{};   // the ctor leaves it unset; 007B7870 writes it
    std::size_t target_plus_one_38{0};
    bool target_is_plane_3c{false};        // ctor 0; 007B7870: target vt[5Ch](0Fh)
    std::size_t rocket_plus_one_58{0};
};

// 009FCEF0's values (the rest of the ctor is the host's pointers).
void opportunistic_rocket_construct_009fcef0(OpportunisticRocketState& s) noexcept;

// 007B7870, __thiscall(this = task+38Ch; target, point by value), RET 10h.
// Callers: 009A76D3 (BSP_BotTaskDogfight_SetTarget, point = zero vector
// [00F87574]) and 009CA80E (BSP_BotApproachStrafe_RearmTargetPoint, point =
// approach+80h, the strafe aim point). `target_is_plane` is target vt[5Ch](0Fh),
// false for a null target.
void opportunistic_rocket_set_target_007b7870(OpportunisticRocketState& s,
                                              std::size_t target_plus_one,
                                              bool target_is_plane,
                                              const std::array<float, 3>& point) noexcept;

// The clear inlined in the dogfight, strafe and rocket task Stops (slot +58h:
// 009AA9E0, 009CD0D0, 007B7BA0): target 0, point [00F87574] (zero), +3Ch = 0.
void opportunistic_rocket_clear_target(OpportunisticRocketState& s) noexcept;

// The kind answers the tick reads from the target (vt[5Ch]) and 0047B880.
struct OpportunisticRocketTargetKinds {
    bool plane_0f{false};
    bool small_plane_0047b880{false};   // !(kind 10h || kind 16h)
    bool ship_06{false};
    bool kind_09{false};
    bool kind_0d{false};
    bool kind_0b{false};
    bool kind_0a{false};
    bool land_1b{false};
    bool land_45{false};
    bool land_46{false};
};

// 009FD1E6-009FD256: the class row for a target.
OpportunisticRocketClass opportunistic_rocket_class_for_target(
    const OpportunisticRocketTargetKinds& k) noexcept;

// 009F99B0, __fastcall(ECX = target), RET: the attack_dist slot. 0 small plane,
// 1 large plane, 2 ship kind 9 or 0Dh, 3 ship kind 0Bh or 0Ah, 4 other ship,
// 5 anything else (and a null target).
int opportunistic_rocket_range_slot_009f99b0(const OpportunisticRocketTargetKinds* k) noexcept;

// 00419010(0, 0.66, NumRockets, 1.0, salvo_left): the lateral scale, 1.0 on
// the first rocket of a full salvo, 0.66 at none left.
float opportunistic_rocket_salvo_scale(std::int32_t num_rockets, std::int32_t salvo_left) noexcept;

// The fire gates of 009FD3D8-009FD4DF on the body-frame aim point.
struct OpportunisticRocketGateInputs {
    std::array<float, 3> body_aim{};   // 00414D10(00414E10(plane), lead point): x right, y up, z forward
    float body_range{0.0f};            // 0042B2F0 of body_aim
    float flight_time{0.0f};           // 00809E10(body_range)
    float attack_dist{0.0f};           // class row attack_dist_04[009F99B0]
    std::int32_t salvo_left{0};
};
enum class OpportunisticRocketGate : int {
    kPass = 0,
    kBehind,         // body z <= 1.0
    kOutOfRange,     // attack_dist <= range
    kTooSlow,        // AttackTime <= flight time
    kTooClose,       // flight time <= 0.25
    kOffAngle,       // AttackAngle <= lateral / clamp(range, 0.4 dist, dist)
};
OpportunisticRocketGate opportunistic_rocket_fire_gate(const OpportunisticRocketGateInputs& in,
                                                       const OpportunisticRocketClassRow& row) noexcept;

// The reads and sinks 009FD0E0 needs.
struct OpportunisticRocketHost {
    virtual ~OpportunisticRocketHost() = default;
    // The four bytes of the plane (+8h): +5Ch set, +5Dh, +60h, +5Eh clear (009FD10A).
    virtual bool unit_live() = 0;
    // target+5Dh (009FD0F1): set drops the target ref.
    virtual bool target_dead_5d() = 0;
    // [task+3F4h] -> 00F8A30C row (+30h base, +34h index); null when none.
    virtual const OpportunisticRocketRow* robot_row() = 0;
    virtual OpportunisticRocketTargetKinds target_kinds() = 0;
    // 0043F080 BSP_UnitInstance_IsAliveAndVisible on the target.
    virtual bool target_alive_visible_0043f080() = 0;
    // 00923810(rocket, 1) != 0: the rocket still has a parent (+3Ch), i.e. is on the plane.
    virtual bool rocket_attached_00923810(std::size_t rocket_plus_one) = 0;
    // 007C1EB0 BSP_Unit_FindAimRocket(plane, want_small = 1, anti_air): 0 = none.
    virtual std::size_t find_aim_rocket_007c1eb0(bool anti_air) = 0;
    // 00427EB0 world positions; target vt[34h] the world velocity.
    virtual std::array<float, 3> unit_position() = 0;
    virtual std::array<float, 3> target_position() = 0;
    virtual std::array<float, 3> target_velocity_vt34() = 0;
    // 00414D10 with 00414E10's inverse of the plane's pose: world point to body.
    virtual std::array<float, 3> world_to_body(const std::array<float, 3>& p) = 0;
    // 00809E10 on the rocket record (rocket_flight_time_00809e10 with its class).
    virtual float rocket_flight_time_00809e10(std::size_t rocket_plus_one, float range) = 0;
    // 0080AA20 on rocket+468h (rocket_set_homing_target_0080aa20).
    virtual void set_homing_target_0080aa20(std::size_t rocket_plus_one, std::size_t target_plus_one,
                                            const std::array<float, 3>& point, float angle) = 0;
    // 007BBBA0 BSP_Unit_RequestOrdnanceRelease on the plane.
    virtual void release_ordnance_007bbba0() = 0;
};

struct OpportunisticRocketTickResult {
    bool ran{false};        // the plane passed the live gate
    bool decided{false};    // the check timer expired this tick
    bool armed{false};      // the decision drew under RocketChance (salvo = NumRockets)
    bool evaluated{false};  // the geometry ran on an attached rocket
    bool released{false};
    OpportunisticRocketGate gate{OpportunisticRocketGate::kPass};
};

// 009FD0E0. One call per AI pilot bot think, from BSP_PilotBot_Update at
// 0099998C (and from the unreferenced 00997340 at 00997373).
OpportunisticRocketTickResult opportunistic_rocket_tick_009fd0e0(OpportunisticRocketState& s,
                                                                 OpportunisticRocketHost& host,
                                                                 RocketRandom& rng, float dt);

}   // namespace bsp
