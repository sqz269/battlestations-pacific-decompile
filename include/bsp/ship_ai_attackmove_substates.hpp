// The five attackmove sub-state steps and the three predicates that pick them.
//
// Packet cc_ai_attackmove_substates, worker agent/cc-ai-attackmove-substates.
// Project C:/Users/sqz269/bsp.gpr, program /battlestationspacific.exe; Ghidra
// was read-only for this packet. Every descriptive name here is a hypothesis,
// not a recovered symbol. docs/SHIP_AI_ATTACKMOVE_SUBSTATES.md carries the
// evidence, address by address.
//
// This header builds on bsp/ship_ai_state_steps.hpp (ShipAiAttackMoveSubState,
// the sub-state table, ShipAiAttackMoveSelector and its host, the 009DE050 goal
// setter) and on bsp/ship_ai_states.hpp; it redefines none of their types.
//
// Vocabulary, fixed by 009E8450 and reused in every routine below:
//   `state`   the attackmove state object at brain+0C18h.
//   `sub`     one sub-state object embedded in it. 009E8450 writes the SAME
//             owner into every sub-state's +4h (009E8473, 009E848B, 009E84A1,
//             009E84B1, 009E84C7), so `[sub+4h]` is the brain in all five.
//   `brain`   the ship AI brain. `blk` is brain+8h.
//   `unit`    [brain+0AA8h]; `shipclass` [brain+0AACh]; `target` [brain+0B20h].
//   The attackmove command's destination point is (brain+0B2Ch, brain+0B34h);
//   brain+0B30h is never read by any routine in this packet.
#pragma once

#include <cstdint>

#include "bsp/ship_ai_state_steps.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Shared 2D point. The image keeps the two goal floats adjacent on the stack
// and hands their address to 009DE050 as `goal2d`.
// ---------------------------------------------------------------------------
struct ShipAiAttackMoveXZ {
    float x{0.0f};
    float z{0.0f};
};

// ---------------------------------------------------------------------------
// 00852860, the altitude gate the selector asks about a kind-8 target
// ---------------------------------------------------------------------------
//
// __thiscall(entity) -> bool, RET 0, body 00852860-008528AC, complete.
// Callers: 009E86F0 at 009E873B plus six HUD marker routines (00641910,
// 00641E30, 00642040, 00642C20, 00643360, 006434E0), so the contract is the
// union of those: a pure test on one entity, no argument.
//
// Body: if the pose byte at entity+0C8h is zero it calls 00414DB0 first
// (00852863, 0085286C), then compares the entity's world y at +100h against
// ([entity+1204h] + [entity+1200h]) / 3.0 (a double divide at 00852884).
// The tail at 00852892..008528AC is a logical NOT written out: `MOV EAX,1`
// then `TEST AL,AL; SETZ CL; MOV AL,CL` yields 0, and the XOR arm yields 1.
// So the routine returns TRUE on the JBE side, that is when y <= limit.
inline constexpr double kAttackMoveAltitudeGateDivisor = 3.0; // 00D7A2B0

// `ceiling_a` is entity+1204h and `ceiling_b` is entity+1200h. Neither field
// has a producer in the ledger, so their meaning is provisional; only the
// arithmetic is established.
bool ship_ai_attackmove_altitude_gate_00852860(float world_y, float ceiling_a,
                                               float ceiling_b);

// ---------------------------------------------------------------------------
// 009E85B0, the approach -> engage gate
// ---------------------------------------------------------------------------
//
// __thiscall(sub) -> bool, RET 0, body 009E85B0-009E86B5, complete. Its only
// caller is 009E86F0 at 009E87CD, with ECX = state+8h, and the selector runs it
// only while the current member is already state+8h; a true answer moves the
// machine to state+14C0h (009E87E3).
//
// Three conjuncts, in body order:
//   1. 009E85B9  [brain+0AA8h] non-zero, and one of the two floats at
//      [[unit+538h]+510h] / +514h strictly greater than 0.0f (009E85D8,
//      009E85E5). The object at unit+538h also answers vtable[2Ch] for
//      009F3240 and carries a group id at +570h; calling it the unit's
//      armament component is a hypothesis, not a recovered name.
//   2. 009E8658  the destination point (brain+0B2Ch, brain+0B34h) must not lie
//      inside any avoid zone: 0082ADC0 fetches the zone list for that
//      component's +570h from the avoid-zone singleton (004218E0, 004120D0) and
//      004178F0 walks it, returning the first zone that answers 00416B50 for
//      the point, or 0 for none. A non-zero answer fails the gate.
//   3. 009E8698  the squared XZ distance from the unit's refreshed world
//      position (unit+0FCh, unit+104h) to that destination must be strictly
//      less than 4000000.0, that is the unit within 2000 units of the point.
inline constexpr double kAttackMoveEngageGateRangeSq = 4000000.0; // 00D09FE8

