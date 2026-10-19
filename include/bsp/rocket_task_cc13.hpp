#pragma once

// The rocket bot task (kind 0Bh, 708h bytes): factory 007B7FD0 -> constructor
// 007B6A40 -> approach 007B6830 at task+3F8h over the approach base 007B41E0,
// per-tick arm 007B7C40 (vtable 00D05918 slot +64h), the rule 007B7050 and the
// seven states 007B6830 registers by name. Window 007B3E00-007B8100. Packet
// cc13-rocket1; evidence, the twin table and the host glue in
// docs/CC13_ROCKET_TASK.md. Every name here is a hypothesis, not a recovered
// symbol. These are new C++ interfaces, not binary replacements; build-tested
// only, not wired into any host.
//
// The task is the strafe task's twin (docs/SQUADRON_LAND_TASK.md 5bw-5ca):
// local/rk1_twin.py compared 31 function pairs instruction by instruction. Its
// own pieces are the approach base (Strike row, target size class), the range
// test with hysteresis, the rule without a kind-41h target, the aim point lifted
// by g * IgnitionDelay^2, the aim tick's flight-time lead and rocket release,
// the goaway enter's standoff and side, the goaway bank re-plan through the
// fly-to solver 009FD570 and the goaway completion test.
//
// Offsets: "approach+N" is relative to task+3F8h; "task+N" is the whole object;
// "plan+N" is the pilot plan [approach+18h]; "gun+N" the gun block [approach+1Ch].
// Precision: x87 intermediates are carried in double and rounded at each float
// store; qword constants stay double, dword constants float (the main thread's
// 24-bit control word, docs/CC13_FPU_PRECISION.md, can move the last bit).
//
// RNG: every draw in the window is 00BD2F10 BSP_Random_UniformFloatRange with
// ECX = 1 (stream 1, docs/CC13_RNG_STREAM_CENSUS.md sections 10-11). They go
// through RocketRandom with a per-site key (kRocketDrawSites), so a host routes
// them to a keyed per-unit stream (`unit name + "#rk" + key`), never stream 0.
// The interface pattern is the kamikaze task's (include/bsp/kamikaze_task_cc13.hpp).

#include <array>
#include <cstddef>
#include <cstdint>

#include "bsp/approach_target_ref.hpp"
#include "bsp/near_field_probe.hpp"

