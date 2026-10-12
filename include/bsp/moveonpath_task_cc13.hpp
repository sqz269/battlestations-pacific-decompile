#pragma once

// The plane `moveonpath` bot task (kind 6, 520h bytes; factory 009BDBB0, constructor
// 009BCE50, approach 009BCC60/009BC780 at task+3F8h). 0099A170's arm at 0099A24A is
// the factory's only caller. Window 009BC690-009BDD70. Packet cc13-moveonpath1;
// evidence, listings and the suggested host glue in docs/CC13_MOVEONPATH_TASK.md.
// Every name here is a hypothesis, not a recovered symbol. These are new C++
// interfaces, not binary replacements; build-tested only, not wired into any host.
//
// Offsets: "approach+N" is relative to task+3F8h; "task+N" is the whole object;
// "plan+N" is the pilot plan (approach+18h == task+4h); "ctl+N" is the squadron
// control block at approach+0Ch (unit+9D4h, task+404h); "desc+N" the aircraft
// descriptor at approach+8h (unit+538h).
// Precision: the image keeps x87 intermediates in extended precision and rounds
// at each float store; this code computes intermediates in double and rounds at
// the same stores. Constants loaded as qwords stay double, as dwords stay float.
//
// RNG: every draw of the task is 00BD2F10 BSP_Random_UniformFloatRange with
// ECX = 1 (stream 1). They go through MoveOnPathRandom with a per-site key, so a
// host routes them to a keyed per-unit stream (`unit name + "#mp" + key`) and
// never to stream 0.

#include <array>
#include <cstdint>

#include "bsp/bot_tasks.hpp"   // PilotAltitudeChannel

