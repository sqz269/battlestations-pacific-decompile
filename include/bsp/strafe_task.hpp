#pragma once

// The strafe bot task (kind 0Ah, 6FCh bytes; factory 009CD300, constructor
// 009CC230, approach 009CC020 at task+3F8h). Evidence in
// docs/SQUADRON_LAND_TASK.md sections 5bw and 5bx. Descriptive names are
// hypotheses, not recovered symbols.
//
// Coverage: partial. Bound here: the state rule 009CC690 with its engaged
// entry 009CC5F0, the gotowards-ready test 009CC2F0, and the approach's
// attack-range update 009CCED0. Bound in the units host behind
// kStrafeTaskBound (packet cc9_strafe_arm, docs/SQUADRON_LAND_TASK.md 5ca):
// the arm 009CD170, the approach 009CA4A0 with its glide seed 009CA3B0, the
// state ticks (gotowards 009CA870, aim 009CB1B0, goaway 009CBB30, attackrun
// 009CADB0) and enters, the hit notice 009CC400 and the gun controller's read
// of the aim point. Behind kStrafeBreakoffBound (5cm): goaway's evasive task
// pushes (009BC030 / 009BC0A0). Not bound: the cruise profile 009CD020
// (cadence unread) and 009CA780's tail call 007B7870.

