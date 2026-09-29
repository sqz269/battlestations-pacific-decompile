#pragma once

// The bot approach's target-reference sub-object: 009FB200 (construct),
// 009FA260 (pick the body-frame aim offset) and 009FADA0 (per-tick update).
// Packet cc8_hull_aim_point. Every name here is a hypothesis, not a recovered
// symbol.
//
// The sub-object is embedded in every aircraft attack approach: at approach+30h
// in the dive-bomb approach and at approach+B4h in the torpedo approach
// (009D34B2 LEA ECX,[ESI+0B4h] / 009D34BB CALL 009FADA0). Its vtable is
// 00D21CB4, installed at 009FB237. `tools/callsite_census.py 009fada0` lists
// eleven call sites: nine approach classes plus two inside its own code.
//
// What it produces is ONE point: sub+1Ch/+20h/+24h, the world-space aim point
// the approach's slot-0 getter returns. That point is
//
//     target_world_matrix(target+CCh) x body_frame_offset(sub+28h)
//
// stored at 009FAEEA/009FAEF5/009FAF00 after 009FAEDF CALL 004142E0
// BSP_Vector3f_TransformAffinePoint (ECX = the source point &sub+28h, stack =
// destination and the matrix). There is NO lead term anywhere in this object:
// no velocity and no time-of-flight, on either the dive-bomb or the torpedo
// side.
//
// ---------------------------------------------------------------------------
// Where the body-frame offset comes from
// ---------------------------------------------------------------------------
// 009FA260 calls target->vtable[+100h] and stores the returned point to
// sub+28h/+2Ch/+30h (009FA2A2-009FA2B0), then ADDS the vector at
// sub+34h/+38h/+3Ch to it componentwise (009FA2B3-009FA2CB). So the offset is
// pick + bias, not pick. Finally it clears the dirty byte sub+41h (009FA2CE).
//
// The slot +100h ABI, from the dispatch at 009FA272-009FA2A0 (the receiver is
// taken from the listing; the decompiler drops it):
//
//     float3* __thiscall slot100(float3* out, const float3* spread,
//                                const float3* centre, float section_chance,
//                                float w0, float w1, float w2);   // RET 1Ch
//
// and the callee returns `out` in EAX. Exactly two implementations exist in the
// image, found by scanning .rdata for each as a literal dword:
//
//   * 0042D810 - the ORIGIN. Zeroes out[0..2] and returns it. Used by dozens of
//     vtables, including the plane unit instance (00D05F20+100h, confirmed by
//     the constructor store at 007CFD78 in 007CFD20
//     BSP_PlaneUnitInstance_Construct). For every one of those classes aiming
//     at the target's origin is CORRECT, not a defect.
//   * 00816650 - the tapered hull box, bound already as
//     bsp::ship_lead_point_00816650 in include/bsp/gun_bot_remainder.hpp. Used
//     by EXACTLY NINE vtables, each confirmed by a literal store in its own
//     constructor: 00CF90B0 (006DFC90), 00CFA778 (006EB160), 00CFB738
//     (006FB300), 00CFC3D0 (006FE460 BSP_UnitInstance_Construct), 00CFFA30
//     (0074BB00), 00D01630 (00758150), 00D09678 (0081ED40
//     BSP_UnitVehicleBase_Construct, store at 0081ED80), 00D0BF80 (00852F10
//     BSP_SubmarineUnit_Construct) and 00D0C648 (00857CD0).
//
// This header does not re-derive 00816650; it supplies the seeds the approach
// hands it and the cadence around it.
//
// ---------------------------------------------------------------------------
// The one result that decides how this behaves
// ---------------------------------------------------------------------------
// All NINE of those vtables carry 0042BB20 at slot +104h, read as raw bytes
// because Ghidra has no function there: `b0 01 c2 0c 00` = MOV AL,1; RET 0Ch.
// A 3-dword argument, which is the candidate point BY VALUE (009FAE69 SUB
// ESP,0Ch is the push; there is no pointer push before the call at 009FAEA3),
// and the answer is ALWAYS TRUE.
//
// 009FAEA5 TEST AL,AL / JNZ skips the re-pick when the answer is true. So the
// 1.5-2.5 s timer expires over and over and its only consumer always says
// "keep this point". The offset is therefore drawn ONCE, by the dirty byte the
// constructor sets at 009FB272, and never re-rolled for the life of the
// sub-object. No ship class in this image refuses its aim offset.
//
// ---------------------------------------------------------------------------
// Uncertainty
// ---------------------------------------------------------------------------
//  * sub+44h gates a tail block at 009FAF0A against 0.0 (00D7A218) and is
//    initialised 0.0 at 009FB25F. CORRECTED by packet cc9_approach_target_lead:
//    it is "projtime", written by the torpedo and dive-bomb approaches, and the
//    tail is a lead. See approach_target_ref_lead_tail_009faf05 below; the "NO
//    lead term" sentence at the top holds only while projtime is 0.
//  * The named-section path inside 00816650 is live code but dead under this
//    seed (section_chance = -1.0), so nothing here exercises it.
//  * The centre and the bias are both seeded from 00F87574/78/7C, which is
//    .data past raw size and therefore loader zero-fill. That is a statement
//    about the image on disk, not proof that nothing writes it at runtime.

