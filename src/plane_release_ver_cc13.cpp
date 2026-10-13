// Packet cc13-ver20: pure models of the rack drop's release pieces, read from the
// bytes (docs/CC13_VER_20.md). Hypotheses, not recovered symbols.

#include "bsp/plane_release_ver_cc13.hpp"

#include <cmath>

namespace bsp {
namespace plane_release_ver_cc13 {

ThrowMulRow pilot_bot_throw_mul_row_00999b70(bool has_record, int record_kind_8,
                                             bool plane_is_kind_10) noexcept {
    if (!has_record) return ThrowMulRow::kNone;                 // 00999B88
    if (record_kind_8 == 0x0A) return ThrowMulRow::kTorp034;    // 00999B91
    if (record_kind_8 == 0x0B) return ThrowMulRow::kDepthCharge0E0;  // 00999BB9
    if (record_kind_8 == 0x12) return ThrowMulRow::kStrike12C;  // 00999BE3
    // 00999C16 (bot+50h)->vtable[5Ch](10h).
    return plane_is_kind_10 ? ThrowMulRow::kLevelBomb0C8 : ThrowMulRow::kDiveBomb078;
}

float rack_throw_extent_006e4f30(const RackThrowInputs& in) noexcept {
    float s = in.throw_4;                                       // 006E4F0D [+3F8h]+4h
    // 006E4F38 / 006E4F48: no plane owner, or no bot, keeps the bare Throw.
    if (!in.owner_is_plane_0f || !in.has_bot_df4) return s;
    // 006E4F52 CMP 8 / JE, else 006E4F58 00927F10 / JE past the multiply.
    if (in.party_slot_1b0 != 8 && !in.slot_ai_held) return s;
    return in.throw_mul * s;                                    // 006E4F72 FMUL
}

Vec3 rack_drop_cone_006e4f91(float a_draw, float t_draw) noexcept {
    // 006E4FBA 00412E20: FSINCOS, FDIVP, rounded to float.
    const float t = static_cast<float>(std::tan(t_draw));
    const float s = static_cast<float>(std::sin(a_draw));       // 006E4FD7 FSIN
    const float c = static_cast<float>(std::cos(a_draw));       // 006E4FF7 FCOS
    return Vec3{t * c, -s * t, 1.0f};                           // 006E502F, 006E5037, 006E5025
}

Vec3 rack_round_release_velocity_006e4d50(const RackRoundReleaseInputs& in) {
    gameplay_seg53_cc13::BombReleaseOwner owner;
    owner.is_plane_5c_0f = in.owner_is_plane_0f;
    owner.speed_38 = in.owner_speed_38;
    owner.world_velocity_34 = in.owner_world_velocity_34;
    const Vec3 d = rack_drop_cone_006e4f91(in.a_draw, in.t_draw);
    return gameplay_seg53_cc13::bomb_release_velocity_006e0a70_006e1f00(in.pose_matrix, owner, d)
        .velocity;
}

Vec3 host_bomb_release_velocity(const float right[3], const float up[3],
                                const float forward[3], const Vec3& world_velocity,
                                float forward_speed, float a_draw, float t_draw) noexcept {
    float w[3] = {world_velocity.x, world_velocity.y, world_velocity.z};
    if (forward_speed > 1.3888888f) w[1] -= 3.0f;
    const float t = std::tan(t_draw);
    const float dx = t * std::cos(a_draw);
    const float dy = -t * std::sin(a_draw);
    const float m = std::sqrt(w[0] * w[0] + w[1] * w[1] + w[2] * w[2]);
    float v[3];
    for (int i = 0; i < 3; ++i) v[i] = m * (dx * right[i] + dy * up[i] + forward[i]);
    return Vec3{v[0], v[1], v[2]};
}

Vec3 host_torpedo_release_velocity(const float forward[3], float forward_speed) noexcept {
    return Vec3{forward[0] * forward_speed, forward[1] * forward_speed,
                forward[2] * forward_speed};
}

}  // namespace plane_release_ver_cc13
}  // namespace bsp
