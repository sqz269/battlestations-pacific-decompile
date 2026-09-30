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
// of the aim point. Not bound: the cruise profile 009CD020 (cadence unread),
// goaway's evasive task pushes (009BC030 / 009BC0A0, counted gaps) and
// 009CA780's tail call 007B7870.

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

// The arm (009CD170), its states and the gun read in the units host. OFF:
// the group flips only when strafe runs end to end (it needs the feed below,
// since nothing else chooses class 00E08F40).
inline constexpr bool kStrafeTaskBound = false;

// 007EEC50's guns inputs in the PilotSetTarget choice (src/
// game_hosts_script_orders.cpp): guns_available = PilotFires (plane+C24h,
// 007EEB08 / 007EEBB7) and guns_suppressed = 0047B850 (kind 10h or 16h,
// 007EEB2C / 007EEBC2). It also lets PilotSetTarget choose dogfight against
// an aircraft. OFF; it must not flip before kStrafeTaskBound.
inline constexpr bool kAttackChoiceGunsFedBound = false;

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

}  // namespace bsp