#include <array>
#include <cstdint>

#include "bsp/camera_affine.hpp"
#include "bsp/gun_bot_remainder.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The seeds 009FB200 installs
// ---------------------------------------------------------------------------
// sub+48h, the per-axis spread the sampler draws over. 009FB276/7B/80 store
// 00CE3860, 00CE3800, 00CE3860.
inline constexpr float kApproachTargetRefSpreadX = 0.9f;  // 00CE3860
inline constexpr float kApproachTargetRefSpreadY = 0.5f;  // 00CE3800
inline constexpr float kApproachTargetRefSpreadZ = 0.9f;  // 00CE3860

// sub+64h, the probability of taking 00816650's named-section path. 009FB2F9
// stores 00D7A260 = -1.0, and 00816659 COMISS/JBE against 00D7A218 = 0.0 sends
// every call with a non-positive value straight to the hull-box fallback.
inline constexpr float kApproachTargetRefSectionChance = -1.0f;  // 00D7A260

// sub+68h/+6Ch/+70h, the three section weights. 009FB2FE/306/30B/310 store
// 00D7A24C = 1.0 into all three.
inline constexpr float kApproachTargetRefSectionWeight = 1.0f;  // 00D7A24C

// sub+60h, the re-pick timer. It counts UP and is tested against 00CE3958 at
// 009FAE38; on expiry 009FAE61 re-arms it to Uniform(00CE69D0, 00CE3800), so
// the nominal period is 2.0 - Uniform(-0.5, 0.5) = 1.5 to 2.5 s. The first
// value is Uniform(0, 2.0) at 009FB2E7.
inline constexpr float kApproachTargetRefRepickPeriod = 2.0f;  // 00CE3958
inline constexpr float kApproachTargetRefRearmLo = -0.5f;      // 00CE69D0
inline constexpr float kApproachTargetRefRearmHi = 0.5f;       // 00CE3800

// The sub-object's live state, named by original displacement.
struct ApproachTargetRefState {
    // sub+1Ch/+20h/+24h, the world aim point the approach reads back.
    std::array<float, 3> world_point_1c{{0.0f, 0.0f, 0.0f}};
    // sub+28h/+2Ch/+30h, the body-frame offset: the pick plus the bias.
    std::array<float, 3> body_offset_28{{0.0f, 0.0f, 0.0f}};
    // sub+34h/+38h/+3Ch, the bias 009FA2B3 adds after each pick.
    std::array<float, 3> bias_34{{0.0f, 0.0f, 0.0f}};
    // sub+54h, the centre 00816650 samples around.
    std::array<float, 3> centre_54{{0.0f, 0.0f, 0.0f}};
    // sub+48h, the per-axis spread.
    std::array<float, 3> spread_48{{kApproachTargetRefSpreadX,
                                    kApproachTargetRefSpreadY,
                                    kApproachTargetRefSpreadZ}};
    float repick_timer_60 = 0.0f;  // sub+60h
    float section_chance_64 = kApproachTargetRefSectionChance;   // sub+64h
    float weight_68 = kApproachTargetRefSectionWeight;           // sub+68h
    float weight_6c = kApproachTargetRefSectionWeight;           // sub+6Ch
    float weight_70 = kApproachTargetRefSectionWeight;           // sub+70h
    bool dirty_41 = true;   // sub+41h, set 1 at 009FB272
    bool has_target_40 = false;  // sub+40h, set 1 at 009FB317
    // sub+18h != 0. Cleared at 009FAE11 when the target dies, which freezes
    // world_point_1c at its last sample.
    bool tracking = false;
    // True once the death path at 009FADAE-009FAE18 has run.
    bool frozen = false;
    // sub+44h "projtime", 0.0 at 009FB25F. The approach writes it after its
    // own 009FADA0 call (torpedo approach+F8h, dive approach+74h).
    float projtime_44 = 0.0f;
    // HOST-ONLY: the projtime 009FADA0's tail read this tick. The image adds
    // the lead once, into sub+1Ch, at the 009FADA0 call; this host recomputes
    // the point at every consumer, so it keeps the value that call would
    // have seen and every consumer in the tick uses it.
    float lead_projtime = 0.0f;
};

