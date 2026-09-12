// 009E76D0, the bearing-ring scan that gives an attackmove ship its commanded
// heading and throttle, and the four slot scorers behind it.
//
// Packet cc_ai_ring_scan, worker agent/cc-ai-ring-scan.
// Project C:/Users/sqz269/bsp.gpr, program /battlestationspacific.exe; Ghidra
// was READ-ONLY for this packet. Every descriptive name here is a hypothesis,
// not a recovered symbol. docs/SHIP_AI_RING_SCAN.md carries the evidence,
// address by address, and the Coverage table.
//
// This header builds on bsp/ship_ai_approach_update.hpp (ShipAiApproachState,
// ShipAiApproachSlotScore, ShipAiApproachMode, kShipAiApproachSlotCount and the
// pure rules ship_ai_approach_slot_total_009e76d0 and
// ship_ai_approach_heading_from_delta) and on bsp/ship_ai_attackmove_substates.hpp
// (ShipAiAttackMoveRingSlot, ShipAiAttackMoveXZ). It redefines none of their
// types and declares no host method those headers already declare.
//
// What this packet adds over the earlier partial projection
// ship_ai_approach_select_slot_009e76d0:
//   * the 6-way unrolled accept/reject body 009E7822-009E79AA and the 8-way
//     unrolled winner scan 009E79E5-009E7BC9 plus its 009E7BD4 remainder are
//     now read step by step, so the ring scan is complete, not partial;
//   * the four scorers 009E6400, 009E5DA0, 009E6870 and 009E6640, which the
//     earlier packet carried as host methods with "contract unread", are
//     reconstructed here, so the executable runs the obstacle probe itself
//     instead of asking a host for a score.
//
// Vocabulary is the one docs/SHIP_AI_APPROACH_UPDATE.md fixes: `nested` is the
// ring object at sub+8h, `slot i` is the 4Ch-byte record at nested + 4h + i*4Ch,
// `tune` is [brain+0AB0h] and `unit` is [brain+0AA8h].

#pragma once

#include <cstdint>

#include "bsp/ship_ai_approach_update.hpp"
#include "bsp/ship_ai_attackmove_substates.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Constants, every one read from the image at the address in the comment
// ---------------------------------------------------------------------------

// 00CE3D10, the pooled double 0.2 (the float 0.2f widened). 009E76DA scales the
// wobble phase advance by it and 009E692B scales the arc span by it; the image
// loads the same eight bytes at both sites.
inline constexpr double kShipAiRingScanOneFifth = 0.20000000298023224;

// 009E6400, the mode-4 decay score
inline constexpr double kShipAiRingScanDecayPeak = 1.5707963705062866;  // 00CE3830, pi/2
inline constexpr double kShipAiRingScanDecayEdge = 3.1415927410125732;  // 00CE3D28, pi
inline constexpr double kShipAiRingScanDecaySpan = 1.2000000476837158;  // 00CEC160
inline constexpr float kShipAiRingScanScoreCeiling = 1.0f;              // 00D7A24C

// 009E6870, the standoff-arc score
inline constexpr float kShipAiRingScanArcZeroAt = 3.1415927f;           // 00D7A264, pi

// 009E6640, the obstacle probe
inline constexpr float kShipAiRingScanProbePeriod = 2.0f;               // 00CE3958
inline constexpr float kShipAiRingScanProbeOriginClearance = 3.0f;      // 00CE3854
inline constexpr int kShipAiRingScanProbeOriginMode = 1;                // 009E6720
inline constexpr float kShipAiRingScanProbeNearAngle = 0.34906587f;     // 00CE398C, 20 deg
inline constexpr float kShipAiRingScanProbeFarAngle = 2.0943952f;       // 00D2017C, 120 deg

// 009E76D0, the scan itself
inline constexpr double kShipAiRingScanAcceptAbove = 0.8500000238418579; // 00CF0B58
inline constexpr float kShipAiRingScanRejectBase = -0.0f;                // 00D7A208
inline constexpr float kShipAiRingScanGoalTurnNear = 0.2f;               // 00CE54A0
inline constexpr float kShipAiRingScanGoalMarginNear = 400.0f;           // 00CFD710
inline constexpr float kShipAiRingScanGoalTurnFar = 1.2f;                // 00CE3814
inline constexpr float kShipAiRingScanGoalMarginFar = 50.0f;             // 00CEB4D4
inline constexpr float kShipAiRingScanGoalReverse = 3.1415927f;          // 00D7A264, pi

