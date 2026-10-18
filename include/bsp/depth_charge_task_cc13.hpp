#pragma once

// The plane depth-charge bot task (kind 1, 7BCh bytes): factory 009A6970 ->
// constructor 009A5240 -> approach 009A4DC0 over the approach base 009A3390
// (009F9CE0 head), per-think arm 009A66C0 (vtable 00D1F738 slot +64h), the
// approach update 009A5F50, the rule 009A5870 and the nine states 009A3090
// registers by name. Window 009A3090-009A6A60. Packet cc13-depthcharge1;
// evidence, the draw table and the host glue in docs/CC13_DEPTH_CHARGE_TASK.md.
// Every name here is a hypothesis, not a recovered symbol. These are new C++
// interfaces, not binary replacements; build-tested only, not wired into any
// host.
//
// Reused, not duplicated: the state enum, the state names, the rule 009A5870
// and its entry chooser 009A57D0, the attack-state set 009A5420, the release
// window 009A3FA0 and the manual-release exclusion of 009A66C0 step 7 are in
// include/bsp/bot_task_states.hpp (packet cc2_bot_task_states), re-read here
// against the listing and confirmed. That header's void* host is not used.
//
// Offsets: "approach+N" is relative to task+3F8h; "task+N" is the whole object;
// "plan+N" is the pilot plan [approach+18h]; "gun+N" the gun block
// [approach+1Ch]; "ctl+N" the pilot control block (the squadron) [approach+0Ch].
// task+46Ch/+46Dh/+46Eh are approach+74h/+75h/+76h and task+48Ch is
// approach+94h (the target ref's sub+18h), which names the transition inputs
// bot_task_states.hpp calls flag_46C/flag_46D: has-ordnance and in-range.
// Precision: x87 intermediates are carried in double and rounded at each float
// store; qword constants stay double, dword constants float.
//
// CADENCE: the cruise profile (slot +54h, 009A6500) runs on ACTIVATION only,
// when the task becomes the head of the bot's task list, not per think
// (docs/CC13_TASK_PROFILE_CADENCE.md). The per-think entry is slot +64h, the
// arm below.
//
// RNG: every draw in the window is 00BD2F10 BSP_Random_UniformFloatRange with
// ECX = 1 (stream 1). A rel32 scan of the window finds 12 sites
// (local/dc1_calls.py). They go through DepthChargeRandom with a per-site key
// (kDepthChargeDrawSites), so a host routes them to a keyed per-unit stream
// (`unit name + "#dc" + key`), never stream 0.

#include <array>
#include <cstddef>
#include <cstdint>

#include "bsp/approach_target_ref.hpp"
#include "bsp/bot_task_states.hpp"
#include "bsp/near_field_probe.hpp"