struct ShipAiAttackMoveEngageGateHost {
    virtual ~ShipAiAttackMoveEngageGateHost() = default;
    // 009E85B9, [brain+0AA8h]. Zero fails the gate at 009E85C1.
    virtual std::uint32_t brain_unit_0aa8() = 0;
    // 009E85CD, the two floats at [[unit+538h]+510h] and +514h.
    virtual void armament_readiness_0510(float& a, float& b) = 0;
    // 009E85F5 and 009E8600, the unit pose-valid byte and 00414DB0.
    virtual bool unit_pose_valid_00c8() = 0;
    virtual void refresh_unit_pose_00414db0() = 0;
    // 009E8605 and 009E861E, the unit's world x at +0FCh and z at +104h.
    virtual void unit_position_xz_00fc(float& x, float& z) = 0;
    // 009E8610 and 009E862C, the destination (brain+0B2Ch, brain+0B34h).
    virtual void brain_destination_0b2c(float& x, float& z) = 0;
    // 009E864C then 009E8658: 0082ADC0 on [unit+538h] gives the zone list for
    // its group id at +570h, and 004178F0 returns the first zone containing the
    // point (0 for none). Both bodies read; the zone-membership leaf 00416B50
    // was not read, so "contains" is the caller's use, not that body's contract.
    virtual std::uint32_t avoid_zone_containing_004178f0(float x, float z) = 0;
};

bool ship_ai_attackmove_engage_gate_009e85b0(ShipAiAttackMoveEngageGateHost& host);

// ---------------------------------------------------------------------------
// state+8h, the approach sub-state: 009E5CA0 / 009E5530 construct, 009F3240 steps
// ---------------------------------------------------------------------------
//
// 009E5CA0: __thiscall(sub)(owner), RET 4, body 009E5CA0-009E5CEC, complete.
// Stores the owner at sub+4h (009E5CC1) and vtable 00D21994 at sub+0h
// (009E5CD0), then constructs a nested object at sub+8h (= state+10h) through
// 009E5530 (009E5CC5, 009E5CD6). The nested object is what 009F3240 steps each
// frame through 009F3090.
//
// 009E5530: __thiscall(nested)(owner), RET 4, body 009E5530-009E5769, complete.
// It stores the owner at nested+0h (009E5540) and then fills a 60-entry array
// of 0x4Ch-byte records starting at nested+4h in two passes.
inline constexpr int kAttackMoveRingSlotCount = 60;           // 009E5542, 009E5751
inline constexpr std::uint32_t kAttackMoveRingSlotStride = 0x4Cu; // 009E55B6, 009E574E
inline constexpr double kAttackMoveRingFullTurn = 6.2831854820251465; // 00CE3828
inline constexpr double kAttackMoveRingQuarterTurn = 1.5707963705062866; // 00CE3830
inline constexpr double kAttackMoveRingWrapLow = -3.1415927410125732;  // 00CE3D18
inline constexpr double kAttackMoveRingWrapHigh = 3.1415927410125732;  // 00CE3D28
inline constexpr double kAttackMoveRingDivisor = 60.0;   // 00CE3D68
inline constexpr float kAttackMoveRingScoreReset = 1000.0f; // 00CE3804, record +44h
inline constexpr float kAttackMoveRingJitterHigh = 2.0f;    // 00CE3958, record +48h

// One ring record, as the two construction passes write it. Offsets are from
// the record base, which is nested + 4h + index * 4Ch.
struct ShipAiAttackMoveRingSlot {
    std::uint32_t unit_00{0};    // +00h, [owner+0AA8h]      (009E5692)
    std::uint32_t nested_04{0};  // +04h, the nested object  (009E5695)
    float angle_08{0.0f};        // +08h, the slot's bearing (009E56F9)
    float dir_x_0c{0.0f};        // +0Ch, cos of the heading (009E5734)
    float dir_y_10{0.0f};        // +10h, always 0.0f        (009E573C)
    float dir_z_14{0.0f};        // +14h, sin of the heading (009E574A)
    // Pass one zeroes +00h and +18h..+3Ch and the byte at +40h, then writes
    // +44h = 1000.0f (009E55B1) and +48h = a random draw in [0, 2) taken from
    // stream 1 through 00BD2F10 (009E55A1). Those two are the only non-zero
    // members of the first pass; the rest are left as zero here because the
    // packet did not establish what reads them.
    float reset_44{kAttackMoveRingScoreReset};
    float jitter_48{0.0f};
};

// The pure part of the second pass, for slot `index` in [0, 60).
// 009E5680..009E5758, read instruction by instruction:
//   a = fmod(index * 2*pi / 60, 2*pi)                009E5680..009E56AA
//       (00BF857A is _CIfmod, which computes fmod(ST(1), ST(0)); the operands
//        are put in that order by the FXCH at 009E56A8.)
//   a = a + 2*pi if a <= -pi else a - 2*pi if a > pi   009E56BF..009E56E5
//   slot.angle_08 = a                                 009E56F9
//   h = pi/2 - a; h += 2*pi while h < 0                009E56ED..009E5712
//   slot.dir = (cos h, 0, sin h)                       009E571E, 009E572E
// The wrap is the same idiom 00605070 uses on brain+1E0h, so the ring is 60
// bearings one sixth of a radian apart around the owner, each with a unit XZ
// direction in the game's from-+Z heading convention.
ShipAiAttackMoveRingSlot ship_ai_attackmove_ring_slot_009e5530(int index,
                                                               std::uint32_t unit,
                                                               std::uint32_t nested);