// ---------------------------------------------------------------------------
// 009E6400, the mode-4 decay score
// ---------------------------------------------------------------------------
//
// __thiscall(slot)(float bearing), RET 4 at 009E646A and 009E6479, body
// 009E6400-009E647B, complete. No host: its only callee is 00438B10.
// 009E7FC0's mode-4 arm is its only caller (009E8080), with the bearing
// nested+1290h, and only while nested+11E0h - nested+11E4h < 300.0.
//
// It is the ONLY writer of slot+2Ch outside 009E7FC0's mode-0 normalisation
// (009E8292) and 009E76D0's reject arm, so "decay" names the arm, not a decay
// over time: the value is a triangular ramp on the angle between the slot's
// bearing and the reference, zero head-on and astern, peaking at a beam
// bearing and clipped at 1.0 from |e| = 1.2 rad to pi - 1.2 rad.
//
//   e = wrap(bearing - slot.angle_08)                    009E6414
//   a = |e| by masking the sign bit at 009E6421
//   a = (pi/2 <= a) ? (pi - a) / 1.2 : a / 1.2           009E6434 FCOMIP, JBE
//   slot+2Ch = (a > 1.0f) ? 1.0f : a                     009E6456 FCOMIP, JBE
//
// The comparison at 009E6434 is FCOMIP of the constant against a, so a NaN
// angle takes the dividing arm and the 009E6456 test then stores the NaN.
float ship_ai_ring_scan_decay_score_009e6400(float bearing,
                                             float slot_angle) noexcept;
void ship_ai_ring_scan_decay_slot_009e6400(ShipAiApproachSlotScore& score,
                                           float bearing,
                                           float slot_angle) noexcept;

// ---------------------------------------------------------------------------
// 009E5DA0, the ship-class rating that fills slot+18h
// ---------------------------------------------------------------------------
//
// __thiscall(slot)(a 44h-byte block by value), RET 44h at 009E5DF9, body
// 009E5DA0-009E5DFB, complete. 009E7FC0's mode-0 arm is its only caller
// (009E81A7); the block is a verbatim REP MOVSD of 0x11 dwords out of the
// nested object at nested+127Ch (009E8197..009E81A2).
//
// The routine is a thin adapter, not a scorer: it replaces word 5 of its own
// copy of the block with wrap(word5 - slot.angle_08) (009E5DB4, 009E5DBB) and
// hands a pointer to the block to 0095EB40 with ECX = [slot+0h], the unit.
// 0095EB40 returns the rating, which lands in slot+18h; 009E7FC0 then divides
// every slot+18h by the frame maximum and scales it by tune+0h into slot+2Ch.
//
// The block is carried as raw dwords because the image copies it with MOVSD and
// this packet established a producer for only three of the seventeen words
// (docs/WORKER_VERIFICATION_CHECKLIST.md rule 4): word 5 is nested+1290h, which
// 009E7FC0 seeds from nested+11DCh at 009E8153, and words 6 and 7 are
// nested+1294h = 20.0f and nested+1298h = 60.0f (009E814B, 009E8161). Words 10,
// 11, 12 and 13 are read back as floats into slot+24h, +20h, +1Ch and +28h
// (009E5DCC..009E5DED); nothing in this packet reads those four fields again.
struct ShipAiRingScanClassQuery {
    std::uint32_t word[17]{}; // word[i] = the dword at nested+127Ch + 4*i
};

inline constexpr int kShipAiRingScanQueryBearingWord = 5; // nested+1290h
inline constexpr int kShipAiRingScanQueryLowWord = 6;     // nested+1294h
inline constexpr int kShipAiRingScanQueryHighWord = 7;    // nested+1298h

struct ShipAiRingScanClassScoreHost {
    virtual ~ShipAiRingScanClassScoreHost() = default;
    // 009E5DC4, 0095EB40 with ECX = [slot+0h] (the unit 009E5692 stores) and a
    // pointer to the block. Body unread (it belongs to the vehicle-class
    // cluster at 00951F40): contract unread beyond "returns the rating that
    // becomes slot+18h".
    virtual float rate_bearing_0095eb40(const ShipAiRingScanClassQuery& query) = 0;
};

