// 006E4D50 BSP_MBombPlatform_Drop, full body (packet cc13-bombdrop, docs/CC13_BOMBDROP.md).
// Hypothesis names; offsets and constants from the listing and bytes of 006E4D50-006E56EF.

#include "bsp/cc13_bombdrop.hpp"

#include <cmath>
#include <cstring>

#include "bsp/native_camera_projection.hpp"

namespace bsp::cc13_bombdrop {

using cc13_seg33a::at;
using cc13_seg33a::ptr;

namespace {

float tangent_00412e20(float x) {
    std::uint32_t bits;
    std::memcpy(&bits, &x, sizeof bits);
    return native_projection_tangent_00412e20(bits);
}

void refresh_if_stale(BombDropHost& host, void* entity) {
    if (at<std::uint8_t>(entity, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(entity);
}

}  // namespace

// 006E4D50, __thiscall(ESI = rack, [ESP+4] byte use_given, [ESP+8] float angle, [ESP+0Ch] float
// spread), RET 0Ch. SEH frame (handler 00C8278B); state 0 only around the launch-effect handle
// (006E4E5F-006E4E8A). Float arithmetic: every intermediate the image spills is a float store;
// where x87 keeps an extended value across an operation the comment says so.
void bomb_platform_drop_006e4d50(BombDropHost& host, void* rack, std::uint8_t use_given, float angle,
                                 float spread) {
    // 006E4D7B-006E4D8F: launch effect when the definition [rack+3F8h] has an effect at +0Ch
    // (the NEG/SBB/TEST 0E17BF8h idiom is a smart-pointer "non-null" test).
    if (ptr(ptr(rack, 0x3F8), 0x0C) != nullptr) {
        void* const owner = ptr(rack, 0x3F0);
        // 006E4DA5-006E4E14: identity 4x4 at [ESP+54h].
        float matrix[16] = {1.0f, 0, 0, 0, 0, 1.0f, 0, 0, 0, 0, 1.0f, 0, 0, 0, 0, 1.0f};
        void* const parent = ptr(owner, 0x4A4);                    // 006E4E1D EDI -> EDX
        // 006E4E25-006E4E3E: the by-value handle argument; AddRef on effect+4 when non-null.
        void* consumed = nullptr;
        void* const effect = ptr(ptr(rack, 0x3F8), 0x0C);           // 006E4E2D re-read
        if (effect != nullptr) {
            consumed = effect;
            host.interlocked_increment_00ce221c(&at<std::int32_t>(effect, 4));
        }
        void* handle = nullptr;                                     // [ESP+1Ch]
        host.point_effect_create_with_parent_matrix_008687c0(&handle, parent, consumed, matrix, 1, 0, 0);
        if (handle != nullptr) at<std::uint8_t>(handle, 9) = 1;     // 006E4E6C
        host.owner_vslot_200(ptr(rack, 0x3F0), 0, 0);               // 006E4E82
        // 006E4E84-006E4EAD: release the handle; destroy through vtable+0 at zero.
        if (handle != nullptr && host.interlocked_decrement_00ce2220(&at<std::int32_t>(handle, 4)) == 0)
            host.effect_handle_destroy_vslot_0(handle);
    }

    // 006E4EAF-006E4ED6: first child (list [rack+48h], next +44h) answering kind 2Ah.
    void* child = ptr(rack, 0x48);
    while (child != nullptr && !host.is_kind_vslot_5c(child, 0x2A)) child = ptr(child, 0x44);

    if (child != nullptr) {
        // ---- cone: rack+400h angle, rack+404h spread (006E4EDB-006E4FBF)
        if (use_given != 0) {
            at<float>(rack, 0x400) = angle;                         // 006E4EF2
            at<float>(rack, 0x404) = spread;                        // 006E4F03
        } else {
            float throw_extent = at<float>(ptr(rack, 0x3F8), 0x4);  // 006E4F1E -> [ESP+18h]
            void* plane = ptr(rack, 0x3F0);
            void* bot = nullptr;
            if (plane != nullptr && host.is_kind_vslot_5c(plane, 0x0F))  // 006E4F30
                bot = ptr(plane, 0xDF4);                            // 006E4F3C
            else
                plane = nullptr;                                    // 006E4F44
            if (bot != nullptr) {
                const std::int32_t slot = at<std::int32_t>(plane, 0x1B0);  // 006E4F4C
                if (slot == 8 || host.party_slot_is_ai_held_00927f10(slot)) {   // 006E4F52 / 006E4F58
                    void* const record = ptr(ptr(rack, 0x3F8), 0x34);           // 006E4F67
                    // 006E4F6D-006E4F76: ST0 * float, stored as float.
                    throw_extent = host.pilot_bot_throw_mul_00999b70(bot, record) * throw_extent;
                }
            }
            at<float>(rack, 0x400) = host.random_uniform_float_range_00bd2f10(
                1, 0.0f, kPi_00d7a264, DrawSite::kConeAngle_006e4f91);          // 006E4F91
            const float spread_draw = host.random_uniform_float_range_00bd2f10(
                1, 0.0f, throw_extent, DrawSite::kConeSpread_006e4fb1);         // 006E4FB1
            at<float>(rack, 0x404) = tangent_00412e20(spread_draw);             // 006E4FBA
        }

        // 006E4FC5-006E5045: D = (t cos a, -t sin a, 1) at [ESP+3Ch]. FSIN/FCOS on the float
        // angle, each rounded to float before use.
        const float a = at<float>(rack, 0x400);
        const float sin_a = static_cast<float>(std::sin(static_cast<double>(a)));   // 006E4FD7
        const float cos_a = static_cast<float>(std::cos(static_cast<double>(a)));   // 006E4FF7
        const float t = at<float>(rack, 0x404);
        float direction[3];
        direction[2] = 1.0f;                                         // 006E5025
        direction[0] = t * cos_a;                                    // 006E502B-006E502F
        direction[1] = -sin_a * t;                                   // 006E5037-006E503B

        // 006E503F: w = [rack+3F8h]+8h (drift extent, [ESP+18h]).
        const float w = at<float>(ptr(rack, 0x3F8), 0x8);
        // 006E5054: owner world velocity into [ESP+20h]; copied to [ESP+2Ch..34h].
        float velocity_buffer[3];
        const float* const vp = host.owner_world_velocity_vslot_34(ptr(rack, 0x3F0), velocity_buffer);
        const float vx = vp[0], vy = vp[1], vz = vp[2];
        // 006E506A-006E50A0: each square spilled to float; the two adds in extended, one float store.
        const float sq_y = vy * vy;
        const float sq_x = vx * vx;
        const float sq_z = vz * vz;
        const float sum = static_cast<float>(static_cast<double>(sq_y) + static_cast<double>(sq_x) +
                                             static_cast<double>(sq_z));
        const float len = static_cast<float>(std::sqrt(static_cast<double>(sum)));  // 006E50A8 CRT _CIsqrt
        float inv;
        if (len > 0.0f)                                              // 006E50BF FCOMI / JBE
            inv = static_cast<float>(1.0 / static_cast<double>(len));   // 006E50C5 FLD1 / FDIVRP
        else
            inv = 0.0f;                                              // 006E50CF
        // 006E50DA-006E5111: n = inv * v at [ESP+48h..50h].
        const float nx = inv * vx;
        const float ny = vy * inv;
        const float nz = inv * vz;

        // ---- drift at rack+4ECh (006E50E3-006E51EE)
        float* const drift = &at<float>(rack, 0x4EC);
        drift[1] = 0.0f;                                             // 006E50F5
        drift[0] = nz;                                               // 006E5117
        drift[2] = -nx;                                              // 006E511F
        const float side = host.random_uniform_float_range_00bd2f10(
            1, -w, w, DrawSite::kDriftSide_006e513c);                // 006E513C
        {
            const float px = drift[0] * side;
            const float py = drift[1] * side;
            const float pz = side * drift[2];
            drift[0] = px;
            drift[1] = py;
            drift[2] = pz;                                           // 006E516D-006E517E
        }
        // 006E5181-006E51A9: hi = (float)(w * 0.30000001192092896), lo = (float)(-w * 0.5).
        const float along_hi = static_cast<float>(static_cast<double>(w) * kPointThree_00ce3dc8);
        const float along_lo = static_cast<float>(static_cast<double>(-w) * kHalf_00d7a280);
        const float along = host.random_uniform_float_range_00bd2f10(
            1, along_lo, along_hi, DrawSite::kDriftAlong_006e51ac);  // 006E51AC
        {
            const float ax = along * nx;
            const float ay = ny * along;
            const float az = along * nz;
            drift[0] = ax + drift[0];                                // 006E51D9
            drift[1] = drift[1] + ay;                                // 006E51E0
            drift[2] = drift[2] + az;                                // 006E51EA
        }

        refresh_if_stale(host, child);                               // 006E51F1-006E51FC
        void* const mover = host.round_mover_vslot_108(child);      // 006E520B -> EBP

        // 006E5214-006E525A: kind 33h with [[child+314h]+E0h] <= 0 (COMISS / JC, NaN skips):
        // D = (1, 1, 1) and no drift.
        if (host.is_kind_vslot_5c(child, 0x33) && at<float>(ptr(child, 0x314), 0xE0) <= 0.0f) {
            direction[2] = 1.0f;
            direction[1] = 1.0f;
            direction[0] = 1.0f;
            drift[2] = 0.0f;
            drift[1] = 0.0f;
            drift[0] = 0.0f;
        }

        // 006E525E-006E528A: (now - rack+480h) in extended against the double 0.3.
        const float now = host.game_time_00f876a4();
        if (static_cast<double>(now) - static_cast<double>(at<float>(rack, 0x480)) > kPointThree_00ce3dc8)
            at<float>(rack, 0x480) = host.game_time_00f876a4();     // 006E5280
        else
            at<std::uint8_t>(mover, 0x45) = 0;                      // 006E528A

        host.mover_launch_vslot_34(mover, direction, drift);        // 006E529C

        // 006E529E-006E5301: mover+8h += [[rack+3F4h]+DCh] * rack row (+DCh..+E4h).
        void* const rack_class = ptr(rack, 0x3F4);                  // 006E52A5
        refresh_if_stale(host, rack);
        {
            const float f = at<float>(rack_class, 0xDC);            // 006E52B4 -> [ESP+10h]
            const float ox = f * at<float>(rack, 0xDC);
            const float oy = at<float>(rack, 0xE0) * f;
            const float oz = f * at<float>(rack, 0xE4);
            at<float>(mover, 0x08) = ox + at<float>(mover, 0x08);
            at<float>(mover, 0x0C) = oy + at<float>(mover, 0x0C);
            at<float>(mover, 0x10) = at<float>(mover, 0x10) + oz;
        }

        // 006E5304-006E5419: the round shows; the rack copies its id, +460h and pose.
        void* const round = ptr(mover, 0xCC);                       // EDI
        cc13_seg33a::bomb_projectile_set_visible_006e0b40(host.seg33a_host(), round, 1);  // 006E530C
        at<std::uint16_t>(rack, 0x49A) = at<std::uint16_t>(round, 0x174);
        at<std::uint32_t>(rack, 0x49C) = at<std::uint32_t>(round, 0x460);
        refresh_if_stale(host, round);                              // 006E532B-006E5336
        for (int i = 0; i < 16; ++i)                                // 006E533B-006E53F5, FLD/FSTP float
            at<float>(rack, 0x4A0 + 4 * i) = at<float>(round, 0xCC + 4 * i);
        at<float>(rack, 0x4E0) = at<float>(round, 0x318);
        at<float>(rack, 0x4E4) = at<float>(round, 0x31C);
        at<float>(rack, 0x4E8) = at<float>(round, 0x320);

        // ---- paratrooper branch (006E541F-006E5515): rack+4F8h set and the child is kind 31h.
        if (at<std::uint8_t>(rack, 0x4F8) != 0 && host.is_kind_vslot_5c(child, 0x31)) {
            float landing[3] = {at<float>(rack, 0x4FC), at<float>(rack, 0x500), at<float>(rack, 0x504)};
            landing[0] = host.random_uniform_float_range_00bd2f10(
                0, kParaScatterLo_00ceca0c, kParaScatterHi_00ceb4d4, DrawSite::kParaX_006e5479) + landing[0];
            landing[2] = host.random_uniform_float_range_00bd2f10(
                0, kParaScatterLo_00ceca0c, kParaScatterHi_00ceb4d4, DrawSite::kParaZ_006e549e) + landing[2];
            host.paratrooper_set_landing_pos_007ab230(child, landing);   // 006E54B2
            // 006E54B7-006E5515: rack+4FCh += [rack+494h] * rack+508h..510h.
            const float f = at<float>(rack, 0x494);
            const float sx = at<float>(rack, 0x508) * f;
            const float sy = at<float>(rack, 0x50C) * f;
            const float sz = f * at<float>(rack, 0x510);
            at<float>(rack, 0x4FC) = sx + at<float>(rack, 0x4FC);
            at<float>(rack, 0x500) = at<float>(rack, 0x500) + sy;
            at<float>(rack, 0x504) = sz + at<float>(rack, 0x504);
        }

        // 006E551B-006E5527: stamp the owner's party slot.
        at<std::int32_t>(mover, 0x1C) = at<std::int32_t>(ptr(rack, 0x3F0), 0x1B0);

        // 006E552A-006E55C4: clear the stamp when the drop's +ACh is the local player entity (or,
        // for a kind-18h player, one of its first five slot entries).
        void* const game = host.game_00e188a8();
        if (at<std::int32_t>(game, 0x1FE4) == 0 && host.local_player_entity_00e188d8() != nullptr) {
            void* const gate = ptr(game, 0x1ED4);
            if (gate != nullptr && host.is_kind_vslot_5c(gate, 0x4E)) {            // 006E555A
                if (host.is_kind_vslot_5c(host.local_player_entity_00e188d8(), 0x18)) {   // 006E556D
                    void* const player = host.local_player_entity_00e188d8();
                    const std::int32_t count = at<std::int32_t>(player, 0x3CC);
                    void* const target = ptr(mover, 0xAC);
                    for (std::int32_t i = 0; i < count; ++i) {
                        // 006E5591-006E559A: entries past index 4 read as null (unsigned JA).
                        void* const entry = static_cast<std::uint32_t>(i) <= 4u
                                                ? ptr(player, 0x3D0 + 4 * static_cast<std::size_t>(i))
                                                : nullptr;
                        if (target == entry) {
                            at<std::int32_t>(mover, 0x1C) = 0;       // 006E55C4
                            break;
                        }
                    }
                } else if (ptr(mover, 0xAC) == host.local_player_entity_00e188d8()) {   // 006E55B6
                    at<std::int32_t>(mover, 0x1C) = 0;
                }
            }
        }

        host.owner_register_projectile_009555a0(ptr(rack, 0x3F0), mover);   // 006E55D2
        refresh_if_stale(host, child);                               // 006E55D7-006E55E2
        if (at<std::int32_t>(rack, 0x484) > 0)                       // 006E55E7 JLE (signed)
            host.rack_vslot_224(rack);                               // 006E55FA
        else
            at<std::uint8_t>(rack, 0x4F8) = 0;                       // 006E55FE
    }

    // 006E5605-006E5633: the ammo-type warning for a plane ancestor's squadron.
    if (host.is_kind_vslot_5c(host.entity_ancestor_00923810(rack, 1), 0x0F)) {
        void* const plane = host.entity_ancestor_00923810(rack, 1);
        host.warning_dispatch_ammo_type_00984ba0(host.warning_manager_00f8a0c4(), ptr(plane, 0x9D4));
    }

    // 006E5638-006E56C4: the time since the plane's previous drop, recorded per slot.
    if (host.is_kind_vslot_5c(host.entity_ancestor_00923810(rack, 1), 0x0F)) {
        void* const plane = host.entity_ancestor_00923810(rack, 1);
        const float last = at<float>(plane, 0xE8C);                 // 006E5659 -> [ESP+14h]
        at<float>(plane, 0xE8C) = 0.0f;                             // 006E566C
        void* const squadron = ptr(plane, 0x9D4);
        const float interval = host.game_time_00f876a4() - last;    // 006E5674-006E5684, float store
        if (ptr(squadron, 0x3D0) == plane &&                        // 006E567A squadron leader
            at<std::uint32_t>(plane, 0x1B0) > 7u &&                 // 006E5691 JBE (unsigned)
            interval > 0.0f && last > 0.0f) {                       // 006E5699 / 006E56A4 COMISS
            host.slot_drop_interval_record_0090e670(ptr(host.game_00e188a8(), 0x21A0),
                                                    at<std::int32_t>(plane, 0xE90), plane, interval);
        }
    }

    at<float>(rack, 0x490) = at<float>(rack, 0x48C);                // 006E56C9-006E56D7
}

}  // namespace bsp::cc13_bombdrop