// 009F3240: __thiscall(sub)(float seconds), RET 4, body 009F3240-009F3664,
// complete. vtable 00D21994 slot +0Ch.
//
// The throttle the step pushes into brain+258h and brain+2C0h, from
// 009F3294..009F32EB. `depth_ref` is [unit+494h] and `bias` is sub+11E8h,
// which 009E5530 seeds to 0 at nested+11E0h (009E561F writes nested+11E8h;
// sub+11E8h is nested+11E0h, so the seed is the zero store at 009E55F7 region).
// The producer of sub+11E8h during play was not established: treat it as an
// input, not as a constant.
inline constexpr double kAttackMoveApproachDepthOffset = 500.0;  // 00CE3840
inline constexpr double kAttackMoveApproachThrottleKnee = 1000.0; // 00CE47A0
inline constexpr float kAttackMoveApproachThrottleCap = 1000.0f;  // 00CE3804
float ship_ai_attackmove_approach_throttle_009f3240(float depth_ref, float bias);

// 009F3603..009F3635, the value written to brain+1D8h on every arm: sub+1218h
// clamped to [-1, 1]. The comparison order is `COMISS -1.0f, v` (009F3618) then
// `COMISS v, 1.0f` (009F362D), and both are JA, so a NaN input takes neither
// branch and reaches the MOVAPS at 009F3632 unchanged. This rule reproduces
// that, which is why it is not written as a std::clamp.
inline constexpr float kAttackMoveApproachCommandLow = -1.0f; // 00D7A260
inline constexpr float kAttackMoveApproachCommandHigh = 1.0f; // 00D7A24C
float ship_ai_attackmove_approach_command_009f3240(float raw);

// 009F34E4..009F3585, the two gates of the warn sweep, kept apart because the
// image runs the pose refresh between them (009F3521).
// The speed gate at 009F34EA..009F3510 divides 0092D730 by 0080FC30 in double
// and passes only while the ratio is under 0.4.
inline constexpr double kAttackMoveApproachSweepSpeedRatio = 0.4000000059604645; // 00CE65D0
bool ship_ai_attackmove_approach_sweep_speed_gate_009f3240(float speed,
                                                           float reference_speed);

// The range gate at 009F3526..009F3585. The radius is the INTEGER field at
// [target+7C4h], squared with IMUL at 009F3546 and converted with FILD at
// 009F357D, so the comparison is against (float)(r*r) computed in 32-bit
// integer arithmetic, not against a float radius squared.
bool ship_ai_attackmove_approach_sweep_range_gate_009f3240(float candidate_x,
                                                           float candidate_z,
                                                           float target_x,
                                                           float target_z,
                                                           std::int32_t target_radius_07c4);