void ship_ai_ring_scan_class_score_009e5da0(const ShipAiAttackMoveRingSlot& ring_slot,
                                            ShipAiApproachSlotScore& score,
                                            ShipAiRingScanClassQuery query,
                                            ShipAiRingScanClassScoreHost& host);

// ---------------------------------------------------------------------------
// 009E6870, the standoff-arc score that fills slot+30h
// ---------------------------------------------------------------------------
//
// __thiscall(slot)(const float block[4]), RET 4 at 009E697D, body
// 009E6870-009E697F, complete. No host: its callees are 00438AA0, 00438B10 and
// 00419010. 009E6E80's tail is its only caller (009E74B7), once per slot, with
// the block {side, span, nested+11DCh, tune+4h} laid out at 009E7483..009E74A0.
//
// It is the producer of slot+30h in the normal case; ShipAiApproachSlotScore
// calls that field `penalty_30` because the earlier packet had only seen
// 009E76D0's reject arm write it (see docs/SHIP_AI_RING_SCAN.md, Corrections).
//
// The two arc edges are the reference bearing stepped by ±side:
//   edge_add = wrap(centre + side)                        009E688A
//   edge_sub = wrap(centre - side)                        009E68A2
//   g = min(|wrap(slot.angle - edge_add)|,                009E68BC, 009E68E3
//           |wrap(slot.angle - edge_sub)|)                009E6902 FCOMI, JBE
// and the cost is piecewise linear in g with the knee at the span:
//   cost = (span >= g) ? (g / span) * (0.2 * span)        009E6931 FCOMIP, JC
//                      : g - (span - 0.2 * span)
// Inside the span the cost rises at one fifth of g, outside it at one for one,
// and the two arms agree at g == span. The score is the interpolation
//   slot+30h = interp(0, block[3], pi, 0, cost)           009E6970
// so it is block[3] (tune+4h) on either edge and falls to zero at pi of cost.
// The image float-stores 0.2*span before the compare (009E692D), which is why
// the multiply is written out rather than folded.
float ship_ai_ring_scan_arc_cost_009e6870(float centre, float side, float span,
                                          float slot_angle) noexcept;
float ship_ai_ring_scan_arc_score_009e6870(float centre, float side, float span,
                                           float slot_angle,
                                           float tune_04) noexcept;
void ship_ai_ring_scan_arc_slot_009e6870(const ShipAiAttackMoveRingSlot& ring_slot,
                                         ShipAiApproachSlotScore& score,
                                         float centre, float side, float span,
                                         float tune_04) noexcept;

// ---------------------------------------------------------------------------
// 009E6640, the obstacle probe behind the accept/reject test
// ---------------------------------------------------------------------------
//
// __thiscall(slot)(float seconds, float bearing, char override_a,
//                  char override_b, float range), RET 14h at 009E684F and
// 009E6866, body 009E6640-009E6868, complete, ST0 result. 009E76D0 is its only
// caller (009E7755).
//
// The routine owns three fields of the slot record that ShipAiAttackMoveRingSlot
// and ShipAiApproachSlotScore split between them:
//   slot+40h  the blocked byte (ShipAiApproachSlotScore::blocked_40)
//   slot+44h  the clear distance along the slot's direction, seeded to the
//             probe range on every re-probe (ShipAiAttackMoveRingSlot::reset_44)
//   slot+48h  the seconds left before the next re-probe
//             (ShipAiAttackMoveRingSlot::jitter_48)
// The two ring-record names come from the construction values 009E5530 and
// 009F30F0 write (1000.0f and a draw in [0, 2)); the running meaning is the one
// above. docs/SHIP_AI_RING_SCAN.md records that under Corrections.
//
// The side gate (009E6644..009E6697) is the only use of the two override bytes
// here: with override_a set every slot whose wrap(slot.angle - bearing) is
// negative is refused, with override_b set every slot whose difference is
// positive is refused, and a refused slot has its clear distance forced to 0.
//
// The probe is re-cast only when the timer goes negative, so a slot keeps its
// verdict for 2.0 seconds of game time (009E66C8). The probe length is short
// for slots near the reference bearing and full length for slots 120 degrees or
// more away from it, which is why a slot pointing at the goal is never blocked:
//   length = range * interp(20 deg, 0, 120 deg, 1,
//                           |wrap(bearing - slot.angle)|)   009E67AA, 009E67AF
//
// The score the caller ranks is 1.0 for an allowed, unblocked slot and
// slot+44h / range otherwise (009E6848, 009E6856), so a refused slot scores 0
// and a blocked one scores the fraction of the probe that was clear.
struct ShipAiRingScanHost {
    virtual ~ShipAiRingScanHost() = default;