// 009FB200. `first_timer_draw` is the Uniform(0, 2.0) at 009FB2E7.
ApproachTargetRefState approach_target_ref_construct_009fb200(
    float first_timer_draw) noexcept;

// 009FA260, the pick. Returns the new body-frame offset (pick + bias) and
// clears the dirty byte. `hull` is [target+538h]+A0h/+A4h/+A8h, the authored
// vehicle-class Length/Width/Height; pass a zeroed `hull` for a target whose
// vtable slot +100h is 0042D810, which yields exactly the bias.
std::array<float, 3> approach_target_ref_pick_009fa260(
    ApproachTargetRefState& state, const LeadAimHullExtents& hull,
    const ShipLeadRandomDraws& draws, bool target_is_hull_sampler) noexcept;

// 009FADA0's cadence, without the transform. Advances the timer, re-arms it on
// expiry, and answers whether 009FA260 must run this tick. `rearm_draw` is the
// Uniform(-0.5, 0.5) at 009FAE61.
//
// Note the result this encodes: slot +104h is 0042BB20 for every class that
// samples a hull, so `validity_still_holds` is true there and a timer expiry
// alone never forces a re-pick. Only the dirty byte does.
bool approach_target_ref_needs_pick_009fada0(ApproachTargetRefState& state,
                                             float dt, float rearm_draw,
                                             bool validity_still_holds) noexcept;

// 009FADAE-009FAE18, the death path: take one last sample and stop tracking.
void approach_target_ref_freeze_009fadae(ApproachTargetRefState& state,
                                         const CameraMatrix& matrix) noexcept;

// 009FAED0-009FAF00: transform the body-frame offset by the target's world
// matrix (target+CCh) and store it as the aim point.
void approach_target_ref_store_world_point_009faeea(
    ApproachTargetRefState& state, const CameraMatrix& matrix) noexcept;

// ---------------------------------------------------------------------------
// The lead: sub+44h "projtime" and the tail 009FAF05-009FAF7D
// ---------------------------------------------------------------------------
// Packet cc9_approach_target_lead. CORRECTS "Uncertainty" above: sub+44h has
// writers, and the tail is live. 009FB3E0 registers the field under the name
// "projtime" (00D21C8C, type 2, LEA EDX,[ESI+44h] at 009FB516). The torpedo
// approach writes it as approach+F8h (B4h + 44h), the engagement estimate
// clamped to [0, 30] s (009D3D2F/009D3D52/009D3D65, torpedo_engagement_eta_009d3c93),
// and the dive-bomb approach as approach+74h (30h + 44h) at 009C7D65/7E72/7E85.
// Both writes land after the approach's own 009FADA0 call, so a tick leads by
// the previous tick's estimate.
//
// 009FAF05 COMISS [sub+44h], 00D7A218 (0.0); JBE skips. Otherwise
// 009FAF3A calls target->vtable[48h](&tmp, projtime) (__thiscall, RET 8, EAX =
// &tmp) and adds (tmp - target pose origin +FCh/+100h/+104h) to sub+1Ch..24h,
// the hull point 009FAEEA-009FAF00 has just stored. Each component is rounded
// to float at 009FAF44/51/5E and again at 009FAF69/73/7D.
void approach_target_ref_lead_tail_009faf05(ApproachTargetRefState& state,
                                            const std::array<float, 3>& predicted,
                                            const std::array<float, 3>& origin) noexcept;