struct ShipAiAttackMoveApproachHost {
    virtual ~ShipAiAttackMoveApproachHost() = default;
    // 009F3262, [brain+0B20h]; 009F3277, the byte at target+5Dh. Either a null
    // target or a non-zero byte takes the hold arm.
    virtual std::uint32_t brain_target_0b20() = 0;
    virtual bool target_retired_005d(std::uint32_t target) = 0;
    // 009F3647, 009E00A0 BSP_ShipAi_HoldHeadingAndStop with ECX = sub+4h, the
    // ADDRESS of the owner field, which is what that routine dereferences.
    virtual void hold_heading_and_stop_009e00a0() = 0;
    // 009F328F, 009F3090(nested at sub+8h, seconds): __thiscall, RET 4, body
    // 009F3090-009F30E3. Seven calls on the nested object in a fixed order:
    // 009F1BC0(seconds), 009E7FC0(), 009E6E80(), 009E9190(seconds),
    // 009E74D0(seconds), 009E76D0(seconds), 009E6A90(). None of those seven
    // bodies was read in this packet, so the nested update is one opaque step
    // here; it is the follow-up packet ship_ai_attackmove_ring_update.
    virtual void nested_update_009f3090(float seconds) = 0;
    // 009F32A0 and 009F32A6, [brain+0AA8h] then [unit+494h].
    virtual float unit_depth_reference_0494() = 0;
    // 009F3294, sub+11E8h.
    virtual float sub_throttle_bias_11e8() = 0;
    // 009F32EB, 009F3300, 009F3314, 009F339A: sub+1238h, sub+1230h, sub+1214h
    // and sub+1218h. The producer of these four is the nested update, not this
    // body; 009E5530 seeds the corresponding nested fields to zero.
    virtual void sub_goal_1230(float& x, float& z) = 0;
    virtual float sub_heading_command_1214() = 0;
    virtual float sub_throttle_command_1218() = 0;
    // 009F332B, 009DE050(blk, &goal, keep_mode = 1, final_leg = 0). The PUSH 1
    // at 009F3308 is keep_mode and the PUSH 0 at 009F32F5 is final_leg; see the
    // doc's Corrections heading.
    virtual void set_navigation_goal_009de050(const ShipAiAttackMoveXZ& goal,
                                              int keep_mode, int final_leg) = 0;
    // 009F3335..009F3348 and 009F336B..009F3383: when brain+1CCh is not 3 the
    // step first clears brain+370h and brain+368h, then writes brain+1E0h,
    // brain+1D4h = 0, brain+1CCh = 3 and the throttle into brain+258h/2C0h.
    virtual int brain_steering_mode_01cc() = 0;
    virtual void clear_brain_turn_accumulators_0368() = 0;
    virtual void set_brain_heading_01e0(float heading) = 0;
    // 009F3360, 00605070 with ECX = brain+1E0h: __thiscall(float*), RET 0, body
    // 00605070-006050BF. Wraps the float in place into (-pi, pi] with _CIfmod.
    virtual void wrap_brain_heading_00605070() = 0;
    // 009F336B and 009F363D write brain+1D4h; 009F3375 writes brain+1CCh;
    // 009F361C writes brain+1D0h. Three separate dwords, three setters.
    virtual void set_brain_replan_01d4(int value) = 0;
    virtual void set_brain_steering_mode_01cc(int mode) = 0;
    virtual void set_brain_goal_hold_01d0(int value) = 0;
    // 009F337B and 009F3383, brain+258h and brain+2C0h, both the same float.
    virtual void set_brain_throttle_0258(float throttle) = 0;
    // 009F338B and 009F33A8, the sweep timer at sub+14B4h. The sweep runs only
    // when it goes strictly negative (009F33E2), and 009F340B resets it to 1.0f.
    virtual float sub_sweep_timer_14b4() = 0;
    virtual void set_sub_sweep_timer_14b4(float seconds_left) = 0;
    // 009F33BF, target->vtable[5Ch](1Ch): a second entity-kind predicate, the
    // same slot the composite step uses with 9. Callee body unread.
    virtual bool target_is_kind_vtable_005c(std::uint32_t target, int kind) = 0;
    // 009F33FF and 009F3402, [unit+54h] against [target+54h]: the sweep is
    // skipped when they match. The field is the same one the engage step copies
    // into the avoidance request, so it identifies a side or an owner.
    virtual bool unit_and_target_share_side_0054(std::uint32_t target) = 0;
    // 009F3429, 00778890(unit): MOV EAX,[ECX+284h]; returns [group+14h] == unit,
    // that is true only for the group leader. Body read (00778890-007788A7).
    virtual bool unit_is_group_leader_00778890() = 0;
    // 009F343E and 009F3444, the group at unit+284h and its count at +4F8h;
    // 009F3457, 0070D060(group, i); 009F346B, member->vtable[5Ch](6);
    // 009F347E, [member+538h]->vtable[2Ch](). Callee bodies of the two virtuals
    // were not read.
    virtual int group_member_count_04f8() = 0;
    virtual std::uint32_t group_member_at_0070d060(int index) = 0;
    virtual bool member_is_kind_vtable_005c(std::uint32_t member, int kind) = 0;
    virtual bool member_armament_ready_vtable_002c(std::uint32_t member) = 0;
    // 009F35D8, the non-leader arm: the same +538h vtable[2Ch] on the unit, and
    // 009F35F1 puts the unit itself into the one-entry candidate list.
    virtual bool unit_armament_ready_vtable_002c() = 0;
    virtual std::uint32_t brain_unit_0aa8() = 0;
    // 009F34B3, 00427EB0(target) BSP_EntityPose_GetWorldPositionRefreshed; the
    // step keeps components 0 and 2.
    virtual void target_position_xz_00427eb0(std::uint32_t target, float& x, float& z) = 0;
    // 009F34EA and 009F34FD, 0092D730([candidate+1018h]) and 0080FC30(candidate).
    virtual float candidate_body_speed_0092d730(std::uint32_t candidate) = 0;
    virtual float candidate_reference_speed_0080fc30(std::uint32_t candidate) = 0;
    // 009F3516, 009F3521 and 009F3526: the candidate's pose byte, 00414DB0, and
    // its world x/z at +0FCh/+104h.
    virtual bool candidate_pose_valid_00c8(std::uint32_t candidate) = 0;
    virtual void refresh_candidate_pose_00414db0(std::uint32_t candidate) = 0;
    virtual void candidate_position_xz_00fc(std::uint32_t candidate, float& x, float& z) = 0;
    // 009F3534, the integer at target+7C4h.
    virtual std::int32_t target_warn_radius_07c4(std::uint32_t target) = 0;
    // 009F3592, candidate->vtable[234h](target). Callee body unread.
    virtual bool candidate_accepts_warning_vtable_0234(std::uint32_t candidate,
                                                       std::uint32_t target) = 0;
    // 009F359C then 009F35B3: 0064A820 builds a message object on the stack and
    // 0077C2A0 routes it with ECX = candidate and the two constants 2 and 0.
    // Neither body was read in this packet.
    virtual void route_warning_message_0077c2a0(std::uint32_t candidate) = 0;
    // 009F3635, brain+1D8h = the clamped command.
    virtual void set_brain_command_01d8(float command) = 0;
};

void ship_ai_attackmove_approach_step_009f3240(float seconds,
                                               ShipAiAttackMoveApproachHost& host);