    // 009E76F4, 00605070 with ECX = &nested+1214h: wraps the phase in place
    // into (-pi, pi]. Body read by the attackmove-substates packet.
    virtual float wrap_phase_00605070(float value) = 0;

    // 009E66EB, unit->vtable[218h]() with ECX = the unit. The returned object
    // is the `this` of both 00417B10 and 0041B4E0 below. Body unread:
    // contract unread beyond "the space the probe is cast in".
    virtual std::uint32_t probe_space_vtable_0218() = 0;

    // 009E66F8 and 009E7CA1, the byte at [unit+0C8h]: the world matrix at
    // [unit+0CCh..+108h] is current when it is non-zero.
    virtual bool unit_pose_fresh_00c8() = 0;

    // 009E6705, 00414DB0 with ECX = the unit, taken only when the byte above
    // is clear. Body read by the entity-matrix packet.
    virtual void refresh_unit_pose_00414db0() = 0;

    // 009E6710 and 009E7D92/009E7DA4, the unit's world x and z at [unit+0FCh]
    // and [unit+104h].
    virtual ShipAiAttackMoveXZ unit_world_xz() = 0;

    // 009E673E, 00417B10 with ECX = the probe space and (out, in, 3.0f, 1).
    // The two floats it returns replace the probe's start point (009E6751,
    // 009E6763). Body unread: contract unread.
    virtual ShipAiAttackMoveXZ probe_origin_00417b10(std::uint32_t space,
                                                     const ShipAiAttackMoveXZ& point,
                                                     float clearance,
                                                     int mode) = 0;

    // 009E6808, 0041B4E0 with ECX = the probe space and (&start, &end, &hit).
    // Returns the AL the image tests at 009E680D. Body unread: contract unread
    // beyond "fills `hit` when it returns true".
    virtual bool probe_hit_0041b4e0(std::uint32_t space,
                                    const ShipAiAttackMoveXZ& start,
                                    const ShipAiAttackMoveXZ& end,
                                    ShipAiAttackMoveXZ& hit) = 0;

    // 009E6833, 00414C60 with ECX = the two-float delta (start - hit).
    // Body read by the vector packet.
    virtual float planar_length_00414c60(const ShipAiAttackMoveXZ& delta) = 0;

    // 009E784B, 009E7877, 009E78B7, 009E78FD, 009E7943 and 009E7989: the float
    // at tune+4h, reloaded through [slot+4h] -> [nested+0h] -> [brain+0AB0h] at
    // every one of the six unrolled reject arms.
    virtual float tune_reject_penalty_04() = 0;

    // 009E7CAD..009E7D71, the pose refresh 009E76D0 open-codes when the byte at
    // [unit+0C8h] is clear: 00414DB0 on the parent at [unit+3Ch] when that is
    // non-null, then 00413920 multiplying [unit+74h] by the parent's world
    // matrix at [parent+0CCh] (or the local matrix verbatim with no parent),
    // then a sixteen-float copy into [unit+0CCh], [unit+0C8h] = 1 and
    // [unit+10Ch] = 0. Bodies of 00413920 and 00414DB0 read elsewhere.
    virtual void rebuild_unit_world_matrix() = 0;

    // 009E7D7A and 009E7D84, the attackmove goal x and z at brain+0B2Ch and
    // brain+0B34h. brain+0B30h, the y, is not read here.
    virtual ShipAiAttackMoveXZ brain_goal_0b2c() = 0;

    // 009E7DE7, the float at [unit+490h].
    virtual float unit_cruise_speed_0490() = 0;

    // 009E7ECB, 009E5E90 with ECX = nested and (bearing, seconds): the routine
    // that turns the chosen bearing into the commanded heading nested+120Ch.
    // Reconstructed as ship_ai_approach_commit_bearing_009e5e90.
    virtual void commit_bearing_009e5e90(float bearing, float seconds) = 0;
};