// 008120E0, slot +48h of all nine hull-sampling vtables (00CF90B0, 00CFA778,
// 00CFB738, 00CFC3D0, 00CFFA30, 00D01630, 00D09678, 00D0BF80, 00D0C648, each
// read from .rdata). __thiscall(out, t), RET 8 at 0081229B/0081230A, body
// 008120E0-0081230D. The plane class carries 00954650 there instead.
//
//   A = (float)(00811890(unit, unit+984h) * t)                 008120F8-00812101
//   if |A| > 0.1 [00D7A3A0] and vtable[38h]() > 0.8333 [00D09450]:   the arc
//     A = ClampInPlace(A, -1 [00D7A260], 1 [00D7A24C])         00812185
//     v = vtable[34h](); h = pi/2 - atan2(v.z, v.x), +2pi if < 0   0081219D
//     m = 00438B10(h, (float)(A * 0.5 [00D7A280]))             008121E8
//     d = 006BC0C0(m)  (x = sin m, z = cos m)                  008121FD
//     s = vtable[38h]()                                        00812209
//     out = (x + (float)((float)(s*d.x) * t), y + 0.0 [00D7A258],
//            z + (float)((float)(s*d.z) * t))
//   else, the straight arm 0081229E:
//     v = vtable[34h](); out = pos + (float)(v * t) per component
//
// On the nine classes vtable[34h] is 00812090 (body axis unit+94h..9Ch times
// 0092D730) and vtable[38h] is 0080E0F0 (0092D730 alone).
// 009C7D61-009C7E85, the dive-bomb approach's projtime write (approach+74h =
// sub+44h). Outside the impact arm (before the dive and before the range
// latch, the 009C7D27 raw-position arm) it stores 0.0 at 009C7D65. In the
// impact arm 009C7E3C-009C7E46 forms approach+C8h + tf, where tf is
// 009C7D71's 007BCC80(...) + 0.1 kept in the argument slot [ESP+38h]; a
// negative sum jumps back to the 0.0 store (009C7E56), a sum above the double
// 30.0 [00CE7630] stores the float 30.0 [00CE38C8] (009C7E72), else the sum
// (009C7E85). approach+C8h is 009C3DA0's Uniform(-row+2Ch, row+2Ch) at
// 009C3E8C, drawn at the task seed and at every fly-over enter.
float dive_bomb_projtime_009c7e3c(bool impact_arm, float fall_time,
                                  float aim_time_error_c8) noexcept;

struct ShipPredictInputs {
    std::array<float, 3> position{};  // unit+FCh/+100h/+104h after 00414DB0
    std::array<float, 3> velocity{};  // vtable[34h]
    float speed = 0.0f;               // vtable[38h]
    float yaw_rate = 0.0f;            // 00811890(unit, unit+984h)
};
std::array<float, 3> ship_predict_position_008120e0(const ShipPredictInputs& in,
                                                    float t) noexcept;

// ---------------------------------------------------------------------------
// A deterministic stand-in for the four draws
// ---------------------------------------------------------------------------
// LABELLED SUBSTITUTION, not a reconstruction. 00816650 takes its draws from
// 00BD2F10 BSP_Random_UniformFloatRange on stream ECX=1, whose state this
// process does not carry. The host's usual stand-in (`random_between(lo, _)`
// returns `lo`) is wrong here: it would peg every attack at the stern-port
// corner of the hull instead of sampling it. The mean is equally wrong in the
// other direction - it collapses to the origin, which is the behaviour this
// binding exists to replace.
//
// So this returns a reproducible hash-driven draw in [0,1) per call index. It
// is distributionally faithful and identical run to run, which keeps the
// per-round census lines comparable. It is NOT the image's sequence.
std::array<float, 4> approach_target_ref_unit_draws_substitute(
    std::uint32_t seed) noexcept;

// Map the four unit draws onto the ranges 00816650 draws over, given `spread`.
ShipLeadRandomDraws approach_target_ref_draws_from_unit(
    const std::array<float, 4>& unit, const std::array<float, 3>& spread) noexcept;

}  // namespace bsp