namespace bsp {

struct PilotBotParameters;

// For the units host (cc9). True: an aircraft whose 007EEC50 class is rocket
// (00E08F48) flies this task. It must land with the PilotSetTarget feed of
// has_rocket_ordnance (007ED970), never before or after it. Committed OFF.
inline constexpr bool kRocketTaskBound = false;   // OFF: docs/CC13_ROCKET_TASK.md 7

// The task's +310h values, named by the strings 007B6830 registers.
enum class RocketState : int {
    kNone = 0,
    kMoveTo = 0x4E4,     // "moveto (Rocket)", BSP_BotStateMoveTo_Construct
    kFollow = 0x520,     // "follow (Rocket)", BSP_BotStateFollow_Construct
    kPrepare = 0x5B8,    // "Rocket/prepare", a follow-state variant
    kGoTowards = 0x650,  // "Rocket/gotowards", vtable 00D057B8
    kAim = 0x674,        // "Rocket/aim", vtable 00D05808
    kGoAway = 0x694,     // "Rocket/goaway", 007B5780, vtable 00D05824
    kAttackRun = 0x6E4,  // "Rocket/attackrun", 007B4D90, vtable 00D057E8
};

const char* rocket_state_name(RocketState s) noexcept;

namespace rocket {
inline constexpr std::uint32_t kCommandClass = 0x00E08F48u;    // 007B6CD0, slot +3Ch
inline constexpr std::uint32_t kAttackAnyClass = 0x00E08F78u;  // 007B7200
inline constexpr int kTaskKind = 0x0B;                          // 007B6A5D
inline constexpr std::size_t kTaskSize = 0x708;                 // 007B7FF1
inline constexpr float kGoTowardsDistanceSeed = 1400.0f;        // 00CED560 (strafe: 800)
inline constexpr float kGoAwayStandoffSeed = 1000.0f;           // 00CE3804, goaway +4Ch
inline constexpr double kAimLiftGravity = 9.8100004196167;      // 00CF9058, FMUL qword
inline constexpr float kProjtimeCap = 30.0f;                    // 00CE38C8, the store (compared as double 00CE7630)
inline constexpr double kForwardRangeScale = 0.75;              // 00CEC9D8, FMUL qword
inline constexpr double kRangeHysteresis = 100.0;               // 00D7A220, FADD qword
inline constexpr double kAltLoadoutGoAwayScale = 0.4000000059604645;   // 00CE65D0
inline constexpr double kAttackDistCap = 0.800000011920929;     // 00CE3D40, FMUL qword
}  // namespace rocket

// ---------------------------------------------------------------------------
// The random draws
// ---------------------------------------------------------------------------
// 00BD2F10 with ECX = 1. `key` is the site's suffix; the draw is U(low, high).
// The image draws all of them from one stream in tick order; a keyed host
// stream is a labelled substitution for that order. Same shape as
// KamikazeRandom, so one host adapter can serve both tasks.
struct RocketRandom {
    virtual ~RocketRandom() = default;
    virtual float uniform_00bd2f10(const char* key, float low, float high) = 0;
};

enum class RocketDraw : int {
    kGoAwayDist34,      // 007B4240 U(0.9, 1.05)              approach base
    kFireDist38,        // 007B42CB U(0.9, 1.1)               approach base
    kCountdownD0,       // 007B4332 U(0, 1), negated           approach base
    kGlideAngle2C,      // 007B3F0E U(0.8, 1.2)               glide seed
    kGlideDist40,       // 007B3F49 U(-10, 40)                glide seed
    kRunSpeed48,        // 007B4912 U(0.8, 1.1)               gotowards enter
    kAttackRunCd1C,     // 007B4DEA U(0, 1), negated           attackrun ctor
    kAimTimer18,        // 007B5322 U(0.2, 1)                 aim enter
    kAimRepeat18,       // 007B5725 U(0.8, 1.2)               aim tick, after a release
    kGoAwayHdgTimer30,  // 007B57EE U(0, 5)                   goaway ctor
    kGoAwayPitchTimer3C,// 007B5827 U(0, 5)                   goaway ctor
    kGoAwayCountdown44, // 007B584F U(0, 1), negated           goaway ctor
    kGoAwayProb18,      // 007B5874 U(0.3, 0.9)               goaway ctor
    kGoAwayPitch20,     // 007B5AC0 U(min(p,0), min(p+10deg, climb))  pitch re-plan
    kGoAwayBank1C,      // 007B5BAB U(r - 1, r + 1)           bank re-plan
    kGoAwayStandoff4C,  // 007B5FC4 U(1.0, 1.1)               goaway enter
    kGoAwayWindow,      // 007B6053 U(10, 16)                 goaway enter, evasive window
    kGoAwayEnterR,      // 007B607B U(0, 1)                   goaway enter, flikflak or tightturn
    kGoAwayEnterDist,   // 007B60FD U(0.5, 0.7)               goaway enter, flikflak distance
    kGoAwayEnterSide,   // 007B6180 U(0, 1)                   goaway enter, tightturn side
    kGoAwayHdgRearm,    // 007B6296 U(+28h, +2Ch) = U(2, 5)   goaway tick
    kGoAwayPitchRearm,  // 007B62D8 U(+34h, +38h) = U(2, 5)   goaway tick
    kGoAwayTickR,       // 007B6436 U(0, 1)                   goaway tick, evasive
    kGoAwayTickDist,    // 007B6476 U(0.5, 0.7)               goaway tick, flikflak distance
    kGoAwayTickSide,    // 007B64F1 U(0, 1)                   goaway tick, tightturn side
    // Census sites with no code reference in the image (local/rk1_xref.py: no
    // rel32 call, no absolute dword). Never drawn by this reconstruction.
    kUnreachedTimerCtor,  // 007B3D8D U(0, hi), 007B3D60
    kUnreachedRunSpeed,   // 007B48B2 U(0.8, 1.1), 007B4890
    kUnreachedTimerTick,  // 007B59B4 U(lo, hi), 007B5980
    kCount
};

struct RocketDrawSite {
    RocketDraw id;
    std::uint32_t site;       // the CALL 00BD2F10
    std::uint32_t function;   // its function
    const char* key;          // RocketRandom key (the host prefixes "#rk")
    const char* range;        // the distribution, as the listing loads it
    bool reached;             // false: no code reference in the image
};

extern const std::array<RocketDrawSite, static_cast<std::size_t>(RocketDraw::kCount)>
    kRocketDrawSites;

// The target ref's first timer, U(0, 2.0) at 009FB2E7 inside 009FB200, drawn by
// rocket_task_construct_007b6a40 between kFireDist38 and kCountdownD0.
inline constexpr const char* kRocketRefTimerKey = "ref60";

// ---------------------------------------------------------------------------
// Inputs from the robot row, the target and the aimed rocket
// ---------------------------------------------------------------------------
// The Strike block of the 248h robots row (include/bsp/robot_config.hpp names it
// by row offset; the approach reads it as [approach+14h] = row+0Ch).
struct RocketStrikeRow {
    float too_close_104 = 0.0f;       // desc+F8h, approach+3Ch
    float go_away_distance_108 = 0.0f;// desc+FCh, approach+34h
    float attack_angle_10c = 0.0f;    // desc+100h, the glide seed's cap
    float target_prec_110 = 0.0f;     // desc+104h, the target ref's spread
    float homing_angle_114 = 0.0f;    // desc+108h, the rocket's homing block +24h
    float repeat_time_118 = 0.0f;     // desc+10Ch, aim +18h after a release
    float fire_angle_11c = 0.0f;      // desc+110h, the lateral/forward gate
    std::array<float, 3> fire_dist_120{{0.0f, 0.0f, 0.0f}};  // desc+114h + 4 * size
    float section_chance_130 = 0.0f;  // desc+124h -> ref+64h
    float engine_room_134 = 0.0f;     // desc+128h -> ref+68h
    float magazine_138 = 0.0f;        // desc+12Ch -> ref+6Ch
    float fueltank_13c = 0.0f;        // desc+130h -> ref+70h
};

RocketStrikeRow rocket_strike_row(const PilotBotParameters& row) noexcept;

// 007B3C00(target): 0 for a carrier (kind 9) or battleship (0Dh), 1 for a cargo
// ship (0Bh) or cruiser (0Ah), 2 for any other ship, a non-ship or none.
struct RocketTargetKinds {
    bool present = false;
    bool ship_06 = false;
    bool carrier_09 = false;
    bool cruiser_0a = false;
    bool cargo_0b = false;
    bool battleship_0d = false;
};
int rocket_target_size_007b3c00(const RocketTargetKinds& k) noexcept;

// 007C1EB0(unit, 0, 0), the rocket the task aims (approach+D4h): the first
// weapon platform (IsKindOf 25h) whose vt[210h](33h, 0) holds, whose bullet
// class (vt[220h](0)) has IgnitionDelay (+E0h) > 0, and whose AntiAir (+E4h) is
// clear unless [00F874FD]; the answer is that platform's vt[218h](2Ah).
struct RocketPlatformView {
    bool weapon_platform_25 = false;
    bool carries_rocket_33 = false;
    float ignition_delay_e0 = 0.0f;
    bool anti_air_e4 = false;
};
bool rocket_platform_qualifies_007c1eb0(const RocketPlatformView& p,
                                        bool air_ground_same_f874fd) noexcept;

// What the task reads off the aimed rocket (record = approach+D4h).
struct RocketAimedRocket {
    float ignition_delay_e0 = 0.0f;  // [record+314h]+E0h, 00809DE0
    float vmax_d8 = 0.0f;            // [record+314h]+D8h, 00809E10
    float accel_dc = 1.0f;           // [record+314h]+DCh
    float launch_speed = 0.0f;       // [record+3Ch]->vt[38h], else |record+318h|
};

// ---------------------------------------------------------------------------
// The approach (task+3F8h) and the states
// ---------------------------------------------------------------------------
struct RocketApproach {
    float speed_ratio_24 = 1.0f;     // +24h, 009F9CE0
    float glide_angle_2c = 0.0f;     // +2Ch
    float glide_tan_30 = 0.0f;       // +30h
    float goaway_dist_34 = 0.0f;     // +34h
    float fire_dist_38 = 0.0f;       // +38h
    float too_close_3c = 0.0f;       // +3Ch
    float glide_dist_40 = 0.0f;      // +40h
    float elapsed_44 = 3600.0f;      // +44h, [00CFDEB0]; zeroed by the hit notice 007B6D00
    float run_speed_48 = 0.0f;       // +48h
    bool in_range_4c = false;        // +4Ch (task+444h)
    float dist3_50 = 0.0f;           // +50h
    float horiz_54 = 0.0f;           // +54h
    // +58h: the target-reference sub-object. Its sub+18h (approach+70h,
    // task+468h) is the target, sub+1Ch the world point, sub+28h (approach+80h)
    // the body-frame offset, sub+44h (approach+9Ch) the projtime.
    ApproachTargetRefState ref;
    float period_cc = 1.0f;          // +CCh, [00D7A24C]
    float countdown_d0 = 0.0f;       // +D0h
    bool has_rocket_d4 = false;      // +D4h != 0, 007C1EB0
    RocketAimedRocket rocket;        // what +D4h points at, this tick
    std::array<float, 3> aim_point{{0.0f, 0.0f, 0.0f}};   // vt[0] 007B4390, this tick
};

struct RocketGoTowardsState {        // task+650h
    float dist_18 = rocket::kGoTowardsDistanceSeed;   // +18h
    float horiz_1c = rocket::kGoTowardsDistanceSeed;  // +1Ch
    bool aligned_20 = false;                          // +20h
};
struct RocketAimState {              // task+674h
    float timer_18 = 0.0f;           // +18h (strafe seeds 0.5)
    bool no_rocket_1c = false;       // +1Ch (task+690h)
    bool off_1d = false;             // +1Dh (task+691h)
};
struct RocketAttackRunState {        // task+6E4h
    float period_18 = 1.0f;          // +18h, 0.4 after the enter
    float countdown_1c = 0.0f;       // +1Ch
    float offset_20 = 0.0f;          // +20h
};
struct RocketGoAwayState {           // task+694h
    float prob_18 = 0.0f;            // +18h
    float bank_1c = 0.0f;            // +1Ch
    float pitch_20 = 0.0f;           // +20h
    bool done_24 = false;            // +24h (task+6B8h)
    float hdg_lo_28 = 2.0f, hdg_hi_2c = 5.0f, hdg_timer_30 = 0.0f;       // +28h..+30h
    float pitch_lo_34 = 2.0f, pitch_hi_38 = 5.0f, pitch_timer_3c = 0.0f; // +34h..+3Ch
    float period_40 = 1.0f;          // +40h
    float countdown_44 = 0.0f;       // +44h
    float side_48 = 1.0f;            // +48h, the fly-to solver's side
    float standoff_4c = rocket::kGoAwayStandoffSeed;   // +4Ch
};

// The whole task, as one value a host can keep per unit.
struct RocketTask {
    RocketState state = RocketState::kNone;   // +310h
    RocketApproach ap;
    RocketGoTowardsState gt;
    RocketAimState aim;
    RocketAttackRunState ar;
    RocketGoAwayState ga;
    int task_2e4 = 0;                // task+2E4h = FFh after each arm
};

// ---------------------------------------------------------------------------
// What one state tick writes
// ---------------------------------------------------------------------------
struct RocketPlanCommand {
    bool speed_written = false;      // plan+2B4h = speed, +2B0h = 0, +2D8h = 1
    float speed_2b4 = 0.0f;
    bool heading_written = false;    // plan+2C0h, mode +2CCh = 2
    float heading_2c0 = 0.0f;
    bool bank_written = false;       // plan+2C4h, mode +2CCh = 1
    float bank_2c4 = 0.0f;
    bool pitch_written = false;      // plan+2BCh with mode +2D0h
    float pitch_2bc = 0.0f;
    int pitch_mode_2d0 = 0;
    bool throttle_written = false;   // plan+278h, +27Ch = 1, +2A8h, +2ACh = 1, +2D8h = 0
    float throttle_278 = 0.0f;
    float air_brake_2a8 = 0.0f;
    // 009FBA50(base, range_low, range_high, scale), ECX = state+4: the attack
    // run's cruise-altitude pitch (cruise_altitude_command_009fba50).
    bool cruise_written = false;
    float cruise_base = 0.0f, cruise_low = 0.0f, cruise_high = 0.0f, cruise_scale = 0.0f;
    bool cone_written = false;       // gun+40h
    float cone_40 = 0.0f;
    // 009FABE0(gun block, heading, pitch): heading = 009A1A20(plan), which is
    // plan+2C0h when the heading mode is 2 and is derived from the unit's roll
    // otherwise (gun_heading_from_plan false).
    bool gun_direction_written = false;
    bool gun_heading_from_plan = false;
    float gun_pitch = 0.0f;
};

// A manoeuvre the goaway state front-pushes (00999F50): 1 = tightturn
// (009BC030 with 007B5C70/007B5D30, ends at 00996510), 2 = flikflak (009BC0A0
// with 007B5E20, ends at 00996300). The strafe host flies both already.
struct RocketManoeuvre {
    int kind = 0;
    bool side = false;               // tightturn: 0.5 > U(0, 1)
    float distance = 0.0f;           // flikflak: U(0.5, 0.7) * approach+34h
    float point[3] = {0.0f, 0.0f, 0.0f};   // the approach's vt[0] point
};

// The release hand-off (007B56C7-007B5700): 0080AA20 on [approach+D4h]+468h
// writes the rocket record's homing block (+47Ch target = approach+70h,
// +480h..488h the body-frame point, +48Ch the homing angle), then 007BBBA0
// (BSP_Unit_RequestOrdnanceRelease) on the unit.
struct RocketRelease {
    bool fire = false;
    float homing_point[3] = {0.0f, 0.0f, 0.0f};   // approach+80h..88h (007B5400)
    float homing_angle = 0.0f;                     // row+114h
};

// ---------------------------------------------------------------------------
// The host
// ---------------------------------------------------------------------------
// Values the arm cannot compute and the calls it hands out. Each is a native
// call or read; the comment names it.
struct RocketTuning {
    float attack_dist_664 = 1800.0f;   // Pilot/Strike/AttackDist
    float cruising_alt_660 = 500.0f;   // Pilot/Strike/CruisingAlt
    float reference_speed_668 = 280.0f / 3.6f;   // Pilot/Strike/ReferenceSpeed, KMH(280)
    float cone_670 = 0.0f;             // Pilot/AutoStrafeAngle (+670h, attack run)
    float cone_674 = 0.0f;             // (+674h, goaway)
    float cone_678 = 0.0f;             // (+678h, gotowards)
    float ceiling_210 = 1500.0f;       // Dynamics ceiling
    float climb_dist_544 = 1.0f;       // Pilot/General/ClimbDist
    float drop_dist_548 = 1.0f;        // Pilot/General/DropDist
};
struct RocketTaskHost {
    virtual ~RocketTaskHost() = default;
    // 009FADA0 on ap.ref with dt (approach_target_ref.hpp); returns sub+1Ch..+24h
    // and whether sub+18h (the target) is still set.
    virtual bool update_target_ref_009fada0(ApproachTargetRefState& ref, float dt,
                                            std::array<float, 3>& point) = 0;
    // 009FA260 on ap.ref after 007B48E0 re-armed it (the pick).
    virtual void pick_target_ref_009fa260(ApproachTargetRefState& ref) = 0;
    virtual bool target_marked_5d() = 0;          // target+5Dh
    virtual RocketTargetKinds target_kinds() = 0; // vt[5Ch](6, 9, 0Ah, 0Bh, 0Dh)
    virtual bool target_is_unit_5() = 0;          // target vt[5Ch](5)
    virtual float target_extent_007b5be0() = 0;   // max(target+434h, +444h, +448h)
    virtual float target_y_100() = 0;             // target+100h
    // 007C1EB0(unit, 0, 0): false when no platform qualifies.
    virtual bool aimed_rocket_007c1eb0(RocketAimedRocket& out) = 0;
    virtual std::array<float, 3> unit_position() = 0;   // unit+FCh..104h
    virtual float heading_50() = 0;               // unit vt[50h]
    // 004142E0 with unit+110h, the world inverse.
    virtual std::array<float, 3> world_to_body(const std::array<float, 3>& p) = 0;
    virtual float class_speed_188() = 0;          // [approach+8]+188h MaxSpd
    virtual float class_travel_speed_18c() = 0;   // [approach+8]+18Ch
    virtual float class_climb_1e4() = 0;          // [approach+8]+1E4h
    virtual float class_climb_1ec() = 0;          // [approach+8]+1ECh
    virtual float class_drop_1f0() = 0;           // [approach+8]+1F0h
    virtual bool bomber_kind() = 0;               // unit vt[5Ch](10h) || (16h)
    virtual bool flight_leader_007b8ad0() = 0;
    virtual int attack_mode_370() = 0;            // ctl+370h
    virtual float ctl_alt_398() = 0;              // ctl+398h
    virtual bool alt_loadout_007b58d0() = 0;      // ctl+369h && [00E17BF2]
    virtual int parity_f876b0() = 0;              // [00F876B0]
    virtual RocketTuning tuning() = 0;
    virtual RocketStrikeRow robot_row() = 0;      // rocket_strike_row(the unit's row)
    // 007F0280 in mode 0 (the squadron) with zero weights, ECX = [approach+0Ch].
    virtual NearFieldProbeResult near_field_probe_007f0280(const float extents[3]) = 0;
    // 009FD570 (fly_to_point_heading_009fd570) with ECX = the goaway state,
    // `point`, the unit, `standoff`, &side (read and written back), `range`, 1.0.
    virtual float fly_to_heading_009fd570(const std::array<float, 3>& point, float standoff,
                                          float& side, float range) = 0;
    // The shared formation states: their enter/exit/tick (BotStateMoveTo
    // 009BDxxx, BotStateFollow 009C1xxx) are the host's existing ones. The
    // moveto ranges come first each arm (009BDE80: near, far, speed range).
    virtual void set_moveto_ranges_009bde80(float near_range, float far_range,
                                            float speed_range) = 0;
    virtual void enter_formation_state(RocketState s) = 0;
    virtual void exit_formation_state(RocketState s) = 0;
    virtual void tick_formation_state(RocketState s, float dt) = 0;
    // The sinks.
    virtual void apply_plan(const RocketPlanCommand& c) = 0;
    virtual void push_manoeuvre_00999f50(const RocketManoeuvre& m) = 0;
    virtual void release_rocket(const RocketRelease& r) = 0;
};

// ---------------------------------------------------------------------------
// The whole task
// ---------------------------------------------------------------------------
// 007B6A40 -> 007B6830 -> 007B41E0: the approach base (with 009FB200, the
// target ref, constructed here), the state seeds, the goaway and attackrun
// constructors, then the initial state's enter. Draw order: 34, 38, ref60,
// d0, 2c, 40, ga30, ga3c, ga44, ga18, ar1c.
void rocket_task_construct_007b6a40(RocketTask& t, RocketTaskHost& host, RocketRandom& rng);

struct RocketArmResult {
    RocketState before = RocketState::kNone;
    RocketState after = RocketState::kNone;
    bool has_target = false;
    bool released = false;
    RocketPlanCommand plan;          // also handed to host.apply_plan
};
// 007B7C40 (task vtable +64h, RET 4): 007B78F0, 009BDE80, 007B7050 with the
// enters, the current state's vt[0Ch](dt), task+2E4h = FFh.
RocketArmResult rocket_task_arm_007b7c40(RocketTask& t, RocketTaskHost& host,
                                         RocketRandom& rng, float dt);

// 007B7AF0 (slot +54h, twin of 009CD020). For the flight leader both ctl+394h
// and ctl+398h take Pilot/Strike/CruisingAlt where the base rule allows, then
// the glide seed; then 007B78F0(0) for every unit (run here) and 0099B740
// (BSP_BotTask_AbandonIfStale, the host's).
struct RocketCruiseProfileInputs {
    bool ctl_38d = false, ctl_38c = false;   // already-overridden flags
    float ctl_380 = 0.0f, ctl_37c = 0.0f;
    bool ctl_3a9 = false, ctl_3aa = false;   // one-shots, always cleared
};
struct RocketCruiseProfileWrite {
    bool write_394 = false;          // ctl+394h = cruising_alt_660, ctl+3ADh = 1
    bool write_398 = false;          // ctl+398h = cruising_alt_660, ctl+3ADh = 1
    bool clear_3a9 = false;          // ctl+3A9h = 0 (leader, +38Dh clear)
    bool clear_3aa = false;          // ctl+3AAh = 0 (leader, +38Ch clear)
    float cruising_alt_660 = 500.0f;
};
RocketCruiseProfileWrite rocket_task_cruise_profile_007b7af0(RocketTask& t, RocketTaskHost& host,
                                                             RocketRandom& rng,
                                                             const RocketCruiseProfileInputs& in);

// ---------------------------------------------------------------------------
// The pieces (each one native routine; the arm composes them)
// ---------------------------------------------------------------------------
// 007B3EA0: +2Ch = U(0.8, 1.2) * min(atan2(ctl+398h, +34h), AttackAngle),
// +30h = tan(+2Ch), +40h = +30h * +34h + U(-10, 40).
void rocket_glide_seed_007b3ea0(RocketApproach& ap, float ctl_alt_398,
                                float attack_angle_10c, RocketRandom& rng);

struct RocketRangeInputs {
    bool target_present_70 = false;  // sub+18h after 009FADA0
    bool target_disabled_5d = false; // target+5Dh
    float attack_dist_664 = 1800.0f;
    bool alt_loadout = false;        // 007B58D0: ctl+369h && [00E17BF2]
};
// 007B78F0 after its 009FADA0 and 007C1EB0 calls: +44h, +50h, +54h and the
// once-a-period range test, against ap.aim_point and `unit_position`.
void rocket_approach_update_007b78f0(RocketApproach& ap, float dt,
                                     const std::array<float, 3>& unit_position,
                                     const RocketRangeInputs& in) noexcept;

struct RocketRuleInputs {
    RocketState current = RocketState::kNone;
    bool in_range_444 = false;       // approach+4Ch
    int control_mode_370 = 0;        // ctl+370h
    bool ref_target_468 = false;     // approach+70h != 0
    bool gotowards_ready = false;    // 007B6C00
    bool goaway_done_6b8 = false;
    bool aim_no_rocket_690 = false;
    bool aim_off_691 = false;
    bool flight_leader = false;      // 007B8AD0
};
// 007B7050 (with 007B6FB0). The caller performs a change: old vt[8], new vt[4].
RocketState rocket_rule_007b7050(const RocketRuleInputs& in) noexcept;
RocketState rocket_engaged_entry_007b6fb0(int control_mode_370, bool in_range_444) noexcept;
// 007B6C00 = 009CC2F0 instruction for instruction; field_188 is MaxSpd.
bool rocket_gotowards_ready_007b6c00(const RocketGoTowardsState& gt, float field_188,
                                     float fire_dist_38) noexcept;

// 00809DE0 on the rocket's class: 9.81 * IgnitionDelay^2 (float stores).
float rocket_aim_lift_00809de0(float ignition_delay_e0) noexcept;
// 007B4390, approach vt[0]: the target ref's world point with y raised by the
// lift when the task has a rocket.
std::array<float, 3> rocket_aim_point_007b4390(const std::array<float, 3>& ref_point,
                                               bool has_rocket_d4,
                                               float ignition_delay_e0) noexcept;
// 00809E10(range) on the rocket record: the time to cover `range` from the
// launch speed, accelerating at Acceleration up to VMax.
float rocket_flight_time_00809e10(float range, float launch_speed, float vmax_d8,
                                  float accel_dc) noexcept;
// 007B5390's clamp of that time into the target ref's projtime.
float rocket_projtime_007b5390(float flight_time) noexcept;

// 007B48E0 gotowards enter: +20h = 0, the glide seed, +48h, and the target ref
// re-armed with the Strike row (spread, dirty byte, section chance, weights).
// The host's 009FA260 pick follows (the tail JMP).
void rocket_gotowards_enter_007b48e0(RocketTask& t, float ctl_alt_398, float travel_speed_18c,
                                     const RocketStrikeRow& row, RocketRandom& rng);

struct RocketFlightInputs {
    std::array<float, 3> unit_position{{0.0f, 0.0f, 0.0f}};
    float unit_heading_50 = 0.0f;
    float class_climb_1e4 = 0.0f;
    RocketTuning tuning{};
};
// 007B4980 (twin of 009CA870), and 007B4F40 / 007B4F60 (twins of 009CAD90 /
// 009CADB0 with approach+54h and +664h in place of 009CAD00 and +658h).
RocketPlanCommand rocket_gotowards_tick_007b4980(RocketTask& t, const RocketFlightInputs& in,
                                                 RocketTaskHost& host);
void rocket_attackrun_enter_007b4f40(RocketAttackRunState& ar) noexcept;
RocketPlanCommand rocket_attackrun_tick_007b4f60(RocketTask& t, const RocketFlightInputs& in,
                                                 float dt, RocketTaskHost& host);

struct RocketAimInputs {
    std::array<float, 3> unit_position{{0.0f, 0.0f, 0.0f}};
    std::array<float, 3> local_point{{0.0f, 0.0f, 0.0f}};  // 004142E0 of ap.aim_point
    RocketStrikeRow row{};
};
struct RocketAimResult {
    RocketPlanCommand plan;
    bool projtime_written = false;   // ap.ref.projtime_44 (only with a rocket)
    float forward_range = 0.0f;      // max(z, 0.75 |local|)
    RocketRelease release;
};
// 007B5300 aim enter and 007B5430 aim tick.
void rocket_aim_enter_007b5300(RocketAimState& aim, RocketRandom& rng);
RocketAimResult rocket_aim_tick_007b5430(RocketTask& t, const RocketAimInputs& in, float dt,
                                         RocketRandom& rng);

struct RocketGoAwayInputs {
    std::array<float, 3> unit_position{{0.0f, 0.0f, 0.0f}};
    float unit_heading_50 = 0.0f;
    bool bomber_kind = false;        // unit IsKindOf(10h) || IsKindOf(16h)
    bool target_present_70 = false;  // approach+70h != 0
    bool target_is_unit_5 = false;   // target IsKindOf(5)
    float target_extent = 0.0f;      // 007B5BE0
    float target_y = 0.0f;           // target+100h
    int parity_f876b0 = 0;           // [00F876B0]
    float class_climb_1ec = 0.0f;
    float class_drop_1f0 = 0.0f;
    bool alt_loadout = false;        // ctl+369h && [00E17BF2]
    RocketTuning tuning{};
};
struct RocketGoAwayResult {
    RocketPlanCommand plan;
    RocketManoeuvre manoeuvre;
    bool elapsed_set_25 = false;     // approach+44h = 25.0 (tick only)
};
void rocket_goaway_construct_007b5780(RocketGoAwayState& ga, RocketRandom& rng);
// 007B59D0 / 007B5AF0, the re-plans.
void rocket_goaway_pitch_007b59d0(RocketGoAwayState& ga, const RocketApproach& ap,
                                  const RocketGoAwayInputs& in, RocketRandom& rng);
void rocket_goaway_bank_007b5af0(RocketGoAwayState& ga, const RocketApproach& ap,
                                 const RocketGoAwayInputs& in, RocketTaskHost& host,
                                 RocketRandom& rng);
RocketGoAwayResult rocket_goaway_enter_007b5f10(RocketTask& t, const RocketGoAwayInputs& in,
                                                RocketTaskHost& host, RocketRandom& rng);
RocketGoAwayResult rocket_goaway_tick_007b6240(RocketTask& t, const RocketGoAwayInputs& in,
                                               float dt, RocketTaskHost& host,
                                               RocketRandom& rng);

// 007B6F90, slot +50h: 2 in the aim state, else 0 (the base 007B4140 answers 0).
int rocket_step_result_007b6f90(RocketState s) noexcept;
// 007B7200, slot +40h (the still-valid predicate): 2 with no control block;
// 0 when ctl vt[114h] answers 0 or the director's command is neither rocket
// (00E08F48) nor 00E08F78; else 1 when task+468h is null or is the command's
// resolved target.
struct RocketStillValidInputs {
    bool has_control_block = true;
    bool ctl_vt114 = false;
    std::uint32_t director_command = 0;
    bool ref_target_468 = false;
    bool command_target_matches = false;
};
int rocket_still_valid_007b7200(const RocketStillValidInputs& in) noexcept;

// Newly defined vtable leaves. OnHit's ignored stack word is real (RET4).
bool rocket_on_hit_007b6d00(RocketTask&, std::uint32_t unused) noexcept;
int rocket_slot_return9_007b6ce0() noexcept;
// 007B7270 tail-selects 009BE3E0 with ECX=task+520h only for that exact state;
// otherwise 0099B720 returns its confirmed constant -1 (FLD[D7A260]; RET).
double rocket_follow_wait_007b7270(const RocketTask&, double (*follow_wait)(void*), void* follow);

}  // namespace bsp