// ---------------------------------------------------------------------------
// state+14C0h, the engage sub-state: 009E23B0 steps
// ---------------------------------------------------------------------------
//
// __thiscall(sub)(float seconds), RET 4, body 009E23B0-009E26BD, complete,
// read from the listing. vtable 00D2174C slot +0Ch. `seconds` is never used.
//
// The sub-state's only storage is the byte at sub+8h, a two-mode latch:
//   mode 0, "close": brain+3FCh = 1, a navigation goal at the intercept point,
//           and the latch flips to 1 once the plan accepts the goal and the
//           range is under 250 (009E2691, 009E26A1, 009E26A3).
//   mode 1, "run":   brain+3FCh = 0, a direct heading through 009DFF40 and
//           brain+0AF0h = 1.0f. The latch falls back to 0 as soon as the range
//           exceeds 300 (009E2483..009E24B2).
struct ShipAiAttackMoveEngageState {
    bool attack_run_08{false}; // sub+8h
};

inline constexpr float kAttackMoveEngageRunExitRange = 300.0f;   // 00CE3AE8
inline constexpr double kAttackMoveEngageRunEnterRange = 250.0;  // 00CF8850
inline constexpr double kAttackMoveEngageRangeEpsilonSq = 1e-10; // 00CE3820
inline constexpr float kAttackMoveEngageClosingFloor = 0.4000000059604645f; // 00CE7804
inline constexpr double kAttackMoveEngageClosingGate = 0.4000000059604645;  // 00CE65D0
inline constexpr double kAttackMoveEngageLeadBias = 2.0;   // 00D7A308
inline constexpr float kAttackMoveEngageLeadCap = 12.0f;   // 00CEB4B8

// 009E2429..009E2547 and 009E2641..009E265E, the intercept solve, in listing
// order. `closing` is the relative velocity (unit minus target) projected on
// the unit-to-target unit vector; when it is below 0.4 the divisor is pinned to
// 0.4 (009E250F selects between the measured value and 00CE7804, both 0.4, so
// the branch is a floor, not a switch). The lead time is then
// range / closing - 2.0, floored at 0 and capped at 12.
float ship_ai_attackmove_engage_lead_time_009e23b0(float range, float closing);

struct ShipAiAttackMoveEngageHost {
    virtual ~ShipAiAttackMoveEngageHost() = default;
    // 009E23BE, [brain+0B20h]. Null takes 009E26B0: 009E00A0 with ECX = sub+4h
    // and nothing else.
    virtual std::uint32_t brain_target_0b20() = 0;
    virtual void hold_heading_and_stop_009e00a0() = 0;
    // 009E23D0/009E23DB and 009E23F1/009E2410, the two pose guards.
    virtual bool target_pose_valid_00c8() = 0;
    virtual void refresh_target_pose_00414db0() = 0;
    virtual bool unit_pose_valid_00c8() = 0;
    virtual void refresh_unit_pose_00414db0() = 0;
    // 009E23E2/009E23FE and 009E2415/009E241F, the two world positions.
    virtual void target_position_xz_00fc(float& x, float& z) = 0;
    virtual void unit_position_xz_00fc(float& x, float& z) = 0;
    // 009E24CA and 009E24DB, target->vtable[34h]() and unit->vtable[34h]().
    // MSVC hidden-return-buffer ABI: the pushed pointer is the 12-byte result
    // slot and EAX comes back holding it. Components 0 and 2 are used. The
    // callee bodies were not read, so "velocity" is this packet's reading of
    // the use at 009E24EF, not a recovered contract.
    virtual void target_velocity_xz_vtable_0034(float& x, float& z) = 0;
    virtual void unit_velocity_xz_vtable_0034(float& x, float& z) = 0;
    // 009E2588, brain+3F8h = [unit+54h]. Written on both arms.
    virtual std::int32_t unit_side_0054() = 0;
    virtual void set_avoidance_side_03f8(std::int32_t side) = 0;
    // 009E25CC and 009E2669, brain+3FCh = 0 on the run arm and 1 on the close
    // arm. This is the ShipAiAvoidanceRequest::flag_3fc the stop step writes.
    virtual void set_avoidance_flag_03fc(bool enable) = 0;
    // 009E267B, 009DE050(blk, &intercept, keep_mode = 0, final_leg = 0).
    virtual void set_navigation_goal_009de050(const ShipAiAttackMoveXZ& goal,
                                              int keep_mode, int final_leg) = 0;
    // 009E268A, 009DA610(blk, &intercept): __thiscall(blk)(const float*), RET 4,
    // body 009DA610-009DA66F, read. False unless blk+2FDh is set and the point
    // is within 2500 squared units of the stored plan point at blk+1DCh/1E0h;
    // on the far side it also clears blk+2FDh.
    virtual bool plan_accepts_goal_009da610(const ShipAiAttackMoveXZ& goal) = 0;
    // 009E2620, 009DFF40(brain, heading). ECX is the brain itself here, not
    // sub+4h, which is what that routine's own body expects.
    virtual void set_heading_and_drop_path_009dff40(float heading) = 0;
    // 009E262F, brain+0AF0h = 1.0f, run arm only. No consumer of brain+0AF0h
    // was found in this packet; the name is provisional.
    virtual void set_brain_speed_scale_0af0(float scale) = 0;
};