namespace bsp {

// The task's +310h values, named by the strings 009CC020 registers.
// Whole-object offsets.
enum class StrafeState : int {
    kNone = 0,
    kMoveTo = 0x4E0,     // "moveto (strafe)"
    kFollow = 0x51C,     // "follow (strafe)"
    kPrepare = 0x5B4,    // "strafe/prepare", a follow-state variant
    kGoTowards = 0x64C,  // "strafe/gotowards", vtable 00D20FE8
    kAim = 0x670,        // "strafe/aim", vtable 00D21020
    kGoAway = 0x690,     // "strafe/goaway", vtable 00D2103C
    kAttackRun = 0x6D8,  // "strafe/attackrun", vtable 00D21004
};

// The arm (009CD170), its states and the gun read in the units host. Flipped
// with the feed below as one group (nothing else chooses class 00E08F40);
// goaway and script dogfights are unmeasured (5cb).
inline constexpr bool kStrafeTaskBound = true;   // ON: SQUADRON_LAND_TASK 5cb

// 007EEC50's guns inputs in the PilotSetTarget choice (src/
// game_hosts_script_orders.cpp): guns_available = PilotFires (plane+C24h,
// 007EEB08 / 007EEBB7) and guns_suppressed = 0047B850 (kind 10h or 16h,
// 007EEB2C / 007EEBC2). It also lets PilotSetTarget choose dogfight against
// an aircraft. It must not be ON without kStrafeTaskBound.
inline constexpr bool kAttackChoiceGunsFedBound = true;   // ON: SQUADRON_LAND_TASK 5cb

// 009CC690's inputs. `ctl` is [task+404h] (approach+0Ch).
struct StrafeRuleInputs {
    StrafeState current = StrafeState::kNone;
    bool in_attack_range_448 = false;  // task+448h (approach+50h), 009CCED0
    bool strafe_target_44c = false;    // task+44Ch (approach+54h) != 0, a kind-41h target
    bool ref_target_468 = false;       // task+468h (approach+70h) != 0, the target ref's entity
    int control_mode_370 = 0;          // ctl+370h
    bool gotowards_ready = false;      // 009CC2F0 on the gotowards state
    bool goaway_done_6b4 = false;      // task+6B4h, goaway state byte +24h
    bool aim_done_68c = false;         // task+68Ch, aim state byte +1Ch
    bool aim_done_68d = false;         // task+68Dh, aim state byte +1Dh
    bool flight_leader = false;        // BSP_Unit_IsSquadronFlightLeader (007B8AD0)
};

// 009CC690. Returns the state +310h holds after the rule; equal to `current`
// when nothing changes. A change calls the old state's vt[8] and the new
// state's vt[4] (009CC5B0), which the caller performs.
StrafeState strafe_rule_009cc690(const StrafeRuleInputs& in) noexcept;

// 009CC2F0 on the gotowards state: its byte +20h (heading aligned, set by the
// tick 009CA870) and its +18h (3-D distance to the aim point) below
// 2 * [[approach+8]+188h] + approach+38h.
bool strafe_gotowards_ready_009cc2f0(bool aligned_20, float distance_18,
                                     float field_188, float approach_38) noexcept;

// 009CCED0's inputs and the approach fields it writes.
struct StrafeApproachUpdate {
    float elapsed_44 = 3600.0f;   // approach+44h, 009CA4A0 seeds [00CFDEB0] 3600
    float period_cc = 1.0f;       // approach+CCh, [00D7A24C]
    float countdown_d0 = 0.0f;    // approach+D0h, 009CA4A0 seeds -Random(0, 1)
    bool in_range_50 = false;     // approach+50h (task+448h)
};

struct StrafeRangeInputs {
    bool has_target = false;        // approach+54h, else approach+70h, non-null
    bool target_disabled_5d = false;// target+5Dh
    float unit_height_100 = 0.0f;   // [approach+4]+100h
    float aim_height = 0.0f;        // the aim point's y (approach vt[0] 009CA680: +78h)
    float glide_tan_30 = 1.0f;      // approach+30h, 009CA3B0
    float field_188 = 0.0f;         // [approach+8]+188h
    bool not_bomber_kind = true;    // 009CA310: neither IsKindOf(10h) nor (16h)
    float attack_dist_658 = 0.0f;   // tuning+658h Pilot/Strafe/AttackDist
    float horizontal_distance = 0.0f;// 009CAD00 to the aim point
};

// 009CCED0 without its first call (009FADA0, the target ref update, which the
// caller runs). Adds dt to +44h; runs the range test once per +CCh seconds.
void strafe_approach_update_009cced0(StrafeApproachUpdate& ap, float dt,
                                     const StrafeRangeInputs& in) noexcept;

// Packet cc9_strafe_breakoff (docs/SQUADRON_LAND_TASK.md 5cl, 5cm). True: at
// each opening of goaway's evasive gate (the tick 009CBC96-009CBE27 and the
// enter 009CB8FC-009CBB1E) the units host flies the manoeuvre the image
// front-pushes: "tightturn" (009BC030, tick 009BA020, ends at 00996510) when
// U(0, 1) >= the goaway state's +18h, else "flikflak" (009BC0A0 -> 009BB910,
// tick 009B99E0, ends at 00996300). The strafe arm is suspended while one
// runs and resumes when it ends. False: the gate is only counted.
inline constexpr bool kStrafeBreakoffBound = false;

// 009BAFC0's pitch reference for tightturn (009BB018-009BB12B, read from the
// listing). altitude = unit+100h, speed = unit vtable[38h], pitch = unit+C64h,
// climb_angle = desc+1ECh. Below 250 m the floor +400h is the climb angle;
// above it, -acos(r) with r = ((altitude - 250) / 10) / speed, 0 when r > 1
// (00BF9940 is taken as acos: math_acos 00A617C0 calls it; provisional).
// Returns +3FCh = max(+400h, pitch).
float tight_turn_pitch_ref_009bafc0(float altitude, float speed, float pitch,
                                    float climb_angle) noexcept;

// 009BA020's plan writes once started (009BC030 stores +404h = 1, so the start
// gate 009B9680 is never asked on this path). Plan offsets are the task's
// minus 4.
struct TightTurnCommand {
    float bank_target = 0.0f;    // +2C8h = side * pi/2, mode +2D0h = 1
    float pitch_desired = 0.0f;  // +2A0h = clamp(2 - |wrap(roll - bank)| / 10 deg, 0, 1), mode +2D4h = 0
    float yaw_desired = 0.0f;    // +288h = 00419010(-2 deg, 0, 2 deg, 1, ref - pitch) * side, mode +2D8h = 0
};
TightTurnCommand tight_turn_tick_009ba020(bool side_408, float roll_c68, float pitch_c64,
                                          float ref_3fc) noexcept;

// 00996510: the point in the plane's frame (x right, z forward, from the
// world inverse) lies more than 45 degrees off the nose: z < |x|.
bool point_off_nose_00996510(const float local[3]) noexcept;

// 00996300 (00996300-00996385): the horizontal distance squared from the
// plane to the point is no longer below dist2 (+28h).
bool point_distance_reached_00996300(float plane_x, float plane_z, float point_x,
                                     float point_z, float dist2) noexcept;

}  // namespace bsp