namespace bsp {

// 00BD2F10 with ECX = 1, U(low, high). Keys: "2c" (009BC7C0), "74" (009C2A0A, the
// shared follow state's constructor), "18" (009BC9AE), "51c" (009BCECE) at
// construction, in that order; "30" (009BD700) on every slot +54h call.
struct MoveOnPathRandom {
    virtual ~MoveOnPathRandom() = default;
    virtual float uniform_00bd2f10(const char* key, float low, float high) = 0;
};

inline constexpr std::uint32_t kMoveOnPathCommandClass = 0x00E08F80u;  // slot +3Ch 009BCFC0
inline constexpr int kMoveOnPathTaskKind = 6;          // 009BCE6D PUSH 6 into 0099C6F0
inline constexpr int kMoveOnPathTaskSlot10 = 3;        // slot +10h 009BCFD0 (MOV EAX,3)
inline constexpr const char* kMoveOnPathTaskName = "moveonpath";  // slot +14h 009BCFE0
inline constexpr std::uint32_t kMoveOnPathTaskSize = 0x520u;      // 009BDBC7 PUSH 520h

// task+310h, whole-object offsets, named by the strings 009BCC60 registers.
enum class MoveOnPathState : int {
    kPath = 0x44C,    // "path (path)"   vtable 00D208BC (approach+54h), tick 009BC8E0
    kFollow = 0x464,  // "follow (path)" vtable 00D20AB8 (approach+6Ch), the shared follow
    kCircle = 0x4FC,  // "circle (path)" vtable 00D208D8 (approach+104h), tick 009BCA50;
                      // no live writer of task+310h selects it (load path only)
};

// The approach sub-object fields the task's own code reads or writes. The shared
// head (009F9CE0, 009F9980) is host-side.
struct MoveOnPathApproach {
    float leg_weight_28 = -1.0f;           // approach+28h, written by 009FD050
    float alt_offset_2c = 0.0f;            // approach+2Ch "altOffset", U(0, MaxAltOffset)
    float travel_alt_offset_30 = 50.0f;    // approach+30h "travelAltOffset", [00CEB4D4] = 50.0
    std::array<float, 3> ref_34{};         // approach+34h..+3Ch, approach vtable[0] 009BC760
};

struct MoveOnPathTask {
    MoveOnPathApproach ap;
    MoveOnPathState state = MoveOnPathState::kPath;   // task+310h
    float rule_period_518 = 1.0f;          // task+518h = [00D7A24C] = 1.0
    float rule_timer_51c = 0.0f;           // task+51Ch = -U(0, 1)
    bool circle_clockwise_18 = false;      // circle state+18h = (U(0, 1) >= 0.5)
    float follow_stagger_74 = 0.0f;        // follow state+74h = -U(0, [00CE3D30] 0.6)
};

// 009FD050 on the approach, read from the listing (009FD050-009FD0D8, RET 8): the
// leg weight for a distance `d`. 0 <= d < 3000 ([00CFA424]): the converging-leg
// manager (009BD400) registers the point (009FC000) and approach+28h =
// 00419010(1200 ([00CFD714]), 1.0, 3000, 0.0, d). Otherwise the manager drops the
// unit (009FC1C0) and approach+28h = -1.0 ([00D7A260]).
struct MoveOnPathLegUpdate {
    bool registers_009fc000 = false;   // false: 009FC1C0 removes
    float weight_28 = -1.0f;
};
MoveOnPathLegUpdate moveonpath_leg_weight_009fd050(float d) noexcept;

// What the cursor and the squadron control block answer to 009BD4D0.
struct MoveOnPathCursorView {
    bool present = false;                  // ctl, director+54h == 00E08F80, 0071BFF0(0) != 0
    std::array<float, 3> waypoint{};       // cursor->vtable[4](&out, -1), the current point
    bool final_valid = false;              // 007B1350: follow mode 1 (SIMPLE) with points
    std::array<float, 3> final_point{};    // 007B1350: the last point
};
struct MoveOnPathApproachInputs {
    MoveOnPathCursorView cursor;
    std::array<float, 3> ctl_position_fc{};   // ctl+FCh..+104h after 00414DB0
    bool ctl_alt_set_3ad = false;             // ctl+3ADh
    float ctl_alt_394 = 0.0f;                 // ctl+394h
};

// The converging-leg manager 009BD400 (a lazily built singleton; 009FAA40 its
// constructor) and the altitude it hands back. Host-side.
struct MoveOnPathLegHost {
    virtual ~MoveOnPathLegHost() = default;
    virtual void leg_register_009fc000(const std::array<float, 3>& point) = 0;
    virtual void leg_remove_009fc1c0() = 0;
    virtual float leg_altitude_009fc260(const std::array<float, 3>& point, float weight) = 0;
};

// 009BD4D0, the approach update (the arm's first call). Writes ap.ref_34 (the steer
// point, its y replaced by the altitude target) and ap.leg_weight_28; zeroes
// alt_offset_2c and travel_alt_offset_30 while ctl+3ADh is clear.
void moveonpath_approach_update_009bd4d0(MoveOnPathApproach& ap,
                                         const MoveOnPathApproachInputs& in,
                                         MoveOnPathLegHost& legs);

// 009BD150, the state rule's countdown: true when the period has elapsed (the
// timer is reloaded), false when it only counted down. Only then does 009BD150 ask
// 007B8AD0 (unit+9D8h == 0) and pick the state; a different state is exited
// (vtable[8]) and the new one entered (vtable[4]). No path leads to kCircle.
bool moveonpath_rule_timer_009bd150(MoveOnPathTask& task, float dt) noexcept;
MoveOnPathState moveonpath_rule_state_009bd17f(bool leader) noexcept;

// What a path or circle tick writes into the pilot plan and the gun block.
struct MoveOnPathPlanCommand {
    bool writes_neutral_26c = false;      // plan+26Ch = 1 (path only)
    float speed_2b4 = 0.0f;               // plan+2B4h; plan+2B0h = 0, plan+2D8h = 1
    float pitch_altitude = 0.0f;          // 009FB800(state, altitude, 1.0)
    float pitch_gain = 1.0f;
    bool heads_to_point_009f9e40 = false; // path: 009F9E40(state, &ref)
    std::array<float, 3> head_point{};
    bool circles_009fbb20 = false;        // circle: 009FBB20(state, &(x, z), radius, cw, &out)
    float circle_center_x = 0.0f;
    float circle_center_z = 0.0f;
    float circle_radius = 0.0f;           // desc+268h
    bool circle_clockwise = false;        // circle state+18h
    bool ends_command_0071e430 = false;   // director->0071E430(00E08F80, 1) when a director exists
    bool writes_gun_cone_40 = false;      // (approach+1Ch)+40h = tuning+670h Angle_MoveTo
    bool calls_direction_009fabe0 = false;// 009FABE0(approach+1Ch, 009A1A20(plan, 0099B630(plan)))
};

// 009BC8E0, the path state's tick (vtable 00D208BC +0Ch). `ref` is approach
// vtable[0] (009BC760: approach+34h..+3Ch); `ctl_speed_3a0` is ctl+3A0h.
MoveOnPathPlanCommand moveonpath_path_tick_009bc8e0(const std::array<float, 3>& ref,
                                                    float ctl_speed_3a0) noexcept;

// 009BCA50, the circle state's tick (vtable 00D208D8 +0Ch). `desc_speed_18c` is
// desc+18Ch, `desc_radius_268` desc+268h, `has_director` ctl->vtable[114h]() != 0.
MoveOnPathPlanCommand moveonpath_circle_tick_009bca50(const std::array<float, 3>& ref,
                                                      float desc_speed_18c,
                                                      float desc_radius_268,
                                                      bool clockwise_18,
                                                      bool has_director) noexcept;

// 009BD6E0, slot +54h, the cruise profile. Draws approach+30h first, then (leader
// only) picks the cruising altitude by ordnance and writes it through ctl+37Ch's
// channel (ctl+38Dh lock, +380h current, +3A9h one-shot, +394h altitude, +3ADh dirty).
struct MoveOnPathCruiseInputs {
    bool flight_leader = false;            // 007B8AD0 at 009BD711
    float travel_alt_random_364 = 0.0f;    // tuning+364h Pilot/MoveTo/TravelAltRandom
    bool has_torpedo_007b93f0 = false;     // checked in this order
    bool has_depth_charge_007b94f0 = false;
    bool has_general_bomb_007b9320 = false;
    bool level_bomber_0047b850 = false;    // general bomb only: true LevelBomb, false DiveBomb
    bool small_turn_roll_0047b880 = false; // fallback: true +35Ch, false +360h
    // Pilot/<class>/MoveOnCruisingAlt (int32 slots, compared as floats with 0.0)
    // and Pilot/<class>/CruisingAlt.
    std::int32_t torpedo_move_on_43c = 0;  float torpedo_cruising_alt_430 = 0.0f;
    std::int32_t depth_move_on_4b4 = 0;    float depth_cruising_alt_4a4 = 0.0f;
    std::int32_t level_move_on_454 = 0;    float level_cruising_alt_444 = 0.0f;
    std::int32_t dive_move_on_4d4 = 0;     float dive_cruising_alt_4c0 = 0.0f;
    float small_plane_travel_alt_35c = 0.0f;   // Pilot/MoveTo/SmallPlaneTravelAlt
    float large_plane_travel_alt_360 = 0.0f;   // Pilot/MoveTo/LargePlaneTravelAlt
    PilotAltitudeChannel cruise;           // ctl+38Dh / +380h / +3A9h
};
struct MoveOnPathCruiseResult {
    float travel_alt_offset_30 = 0.0f;     // always written (task+428h)
    bool leader_block_ran = false;
    bool writes_alt_394 = false;           // ctl+394h = alt_394 and ctl+3ADh = 1
    float alt_394 = 0.0f;
    bool clears_one_shot_3a9 = false;      // ctl+3A9h = 0
};
MoveOnPathCruiseResult moveonpath_cruise_profile_009bd6e0(const MoveOnPathCruiseInputs& in,
                                                          MoveOnPathRandom& rng);

// 009BD280, slot +40h, the still-valid predicate. 2 without a control block; 1
// while the current command (0071BE40) is `moveonpath` and its descriptor's object
// (0071EB60 -> 00521EA0) is the path's entity (009BD0B0: cursor+4 ->vtable[28h]
// -> 007ACA30); else 0.
int moveonpath_command_still_valid_009bd280(bool has_ctl, std::uint32_t current_command,
                                            std::uintptr_t command_object,
                                            std::uintptr_t path_object) noexcept;

// The host interface the driver runs over.
struct MoveOnPathTaskHost : MoveOnPathLegHost {
    virtual MoveOnPathApproachInputs approach_inputs() = 0;   // 009BD4D0's reads
    virtual bool flight_leader_007b8ad0() = 0;
    virtual float ctl_speed_3a0() = 0;
    virtual float desc_speed_18c() = 0;
    virtual float desc_radius_268() = 0;
    virtual bool has_director() = 0;                          // ctl->vtable[114h]() != 0
    // The shared follow state (vtable 00D20AB8: enter 009BED80, exit 009BDE40, tick
    // 009C1FD0); the path and circle states' enter/exit are 007B3DB0/007B3DC0, RET.
    virtual void enter_follow_009bed80() = 0;
    virtual void exit_follow_009bde40() = 0;
    virtual void tick_follow_009c1fd0(float dt) = 0;
    virtual void apply_plan(const MoveOnPathPlanCommand& c) = 0;
};

// 009BCE50 (with 009BCC60, 009BC780, 009C2980, 009BC960): the draws in the image's
// order, the rule period, and the initial state (007B8AD0 at 009BCEE1: a leader
// takes path, a wingman follow), entered through the host.
void moveonpath_task_construct_009bce50(MoveOnPathTask& task, MoveOnPathTaskHost& host,
                                        MoveOnPathRandom& rng, float max_alt_offset_540);

// 009BD990, slot +64h, the arm: 009BD4D0 on the approach, 009BD150, then the
// current state's vtable[0Ch](dt).
void moveonpath_task_arm_009bd990(MoveOnPathTask& task, MoveOnPathTaskHost& host, float dt);

// 009BD120, slot +58h: 009FD050(-1.0, &[00F87574]) drops the leg, then the current
// state's vtable[8] (exit).
void moveonpath_task_stop_009bd120(MoveOnPathTask& task, MoveOnPathTaskHost& host);

}  // namespace bsp