// Returns the latch as the step leaves it.
bool ship_ai_attackmove_engage_step_009e23b0(ShipAiAttackMoveEngageState& state,
                                             ShipAiAttackMoveEngageHost& host);

// ---------------------------------------------------------------------------
// state+14CCh, the lead-pursuit sub-state: 009E26C0 steps
// ---------------------------------------------------------------------------
//
// __thiscall(sub)(float seconds), RET 4, body 009E26C0-009E2B58, complete.
// vtable 00D2177C slot +0Ch. The selector picks it at 009E8756 when
// brain+0B28h is set and the kind-8 altitude gate passed.
struct ShipAiAttackMoveLeadPursuitState {
    float turn_budget_08{0.0f};  // sub+8h,  accumulates |yaw rate * seconds|
    bool budget_mode_0c{false};  // sub+0Ch, the two-mode flag
    float arm_distance_10{0.0f}; // sub+10h, scaled by 1.5 on each arming
};

inline constexpr double kAttackMoveLeadTimeDivisor = -10.0; // 00D0A198
inline constexpr float kAttackMoveLeadTimeFloor = 3.0f;     // 00CE3854
inline constexpr double kAttackMoveLeadRadiusScale = 1.5;   // 00CE3D78
inline constexpr float kAttackMoveLeadRadiusFloor = 300.0f; // 00CE3AE8
inline constexpr float kAttackMoveLeadRadiusDirect = 200.0f; // 00CE386C
inline constexpr float kAttackMoveLeadHeadingErrorGate = 1.5f; // 00CE380C
inline constexpr float kAttackMoveLeadHeadingRateLimit = 1.0f; // 00D7A24C
inline constexpr float kAttackMoveLeadScaleNear = 1.0f;   // 00CE3990 pairs 10 deg
inline constexpr float kAttackMoveLeadScaleFar = 0.5f;    // 00CE3800 pairs 60 deg
inline constexpr float kAttackMoveLeadScaleNearAngle = 0.17453293f; // 00CE3990
inline constexpr float kAttackMoveLeadScaleFarAngle = 1.0471976f;   // 00D05AAC
inline constexpr double kAttackMoveLeadBudgetFullTurn = 6.2831854820251465; // 00CE3828

// 009E2757..009E27E1, the lead point: t = max(target_y / -10, 3) and the goal is
// the target position plus its velocity times t. The divide at 009E275F is a
// double divide whose quotient is rounded to float on the store at 009E276F.
ShipAiAttackMoveXZ ship_ai_attackmove_lead_point_009e26c0(float target_x,
                                                          float target_y,
                                                          float target_z,
                                                          float target_vx,
                                                          float target_vz);

// 009E284E..009E28E7, the arrival radius: max(1.5 * turn_radius, 300), forced to
// 200 whenever the sub-state is in direct mode or the absolute heading error is
// at least 1.5 radians.
float ship_ai_attackmove_lead_arrival_radius_009e26c0(float turn_radius,
                                                      float heading_error,
                                                      bool budget_mode);

struct ShipAiAttackMoveLeadPursuitHost {
    virtual ~ShipAiAttackMoveLeadPursuitHost() = default;
    // 009E26CE, [brain+0B20h]; 009E26DA, 009E00A0 with ECX = sub+4h.
    virtual std::uint32_t brain_target_0b20() = 0;
    virtual void hold_heading_and_stop_009e00a0() = 0;
    // 009E26EF/009E26FA, 009E26FF/009E2726 and 009E272B/009E2752.
    virtual bool unit_pose_valid_00c8() = 0;
    virtual void refresh_unit_pose_00414db0() = 0;
    virtual bool target_pose_valid_00c8() = 0;
    virtual void refresh_target_pose_00414db0() = 0;
    // 009E2706/009E2714 and 009E2732/009E2740/009E2757. The compiler hoisted
    // both position loads above their pose guards; the host keeps the source
    // order because the values are equal either way once the refresh has run.
    virtual void unit_position_xz_00fc(float& x, float& z) = 0;
    virtual void target_position_00fc(float& x, float& y, float& z) = 0;
    // 009E2773, target->vtable[34h](retbuf). Callee body unread.
    virtual void target_velocity_xz_vtable_0034(float& x, float& z) = 0;
    // 009E27EF, 009DB6C0(sub, &lead, 10.0f): clamps the pair into the world box
    // from [00E188A8] inset by 10. Body not read in this packet.
    virtual void clamp_to_world_box_009db6c0(ShipAiAttackMoveXZ& point, float inset) = 0;
    // 009E284E and 009E2B18, 0082E850 on [brain+0AACh].
    virtual float ship_class_turn_radius_0082e850() = 0;
    // 009E2890, 009E293C and 009E2993, unit->vtable[50h](): the current heading.
    virtual float unit_heading_vtable_0050() = 0;
    // 009E28A0 and 009E294C, 00438B10 wrap(a - b); 009E2999, 00438AA0 wrap(a + b).
    virtual float subtract_wrapped_angle_00438b10(float a, float b) = 0;
    virtual float add_wrapped_angle_00438aa0(float a, float b) = 0;
    // 009E29AC, 009E0040 BSP_ShipAi_SetDesiredHeading with ECX = sub+4h.
    virtual void set_desired_heading_009e0040(float heading) = 0;
    // 009E2A0E, 00419010 BSP_Math_InterpolateClamped(x0, y0, x1, y1, x).
    virtual float interpolate_clamped_00419010(float x0, float y0, float x1,
                                               float y1, float x) = 0;
    // 009E2A1F and 009E2A33, brain+0AF0h. Provisional, as in the engage step.
    virtual void set_brain_speed_scale_0af0(float scale) = 0;
    // 009E2A41, brain+1D0h = 1 on the navigation arm only.
    virtual void set_brain_goal_hold_01d0(int value) = 0;
    // 009E2A53, 009DE050(blk, &lead, keep_mode = 0, final_leg = 0).
    virtual void set_navigation_goal_009de050(const ShipAiAttackMoveXZ& goal,
                                              int keep_mode, int final_leg) = 0;
    // 009E2A85, unit->vtable[38h](): the forward speed; 009E2A6A, the ordered
    // rudder at unit+984h; 009E2A97, 0082ECB0 on the ship class, RET 0Ch.
    virtual float unit_forward_speed_vtable_0038() = 0;
    virtual float unit_ordered_rudder_0984() = 0;
    virtual float yaw_rate_from_rudder_0082ecb0(float rudder, float speed,
                                                float efficiency) = 0;
    // 009E2B0D, 00414C60 BSP_Vector2f_LengthWithCutoff on the delta pair.
    virtual float vector2_length_00414c60(float x, float z) = 0;
};

