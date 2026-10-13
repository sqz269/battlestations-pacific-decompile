#ifndef BSP_PLANE_RELEASE_VER_CC13_HPP
#define BSP_PLANE_RELEASE_VER_CC13_HPP

// Packet cc13-ver20: the AI plane weapon release, image against host
// (docs/CC13_VER_20.md). Pure models of the rack drop's decisive pieces, read
// from the bytes. Nothing in the host calls them; wiring them belongs to the
// owners of src/game_hosts_gunnery.cpp, where both drop spawns live.
//
// Every name is a hypothesis, not a recovered symbol. The structs are not
// binary layouts; the offsets in the comments are the native ones.
//
// One path for every round a plane drops. 006E4D50 BSP_MBombPlatform_Drop walks
// the rack's children answering vtable[5Ch](2Ah) (006E4EC5). MTorpedo (2Bh) and
// MDepthCharge (2Ch) have parent 2Ah (src/entity_class_ids.cpp), so an aerial
// torpedo takes the same draws, the same launch hook and the same turn as a bomb:
//   - MBomb vtable 00CF9438 and MTorpedo vtable 00D0C3E8 both hold 006E0A70 at
//     +190h (00CF95C8, 00D0C578);
//   - their +310h sub-object tables 00CF95E0 and 00D0C378 both hold 006E1F00
//     at +34h (00CF9614, 00D0C3AC).

#include "bsp/gameplay_seg53_cc13.hpp"

namespace bsp {
namespace plane_release_ver_cc13 {

using Vec3 = gameplay_seg53_cc13::Vec3;

// ---------------------------------------------------------------------------
// 00999B70 BSP_PilotBot_ThrowMul, __thiscall(bot, record*), RET 4, float in ST0.
// record = [rack+3F8h]+34h (006E4F61-006E4F6A). Null record: 1.0 (00999B77,
// 00999B88). Otherwise the PilotBot row ([00F8A30C] + bot+34h * 248h) field by
// the record's kind at +8h:
//   0Ah -> row+34h TorpThrowMul        (00999B91)
//   0Bh -> row+E0h DepthChargeThrowMul (00999BB9)
//   12h -> row+12Ch StrikeThrowMul     (00999BE3)
//   else (bot+50h)->vtable[5Ch](10h), a level bomber: row+C8h LevelBombThrowMul
//   else row+78h DiveBombThrowMul      (00999C47)
// Field names from include/bsp/robot_config.hpp.
// ---------------------------------------------------------------------------
enum class ThrowMulRow {
    kNone,               // null record: 1.0
    kTorp034,
    kDepthCharge0E0,
    kStrike12C,
    kLevelBomb0C8,
    kDiveBomb078,
};
ThrowMulRow pilot_bot_throw_mul_row_00999b70(bool has_record, int record_kind_8,
                                             bool plane_is_kind_10) noexcept;

// ---------------------------------------------------------------------------
// 006E4F0D-006E4F76, the throw extent s of one drop (draw 2's upper bound).
// s = [rack+3F8h]+4h (Throw). The owner [rack+3F0h] is kept only when it answers
// vtable[5Ch](0Fh) (006E4F30); then the bot is owner+DF4h (006E4F3C). The
// ThrowMul is applied when the bot is non-null AND (owner+1B0h == 8 (006E4F52)
// OR 00927F10(owner+1B0h), BSP_PartySlot_IsAiHeld (006E4F58)).
// ---------------------------------------------------------------------------
struct RackThrowInputs {
    float throw_4 = 0.0f;           // [rack+3F8h]+4h
    bool owner_is_plane_0f = false; // [rack+3F0h]->vtable[5Ch](0Fh)
    bool has_bot_df4 = false;       // owner+DF4h != 0
    int party_slot_1b0 = 0;         // owner+1B0h
    bool slot_ai_held = false;      // 00927F10(owner+1B0h)
    float throw_mul = 1.0f;         // 00999B70's answer
};
float rack_throw_extent_006e4f30(const RackThrowInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 006E4F7A-006E5045, the cone. Two draws on 00BD2F10 with ECX = 1 (stream 1):
//   a = U(0, pi)   (006E4F91, pi = float [00D7A264]) -> rack+400h
//   t = tan(U(0, s)) (006E4FB1, 00412E20 FSINCOS/FDIVP) -> rack+404h
// D = (t cos a, -t sin a, 1.0f) at [ESP+3Ch..44h], the argument 006E1F00 reads.
// The wind drift draws 006E513C / 006E51AC follow and are not modelled here.
// ---------------------------------------------------------------------------
Vec3 rack_drop_cone_006e4f91(float a_draw, float t_draw) noexcept;

// ---------------------------------------------------------------------------
// The release velocity one round leaves with, for a round whose owner chain
// reaches a plane: 006E0A70 then 006E1F00, composed.
//   006E1F1F-006E1F2E  copy the round's world matrix (round+CCh) to [ESP+4Ch]
//   006E1F4D           owner->vtable[190h] = 006E0A70(owner, &copy)
//   006E0AE7-006E0AF4  row 2 of the COPY = w = owner world velocity with
//                      3.0 off y, then 0085DC80 rebuilds rows 0/1 round it
//   006E1F7C           copy -> round+74h (the local the turn reads)
//   006E2062-006E2181  v = |w| (Dx r0 + Dy r1 + Dz r2) from round+74h
// So the turn is applied in the VELOCITY frame: with no cone (t = 0) the round
// keeps w exactly. Built on gameplay_seg53_cc13's 006E0A70/006E1F00 model.
// `pose_matrix` is the round's world matrix while it hangs on the rack (the
// plane pose for an identity attach, docs/GUNNERY_OPEN_ITEMS.md 130.6).
// ---------------------------------------------------------------------------
struct RackRoundReleaseInputs {
    float pose_matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    bool owner_is_plane_0f = true;
    float owner_speed_38 = 0.0f;    // owner->vtable[38h]
    Vec3 owner_world_velocity_34;   // owner->vtable[34h]
    float a_draw = 0.0f;            // 006E4F91
    float t_draw = 0.0f;            // 006E4FB1, before the tangent
};
Vec3 rack_round_release_velocity_006e4d50(const RackRoundReleaseInputs& in);

// ---------------------------------------------------------------------------
// The host's two spawns, as comparison models (not image behaviour).
// release_bomb_drop (src/game_hosts_gunnery.cpp, kBombDropVelocityBound and
// kBombDropScatterBound ON): v = |w'| (Dx right + Dy up + forward) with the
// PLANE's pose rows, w' = w with 3.0 off y when the forward speed > 1.3888888.
// release_ordnance_drop (the torpedo): v = forward * dot(v_body, forward), no
// 3.0, no cone, no draws.
// ---------------------------------------------------------------------------
Vec3 host_bomb_release_velocity(const float right[3], const float up[3],
                                const float forward[3], const Vec3& world_velocity,
                                float forward_speed, float a_draw, float t_draw) noexcept;
Vec3 host_torpedo_release_velocity(const float forward[3], float forward_speed) noexcept;

}  // namespace plane_release_ver_cc13
}  // namespace bsp

#endif