// 009E679B..009E67AF. The probe length for one slot.
float ship_ai_ring_scan_probe_length_009e6640(float range, float bearing,
                                              float slot_angle) noexcept;

// 009E6644..009E6697. False when the slot is on the side an override forbids.
bool ship_ai_ring_scan_side_allows_009e6640(float bearing, float slot_angle,
                                            bool override_a,
                                            bool override_b) noexcept;

// The whole of 009E6640. `ring_slot.reset_44` is slot+44h and
// `ring_slot.jitter_48` is slot+48h, both updated in place; `score.blocked_40`
// is slot+40h. The return value is the raw score 009E76D0 ranks.
float ship_ai_ring_scan_probe_009e6640(ShipAiAttackMoveRingSlot& ring_slot,
                                       ShipAiApproachSlotScore& score,
                                       float seconds, float bearing,
                                       bool override_a, bool override_b,
                                       float range,
                                       ShipAiRingScanHost& host);

// ---------------------------------------------------------------------------
// 009E76D0, the ring scan
// ---------------------------------------------------------------------------
//
// __thiscall(nested)(float seconds), RET 4 at 009E7EDA, body
// 009E76D0-009E7EDC. COMPLETE here: the six-way unrolled accept/reject body and
// the eight-way unrolled winner scan were read step by step for this packet.

// 009E779C..009E7808, the ten-way unrolled normalisation. The image runs it
// only when the frame maximum is below 1.0 (009E7795 COMISS, JBE), and it never
// tests the divisor, so an all-zero frame divides by zero. Returns nothing; the
// array is rewritten in place.
void ship_ai_ring_scan_normalise_009e76d0(float scores[kShipAiApproachSlotCount],
                                          float best) noexcept;

// 009E7822..009E79AA, one unrolled step. A slot scoring strictly above 0.85 is
// accepted and only has its blocked byte cleared (009E7852); a slot at or below
// it keeps the byte and has +2Ch, +38h and +3Ch zeroed and
// +30h = -0.0f - tune+4h (009E7843..009E784B).
void ship_ai_ring_scan_apply_verdict_009e76d0(ShipAiApproachSlotScore& score,
                                              float normalised,
                                              float tune_reject_penalty) noexcept;

// 009E79CA..009E7C17. The winner is the slot with the largest five-float total
// ship_ai_approach_slot_total_009e76d0 computes. Slot 0 seeds the maximum and
// every later comparison is FCOMIP then JBE, so the FIRST slot holding the
// maximum wins and a NaN total never replaces the incumbent.
int ship_ai_ring_scan_winner_009e76d0(
    const ShipAiApproachSlotScore slots[kShipAiApproachSlotCount]) noexcept;

// 009E7DE1..009E7E81. The test that reverses the goal bearing by pi: the image
// widens [unit+490h] and nested+11E0h to double (009E7E23, 009E7E35), subtracts
// the interpolation from the widened speed with FSUBR, float-stores that, and
// compares it against the widened range with FCOMIP then JBE.
//   reverse = (double)cruise_speed
//             - interp(0.2, 400, 1.2, 50, |wrap(unit_heading - goal_bearing)|)
//             > (double)goal_range
bool ship_ai_ring_scan_reverse_goal_009e76d0(float cruise_speed_0490,
                                             float goal_range_11e0,
                                             float unit_heading_11ec,
                                             float goal_bearing) noexcept;

// The whole of 009E76D0, in the image's order.
//
// `ring` is not const: 009E6640 writes slot+44h and slot+48h through it.
// The wobble at 009E7C3E..009E7C78 is not projected: the image multiplies
// sin(phase) by the double 0.0 at 00D7A258, passes the product to 00438AA0 and
// pops the result with FSTP ST0 at 009E7C78 without storing it. The phase
// advance at 009E76DA is projected because it is stored.
void ship_ai_ring_scan_009e76d0(ShipAiApproachState& state,
                                ShipAiAttackMoveRingSlot ring[kShipAiApproachSlotCount],
                                ShipAiApproachSlotScore slots[kShipAiApproachSlotCount],
                                float seconds,
                                ShipAiRingScanHost& host);

} // namespace bsp