namespace bsp {

struct PilotBotParameters;

// For the units host (cc9). True: an aircraft whose 007EEC50 class is
// depthcharge (00E08F38) flies this task. It must land together with the
// chooser feed of has_depth_charge_ordnance (007EDA10 -> 007B94F0, kind 2Ch),
// never before or after it. Committed OFF.
inline constexpr bool kDepthChargeTaskBound = true;   // ON: SQUADRON_LAND_TASK 5ie (JM06 / USN21 pairs)

namespace depth_charge {
inline constexpr std::uint32_t kCommandClass = 0x00E08F38u;   // 009A54D0, slot +3Ch
inline constexpr int kTaskKind = 0x01;                         // 009A525D
inline constexpr std::size_t kTaskSize = 0x7BC;                // 009A6991
inline constexpr int kWeaponSelectNone = 0xFF;                 // 009A66D5, approach+78h
inline constexpr int kWeaponSelectDepthCharge = 3;             // 009A45AF, 009A39F7, 009A3B74, 009A4975
inline constexpr float kStickHalfSeed = 30.0f;                 // 00CE38C8, approach+58h before 009A3290
inline constexpr float kElapsedSeed = 3600.0f;                 // 00CFDEB0, approach+54h
inline constexpr float kLeaveTimerSeed = 5.0f;                 // 00CE3850, leave +18h
inline constexpr float kFollowSeed = 100.0f;                   // 00CE3D08, the follow ctor argument in 009A4DC0
inline constexpr double kAttackRangeHysteresis = 100.0;        // 00D7A220, FADD qword
inline constexpr float kTargetLostRadius = 1500.0f;            // 00CED724, COMISS
inline constexpr double kRangeOverFlyAbove = 1.5;              // 00CE3D78, FMUL qword
inline constexpr double kHighAltitude = 400.0;                 // 00CE3D90, FLD qword
inline constexpr float kPullOutBearing = 1.5f;                 // 00CE380C
inline constexpr double kArmBand = 200.0;                      // 00CE4D70, FLD qword
inline constexpr double kReleaseCeiling = 150.0;               // 00CE3DD8, FADD qword
}  // namespace depth_charge

// ---------------------------------------------------------------------------
// The random draws
// ---------------------------------------------------------------------------
// 00BD2F10 with ECX = 1. `key` is the site's suffix; the draw is U(low, high).
// The image draws all of them from one stream in tick order; a keyed host
// stream is a labelled substitution for that order. Same shape as RocketRandom
// and KamikazeRandom, so one host adapter can serve all three tasks.
struct DepthChargeRandom {
    virtual ~DepthChargeRandom() = default;
    virtual float uniform_00bd2f10(const char* key, float low, float high) = 0;
};

enum class DepthChargeDraw : int {
    kRunSpeed30,        // 009A33F8 U(0.9, 1.1)                      approach ctor
    kAimAltSpread34,    // 009A3440 U(0, AimAltRange/2 - /1)         approach ctor
    kManeuverAlt38,     // 009A346F U(ManeuverAltRange/1, /2)        approach ctor
    kFlyAbove40,        // 009A34AA U(1.1, 1.3)                      approach ctor
    kBiasX,             // 009A31D1 U(-TargetHError, +)              009A31A0 (ctor, aim enter)
    kBiasZ,             // 009A3201 U(-TargetVError, +)              009A31A0
    kProjError6C,       // 009A3281 U(-CalcTargetPosError, +)        009A31A0
    kAttackRunCd1C,     // 009A36FA U(0, 1), negated                 attackrun ctor
    kLeaveTimer18,      // 009A39DE U(4, 7)                          leave enter
    kGoAwayStandoff1C,  // 009A3CD3 U(200, 400)                      goaway enter
    kAimCooldown1C,     // 009A4646 U(0.9, 1.1), * 0.4              aim tick, after a release
    kPrepareGate98,     // 009A5360 U(1.5, 3.0)                      009A5330 (slot +24h)
    kCount
};

struct DepthChargeDrawSite {
    DepthChargeDraw id;
    std::uint32_t site;       // the CALL 00BD2F10
    std::uint32_t function;   // its function
    const char* key;          // DepthChargeRandom key (the host prefixes "#dc")
    const char* range;        // the distribution, as the listing loads it
};

extern const std::array<DepthChargeDrawSite, static_cast<std::size_t>(DepthChargeDraw::kCount)>
    kDepthChargeDrawSites;

// The target ref's first timer, U(0, 2.0) at 009FB2E7 inside 009FB200, drawn
// by the approach constructor between kFlyAbove40 and kBiasX.
inline constexpr const char* kDepthChargeRefTimerKey = "ref60";

// ---------------------------------------------------------------------------
// Inputs from the robots row and the tuning singleton
// ---------------------------------------------------------------------------
// The DepthCharge block of the 248h robots row, read as [approach+14h] = row+0Ch
// (robot_config.hpp suffixes are row offsets).
struct DepthChargeRow {
    float target_h_error_0cc = 0.0f;          // desc+C0h, bias x
    float target_v_error_0d0 = 0.0f;          // desc+C4h, bias z
    float calc_target_pos_error_0d4 = 0.0f;   // desc+C8h, approach+6Ch
    float cancel_target_dist_0d8 = 0.0f;      // desc+CCh, 009A5780
    float target_point_select_prec_0dc = 0.0f;// desc+D0h, the ref's spread (sub+48h..+50h)
};
DepthChargeRow depth_charge_row(const PilotBotParameters& row) noexcept;

// Pilot/DepthCharge/* (singleton +494h..+4BCh) and the three AutoStrafeAngle
// cones. A host reads the live singleton (game_tuning_singleton.hpp).
struct DepthChargeTuning {
    float aim_alt_range_1_494 = 0.0f;
    float aim_alt_range_2_498 = 0.0f;
    float maneuver_alt_range_1_49c = 0.0f;
    float maneuver_alt_range_2_4a0 = 0.0f;
    float cruising_alt_4a4 = 0.0f;
    float fly_above_dist_4a8 = 0.0f;
    float attack_dist_4ac = 0.0f;
    float safe_dist_4b0 = 0.0f;
    float reference_speed_4b8 = 1.0f;
    float target_lost_time_4bc = 20.0f;      // NumberOrDefault, 20
    float angle_prepare_66c = 0.0f;          // Pilot/AutoStrafeAngle/Angle_Prepare
    float angle_move_to_670 = 0.0f;          // Pilot/AutoStrafeAngle/Angle_MoveTo
    float angle_go_away_674 = 0.0f;          // Pilot/AutoStrafeAngle/Angle_GoAway
};

// The plane class block [approach+8] = unit+538h.
struct DepthChargeClass {
    float max_spd_188 = 0.0f;              // MaxSpd, 009F9CE0's ratio
    float travel_speed_190 = 0.0f;         // NewTravelSpeedMul * TravelSpeed (007D240A)
    float turn_roll_25c = 0.0f;            // TurnRoll
    float turn_circle_radius_268 = 0.0f;   // TurnCircleRadius
    float level_flight_speed = 0.0f;       // 007C47F0: LevelFlight * StallSpd
};

// ---------------------------------------------------------------------------
// The approach (task+3F8h) and the states
// ---------------------------------------------------------------------------
struct DepthChargeApproach {
    float speed_ratio_24 = 1.0f;       // +24h, 009F9CE0: max(1, MaxSpd / ReferenceSpeed)
    int drop_balance_2c = 0;           // +2Ch, +1 per drop request (009A5D00), -1 per release
    float run_speed_30 = 0.0f;         // +30h, U(0.9, 1.1) * 007C47F0
    float aim_alt_spread_34 = 0.0f;    // +34h
    float maneuver_alt_38 = 0.0f;      // +38h
    float aim_alt_base_3c = 0.0f;      // +3Ch, AimAltRange/1, capped by ctl+394h
    float fly_above_40 = 0.0f;         // +40h
    float attack_range_44 = 0.0f;      // +44h, 1.5 * +40h, raised by the profile
    float range_48 = 0.0f;             // +48h, planar distance to the aim point
    float bearing_4c = 0.0f;           // +4Ch, heading to the aim point
    float rel_bearing_50 = 0.0f;       // +50h, bearing minus the unit heading
    float elapsed_54 = depth_charge::kElapsedSeed;   // +54h; zeroed by slot +2Ch
    float stick_half_58 = depth_charge::kStickHalfSeed;   // +58h, 009A3290
    float release_bias_5c = 0.0f;      // +5Ch
    float run_in_60 = 0.0f;            // +60h, the along-track distance every interpolation keys on
    float fall_lead_64 = 0.0f;         // +64h, 009A3300
    float off_track_68 = 0.0f;         // +68h
    float proj_error_6c = 0.0f;        // +6Ch
    float lost_timer_70 = 0.0f;        // +70h
    bool has_ordnance_74 = true;       // +74h (task+46Ch), 007B94F0
    bool in_range_75 = false;          // +75h (task+46Dh)
    bool arm_request_76 = false;       // +76h (task+46Eh)
    int weapon_select_78 = depth_charge::kWeaponSelectNone;   // +78h
    // +7Ch: the target-reference sub-object. Its sub+18h (approach+94h,
    // task+48Ch) is the target, sub+1Ch (approach+98h) the world aim point
    // vt[0] 009A35A0 returns, sub+44h (approach+C0h) the projtime.
    ApproachTargetRefState ref;
    std::array<float, 3> release_point_f0{{0.0f, 0.0f, 0.0f}};   // +F0h..+F8h
};

struct DepthChargeAttackRunState {   // task+67Ch, ctor 009A36A0
    float period_18 = 1.0f;          // +18h, 0.4 after the enter 009A3750
    float countdown_1c = 0.0f;       // +1Ch
    float offset_20 = 0.0f;          // +20h
};
struct DepthChargeGoAwayState {      // task+6A0h; the ctor leaves +18h..+20h unwritten
    float altitude_18 = 0.0f;        // +18h
    float standoff_1c = 0.0f;        // +1Ch
    float side_20 = 0.0f;            // +20h, the fly-to solver's side
};
struct DepthChargeAimState {         // task+6C4h
    bool pull_out_18 = false;        // +18h (task+6DCh)
    bool byte_19 = false;            // +19h, cleared and never set in the window
    float cooldown_1c = 0.0f;        // +1Ch
};
struct DepthChargeLeaveState {       // task+784h
    float timer_18 = depth_charge::kLeaveTimerSeed;   // +18h (task+79Ch)
};
struct DepthChargeTurnToState {      // task+7A0h
    float bank_18 = 0.0f;            // +18h
};
// done (task+5DCh) and prepare (task+6E4h): BotStateFollow objects with the
// vtable 00D1F6FC; the follow base fields are the host's.
struct DepthChargeFollowState {
    float speed_8c = 1.0f;           // +8Ch, read by the follow base tick
    float gate_98 = -1.0f;           // +98h, [00D7A260]
    int salvo_9c = 0;                // +9Ch
};

// The whole task, as one value a host can keep per unit.
struct DepthChargeTask {
    DepthChargeState state = DepthChargeState::kMoveTo;   // +310h
    DepthChargeApproach ap;
    DepthChargeFollowState done;
    DepthChargeAttackRunState ar;
    DepthChargeGoAwayState ga;
    DepthChargeAimState aim;
    DepthChargeFollowState prepare;
    DepthChargeLeaveState lv;
    DepthChargeTurnToState tt;
    int task_2e4 = depth_charge::kWeaponSelectNone;   // task+2E4h = approach+78h after each arm
};

// ---------------------------------------------------------------------------
// What one state tick writes
// ---------------------------------------------------------------------------
// Each `writes_`/`calls_` flag says the field is stored (or the routine
// called) this tick; fields not flagged are untouched.
struct DepthChargePlanCommand {
    bool writes_heading_2c0 = false;   // plan+2C0h, plan+2CCh = 2
    float heading_2c0 = 0.0f;
    bool writes_bank_2c4 = false;      // plan+2C4h, plan+2CCh = 1
    float bank_2c4 = 0.0f;
    bool writes_turn_2c8 = false;      // plan+2C8h (aim tick only)
    float turn_2c8 = 0.0f;
    bool writes_pitch_2bc = false;     // plan+2BCh, plan+2D0h = pitch_mode_2d0
    float pitch_2bc = 0.0f;
    int pitch_mode_2d0 = 0;
    bool writes_speed_2b4 = false;     // plan+2B4h, +2B0h = 0, +2D8h = 1
    float speed_2b4 = 0.0f;
    bool writes_throttle = false;      // plan+278h, +27Ch = 1, +2A8h = 0, +2ACh = 1, +2D8h = 0
    float throttle_278 = 0.0f;
    // 009FBA50 (cruise_altitude_command_009fba50) with ECX = state+4.
    bool calls_cruise_altitude_009fba50 = false;
    float cruise_base = 0.0f, cruise_low = 0.0f, cruise_high = 0.0f, cruise_scale = 0.0f;
    // 009FB800 (pitch_command_009fb800) with (altitude, reference).
    bool calls_pitch_command_009fb800 = false;
    float pitch_altitude = 0.0f, pitch_reference = 1.0f;
    bool writes_cone_40 = false;       // gun+40h
    float cone_40 = 0.0f;
    // 009FABE0(gun block, heading, pitch). The pitch is always 0099B630(plan).
    // The heading is 009A1A20(plan) when heading_from_plan, else gun_heading.
    bool calls_gun_direction_009fabe0 = false;
    bool gun_heading_from_plan = false;
    float gun_heading = 0.0f;
};

// ---------------------------------------------------------------------------
// The host
// ---------------------------------------------------------------------------
struct DepthChargeLeaderView {        // [ctl+3D0h], the squadron's slot-0 plane
    bool present = false;
    bool is_self = false;
    std::array<float, 3> position{{0.0f, 0.0f, 0.0f}};   // 00427EB0 (pose refreshed)
    float heading_50 = 0.0f;          // leader vt[50h]
    std::uint8_t arm_9c1 = 0;         // leader+9C1h + [00F876B8] * 8
};

struct DepthChargeControlView {       // the pilot control block, ctl = [approach+0Ch]
    int attack_mode_370 = 0;          // ctl+370h
    float alt_394 = 0.0f;             // ctl+394h
    bool flag_3ad = false;            // ctl+3ADh
    bool alt_loadout = false;         // 007B58D0: ctl+369h && [00E17BF2]
};

struct DepthChargeCruiseProfileWrite;

struct DepthChargeTaskHost {
    virtual ~DepthChargeTaskHost() = default;
    virtual DepthChargeTuning tuning() = 0;
    virtual DepthChargeRow robot_row() = 0;      // depth_charge_row(the unit's robots row)
    virtual DepthChargeClass class_block() = 0;
    virtual DepthChargeControlView control() = 0;
    virtual DepthChargeLeaderView leader() = 0;
    virtual std::array<float, 3> unit_position() = 0;   // unit+FCh..104h, pose refreshed
    virtual std::array<float, 3> unit_velocity_34() = 0;// unit vt[34h]
    virtual float unit_speed_38() = 0;           // unit vt[38h]
    virtual float unit_heading_50() = 0;         // unit vt[50h]
    virtual bool bomber_kind() = 0;              // unit vt[5Ch](10h) || (16h)
    virtual bool flight_leader_007b8ad0() = 0;   // unit+9D8h == 0
    virtual bool has_depth_charge_ordnance_007b94f0() = 0;
    virtual int remaining_rounds_007c1db0() = 0;
    virtual int parity_f876b0() = 0;             // [00F876B0]
    // target + ctl+54h * 34h + 1E8h: the +8h count when its +10h byte is set,
    // else the +4h count. The approach tracks a target seen at least once.
    virtual int target_spotted_count_1e8() = 0;
    virtual bool target_marked_5d() = 0;         // target+5Dh
    // 009FADA0 on ap.ref with dt (approach_target_ref.hpp); returns sub+1Ch..+24h
    // and whether sub+18h (the target) is still set.
    virtual bool update_target_ref_009fada0(ApproachTargetRefState& ref, float dt,
                                            std::array<float, 3>& point) = 0;
    virtual void pick_target_ref_009fa260(ApproachTargetRefState& ref) = 0;
    // 009FB5E0(ref, 0): sub+18h = 0 through 009FB190, with the observer unhooked.
    virtual void clear_target_ref_009fb5e0(ApproachTargetRefState& ref) = 0;
    // 007F0280 in mode 1, ECX = ctl.
    virtual NearFieldProbeResult near_field_probe_007f0280(const float extents[3],
                                                           const float weights[3]) = 0;
    // 009FD570 with ECX = the goaway state: (point, unit, standoff, &side, range, 1.0).
    virtual float fly_to_heading_009fd570(const std::array<float, 3>& point, float standoff,
                                          float& side, float range) = 0;
    // 009A19B0(plan): 0 without plan+2F0h, else 0099D0A0(plan, [plan+2F0h]+C68h).
    virtual float heading_diff_009a19b0() = 0;
    // 0099C230, the base break-off gate every slot +1Ch override calls first.
    virtual bool break_off_base_0099c230() = 0;
    // (unit+72Ch)->vt[38h]: the unit's own device asks for a drop.
    virtual bool device_drop_requested() = 0;
    // The shared formation states. moveto (task+508h) and follow (task+544h)
    // are the host's whole; done and prepare are follow objects whose base
    // enter 009BED80, exit 009BDE40 and tick 009C1FD0 are the host's.
    virtual void set_moveto_ranges_009bde80(float near_range, float far_range,
                                            float speed_range) = 0;
    virtual void enter_formation_state(DepthChargeState s) = 0;
    virtual void exit_formation_state(DepthChargeState s) = 0;
    virtual void tick_formation_state(DepthChargeState s, float dt, float speed_8c) = 0;
    virtual void set_control_3e4(int value) = 0;           // ctl+3E4h
    virtual void assign_formation_indices_007ed260() = 0;  // ECX = ctl
    // The sinks.
    virtual void apply_plan(const DepthChargePlanCommand& c) = 0;
    virtual void release_ordnance_007bbba0() = 0;
    // The profile's ctl writes, stored before its 009A5F50 call reads ctl+394h.
    virtual void apply_cruise_profile_009a6500(const DepthChargeCruiseProfileWrite& w) = 0;
};

// ---------------------------------------------------------------------------
// The whole task
// ---------------------------------------------------------------------------
// 009A6970 -> 009A5240 -> 009A4DC0 -> 009A3390 (009F9CE0 head, 009FB200 ref,
// 009A31A0, 009A3300, 009A3290), the state seeds, then the initial state's
// enter. Draw order: 30, 34, 38, 40, ref60, bx, bz, [the 009FA260 pick], 6c,
// ar1c. The task base 0099C6F0 and the follow constructors are the host's.
void depth_charge_task_construct_009a5240(DepthChargeTask& t, DepthChargeTaskHost& host,
                                          DepthChargeRandom& rng);

struct DepthChargeArmResult {
    DepthChargeState before = DepthChargeState::kMoveTo;
    DepthChargeState after = DepthChargeState::kMoveTo;
    int transitions = 0;
    int releases = 0;                 // 007BBBA0 calls this think
    DepthChargePlanCommand plan;      // the current state's tick, also handed to apply_plan
};
// 009A66C0 (task vtable +64h, RET 4).
DepthChargeArmResult depth_charge_task_arm_009a66c0(DepthChargeTask& t, DepthChargeTaskHost& host,
                                                    DepthChargeRandom& rng, float dt);

// 009A6500 (slot +54h), ACTIVATION ONLY. For the flight leader the two
// one-shot altitude writes; for every unit +44h = max(+44h, AttackDist * +24h)
// and 009A5F50 with dt = 0. The host then runs 0099B740 (abandon if stale).
struct DepthChargeCruiseProfileInputs {
    bool ctl_38d = false, ctl_38c = false;   // already-overridden flags
    float ctl_380 = 0.0f, ctl_37c = 0.0f;
    bool ctl_3a9 = false, ctl_3aa = false;   // one-shots, always cleared
};
struct DepthChargeCruiseProfileWrite {
    bool leader = false;
    bool write_394 = false;          // ctl+394h = CruisingAlt, ctl+3ADh = 1
    bool write_398 = false;          // ctl+398h = AimAltRange/2, ctl+3ADh = 1
    bool clear_3a9 = false;          // ctl+3A9h = 0 (leader, +38Dh clear)
    bool clear_3aa = false;          // ctl+3AAh = 0 (leader, +38Ch clear)
    float value_394 = 0.0f;
    float value_398 = 0.0f;
};
DepthChargeCruiseProfileWrite depth_charge_cruise_profile_009a6500(
    DepthChargeTask& t, DepthChargeTaskHost& host, const DepthChargeCruiseProfileInputs& in);

// ---------------------------------------------------------------------------
// The pieces (each one native routine; the arm composes them)
// ---------------------------------------------------------------------------
// 009A31A0: the ref bias U(+-H, 0, +-V), the spread, the pick, +6Ch.
void depth_charge_pick_aim_offset_009a31a0(DepthChargeApproach& ap, const DepthChargeRow& row,
                                           DepthChargeTaskHost& host, DepthChargeRandom& rng);
// 009A3300: +64h = (0.15 - min(aim y, 0) * 0.02) + 007BCC80(unit y).
float depth_charge_fall_lead_009a3300(float aim_point_y, float unit_y,
                                      float unit_velocity_y) noexcept;
// 009A3290: +58h = 0.5 * max(rounds * 0.4, 0.5) * +30h.
float depth_charge_stick_half_009a3290(int remaining_rounds, float run_speed_30) noexcept;
// 007B5350: sub+44h = clamp(x, 0, 30).
float depth_charge_projtime_007b5350(float x) noexcept;

// 009A5F50 (RET 4), whole.
void depth_charge_approach_update_009a5f50(DepthChargeApproach& ap, DepthChargeTaskHost& host,
                                           float dt);

// 009A5870 with 009A57D0 and the inline SetState; the vt[1Ch] and turnto/goaway
// predicates are evaluated only on the branch that reads them. Returns the
// number of transitions (0 or 1) and adds the drops a done/prepare exit made.
int depth_charge_rule_009a5870(DepthChargeTask& t, DepthChargeTaskHost& host,
                               DepthChargeRandom& rng, int& releases);

// 009A65F0 (slot +1Ch).
bool depth_charge_should_break_off_009a65f0(const DepthChargeTask& t, DepthChargeTaskHost& host);
// 009A5610 (turnto -> aim), 009A5390 (turnto -> goaway), 009A53B0 (goaway done).
bool depth_charge_turnto_aim_ready_009a5610(float heading_diff_009a19b0, float rel_bearing_50) noexcept;
bool depth_charge_turnto_give_up_009a5390(float range_48, float fly_above_40) noexcept;
bool depth_charge_goaway_done_009a53b0(const DepthChargeGoAwayState& ga, float range_48,
                                       bool alt_loadout, bool has_ordnance_74) noexcept;
// 009A5D80 (slot +50h): 1 outside the attack states, else 2 with +76h set, 0 without.
int depth_charge_step_result_009a5d80(const DepthChargeTask& t) noexcept;
// 009A5780 (slot +20h): 0 while the target is live, engageable and inside
// CancelTargetDist with depth charges aboard, else 1.
struct DepthChargeTargetFlags {
    bool present = false, flag_5c = false, flag_5d = false, flag_5e = false, flag_60 = false;
};
int depth_charge_slot20_009a5780(const DepthChargeTask& t, const DepthChargeTargetFlags& f,
                                 float cancel_target_dist_0d8) noexcept;
// 009A5D00 (slot +24h): the unit's drop request; +2Ch += 1 and, in prepare,
// 009A5330. Returns 1.
int depth_charge_drop_request_009a5d00(DepthChargeTask& t, DepthChargeTaskHost& host,
                                       DepthChargeRandom& rng);
// 009A54C0 (slot +2Ch): +54h = 0. Returns 1.
int depth_charge_hit_notice_009a54c0(DepthChargeTask& t) noexcept;
// 009A5710 (slot +58h): drain +2Ch while the device asks, zero it, exit the state.
int depth_charge_flush_009a5710(DepthChargeTask& t, DepthChargeTaskHost& host);

// The state bodies. Each returns what it wrote to the plan.
void depth_charge_attackrun_enter_009a3750(DepthChargeAttackRunState& ar) noexcept;
DepthChargePlanCommand depth_charge_attackrun_tick_009a3770(DepthChargeTask& t,
                                                            DepthChargeTaskHost& host, float dt);
void depth_charge_leave_enter_009a39c0(DepthChargeLeaveState& lv, DepthChargeRandom& rng);
DepthChargePlanCommand depth_charge_leave_tick_009a39f0(DepthChargeTask& t, float dt) noexcept;
void depth_charge_turnto_enter_009a3ae0(DepthChargeTask& t, DepthChargeTaskHost& host);
DepthChargePlanCommand depth_charge_turnto_tick_009a3b70(DepthChargeTask& t,
                                                         DepthChargeTaskHost& host);
void depth_charge_goaway_enter_009a3c60(DepthChargeTask& t, DepthChargeTaskHost& host,
                                        DepthChargeRandom& rng);
float depth_charge_goaway_heading_009a3cf0(DepthChargeTask& t, DepthChargeTaskHost& host);
DepthChargePlanCommand depth_charge_goaway_tick_009a3e70(DepthChargeTask& t,
                                                         DepthChargeTaskHost& host);
void depth_charge_aim_enter_009a3fe0(DepthChargeTask& t, DepthChargeTaskHost& host,
                                     DepthChargeRandom& rng);
struct DepthChargeAimResult {
    DepthChargePlanCommand plan;
    bool released = false;
};
DepthChargeAimResult depth_charge_aim_tick_009a4010(DepthChargeTask& t, DepthChargeTaskHost& host,
                                                    DepthChargeRandom& rng, float dt);
// done/prepare (vtable 00D1F6FC): enter 009A4890, exit 009A48D0, tick 009A4940.
void depth_charge_follow_enter_009a4890(DepthChargeTask& t, DepthChargeState s,
                                        DepthChargeTaskHost& host);
bool depth_charge_follow_exit_009a48d0(DepthChargeTask& t, DepthChargeState s,
                                       DepthChargeTaskHost& host);
struct DepthChargeFollowResult {
    DepthChargePlanCommand plan;
    bool released = false;
};
DepthChargeFollowResult depth_charge_follow_tick_009a4940(DepthChargeTask& t, DepthChargeState s,
                                                          DepthChargeTaskHost& host, float dt);

}  // namespace bsp