void ship_ai_attackmove_lead_pursuit_step_009e26c0(ShipAiAttackMoveLeadPursuitState& state,
                                                   float seconds,
                                                   ShipAiAttackMoveLeadPursuitHost& host);

// ---------------------------------------------------------------------------
// state+14E0h, the tangent-circle sub-state: 009F3670 steps
// ---------------------------------------------------------------------------
//
// __thiscall(sub)(float seconds), RET 4, body 009F3670-009F39B0, complete.
// vtable 00D217AC slot +0Ch. The selector picks it at 009E876C when
// brain+0B28h is clear and the kind-8 altitude gate passed.
struct ShipAiAttackMoveTangentState {
    float dwell_timer_08{0.0f};  // sub+8h,  the range captured on enter
    float budget_0c{0.0f};       // sub+0Ch, counted down by `seconds`
    float elapsed_10{0.0f};      // sub+10h, time in the sub-state
};

inline constexpr double kAttackMoveTangentReleaseDelay = 5.0;  // 00D7A370
inline constexpr double kAttackMoveTangentRadiusFloorD = 500.0; // 00CE3840
inline constexpr float kAttackMoveTangentRadiusFloor = 500.0f;  // 00CE397C
inline constexpr float kAttackMoveTangentJitterLow = 1.2f;      // 00CE3814
inline constexpr float kAttackMoveTangentJitterHigh = 1.5f;     // 00CE380C
inline constexpr double kAttackMoveTangentMinStepD = 300.0;     // 00CE3CA8
inline constexpr float kAttackMoveTangentMinStep = 300.0f;      // 00CE3AE8
inline constexpr float kAttackMoveTangentBoxInset = 10.0f;      // 00CE38B8
inline constexpr float kAttackMoveTangentHeadingEpsilonSq = 1.0f; // FLD1 at 009F3889
inline constexpr double kAttackMoveTangentSpeedScale = 1.5;     // 00CE3D78
inline constexpr float kAttackMoveTangentSwitchCapSq = 80000.0f; // 00D21A98
inline constexpr double kAttackMoveTangentReEnterSlack = 30.0;   // 00CE7630
inline constexpr double kAttackMoveTangentReEnterRange = 200.0;  // 00CE4D70
inline constexpr float kAttackMoveTangentGoalScale = 1.0f;  // 00D7A24C, nav arm
inline constexpr float kAttackMoveTangentHeadingScale = 0.5f; // 00CE3800, heading arm
// 009F370F pushes the same global the attackmove command getter 009E8540
// returns and the composite step hands to 0071E430 at 009E88C1.
inline constexpr std::uint32_t kAttackMoveTangentCommandObject = 0x00e08f78u;

// 009F3763..009F37C6: the circle radius is a random draw in [1.2, 1.5) from
// stream 1 times max(turn_radius, 500).
float ship_ai_attackmove_tangent_radius_009f3670(float turn_radius, float jitter);

// 009F38BA..009F3906: the arm switch. The goal arm runs when the squared XZ
// distance to the chosen point exceeds min((1.5 * speed)^2, 80000).
bool ship_ai_attackmove_tangent_uses_goal_009f3670(float distance_sq, float speed);

struct ShipAiAttackMoveTangentHost {
    virtual ~ShipAiAttackMoveTangentHost() = default;
    // 009F3670/009F3684: sub+10h += seconds, unconditionally and before the
    // null-target test.
    // 009F3687, [brain+0B20h]; 009F3692, 009E00A0 with ECX = sub+4h.
    virtual std::uint32_t brain_target_0b20() = 0;
    virtual void hold_heading_and_stop_009e00a0() = 0;
    // 009F369F and 009F36E1, 00424C40 BSP_GameSettings_GetSingleton then +4D4h.
    virtual float settings_weapon_release_delay_04d4() = 0;
    // 009F36C3, 009F36D9 and 009F3714, unit->vtable[114h](): the weapon
    // director, and its +30h stage compared with 2. Callee body unread.
    virtual std::uint32_t unit_director_vtable_0114() = 0;
    virtual int director_stage_0030(std::uint32_t director) = 0;
    // 009F3718, 0071E430(director, 00E08F78, 1). The two pushes at 009F370D and
    // 009F370F belong to this call, not to the getter: 0071E430 is RET 8 and
    // there is no ADD ESP after 009F3718.
    virtual void raise_command_stage_0071e430(std::uint32_t director,
                                              std::uint32_t command, int flag) = 0;
    // 009F3726/009F3731 and 009F3736/009F373E.
    virtual bool unit_pose_valid_00c8() = 0;
    virtual void refresh_unit_pose_00414db0() = 0;
    virtual void unit_position_xz_00fc(float& x, float& z) = 0;
    // 009F375A, 0082E850 on [brain+0AACh].
    virtual float ship_class_turn_radius_0082e850() = 0;
    // 009F37C1, 00BD2F10(stream 1, 1.2f, 1.5f).
    virtual float random_range_00bd2f10(float low, float high) = 0;
    // 009F379E and 009F37B1, the destination (brain+0B2Ch, brain+0B34h).
    virtual void brain_destination_0b2c(float& x, float& z) = 0;
    // 009F3829, 009D68B0(out, circle {cx, cz, radius}, unit position, min_step,
    // side 1): picks a tangent point on the circle. Body not read here; the
    // follow-up packet ship_ai_nav_circle_tangent owns it.
    virtual void circle_tangent_point_009d68b0(float centre_x, float centre_z,
                                               float radius, float unit_x,
                                               float unit_z, float min_step,
                                               ShipAiAttackMoveXZ& out) = 0;
    // 009F383F, 009DB6C0(sub, &point, 10.0f).
    virtual void clamp_to_world_box_009db6c0(ShipAiAttackMoveXZ& point, float inset) = 0;
    // 009F3869, unit->vtable[50h](); 009F3895, atan2(dx, dz) only when the
    // squared distance exceeds 1.0, otherwise the current heading is kept.
    virtual float unit_heading_vtable_0050() = 0;
    // 009F38BA, [[unit+538h]+0A0h], the speed the switch distance uses.
    virtual float unit_armament_speed_00a0() = 0;
    // 009F390A, brain+1D0h = 1, goal arm only; 009F3920, the goal setter.
    virtual void set_brain_goal_hold_01d0(int value) = 0;
    virtual void set_navigation_goal_009de050(const ShipAiAttackMoveXZ& goal,
                                              int keep_mode, int final_leg) = 0;
    // 009F392F and 009F3952, brain+0AF0h = 1.0f on the goal arm and 0.5f on the
    // heading arm; 009F3943, 009E0040 with ECX = sub+4h.
    virtual void set_brain_speed_scale_0af0(float scale) = 0;
    virtual void set_desired_heading_009e0040(float heading) = 0;
    // 009F396A, 009DB820(sub): the XZ range from the unit to the destination.
    virtual float range_to_destination_009db820() = 0;
    // 009F39A4, 009E2B60(sub): walks the unit's sibling chain at +48h/+44h.
    // Body not read in this packet.
    virtual void notify_siblings_009e2b60() = 0;
};

void ship_ai_attackmove_tangent_step_009f3670(ShipAiAttackMoveTangentState& state,
                                              float seconds,
                                              ShipAiAttackMoveTangentHost& host);

// ---------------------------------------------------------------------------
// state+14F4h, the initial sub-state: 007B3DD0 steps
// ---------------------------------------------------------------------------
//
// 007B3DD0 is `RET 4` and nothing else: three bytes, C2 04 00, followed by 0xCC
// padding to 007B3DDF. It has no Ghidra function; the boundary is in the doc's
// `## no_ghidra_function` table. Twenty vtables reference it, so it is a
// COMDAT-folded empty one-argument virtual shared across classes, not an
// attackmove routine. Every other slot of vtable 00D2171C is a constant or a
// no-op as well, so state+14F4h is the fully defaulted base sub-state.
//
// It is never usefully stepped: 009E8820 runs the selector before the member
// step (009E88D8 then 009E88F0), and 009E84F8/009E84FD seed state+1500h to a
// negative random value, so the very first selector call re-picks and leaves
// state+14F4h for state+8h before any step dispatch. This constant records that
// the routine exists and does nothing; there is no host and no step function.
inline constexpr std::uint32_t kAttackMoveInitialSubStateStep = 0x007b3dd0u;

} // namespace bsp
