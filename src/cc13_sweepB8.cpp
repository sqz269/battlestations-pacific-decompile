// cc13_sweepB8: eighth pass of the segment 15-51 leftover sweep (packet
// cc13-sweepB8, docs/CC13_SWEEPB8.md). Names are hypotheses; see the header
// for the hooks.
//
// x87 note (as cc13_sweepB..B7 and cc13_pkt642): float expressions follow the
// image's operation order with a float store wherever the image stores to a
// float slot; intermediate x87 values are kept in double, and double
// constants are applied in double. EH frames are noted, not modelled.

#include "bsp/cc13_sweepB8.hpp"

#include "bsp/ng2_cc13.hpp"

#include <cmath>
#include <cstring>
#include <initializer_list>

namespace bsp {

namespace {

template <class T>
T& at(void* base, std::size_t offset) {
    return cc13_sweepB_at<T>(base, offset);
}

float f32_bits(std::uint32_t bits) {
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

double f64_bits(std::uint64_t bits) {
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

// The colour vector handed to 005A3830 / 005A7550 travels in the stack slot
// those reconstructions type as an int (their widget vtable +50h argument);
// the image pushes a pointer there (Win32, 32-bit).
std::int32_t pointer_arg(const void* pointer) {
    return static_cast<std::int32_t>(reinterpret_cast<std::intptr_t>(pointer));
}

// [[00E188A8] + 18CCh + 4 * [00E188A8]+18ECh] + 28h: the local player's party.
void* local_party(Cc13SweepB8Host& host) {
    void* game = host.global_00e188a8();
    void* player = at<void*>(game, 0x18CC + 4 * static_cast<std::size_t>(at<std::int32_t>(game, 0x18EC)));
    return at<void*>(player, 0x28);
}

}  // namespace

// ---------------------------------------------------------------------------
// 005A9DB0 BSP_HudMapScreen_BuildUnitOrderOverlays
// __thiscall(ECX = HUD map screen; unit, int flag), RET 8; frame 170h, no EH
// frame. Sole caller 005B2D70. No draw from the shared RNG 00BD2F10.
// Evidence 005A9DB0..005AAACA (listing paged; stack offsets checked against ESP
// after each push; x87 operands are memory forms or commutative FMULs except
// the FDIVR double, ST0 = 20070.0 / screen+30h):
// - c1 = 0059AA40(0.02f, 0.01f, 0.005f, 0.0025f, screen+1Ch),
//   c2 = 0059AA40(0.02f x4, screen+1Ch). c1 sits in slots ESP+1Ch and ESP+30h.
// - Unit kind 1Ch (vtable +5Ch) false: flag != 0 returns; neither kind 5 nor
//   18h jumps to the director part. Otherwise the owner is unit+538h (kind
//   5), [unit+3D0h]+538h (kind 18h), or, when neither (only reachable for a
//   kind-1Ch unit), the unassigned slot ESP+30h, which then holds c1's float
//   bits: the image reads those bits as the owner pointer (modelled as is).
//   table = owner+B4h; null returns. record = table + 60h * (unit+1E4h)
//   ->vtable+4h() + 8.
// - unit->C4h(pos). s = (float)(20070.0 / screen+30h * 4.9825610363727e-05).
//   Scale matrix S = diag(s, s, 0, 1) (row 2 all zero), translation frame
//   F (ESP+B4h) = identity with row 3 (pos.x, 0, pos.z, 1); vector
//   (0.00333f, 0, 0, 1) at ESP+A4h. 00B63B30(inverse, S), then 00B62D10(that
//   vector, out ESP+60h, inverse); the result is not read again.
// - 005A1800(screen+4F4h, it, &unit) and the inlined checked-iterator tests
//   (00BF6713 when the container is null or not the tree, and when a non-end
//   node is the tree's head); the found entry is not used.
// - For each of record+58h entries (record+54h, 20h bytes):
//   * kind 1Ch: gui = 004C12B0(); screen+554h = (gui+5Ch - 0.5) * screen+30h
//     + screen+54Ch, screen+558h = (0.5 - gui+60h) * screen+34h + screen+550h
//     (the .data halves 00E08890/00E08894); x = unit->C4h()[0], z =
//     unit->C4h()[2] (two calls, two buffers); d = sqrt(dx*dx + dz*dz) with
//     float stores after each square and the sum; when (double)unit+7C4h +
//     300.0 > d (FCOMIP, JBE skips: unordered skips) 005A91E0(screen,
//     (float)(f / 500.0), {pos.x, pos.z}, 1070.0f, f = (float)unit+7C4h,
//     (float)unit+7A0h, (float)(c1 * 5000.0)).
//   * flag 0 only: w = [[00E188A8]+21C4h]+74h * entry+8h (float). Entry kind
//     0: h = w / 5000.0; 005A8F50(F, (float)(h + c1), 7) and (F, (float)h,
//     9). Kind 1: the same with layers 3 and 5. Kind 2: v = (float)(+78h *
//     w), h = v / 5000.0; byte entry+4h zero: 005A8F50 layers 0Bh/0Dh; else
//     005A9770(F, unit->C8h(), (float)(h + c1), c1, c2, entry+0Ch, 0Bh) then
//     (F, unit->C8h(), (float)h, 0, 0, entry+0Ch, 0Dh); entry+0Ch and the
//     scale are read before the vtable call.
// - flag != 0 returns after the loop.
// - Director part: 005A16F0(screen+78h, it, &unit) with the container test;
//   found = node != the head read before the call. idx/b17: kind 45h or 46h
//   -> 0 / 1, else 1 / kind 9. follower = 007788B0(unit). Unless found:
//   (follower && !b17) or unit+188h+4*idx == 8 goes on only when
//   game+1FE4h != 0 and unit+54h is the local party, else it ends at the tail.
// - style = 2 when unit->140h() == [00E188D8]; else 1 when found, unit+18Ch
//   == 9 or unit+18Ch == game+18ECh; else 0. Colours A (0.486, 0.0667,
//   0.0353, 0.8), B (0.0667, 0.1725, 0.349, 0.8), C (0.0784, 0.2706, 0.0353,
//   0.8).
// - d = unit->114h(). b17 && d && d->1Ch(): leader = d->1Ch() (second call);
//   bytes +5Ch set, +5Dh/+60h/+5Eh clear and 00803CE0(unit+54h, leader)
//   nonzero: 005A3830(005A4A00(screen), screen, unit->C4h(), leader->C4h()
//   (evaluated first), A, style).
// - follower ends at the tail. d null: no slots. Else byte d+18Ch set: t =
//   00521EA0(d+18Ch); r = t kind 2 ? 00803CE0(unit+54h, t) : 0; 005A3830(
//   arrow, screen, unit->C4h(), t->C4h(), r ? A : C, style); n =
//   0071BE60(d).
// - p = unit->C4h(); for i < n with d->20h(i): the 18h-byte slot
//   d+58h+1Ch*i is copied. Byte 0 set: t = 00521EA0(copy), lead = d->2Ch(),
//   r as above, arrow, colour r ? (t == lead ? A : B) : C, 005A3830(arrow,
//   screen, p, t->C4h(), colour, style), p = t->C4h(). Byte 1 set: arrow,
//   005A3830(arrow, screen, p, copy+8h, B, style), p = copy+8h. Else holder
//   = 0071BDE0(d, i), k = holder->14h(), 0 <= k < 0059CAC0(holder->8h())
//   (signed): 005A7550(arrow, screen, p, holder->8h(), k, B, style), p =
//   0059CAE0(holder->8h(), tmp, size - 1).
// - Tail: o = unit->30h(); o and o+8h non-null and unit+54h the local party:
//   0059DD20(005A4A00(screen), screen, o8+1Ch, o8+28h, o8+34h).
void cc13_sweepB8_hud_map_build_unit_order_overlays_005a9db0(void* screen, void* unit, std::int32_t flag,
                                                            Cc13SweepB8Host& host) {
    const float t = at<float>(screen, 0x1C);
    const float c1 = ng2_cc13::piecewise_lerp4_0059aa40(f32_bits(0x3CA3D70Au), f32_bits(0x3C23D70Au), f32_bits(0x3BA3D70Au),
                                              f32_bits(0x3B23D70Au), t);
    const float k002 = f32_bits(0x3CA3D70Au);  // 00CE9BAC
    const float c2 = ng2_cc13::piecewise_lerp4_0059aa40(k002, k002, k002, k002, t);

    bool director_part_only = false;
    if (!host.entity_vcall_5c_is_kind(unit, 0x1C)) {
        if (flag != 0) return;
        if (!host.entity_vcall_5c_is_kind(unit, 5) && !host.entity_vcall_5c_is_kind(unit, 0x18)) {
            director_part_only = true;
        }
    }

    if (!director_part_only) {
        void* owner;
        if (host.entity_vcall_5c_is_kind(unit, 5)) {
            owner = at<void*>(unit, 0x538);
        } else if (host.entity_vcall_5c_is_kind(unit, 0x18)) {
            owner = at<void*>(at<void*>(unit, 0x3D0), 0x538);
        } else {
            std::uint32_t bits;
            std::memcpy(&bits, &c1, sizeof bits);
            owner = reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
        }
        void* table = at<void*>(owner, 0xB4);
        if (table == nullptr) return;
        const std::int32_t row = host.unit_sub_1e4_vcall_4_index(&at<std::uint8_t>(unit, 0x1E4));
        void* record = static_cast<std::uint8_t*>(table) + 0x60 * row + 8;

        float pos[3];
        host.unit_vcall_c4_position(unit, pos);
        const float s = static_cast<float>(f64_bits(0x40D3998000000000ull) / static_cast<double>(at<float>(screen, 0x30)) *
                                           f64_bits(0x3F0A1F7AEFA5CB4Bull));
        float scale_matrix[16] = {s, 0, 0, 0, 0, s, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1.0f};
        const float frame[16] = {1.0f, 0, 0, 0, 0, 1.0f, 0, 0, 0, 0, 1.0f, 0, pos[0], 0, pos[2], 1.0f};
        const float marker[4] = {f32_bits(0x3B5A3C21u), 0, 0, 1.0f};  // 00CF01E0
        float inverse[16];
        float transformed[4];
        host.vector4_transform_00b62d10(marker, transformed, host.matrix_invert_affine_00b63b30(inverse, scale_matrix));

        void* tree = &at<std::uint8_t>(screen, 0x4F4);
        void* iterator[2];
        void* key = unit;
        host.tree_lower_bound_005a1800(tree, iterator, &key);
        void* head = at<void*>(tree, 4);
        if (iterator[0] == nullptr || iterator[0] != tree) host.crt_invalid_parameter_noinfo_00bf6713();
        if (iterator[1] != head) {
            if (iterator[0] == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();
            if (iterator[1] == at<void*>(iterator[0], 4)) host.crt_invalid_parameter_noinfo_00bf6713();
        }

        const double k5000 = f64_bits(0x40B3880000000000ull);  // 00CF01D0
        for (std::int32_t i = 0; i < at<std::int32_t>(record, 0x58); ++i) {
            if (host.entity_vcall_5c_is_kind(unit, 0x1C)) {
                void* gui = host.gui_manager_get_004c12b0();
                const float gx = at<float>(gui, 0x5C);
                const float gy = at<float>(gui, 0x60);
                const float half_x = *static_cast<float*>(host.image_global(0x00E08890));
                const float half_y = *static_cast<float*>(host.image_global(0x00E08894));
                const float cx = static_cast<float>((static_cast<double>(gx) - half_x) * at<float>(screen, 0x30) +
                                                    at<float>(screen, 0x54C));
                const float cy = static_cast<float>((static_cast<double>(half_y) - gy) * at<float>(screen, 0x34) +
                                                    at<float>(screen, 0x550));
                at<float>(screen, 0x554) = cx;
                at<float>(screen, 0x558) = cy;
                float out_a[3];
                float out_b[3];
                const float x = host.unit_vcall_c4_position(unit, out_a)[0];
                const float z = host.unit_vcall_c4_position(unit, out_b)[2];
                const float dx = x - at<float>(screen, 0x554);
                const float dz = z - at<float>(screen, 0x558);
                const float dz2 = dz * dz;
                const float dx2 = dx * dx;
                const float sum = dz2 + dx2;
                const float dist = static_cast<float>(host.crt_sqrt_00bf7030(sum));
                if (static_cast<double>(at<std::int32_t>(unit, 0x7C4)) + f64_bits(0x4072C00000000000ull) >
                    static_cast<double>(dist)) {
                    const float land = static_cast<float>(at<std::int32_t>(unit, 0x7C4));
                    const float xz[2] = {pos[0], pos[2]};
                    const float band = static_cast<float>(static_cast<double>(c1) * k5000);
                    const float capture = static_cast<float>(at<std::int32_t>(unit, 0x7A0));
                    const float scale = static_cast<float>(static_cast<double>(land) / f64_bits(0x407F400000000000ull));
                    cc13_sweepB5_hud_map_build_capture_radius_model_005a91e0(screen, scale, xz, f32_bits(0x4485C000u),
                                                                             land, capture, band, host);
                }
            }
            if (flag != 0) continue;

            void* entry = static_cast<std::uint8_t*>(at<void*>(record, 0x54)) + 0x20 * i;
            void* tuning = at<void*>(host.global_00e188a8(), 0x21C4);
            const float w = at<float>(tuning, 0x74) * at<float>(entry, 0x8);
            const std::int32_t kind = at<std::int32_t>(entry, 0);
            if (kind == 0 || kind == 1) {
                const double h = static_cast<double>(w) / k5000;
                cc13_sweepB3_hud_map_acquire_line_model_005a8f50(screen, frame, static_cast<float>(h + c1),
                                                                 kind == 0 ? 7u : 3u, host);
                cc13_sweepB3_hud_map_acquire_line_model_005a8f50(screen, frame, static_cast<float>(h),
                                                                 kind == 0 ? 9u : 5u, host);
            } else if (kind == 2) {
                const float v = at<float>(tuning, 0x78) * w;
                const double h = static_cast<double>(v) / k5000;
                if (at<std::uint8_t>(entry, 4) == 0) {
                    cc13_sweepB3_hud_map_acquire_line_model_005a8f50(screen, frame, static_cast<float>(h + c1), 0xB,
                                                                     host);
                    cc13_sweepB3_hud_map_acquire_line_model_005a8f50(screen, frame, static_cast<float>(h), 0xD, host);
                } else {
                    float cone = at<float>(entry, 0xC);
                    float scale = static_cast<float>(h + c1);
                    float heading = host.unit_vcall_c8_heading(unit);
                    cc13_sweepB5_hud_map_acquire_cone_model_005a9770(screen, frame, heading, scale, c1, c2, cone, 0xB,
                                                                     host);
                    cone = at<float>(static_cast<std::uint8_t*>(at<void*>(record, 0x54)) + 0x20 * i, 0xC);
                    scale = static_cast<float>(h);
                    heading = host.unit_vcall_c8_heading(unit);
                    cc13_sweepB5_hud_map_acquire_cone_model_005a9770(screen, frame, heading, scale, 0.0f, 0.0f, cone,
                                                                     0xD, host);
                }
            }
        }
        if (flag != 0) return;
    }

    // Director part (005AA4B1).
    void* tree = &at<std::uint8_t>(screen, 0x78);
    void* head = at<void*>(screen, 0x7C);
    void* iterator_storage[2];
    void* key = unit;
    void** iterator = host.tree_lower_bound_005a16f0(tree, iterator_storage, &key);
    if (iterator[0] == nullptr || iterator[0] != tree) host.crt_invalid_parameter_noinfo_00bf6713();
    const bool found = iterator[1] != head;

    std::int32_t idx = 1;
    bool b17;
    if (host.entity_vcall_5c_is_kind(unit, 0x45) || host.entity_vcall_5c_is_kind(unit, 0x46)) {
        idx = 0;
        b17 = true;
    } else {
        b17 = host.entity_vcall_5c_is_kind(unit, 9);
    }
    const bool follower = host.unit_is_formation_follower_007788b0(unit);
    if (!found) {
        const bool to_check = (follower && !b17) || at<std::int32_t>(unit, 0x188 + 4 * static_cast<std::size_t>(idx)) == 8;
        if (to_check) {
            void* game = host.global_00e188a8();
            if (at<std::int32_t>(game, 0x1FE4) == 0 || at<void*>(unit, 0x54) != local_party(host)) goto tail;
        }
    }

    {
        std::int32_t style = 0;
        if (host.unit_vcall_140(unit) == host.global_00e188d8()) {
            style = 2;
        } else if (found || at<std::int32_t>(unit, 0x18C) == 9 ||
                   at<std::int32_t>(unit, 0x18C) == at<std::int32_t>(host.global_00e188a8(), 0x18EC)) {
            style = 1;
        }
        const float alpha = f32_bits(0x3F4CCCCDu);  // 00CE74F8
        const float colour_a[4] = {f32_bits(0x3EF8F8F9u), f32_bits(0x3D888889u), f32_bits(0x3D109091u), alpha};
        const float colour_b[4] = {f32_bits(0x3D888889u), f32_bits(0x3E30B0B1u), f32_bits(0x3EB2B2B3u), alpha};
        const float colour_c[4] = {f32_bits(0x3DA0A0A1u), f32_bits(0x3E8A8A8Bu), f32_bits(0x3D109091u), alpha};

        void* director = host.unit_vcall_114_director(unit);
        if (b17 && director != nullptr && host.director_vcall_1c(director) != nullptr) {
            void* leader = host.director_vcall_1c(director);
            if (at<std::uint8_t>(leader, 0x5C) != 0 && at<std::uint8_t>(leader, 0x5D) == 0 &&
                at<std::uint8_t>(leader, 0x60) == 0 && at<std::uint8_t>(leader, 0x5E) == 0 &&
                host.recon_relation_if_identified_00803ce0(at<void*>(unit, 0x54), leader) != 0) {
                void* arrow = host.hud_map_acquire_arrow_widget_005a4a00(screen);
                float to_buffer[3];
                float from_buffer[3];
                const float* to = host.unit_vcall_c4_position(leader, to_buffer);
                const float* from = host.unit_vcall_c4_position(unit, from_buffer);
                cc13_sweepB3_hud_map_arrow_set_ai_arrow_005a3830(arrow, screen, from, to, pointer_arg(colour_a), style,
                                                                 host);
            }
        }
        if (follower) goto tail;

        std::int32_t count = 0;
        if (director != nullptr) {
            if (at<std::uint8_t>(director, 0x18C) != 0) {
                void* target = host.command_target_resolve_00521ea0(&at<std::uint8_t>(director, 0x18C));
                const std::uint32_t relation = host.entity_vcall_5c_is_kind(target, 2)
                                                   ? host.recon_relation_if_identified_00803ce0(at<void*>(unit, 0x54), target)
                                                   : 0u;
                void* arrow = host.hud_map_acquire_arrow_widget_005a4a00(screen);
                const float* colour = relation != 0 ? colour_a : colour_c;
                float to_buffer[3];
                float from_buffer[3];
                const float* to = host.unit_vcall_c4_position(target, to_buffer);
                const float* from = host.unit_vcall_c4_position(unit, from_buffer);
                cc13_sweepB3_hud_map_arrow_set_ai_arrow_005a3830(arrow, screen, from, to, pointer_arg(colour), style,
                                                                 host);
            }
            count = host.director_filled_slot_count_0071be60(director);
        }

        float point[3];
        host.unit_vcall_c4_position(unit, point);
        for (std::int32_t i = 0; i < count; ++i) {
            if (!host.director_vcall_20_slot_active(director, i)) continue;
            std::uint8_t* slot = &at<std::uint8_t>(director, 0x58 + 0x1C * static_cast<std::size_t>(i));
            std::uint8_t copy[0x18];
            std::memcpy(copy + 0x8, slot + 0x8, 4);
            std::memcpy(copy + 0x0, slot + 0x0, 2);
            std::memcpy(copy + 0x2, slot + 0x2, 2);
            std::memcpy(copy + 0xC, slot + 0xC, 4);
            std::memcpy(copy + 0x10, slot + 0x10, 4);
            std::memcpy(copy + 0x4, slot + 0x4, 4);
            std::memcpy(copy + 0x14, slot + 0x14, 4);
            if (copy[0] != 0) {
                void* target = host.command_target_resolve_00521ea0(copy);
                void* lead = host.director_vcall_2c(director);
                const std::uint32_t relation = host.entity_vcall_5c_is_kind(target, 2)
                                                   ? host.recon_relation_if_identified_00803ce0(at<void*>(unit, 0x54), target)
                                                   : 0u;
                void* arrow = host.hud_map_acquire_arrow_widget_005a4a00(screen);
                const float* colour = relation == 0 ? colour_c : (target == lead ? colour_a : colour_b);
                float to_buffer[3];
                const float* to = host.unit_vcall_c4_position(target, to_buffer);
                cc13_sweepB3_hud_map_arrow_set_ai_arrow_005a3830(arrow, screen, point, to, pointer_arg(colour), style,
                                                                 host);
                float next_buffer[3];
                const float* next = host.unit_vcall_c4_position(target, next_buffer);
                point[0] = next[0];
                point[1] = next[1];
                point[2] = next[2];
            } else if (copy[1] != 0) {
                void* arrow = host.hud_map_acquire_arrow_widget_005a4a00(screen);
                float to[3];
                std::memcpy(to, copy + 0x8, sizeof to);
                cc13_sweepB3_hud_map_arrow_set_ai_arrow_005a3830(arrow, screen, point, to, pointer_arg(colour_b), style,
                                                                 host);
                std::memcpy(point, copy + 0x8, sizeof point);
            } else {
                void* holder = host.director_slot_path_0071bde0(director, i);
                if (holder == nullptr) continue;
                const std::int32_t first = host.path_holder_vcall_14_index(holder);
                if (first < 0) continue;
                if (first >= host.vector12_size_0059cac0(host.path_holder_vcall_8_points(holder))) continue;
                void* arrow = host.hud_map_acquire_arrow_widget_005a4a00(screen);
                cc13_sweepB4_hud_map_arrow_set_waypoint_path_005a7550(arrow, screen, point,
                                                                      host.path_holder_vcall_8_points(holder), first,
                                                                      pointer_arg(colour_b), style, host);
                const std::int32_t last = host.vector12_size_0059cac0(host.path_holder_vcall_8_points(holder)) - 1;
                float out[3];
                const float* end = host.vector12_copy_at_0059cae0(host.path_holder_vcall_8_points(holder), out, last);
                point[0] = end[0];
                point[1] = end[1];
                point[2] = end[2];
            }
        }
    }

tail:
    void* order = host.unit_vcall_30(unit);
    if (order == nullptr) return;
    void* line = at<void*>(order, 0x8);
    if (line == nullptr) return;
    if (at<void*>(unit, 0x54) != local_party(host)) return;
    void* arrow = host.hud_map_acquire_arrow_widget_005a4a00(screen);
    host.hud_map_arrow_set_line_0059dd20(arrow, screen, &at<float>(line, 0x1C), &at<float>(line, 0x28),
                                         at<float>(line, 0x34));
}

namespace {

// The inlined native-string release: data non-null goes back to the sized
// pool with length + 1 bytes (00419CC0, then 00BD1510(data, length + 1, 1)).
void release_string(Cc13SweepBNativeString& text, Cc13SweepB8Host& host) {
    if (text.data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), text.data, text.length + 1, 1);
    }
}

// Clock [01090AB0] vtable +20h: {int64 counter, int64 frequency}; the image
// divides the two FILD values (DE F9: counter / frequency) and stores a float.
// Kept in double here (exact while the counter stays below 2^53).
float sample_clock_seconds(Cc13SweepB8Host& host, void* buffer) {
    const std::int64_t* clock = host.timer_vcall_20_read(*static_cast<void**>(host.image_global(0x01090AB0)), buffer);
    return static_cast<float>(static_cast<double>(clock[0]) / static_cast<double>(clock[1]));
}

// One of the two file names of 00789560 and its open: "ns_" + name + side +
// tag + month "-" day "-" hour "-" minute "-" second ".csv". Construction and
// release order follow the image: the named strings ".csv", four "-", the tag
// and "ns_" are built first (dash k is used after the (4-k)th number), the
// fourteen temporaries are released newest first after the open, then "ns_",
// the tag, the dashes newest first and ".csv".
void* open_csv_log(void* stats, const Cc13SweepBNativeString& side, const char* tag, const std::uint16_t* time,
                   Cc13SweepB8Host& host) {
    Cc13SweepBNativeString csv = {0, nullptr};
    Cc13SweepBNativeString dash[4] = {{0, nullptr}, {0, nullptr}, {0, nullptr}, {0, nullptr}};
    Cc13SweepBNativeString tag_text = {0, nullptr};
    Cc13SweepBNativeString prefix = {0, nullptr};
    host.native_string_assign_0041e870(&csv, ".csv");                     // 00D04344
    for (auto& text : dash) host.native_string_assign_0041e870(&text, "-");  // 00CEED40
    host.native_string_assign_0041e870(&tag_text, tag);
    void* file_system = *static_cast<void**>(host.image_global(0x0109CEEC));  // read before "ns_"
    void* current = host.native_string_assign_0041e870(&prefix, "ns_");   // 00D04338

    Cc13SweepBNativeString temp[14];
    for (auto& text : temp) text = {0, nullptr};
    int n = 0;
    current = host.native_string_concat_004261a0(current, &temp[n++], &at<std::uint8_t>(stats, 0x7649AC));
    current = host.native_string_concat_004261a0(current, &temp[n++], &side);
    current = host.native_string_concat_004261a0(current, &temp[n++], &tag_text);
    // SYSTEMTIME words: +2h month, +6h day, +8h hour, +Ah minute, +Ch second.
    const std::uint16_t fields[5] = {time[1], time[3], time[4], time[5], time[6]};
    for (int k = 0; k < 5; ++k) {
        current = host.native_string_concat_int_004263b0(current, &temp[n++], fields[k]);
        const Cc13SweepBNativeString* separator = k < 4 ? &dash[3 - k] : &csv;
        current = host.native_string_concat_004261a0(current, &temp[n++], separator);
    }
    void* file = host.file_system_vcall_4_open(file_system, current, 0x35);

    for (int k = n - 1; k >= 0; --k) release_string(temp[k], host);
    release_string(prefix, host);
    release_string(tag_text, host);
    for (int k = 3; k >= 0; --k) release_string(dash[k], host);
    release_string(csv, host);
    return file;
}

}  // namespace

// ---------------------------------------------------------------------------
// 00789560 BSP_NetworkPacketStatistics_StartCsvLog (toggle)
// __fastcall(ECX = statistics block), RET; EH frame 00C8AA9B (the string
// temporaries, not modelled). Sole caller 0088D6E0. No draw from the shared
// RNG 00BD2F10.
// Evidence 00789560..0078A2BF:
// - f = (float)(counter / frequency) of the clock (vtable +20h); +76498Ch = f;
//   +762968h = low dword of FISTP qword of f under a truncating control word.
// - Byte +762900h set (0078A25D): 007884D0(stats); for +7649A0h then +7649A4h,
//   InterlockedDecrement(file+4h) and, at zero, file vtable +0h (no null test).
// - Clear: GetSystemTime; side = a copy of "_server" (game+1FE4h == 1) or
//   "_client" (0041DD40 resize to the source length with preserve 1, memcpy
//   of side.length bytes when the source is non-empty; the self-assignment
//   test the image makes is always false here), the temporary released;
//   +7649A0h = open of the "_send_" name, +7649A4h = open of the "_recv_" name
//   (mode 35h each, open_csv_log above). The 1001 rows of 50h bytes from
//   +74F02Ch: dwords +0/+4/+8/+10h/+14h/+18h = 0, words +0Ch/+1Ch = 9999,
//   +0Eh/+1Eh = 0 (the image's store order kept). +762960h = +762958h =
//   +764994h = 0, +764998h = +76499Ch, +76295Ch = 1000; the clock is sampled
//   again into +764930h and +76492Ch; side released.
// - Byte +762900h = (old byte == 0); AL holds it on return.
bool cc13_sweepB8_packet_stats_toggle_csv_log_00789560(void* stats, Cc13SweepB8Host& host) {
    std::uint8_t clock_buffer[16];
    const float now = sample_clock_seconds(host, clock_buffer);
    at<float>(stats, 0x76498C) = now;
    at<std::int32_t>(stats, 0x762968) = static_cast<std::int32_t>(static_cast<std::int64_t>(now));

    if (at<std::uint8_t>(stats, 0x762900) == 0) {
        std::uint16_t time[8];
        host.get_system_time_00ce226c(time);
        Cc13SweepBNativeString source = {0, nullptr};
        if (at<std::int32_t>(host.global_00e188a8(), 0x1FE4) == 1) {
            host.native_string_assign_0041e870(&source, "_server");  // 00D04354
        } else {
            host.native_string_assign_0041e870(&source, "_client");  // 00D0434C
        }
        Cc13SweepBNativeString side = {0, nullptr};
        host.native_string_resize_0041dd40(&side, source.length, 1);
        if (source.length != 0) host.crt_memcpy_00bf7680(side.data, source.data, side.length);
        release_string(source, host);

        at<void*>(stats, 0x7649A0) = open_csv_log(stats, side, "_send_", time, host);  // 00D0433C
        at<void*>(stats, 0x7649A4) = open_csv_log(stats, side, "_recv_", time, host);  // 00D04330

        for (std::size_t k = 0; k < 1001; ++k) {
            const std::size_t row = 0x74F02C + 0x50 * k;
            at<std::uint16_t>(stats, row + 0xE) = 0;
            at<std::uint32_t>(stats, row + 0x8) = 0;
            at<std::uint32_t>(stats, row + 0x4) = 0;
            at<std::uint32_t>(stats, row + 0x0) = 0;
            at<std::uint16_t>(stats, row + 0xC) = 9999;
            at<std::uint16_t>(stats, row + 0x1E) = 0;
            at<std::uint32_t>(stats, row + 0x18) = 0;
            at<std::uint32_t>(stats, row + 0x14) = 0;
            at<std::uint32_t>(stats, row + 0x10) = 0;
            at<std::uint16_t>(stats, row + 0x1C) = 9999;
        }
        at<std::uint32_t>(stats, 0x762960) = 0;
        at<std::uint32_t>(stats, 0x762958) = 0;
        at<std::uint32_t>(stats, 0x764994) = 0;
        at<std::uint32_t>(stats, 0x764998) = at<std::uint32_t>(stats, 0x76499C);
        at<std::int32_t>(stats, 0x76295C) = 1000;
        const float again = sample_clock_seconds(host, clock_buffer);
        at<float>(stats, 0x764930) = again;
        at<float>(stats, 0x76492C) = again;
        release_string(side, host);
    } else {
        host.packet_stats_write_csv_row_007884d0(stats);
        for (std::size_t slot : {std::size_t{0x7649A0}, std::size_t{0x7649A4}}) {
            void* file = at<void*>(stats, slot);
            if (host.interlocked_decrement_00ce2220(&at<std::int32_t>(file, 4)) == 0) {
                host.refcounted_vcall_0_release(file);
            }
        }
    }
    const bool enabled = at<std::uint8_t>(stats, 0x762900) == 0;
    at<std::uint8_t>(stats, 0x762900) = enabled ? 1 : 0;
    return enabled;
}

namespace {

// The inlined NativeString copy: resize the destination to the source length
// (preserve 1), then memcpy destination-length bytes when the source is
// non-empty.
void copy_string(Cc13SweepBNativeString& dest, const void* source, Cc13SweepB8Host& host) {
    const auto& from = *static_cast<const Cc13SweepBNativeString*>(source);
    host.native_string_resize_0041dd40(&dest, from.length, 1);
    if (from.length != 0) host.crt_memcpy_00bf7680(dest.data, from.data, dest.length);
}

// The inlined construction from a C string: zero header, resize to the
// literal's length, memcpy length + 1 bytes when data was allocated.
void construct_literal(Cc13SweepBNativeString& dest, const char* text, std::uint32_t length, Cc13SweepB8Host& host) {
    dest = {0, nullptr};
    host.native_string_resize_0041dd40(&dest, length, 1);
    if (dest.data != nullptr) host.crt_memcpy_00bf7680(dest.data, text, dest.length + 1);
}

std::int32_t listbox_key(void* listener, Cc13SweepB8Host& host) {
    void* listbox = at<void*>(listener, 0x24);
    const std::int32_t top = at<std::int32_t>(listbox, 0x164);
    return host.listbox_selected_index_00a9c920(listbox) + top;
}

void check_iterator_container(void** iterator, void* tree, Cc13SweepB8Host& host) {
    if (iterator[0] == nullptr || iterator[0] != tree) host.crt_invalid_parameter_noinfo_00bf6713();
}

void check_iterator_dereference(void** iterator, Cc13SweepB8Host& host) {
    if (iterator[0] == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();
    if (iterator[1] == at<void*>(iterator[0], 4)) host.crt_invalid_parameter_noinfo_00bf6713();
}

}  // namespace

// ---------------------------------------------------------------------------
// 006003A0 BSP_HudPauseListboxListener_Current08
// __thiscall(ECX = listener (pause screen + 8h); two stack words never read),
// RET 8; EH frame 00C76D75 (the strings, the list and the hint record, not
// modelled). Vtable 00CF3EE0 slot +8h, no static caller. No draw from the
// shared RNG 00BD2F10.
// Evidence 006003A0..006011F1 (x87 from the bytes: DE E2 = FSUBRP ST2 (ST2 =
// ST0 - ST2), DE F9 = FDIVP ST1 (ST1 / ST0), D9 CA / DF EA / DD D9 the
// 15.0 compare, DF F1 FCOMIP then JC):
// - Byte +8h zero: returns. key = listbox+164h + 00A9C920(listbox) (listbox =
//   +24h re-read each time; the +164h read precedes the call); end = +118h.
// - 005FD220 find in the map +114h; a found key: 00683790(+94h, 0), the key
//   recomputed and re-found twice (dereference checks), text = copy of the
//   node string +10h; when the node's data +14h holds "AAAAAA" (strstr, the
//   offset != -1 test kept) text = text.substr(6, length - 6) (00469840 then
//   00425F40, the temporary released); 00ABAED0(+40h, text, 1), +38h vtable
//   +34h(0), text released; then the row pass below.
// - Not found: 00683790(+94h, 0); +11Ch zero returns. best = the largest map
//   key below the selection key (first wins on ties; checked-iterator tests
//   kept). tracker 004E1CA0, list = copy of tracker+94h[005FF5D0(map,
//   &best)] (006002B0), hint record constructed (005FD650); the (key - best)th
//   list entry (1-based) names the hint: 005FDD90(hint, 005FFB20(tracker+30h,
//   entry+8h)). command = "", description = "Nincs description a hintben.".
//   Local party +28h == 0 and hint+30h non-empty: title hint+20h, command
//   hint+28h, description hint+30h; == 1 and hint+48h non-empty: description
//   hint+48h, title hint+38h, command hint+40h, description hint+48h again;
//   else hint+10h non-empty: title hint+0h, command hint+18h, description
//   hint+10h. In those three arms, unless byte 00F88A30: key = description +
//   "_pc"; the localisation (00A9FAD0 on [00F8BC4C]) of key, narrowed
//   (00436630; null text -> 00E19688), differing case-insensitively
//   (00449AF0) from key replaces the description with key; wide and key
//   released. Then 005FD360(screen, copy of command), +38h: 00ABBE50("", 1),
//   00ABAED0(description, 1), vtable +34h(1); +38h vtable +58h({+108h,
//   (float)(+10Ch - (h(+40h) + 0.01))}); +38h position {+FCh, h(+40h) + 0.01
//   + +100h, +104h}; +F0h position {x, y(+38h) - 0.001, z}; 006834A0(+94h,
//   h(+38h) + h(+40h) + 0.03 - size(+20h).y); description, command, hint
//   (004C8F00), list (004D05E0 then 00BF65AC of its head) released.
// - Rows i < listbox+104h whose key +164h + i is not in the map: position
//   {x(+3Ch) + 0.017999999225139618, y, z} (row from 00A9BE00).
// - selF = (float)(00A9C920 + (byte listbox+148h ? +164h : 0)); s = (float)
//   00A9C920. s == 15.0: +78h position {+80h, (selF - 15) * (+7Ch - +90h) /
//   (listbox+154h - 16.0) + +84h, +88h}. Else s == 0.0, +F8h != 15.0
//   (unordered passes) and listbox+154h != 10h: the same with (+7Ch - +90h)
//   * selF. Then when size(+78h).y + y(+78h) >= +7Ch + +84h (ordered):
//   +78h position {+80h, +84h + +7Ch - size.y, +88h}. +F8h = s.
void cc13_sweepB8_pause_listbox_current08_006003a0(void* listener, std::uint32_t, std::uint32_t,
                                                   Cc13SweepB8Host& host) {
    if (at<std::uint8_t>(listener, 8) == 0) return;
    void* map = &at<std::uint8_t>(listener, 0x114);
    void* scroller = &at<std::uint8_t>(listener, 0x94);

    std::int32_t key = listbox_key(listener, host);
    void* end = at<void*>(listener, 0x118);
    void* buffer[2];
    void** it = host.int_string_map_find_005fd220(map, buffer, &key);
    check_iterator_container(it, map, host);
    if (it[1] != end) {
        host.gui_scroller_attach_content_00683790(scroller, 0);
        key = listbox_key(listener, host);
        it = host.int_string_map_find_005fd220(map, buffer, &key);
        check_iterator_dereference(it, host);
        Cc13SweepBNativeString text = {0, nullptr};
        host.native_string_copy_construct_00426060(&text, static_cast<std::uint8_t*>(it[1]) + 0x10);
        key = listbox_key(listener, host);
        void* second[2];
        it = host.int_string_map_find_005fd220(map, second, &key);
        check_iterator_dereference(it, host);
        void* node = it[1];
        const char* data = at<const char*>(node, 0x14);
        if (data != nullptr) {
            const char* marker = host.crt_strstr_00bf9440(data, "AAAAAA");  // 00CF3F58
            if (marker != nullptr && marker - at<const char*>(node, 0x14) != -1) {
                Cc13SweepBNativeString part = {0, nullptr};
                void* sub = host.native_string_substring_00469840(&text, &part, 6,
                                                                  static_cast<std::int32_t>(text.length) - 6);
                host.native_string_assign_header_00425f40(&text, sub);
                release_string(part, host);
            }
        }
        host.gui_text_set_localised_source_00abaed0(at<void*>(listener, 0x40), &text, 1);
        host.widget_vcall_34_set_visible(at<void*>(listener, 0x38), 0);
        release_string(text, host);
    } else {
        host.gui_scroller_attach_content_00683790(scroller, 0);
        if (at<std::int32_t>(listener, 0x11C) == 0) return;
        key = listbox_key(listener, host);

        std::int32_t best = 0;
        void* walk[2] = {map, at<void*>(at<void*>(map, 4), 0)};
        for (;;) {
            void* head = at<void*>(map, 4);
            check_iterator_container(walk, map, host);
            if (walk[1] == head) break;
            check_iterator_dereference(walk, host);
            if (at<std::int32_t>(walk[1], 0xC) < key) {
                if (walk[1] == at<void*>(walk[0], 4)) host.crt_invalid_parameter_noinfo_00bf6713();
                if (key - best > key - at<std::int32_t>(walk[1], 0xC)) {
                    if (walk[1] == at<void*>(walk[0], 4)) host.crt_invalid_parameter_noinfo_00bf6713();
                    best = at<std::int32_t>(walk[1], 0xC);
                }
            }
            host.tree_iterator_increment_005fce00(walk);
        }

        void* tracker = host.award_tracker_get_004e1ca0();
        void* name = host.int_string_map_lookup_005ff5d0(map, &best);
        void* names = host.string_map_ci_subscript_006002b0(&at<std::uint8_t>(tracker, 0x94), name);
        void* list[2] = {nullptr, nullptr};  // {+0h, head}
        host.pooled_string_list_copy_construct_004d48a0(list, names);
        alignas(8) std::uint8_t hint[0x120];
        host.hint_definition_construct_005fd650(hint);
        {
            std::int32_t n = 1;
            void* container = list;
            void* entry = *static_cast<void**>(list[1]);
            bool found = false;
            for (;;) {
                void* head = list[1];
                if (container == nullptr || container != static_cast<void*>(list)) {
                    host.crt_invalid_parameter_noinfo_00bf6713();
                }
                if (entry == head) break;
                if (key - best == n) {
                    found = true;
                    break;
                }
                ++n;
                if (container == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();
                if (entry == at<void*>(container, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
                entry = *static_cast<void**>(entry);
            }
            if (found) {
                if (container == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();
                if (entry == at<void*>(container, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
                void* tracker2 = host.award_tracker_get_004e1ca0();
                void* stored = host.hint_table_lookup_005ffb20(&at<std::uint8_t>(tracker2, 0x30),
                                                               static_cast<std::uint8_t*>(entry) + 8);
                host.hint_definition_assign_005fdd90(hint, stored);
            }
        }

        Cc13SweepBNativeString command;
        construct_literal(command, "", 0, host);  // 00CE3A0C
        Cc13SweepBNativeString description;
        construct_literal(description, "Nincs description a hintben.", 0x1C, host);  // 00CF3F38
        void* game = host.global_00e188a8();
        void* player = at<void*>(game, 0x18CC + 4 * static_cast<std::size_t>(at<std::int32_t>(game, 0x18EC)));
        void* title_widget = at<void*>(listener, 0x40);
        auto field = [&](std::size_t offset) { return static_cast<void*>(hint + offset); };
        auto length_at = [&](std::size_t offset) {
            std::uint32_t length;
            std::memcpy(&length, hint + offset, sizeof length);
            return length;
        };
        bool filled = true;
        if (at<std::int32_t>(player, 0x28) == 0 && length_at(0x30) != 0) {
            host.gui_text_set_localised_source_00abaed0(title_widget, field(0x20), 1);
            copy_string(command, field(0x28), host);
            copy_string(description, field(0x30), host);
        } else if (at<std::int32_t>(player, 0x28) == 1 && length_at(0x48) != 0) {
            copy_string(description, field(0x48), host);
            host.gui_text_set_localised_source_00abaed0(title_widget, field(0x38), 1);
            copy_string(command, field(0x40), host);
            copy_string(description, field(0x48), host);
        } else if (length_at(0x10) != 0) {
            host.gui_text_set_localised_source_00abaed0(title_widget, field(0x0), 1);
            copy_string(command, field(0x18), host);
            copy_string(description, field(0x10), host);
        } else {
            filled = false;
        }
        if (filled && *static_cast<std::uint8_t*>(host.image_global(0x00F88A30)) == 0) {
            Cc13SweepBNativeString suffix = {0, nullptr};
            void* pc = host.native_string_assign_0041e870(&suffix, "_pc");  // 00CEBD30
            Cc13SweepBNativeString localised_key = {0, nullptr};
            host.native_string_concat_004261a0(&description, &localised_key, pc);
            release_string(suffix, host);
            struct {
                std::uint32_t length;
                std::uint16_t* data;
            } wide = {0, nullptr};
            host.localization_resolve_key_list_00a9fad0(*static_cast<void**>(host.image_global(0x00F8BC4C)), &wide,
                                                        &localised_key);
            const std::uint16_t* wide_text =
                wide.data != nullptr ? wide.data : static_cast<const std::uint16_t*>(host.image_global(0x00E19688));
            Cc13SweepBNativeString narrow = {0, nullptr};
            void* narrowed = host.native_string_from_wide_00436630(&narrow, wide_text);
            const bool differs = host.native_string_not_equal_ci_00449af0(narrowed, &localised_key);
            release_string(narrow, host);
            if (differs) copy_string(description, &localised_key, host);
            if (wide.data != nullptr) {
                host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), wide.data, wide.length * 2 + 2, 1);
            }
            release_string(localised_key, host);
        }

        const std::uint32_t command_length = command.length;
        Cc13SweepBNativeString argument = {0, nullptr};
        host.native_string_resize_0041dd40(&argument, command_length, 1);
        if (command_length != 0) host.crt_memcpy_00bf7680(argument.data, command.data, argument.length);
        host.pause_screen_run_lua_command_005fd360(static_cast<std::uint8_t*>(listener) - 8, argument.length,
                                                   argument.data);
        host.gui_text_set_localised_cstring_00abbe50(at<void*>(listener, 0x38), "", 1);  // 00CE3A0C
        host.gui_text_set_localised_source_00abaed0(at<void*>(listener, 0x38), &description, 1);
        host.widget_vcall_34_set_visible(at<void*>(listener, 0x38), 1);

        const double k001 = f64_bits(0x3F847AE147AE147Bull);  // 00CE6638
        const double top = at<float>(listener, 0x10C);
        const float left = at<float>(listener, 0x108);
        double height = host.text_normalized_height_00ab6bd0(at<void*>(listener, 0x40));
        const float xy[2] = {left, static_cast<float>(top - (height + k001))};
        host.widget_vcall_58_set_position(at<void*>(listener, 0x38), xy);

        const float base_y = at<float>(listener, 0x100);
        const float body_z = at<float>(listener, 0x104);
        const float body_x = at<float>(listener, 0xFC);
        height = host.text_normalized_height_00ab6bd0(at<void*>(listener, 0x40));
        const float body_position[3] = {body_x, static_cast<float>(height + k001 + base_y), body_z};
        host.widget_set_resolved_position_00aa8240(at<void*>(listener, 0x38), body_position);

        float bar[3];
        host.widget_get_resolved_position_00aa6750(at<void*>(listener, 0xF0), bar);
        const float bar_z = bar[2];
        float text_position[3];
        const float text_y = host.widget_get_resolved_position_00aa6750(at<void*>(listener, 0x38), text_position)[1];
        const float bar_position[3] = {bar[0], static_cast<float>(text_y - f64_bits(0x3F50624DD2F1A9FCull)), bar_z};
        host.widget_set_resolved_position_00aa8240(at<void*>(listener, 0xF0), bar_position);

        const float frame_height = host.widget_get_size_00aa6740(at<void*>(listener, 0x20))[1];
        const double title_height = host.text_normalized_height_00ab6bd0(at<void*>(listener, 0x40));
        const double body_height = host.text_normalized_height_00ab6bd0(at<void*>(listener, 0x38));
        const float range =
            static_cast<float>(body_height + title_height + f64_bits(0x3F9EB851EB851EB8ull) - frame_height);
        host.gui_scroller_set_scroll_range_006834a0(scroller, range);

        release_string(description, host);
        if (command.data != nullptr) {
            host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), command.data, command_length + 1, 1);
        }
        host.hint_definition_destruct_004c8f00(hint);
        host.pooled_string_list_clear_004d05e0(list);
        host.crt_operator_delete_00bf65ac(list[1]);
    }

    for (std::int32_t i = 0; i < at<std::int32_t>(at<void*>(listener, 0x24), 0x104); ++i) {
        std::int32_t row_key = at<std::int32_t>(at<void*>(listener, 0x24), 0x164) + i;
        void* row_end = at<void*>(listener, 0x118);
        void* row_it[2];
        void** found = host.int_string_map_find_005fd220(map, row_it, &row_key);
        check_iterator_container(found, map, host);
        if (found[1] != row_end) continue;
        void* row = host.listbox_row_00a9be00(at<void*>(listener, 0x24), i);
        float row_position[3];
        host.widget_get_resolved_position_00aa6750(row, row_position);
        const float row_z = row_position[2];
        const float row_y = row_position[1];
        float column[3];
        const float column_x = host.widget_get_resolved_position_00aa6750(at<void*>(listener, 0x3C), column)[0];
        const float moved[3] = {static_cast<float>(column_x + f64_bits(0x3F926E9780000000ull)), row_y, row_z};
        host.widget_set_resolved_position_00aa8240(row, moved);
    }

    void* listbox = at<void*>(listener, 0x24);
    const std::int32_t top_row = at<std::uint8_t>(listbox, 0x148) != 0 ? at<std::int32_t>(listbox, 0x164) : 0;
    const float selected = static_cast<float>(host.listbox_selected_index_00a9c920(listbox) + top_row);
    const float index = static_cast<float>(host.listbox_selected_index_00a9c920(at<void*>(listener, 0x24)));
    const double k15 = f64_bits(0x402E000000000000ull);  // 00CF3F20
    const double k16 = f64_bits(0x4030000000000000ull);  // 00CED9F8
    if (static_cast<double>(index) == k15) {
        const double span = static_cast<double>(at<float>(listener, 0x7C)) - at<float>(listener, 0x90);
        const float x = at<float>(listener, 0x80);
        const double rows = static_cast<double>(at<std::int32_t>(at<void*>(listener, 0x24), 0x154)) - k16;
        const float z = at<float>(listener, 0x88);
        const float y =
            static_cast<float>((static_cast<double>(selected) - k15) * span / rows + at<float>(listener, 0x84));
        const float position[3] = {x, y, z};
        host.widget_set_resolved_position_00aa8240(at<void*>(listener, 0x78), position);
    } else if (index == 0.0f && !(static_cast<double>(at<float>(listener, 0xF8)) == k15) &&
               at<std::int32_t>(at<void*>(listener, 0x24), 0x154) != 0x10) {
        const double span = static_cast<double>(at<float>(listener, 0x7C)) - at<float>(listener, 0x90);
        const float x = at<float>(listener, 0x80);
        const double rows = static_cast<double>(at<std::int32_t>(at<void*>(listener, 0x24), 0x154)) - k16;
        const float z = at<float>(listener, 0x88);
        const float y = static_cast<float>(span * selected / rows + at<float>(listener, 0x84));
        const float position[3] = {x, y, z};
        host.widget_set_resolved_position_00aa8240(at<void*>(listener, 0x78), position);
    }

    float thumb[3];
    const float thumb_y = host.widget_get_resolved_position_00aa6750(at<void*>(listener, 0x78), thumb)[1];
    const float* thumb_size = host.widget_get_size_00aa6740(at<void*>(listener, 0x78));
    const float lower = at<float>(listener, 0x84);
    if (static_cast<double>(thumb_size[1]) + thumb_y >= static_cast<double>(at<float>(listener, 0x7C)) + lower) {
        const float z = at<float>(listener, 0x88);
        const float* size_now = host.widget_get_size_00aa6740(at<void*>(listener, 0x78));
        const float y = static_cast<float>(static_cast<double>(lower) + at<float>(listener, 0x7C) - size_now[1]);
        const float position[3] = {at<float>(listener, 0x80), y, z};
        host.widget_set_resolved_position_00aa8240(at<void*>(listener, 0x78), position);
    }
    at<float>(listener, 0xF8) = index;
}

namespace {

// 005EADC0's two idioms: a literal NativeString temporary (zero header,
// 0041DD40 to the literal length, memcpy length + 1) handed to 00AA5840 /
// 00AA7E00 and released right after the call.
void* load_page(const char* name, std::uint32_t length, Cc13SweepB8Host& host) {
    Cc13SweepBNativeString text;
    construct_literal(text, name, length, host);
    void* page = host.gui_manager_load_page_00aa5840(host.gui_manager_get_004c12b0(), &text, 1, 0);
    release_string(text, host);
    return page;
}

void* find_child(void* parent, const char* name, std::uint32_t length, Cc13SweepB8Host& host) {
    Cc13SweepBNativeString text;
    construct_literal(text, name, length, host);
    void* child = host.widget_find_child_00aa7e00(parent, &text, 1);
    release_string(text, host);
    return child;
}

}  // namespace

// ---------------------------------------------------------------------------
// 005EADC0 BSP_MultiModeSelectorScreen_BindLayout
// __fastcall(ECX = screen), RET; EH frame 00C75630 (the temporaries). Vtable
// 00CF2B44 slot +14h, no static caller. No draw from the shared RNG 00BD2F10.
// Evidence 005EADC0..005EBC6B: every name below is an inlined literal
// temporary (lengths from the PUSHes); 00AA7E00 always gets 1, 00AA5840 (1,
// 0); 00AA6BC0 gets (screen+0Ch, 0); 00A9D750 (row, 0, 0).
// - Note the third and fourth list-box inserts pass min_rank_Group (+B8h) and
//   max_rank_Group (+BCh), not the min/max rank texts (+C8h/+CCh).
// - The scroll bar track length is size(textx_scroll_bg_Icon).y -
//   size(+9Ch).y (float stores), with the arrows +30h/+34h.
// - Tail: +78h = 1 and the listener on +C0h..+CCh; +DCh = size(+CCh).x;
//   00A9CEB0(+A8h, (float)(size(+C0h).x + 0.004166666666666667), 1); list
//   box +12Ch/+130h/+134h = 0.0072916667f, 0.0027777778f, 0; rows +C0h/+C4h
//   offset (-0.0020833334f, 0), +C8h/+CCh (-0.038541667f, 0) at +30h/+34h.
void cc13_sweepB8_multi_mode_selector_bind_layout_005eadc0(void* screen, Cc13SweepB8Host& host) {
    void* listener = &at<std::uint8_t>(screen, 0xC);
    at<void*>(screen, 0x14) = load_page("FE_multimode_listbox", 0x14, host);
    at<void*>(screen, 0x20) = find_child(at<void*>(screen, 0x14), "Main_Listbox", 0xC, host);
    at<void*>(screen, 0x18) = load_page("FE_multimode", 0xC, host);
    at<void*>(screen, 0x24) = find_child(at<void*>(screen, 0x18), "missionpicture_Icon", 0x13, host);
    at<void*>(screen, 0x98) = find_child(at<void*>(screen, 0x18), "test_Clipbox", 0xC, host);
    at<void*>(screen, 0x38) = find_child(at<void*>(screen, 0x98), "textbox_Group", 0xD, host);
    at<void*>(screen, 0x2C) = find_child(at<void*>(screen, 0x38), "mode_Text", 9, host);
    at<void*>(screen, 0x28) = find_child(at<void*>(screen, 0x38), "description_Text", 0x10, host);
    at<void*>(screen, 0x30) = find_child(at<void*>(screen, 0x18), "arrow_top_scroll_Icon", 0x15, host);
    host.gui_widget_set_listener_00aa6bc0(at<void*>(screen, 0x30), listener, 0);
    at<void*>(screen, 0x34) = find_child(at<void*>(screen, 0x18), "arrow_bottom_scroll_Icon", 0x18, host);
    host.gui_widget_set_listener_00aa6bc0(at<void*>(screen, 0x34), listener, 0);
    at<void*>(screen, 0x9C) = find_child(at<void*>(screen, 0x18), "textx_scroll_FrameBox", 0x15, host);
    host.gui_widget_set_listener_00aa6bc0(at<void*>(screen, 0x9C), listener, 0);

    void* scroller = &at<std::uint8_t>(screen, 0x3C);
    host.gui_scroller_attach_content_00683790(scroller, static_cast<std::int32_t>(reinterpret_cast<std::intptr_t>(
                                                            at<void*>(screen, 0x38))));
    {
        Cc13SweepBNativeString text;
        construct_literal(text, "textx_scroll_bg_Icon", 0x14, host);
        void* background = host.widget_find_child_00aa7e00(at<void*>(screen, 0x18), &text, 1);
        const float track = host.widget_get_size_00aa6740(background)[1];
        const float frame = host.widget_get_size_00aa6740(at<void*>(screen, 0x9C))[1];
        const float length = track - frame;
        host.gui_scroller_attach_scroll_bar_00683380(scroller, at<void*>(screen, 0x9C), length, at<void*>(screen, 0x30),
                                                     at<void*>(screen, 0x34));
        release_string(text, host);
    }

    at<void*>(screen, 0x1C) = load_page("FE_createserver", 0xF, host);
    at<void*>(screen, 0xA0) = find_child(at<void*>(screen, 0x1C), "cs_popup_Group", 0xE, host);
    at<void*>(screen, 0xD0) = find_child(at<void*>(screen, 0xA0), "private_slot_Text", 0x11, host);
    at<void*>(screen, 0xA4) = find_child(at<void*>(screen, 0xA0), "ingamemap_Group", 0xF, host);
    at<void*>(screen, 0xB0) = find_child(at<void*>(screen, 0xA4), "scroll_left_Icon", 0x10, host);
    host.gui_widget_set_listener_00aa6bc0(at<void*>(screen, 0xB0), listener, 0);
    at<void*>(screen, 0xB4) = find_child(at<void*>(screen, 0xA4), "scroll_right_Icon", 0x11, host);
    host.gui_widget_set_listener_00aa6bc0(at<void*>(screen, 0xB4), listener, 0);
    at<void*>(screen, 0xA8) = find_child(at<void*>(screen, 0xA4), "popup_Listbox", 0xD, host);
    at<std::uint8_t>(at<void*>(screen, 0xA8), 0x78) = 0;
    at<void*>(screen, 0xAC) = find_child(at<void*>(screen, 0xA0), "Connection_Group", 0x10, host);
    at<void*>(screen, 0xB8) = find_child(at<void*>(screen, 0xA8), "min_rank_Group", 0xE, host);
    at<void*>(screen, 0xBC) = find_child(at<void*>(screen, 0xA8), "max_rank_Group", 0xE, host);
    at<void*>(screen, 0xC0) = find_child(at<void*>(screen, 0xA8), "players_t_Text", 0xE, host);
    host.gui_listbox_insert_row_00a9d750(at<void*>(screen, 0xA8), at<void*>(screen, 0xC0), nullptr, 0);
    at<void*>(screen, 0xC4) = find_child(at<void*>(screen, 0xA8), "private_t_Text", 0xE, host);
    host.gui_listbox_insert_row_00a9d750(at<void*>(screen, 0xA8), at<void*>(screen, 0xC4), nullptr, 0);
    at<void*>(screen, 0xC8) = find_child(at<void*>(screen, 0xB8), "min_rank_t_Text", 0xF, host);
    host.gui_listbox_insert_row_00a9d750(at<void*>(screen, 0xA8), at<void*>(screen, 0xB8), nullptr, 0);
    at<void*>(screen, 0xD4) = find_child(at<void*>(screen, 0xB8), "rank_Icon", 9, host);
    at<void*>(screen, 0xCC) = find_child(at<void*>(screen, 0xBC), "max_rank_t_Text", 0xF, host);
    host.gui_listbox_insert_row_00a9d750(at<void*>(screen, 0xA8), at<void*>(screen, 0xBC), nullptr, 0);
    at<void*>(screen, 0xD8) = find_child(at<void*>(screen, 0xBC), "rank_Icon", 9, host);

    for (std::size_t slot = 0xC0; slot <= 0xCC; slot += 4) {
        at<std::uint8_t>(at<void*>(screen, slot), 0x78) = 1;
        host.gui_widget_set_listener_00aa6bc0(at<void*>(screen, slot), &at<std::uint8_t>(screen, 0xC), 0);
    }
    at<float>(screen, 0xDC) = host.widget_get_size_00aa6740(at<void*>(screen, 0xCC))[0];
    const float column = static_cast<float>(host.widget_get_size_00aa6740(at<void*>(screen, 0xC0))[0] +
                                            f64_bits(0x3F71111111111111ull));  // 00CF2B78
    host.gui_listbox_set_field_124_00a9ceb0(at<void*>(screen, 0xA8), column, 1);
    void* listbox = at<void*>(screen, 0xA8);
    at<float>(listbox, 0x12C) = f32_bits(0x3BEEEEEFu);  // 00CF2B70
    at<float>(listbox, 0x130) = f32_bits(0x3B360B61u);  // 00CEE600
    at<float>(listbox, 0x134) = 0.0f;
    const float near_offset = f32_bits(0xBB088889u);  // 00CF254C
    const float far_offset = f32_bits(0xBD1DDDDEu);   // 00CF2B6C
    at<float>(at<void*>(screen, 0xC0), 0x30) = near_offset;
    at<float>(at<void*>(screen, 0xC0), 0x34) = 0.0f;
    at<float>(at<void*>(screen, 0xC4), 0x30) = near_offset;
    at<float>(at<void*>(screen, 0xC4), 0x34) = 0.0f;
    at<float>(at<void*>(screen, 0xC8), 0x30) = far_offset;
    at<float>(at<void*>(screen, 0xC8), 0x34) = 0.0f;
    at<float>(at<void*>(screen, 0xCC), 0x30) = far_offset;
    at<float>(at<void*>(screen, 0xCC), 0x34) = 0.0f;
}

namespace {

void* checked_pointer_at(void* screen, std::size_t vector, std::uint32_t index, Cc13SweepB8Host& host) {
    void** first = at<void**>(screen, vector);
    if (first == nullptr || index >= static_cast<std::uint32_t>(at<void**>(screen, vector + 4) - first)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    return at<void**>(screen, vector)[index];
}

// [[00E198AC]+5Ch]+24h: the scene table {+4h first, +8h last} of 434h-byte
// rows; the image's bound check is a signed /434h of the byte span and an
// unsigned compare.
std::uint8_t* scene_table_row(Cc13SweepB8Host& host) {
    void* table = static_cast<std::uint8_t*>(at<void*>(*static_cast<void**>(host.image_global(0x00E198AC)), 0x5C)) + 0x24;
    const std::uint32_t index =
        host.lobby_player_index_005d7070(*static_cast<std::uint32_t*>(host.image_global(0x00E19594)));
    std::uint8_t* first = at<std::uint8_t*>(table, 4);
    if (first == nullptr ||
        index >= static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint8_t*>(table, 8) - first) / 0x434)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    return at<std::uint8_t*>(table, 4) + 0x434 * index;
}

}  // namespace

// ---------------------------------------------------------------------------
// 005E3580 BSP_MultiGameLobbyScreen_RefreshSettingRows
// __fastcall(ECX = lobby screen), RET; EH frame 00C74FFB (temporaries). No
// draw from the shared RNG 00BD2F10.
// Evidence 005E3580..005E4478 (x87 from the bytes 005E403D..005E40F6):
// - Bit 0 of +10h ("host"): 00A9AC40(+94h, 0), 00A9BEC0(+94h). Always the
//   same on +228h, then 005D8A70. Position p = +218h's resolved position;
//   total = 0, y2 = 0; row = (float)(max(h(+21Ch), h(+218h)) * 1.05 (double
//   00CF2510)), the first height kept unless it compares above.
// - game+5FCh: r = 005D5930(screen, [game+5FCh]+1098h); r != 0: 005D8B80's
//   dword zero -> 005D8FC0; r == 0 or game+5FCh null: the string game+600h
//   emptied (0041DD40(0), memcpy of its length from "") and +2F0h..+2FCh =
//   -1 (game re-read when r == 0).
// - Slots i = 0..13 (13 needs r >= 2, signed): host and i == 1 and +20Ch:
//   00ABAED0(+20Ch, 005E2320(i), 1). 005D5500(i) < 0 skips. Host and i > 1:
//   a clone of +214h attached under +94h, its setting_Text gets the C string
//   [00E088C0 + 4i] (00ABB000 width -1.0f, 1), setting_var_Text 005E2320(i);
//   shown, appended to +94h; total += h(var) + h(text) (double sum, float
//   store). Then a clone of +218h, shown, text [00E088C0 + 4i] + "|.:"
//   (00ABB000 -1.0f, 1), pushed to +230h, placed at p, p.y += row; a clone of
//   +21Ch at {-0.15208334f, y2, 0}, y2 += row, shown, 005E2320(i) text;
//   +1BCh[+228h's +104h] = i; appended to +228h.
// - 00A9CEB0(+228h, 0.196875f, 1); host: 00A9C7C0(+94h, +274h), +94h vtable
//   +34h(1); 00A9C7C0(+228h, +274h), +228h vtable +34h(1); 00A9AC40(+228h,
//   screen+8h); listener screen+8h vtable +8h(00A9BE00(+228h, +274h), +228h).
//   Host: 00A9CDC0(+94h) unless byte +94h's +85h; 006834A0(+30h, (float)(total
//   - size(+A0h).y)).
// - For each index k of the +1F8h widget vector: sel = 005D7070([00E19594]);
//   host: +1F8h[k] vtable +34h(sel == k) (the image pushes EAX with only AL
//   set); +244h[k] vtable +34h(sel == k). For k == sel: the bounds pair
//   (x0, z0, x1, z1) and texture kind come from [00E19564] (<= 3: switch on
//   [00E19568] 0..3 -> scene record (005D86B0) +364h/+370h kind 6, +37Ch/+388h
//   5, +394h/+3A0h 4, +3ACh/+3B8h 3; [00E19568] > 3 keeps the previous values
//   with kind 0 (the image reads stale stack slots there; this path needs
//   00E19568 above 3, which docs/MISSION_LOBBY_SETTINGS.md says is only
//   written below 4; modelled as the previous iteration's values, zero
//   first); 4 -> scene table row +3F4h/+400h kind 1; 5 -> +40Ch/+418h kind
//   2; 6 -> record +3C4h/+3D0h kind 7; else +3DCh/+3E8h kind 0). u0 = (x0 +
//   15000) / 30000, v0 = 1 - (z0 + 15000) / 30000, u1, v1 likewise (each
//   quotient stored as float), ratio = |x0 - x1| / |z0 - z1| (float stores).
//   Host: +1F8h[k] vtable +88h(kind, 0, 1.0f), +118h..+124h = (u0, v0, u1,
//   v1), vtable +8Ch(), vtable +58h({ratio * 150 / 960, 0.20833333f}) when
//   1.0 > ratio, ({0.15625f, 150 / ratio / 720}) when ratio > 1.0, else
//   ({0.15625f, 0.20833333f}) (COMISS + JBE: unordered takes the last).
//   The same on +244h[k] without the host test.
// - Host: 005D98E0. dword +10h = 0; +94h vtable +70h().
void cc13_sweepB8_lobby_refresh_setting_rows_005e3580(void* screen, Cc13SweepB8Host& host) {
    const bool is_host = (at<std::uint8_t>(screen, 0x10) & 1) != 0;
    if (is_host) {
        host.gui_listbox_set_listener_00a9ac40(at<void*>(screen, 0x94), nullptr);
        host.gui_listbox_clear_rows_00a9bec0(at<void*>(screen, 0x94));
    }
    host.gui_listbox_set_listener_00a9ac40(at<void*>(screen, 0x228), nullptr);
    host.gui_listbox_clear_rows_00a9bec0(at<void*>(screen, 0x228));
    host.lobby_delete_row_objects_005d8a70(screen);

    float total = 0.0f;
    float place[3];
    host.widget_get_resolved_position_00aa6750(at<void*>(screen, 0x218), place);
    float label_y = 0.0f;
    const float text_height = host.text_normalized_height_00ab6bd0(at<void*>(screen, 0x21C));
    const float row_height_name = host.text_normalized_height_00ab6bd0(at<void*>(screen, 0x218));
    const float tallest = text_height > row_height_name ? text_height : row_height_name;
    const float row_height = static_cast<float>(tallest * f64_bits(0x3FF0CCCCC0000000ull));  // 00CF2510

    std::int32_t found_row;
    void* game = host.global_00e188a8();
    bool reset = false;
    if (at<void*>(game, 0x5FC) != nullptr) {
        found_row = host.lobby_find_setting_row_005d5930(screen, at<std::int32_t>(at<void*>(game, 0x5FC), 0x1098));
        if (found_row == 0) {
            game = host.global_00e188a8();
            reset = true;
        } else if (*host.lobby_005d8b80(screen) == 0) {
            host.lobby_005d8fc0(screen);
        }
    } else {
        found_row = 0;
        reset = true;
    }
    if (reset) {
        auto* text = static_cast<Cc13SweepBNativeString*>(static_cast<void*>(&at<std::uint8_t>(game, 0x600)));
        host.native_string_resize_0041dd40(text, 0, 1);
        if (text->data != nullptr) host.crt_memcpy_00bf7680(text->data, "", text->length);  // 00CE3A0C
        at<std::int32_t>(screen, 0x2FC) = -1;
        at<std::int32_t>(screen, 0x2F8) = -1;
        at<std::int32_t>(screen, 0x2F4) = -1;
        at<std::int32_t>(screen, 0x2F0) = -1;
    }

    const float minus_one = f32_bits(0xBF800000u);  // 00D7A260
    for (std::int32_t i = 0; i < 14; ++i) {
        if (i == 0xD && found_row < 2) continue;
        if ((at<std::uint8_t>(screen, 0x10) & 1) != 0 && i == 1 && at<void*>(screen, 0x20C) != nullptr) {
            Cc13SweepBNativeString label = {0, nullptr};
            host.lobby_format_option_label_005e2320(&label, i);
            host.gui_text_set_localised_source_00abaed0(at<void*>(screen, 0x20C), &label, 1);
            release_string(label, host);
        }
        if (host.lobby_menu_row_for_slot_005d5500(i) < 0) continue;
        if ((at<std::uint8_t>(screen, 0x10) & 1) != 0 && i > 1) {
            void* row = host.widget_clone_subtree_00aab4c0(at<void*>(screen, 0x214), nullptr);
            host.gui_widget_attach_child_00aaa5a0(at<void*>(screen, 0x94), row, 0);
            Cc13SweepBNativeString key = {0, nullptr};
            host.native_string_assign_0041e870(&key, "setting_Text");  // 00CEEE20
            void* setting = host.widget_find_child_00aa7e00(row, &key, 1);
            release_string(key, host);
            host.native_string_assign_0041e870(&key, "setting_var_Text");  // 00CEEE0C
            void* value = host.widget_find_child_00aa7e00(row, &key, 1);
            release_string(key, host);
            Cc13SweepBNativeString text = {0, nullptr};
            host.native_string_assign_0041e870(&text, *static_cast<const char**>(
                                                          host.image_global(0x00E088C0 + 4 * static_cast<std::uint32_t>(i))));
            host.gui_text_set_ellipsized_source_00abb000(setting, &text, minus_one, 1);
            release_string(text, host);
            Cc13SweepBNativeString label = {0, nullptr};
            host.gui_text_set_localised_source_00abaed0(value, host.lobby_format_option_label_005e2320(&label, i), 1);
            release_string(label, host);
            host.widget_vcall_34_set_visible(row, 1);
            host.gui_listbox_insert_row_00a9d750(at<void*>(screen, 0x94), row, nullptr, 0);
            const double value_height = host.text_normalized_height_00ab6bd0(value);
            const double setting_height = host.text_normalized_height_00ab6bd0(setting);
            total = static_cast<float>(setting_height + value_height + total);
        }

        void* name_row = host.widget_clone_subtree_00aab4c0(at<void*>(screen, 0x218), nullptr);
        host.widget_vcall_34_set_visible(name_row, 1);
        Cc13SweepBNativeString suffix;
        construct_literal(suffix, "|.:", 3, host);  // 00CEFBDC
        const char* name =
            *static_cast<const char**>(host.image_global(0x00E088C0 + 4 * static_cast<std::uint32_t>(i)));
        Cc13SweepBNativeString base;
        construct_literal(base, name, static_cast<std::uint32_t>(std::strlen(name)), host);
        Cc13SweepBNativeString joined = {0, nullptr};
        void* full = host.native_string_concat_004261a0(&base, &joined, &suffix);
        host.gui_text_set_ellipsized_source_00abb000(name_row, full, minus_one, 1);
        release_string(joined, host);
        release_string(base, host);
        release_string(suffix, host);
        host.pointer_vector_push_back_005dad40(&at<std::uint8_t>(screen, 0x230), &name_row);
        host.widget_set_resolved_position_00aa8240(name_row, place);
        place[1] = place[1] + row_height;

        void* label_row = host.widget_clone_subtree_00aab4c0(at<void*>(screen, 0x21C), nullptr);
        const float label_position[3] = {f32_bits(0xBE1BBBBCu), label_y, 0.0f};  // 00CF250C
        host.widget_set_resolved_position_00aa8240(label_row, label_position);
        label_y = row_height + label_y;
        host.widget_vcall_34_set_visible(label_row, 1);
        Cc13SweepBNativeString label = {0, nullptr};
        host.gui_text_set_localised_source_00abaed0(label_row, host.lobby_format_option_label_005e2320(&label, i), 1);
        release_string(label, host);
        at<std::int32_t>(screen, 0x1BC + 4 * static_cast<std::size_t>(at<std::int32_t>(at<void*>(screen, 0x228), 0x104))) = i;
        host.gui_listbox_insert_row_00a9d750(at<void*>(screen, 0x228), label_row, nullptr, 0);
    }

    host.gui_listbox_set_field_124_00a9ceb0(at<void*>(screen, 0x228), f32_bits(0x3E49999Au), 1);  // 00CF2508
    if ((at<std::uint8_t>(screen, 0x10) & 1) != 0) {
        host.listbox_select_index_00a9c7c0(at<void*>(screen, 0x94), at<std::int32_t>(screen, 0x274));
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x94), 1);
    }
    host.listbox_select_index_00a9c7c0(at<void*>(screen, 0x228), at<std::int32_t>(screen, 0x274));
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x228), 1);
    void* listener = &at<std::uint8_t>(screen, 8);
    host.gui_listbox_set_listener_00a9ac40(at<void*>(screen, 0x228), listener);
    void* names = at<void*>(screen, 0x228);
    host.listener_vcall_8_current(listener, host.listbox_row_00a9be00(at<void*>(screen, 0x228),
                                                                      at<std::int32_t>(screen, 0x274)),
                                  names);
    if ((at<std::uint8_t>(screen, 0x10) & 1) != 0) {
        if (at<std::uint8_t>(at<void*>(screen, 0x94), 0x85) == 0) host.gui_listbox_clear_00a9cdc0(at<void*>(screen, 0x94));
        const double sum = total;
        const float range = static_cast<float>(sum - host.widget_get_size_00aa6740(at<void*>(screen, 0xA0))[1]);
        host.gui_scroller_set_scroll_range_006834a0(&at<std::uint8_t>(screen, 0x30), range);
    }

    float x0 = 0.0f;  // the image's stack slots ESP+28h/30h/18h/20h (see the comment)
    float z0 = 0.0f;
    float x1 = 0.0f;
    float z1 = 0.0f;
    const double origin = f64_bits(0xC0CD4C0000000000ull);  // 00CEF1D0 -15000.0
    const double extent = f64_bits(0x40DD4C0000000000ull);  // 00CEF1C8 30000.0
    for (std::uint32_t k = 0;; ++k) {
        void** first = at<void**>(screen, 0x1F8);
        if (first == nullptr || k >= static_cast<std::uint32_t>(at<void**>(screen, 0x1FC) - first)) break;
        const std::uint32_t selected =
            host.lobby_player_index_005d7070(*static_cast<std::uint32_t*>(host.image_global(0x00E19594)));
        std::int32_t kind = 0;
        if ((at<std::uint8_t>(screen, 0x10) & 1) != 0) {
            host.widget_vcall_34_set_visible(checked_pointer_at(screen, 0x1F8, k, host), selected == k ? 1 : 0);
        }
        host.widget_vcall_34_set_visible(checked_pointer_at(screen, 0x244, k, host), selected == k ? 1 : 0);
        if (selected != k) continue;

        const std::uint32_t mode = *static_cast<std::uint32_t*>(host.image_global(0x00E19564));
        const float* pair = nullptr;
        const float* second = nullptr;
        if (mode <= 3) {
            const std::uint32_t sub_mode = *static_cast<std::uint32_t*>(host.image_global(0x00E19568));
            if (sub_mode <= 3) {
                static const std::uint32_t offsets[4] = {0x364, 0x37C, 0x394, 0x3AC};
                static const std::int32_t kinds[4] = {6, 5, 4, 3};
                pair = reinterpret_cast<const float*>(host.lobby_scene_record_005d86b0(screen) + offsets[sub_mode]);
                x0 = pair[0];
                z0 = pair[2];
                second = reinterpret_cast<const float*>(host.lobby_scene_record_005d86b0(screen) + offsets[sub_mode] + 0xC);
                kind = kinds[sub_mode];
            }
        } else if (mode == 4 || mode == 5) {
            const std::size_t offset = mode == 4 ? 0x3F4 : 0x40C;
            pair = reinterpret_cast<const float*>(scene_table_row(host) + offset);
            x0 = pair[0];
            z0 = pair[2];
            second = reinterpret_cast<const float*>(scene_table_row(host) + offset + 0xC);
            kind = mode == 4 ? 1 : 2;
        } else {
            const std::size_t offset = mode == 6 ? 0x3C4 : 0x3DC;
            pair = reinterpret_cast<const float*>(host.lobby_scene_record_005d86b0(screen) + offset);
            x0 = pair[0];
            z0 = pair[2];
            second = reinterpret_cast<const float*>(host.lobby_scene_record_005d86b0(screen) + offset + 0xC);
            kind = mode == 6 ? 7 : 0;
        }
        if (second != nullptr) {
            x1 = second[0];
            z1 = second[2];
        }

        const float t_z0 = static_cast<float>((static_cast<double>(z0) - origin) / extent);
        const float v0 = static_cast<float>(1.0 - t_z0);
        const float t_z1 = static_cast<float>((static_cast<double>(z1) - origin) / extent);
        const float v1 = static_cast<float>(1.0 - t_z1);
        const float u0 = static_cast<float>((static_cast<double>(x0) - origin) / extent);
        const float u1 = static_cast<float>((static_cast<double>(x1) - origin) / extent);
        const float width = std::fabs(static_cast<float>(static_cast<double>(x0) - x1));
        const float depth = std::fabs(static_cast<float>(static_cast<double>(z0) - z1));
        const float ratio = static_cast<float>(static_cast<double>(width) / depth);

        const double k150 = f64_bits(0x4062C00000000000ull);  // 00CE3DD8
        auto apply = [&](void* widget, std::size_t vector) {
            host.widget_vcall_88(widget, kind, 0, 1.0f);
            void* again = checked_pointer_at(screen, vector, k, host);
            std::uint32_t v1_bits;
            std::memcpy(&v1_bits, &v1, sizeof v1_bits);
            at<float>(again, 0x118) = u0;
            at<float>(again, 0x11C) = v0;
            at<float>(again, 0x120) = u1;
            at<std::uint32_t>(again, 0x124) = v1_bits;
            host.widget_vcall_8c(again);
            void* target = checked_pointer_at(screen, vector, k, host);
            float size[2];
            if (1.0f > ratio) {
                size[0] = static_cast<float>(ratio * k150 / f64_bits(0x408E000000000000ull));  // 00CEC380 960.0
                size[1] = f32_bits(0x3E555555u);                                                // 00CEF1B0
            } else if (ratio > 1.0f) {
                size[0] = f32_bits(0x3E200000u);                                                // 00CEF1C0
                size[1] = static_cast<float>(k150 / ratio / f64_bits(0x4086800000000000ull));  // 00CEF1B8 720.0
            } else {
                size[0] = f32_bits(0x3E200000u);
                size[1] = f32_bits(0x3E555555u);
            }
            host.widget_vcall_58_set_position(target, size);
        };
        if ((at<std::uint8_t>(screen, 0x10) & 1) != 0) apply(checked_pointer_at(screen, 0x1F8, k, host), 0x1F8);
        apply(checked_pointer_at(screen, 0x244, k, host), 0x244);
    }

    if ((at<std::uint8_t>(screen, 0x10) & 1) != 0) host.lobby_refresh_map_availability_005d98e0(screen);
    at<std::uint32_t>(screen, 0x10) = 0;
    host.widget_vcall_70(at<void*>(screen, 0x94));
}

namespace {

std::uint32_t& image_dword(Cc13SweepB8Host& host, std::uint32_t address) {
    return *static_cast<std::uint32_t*>(host.image_global(address));
}

// [[00E198AC]+5Ch] {+18h first, +1Ch last} of 34h-byte mission groups,
// indexed by [00E194D8] (re-read each time); the bound is a signed /34h of
// the byte span with an unsigned compare, a null first failing too.
std::uint8_t* mission_group(Cc13SweepB8Host& host) {
    void* tree = at<void*>(*static_cast<void**>(host.image_global(0x00E198AC)), 0x5C);
    const std::uint32_t index = image_dword(host, 0x00E194D8);
    std::uint8_t* first = at<std::uint8_t*>(tree, 0x18);
    if (first == nullptr ||
        index >= static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint8_t*>(tree, 0x1C) - first) / 0x34)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    return at<std::uint8_t*>(tree, 0x18) + 0x34 * index;
}

// The group's mission vector (+24h: {+4h first, +8h last} of 434h-byte
// records) walked with the image's checked-iterator tests: the start must not
// pass the end, and every pass re-fetches the group, compares its container
// with the iterator's and its first with its last.
struct MissionWalk {
    std::uint8_t* container;
    std::uint8_t* node;
};

MissionWalk mission_walk_begin(Cc13SweepB8Host& host) {
    std::uint8_t* group = mission_group(host);
    std::uint8_t* first = at<std::uint8_t*>(group, 0x28);
    if (first > at<std::uint8_t*>(group, 0x2C)) host.crt_invalid_parameter_noinfo_00bf6713();
    return {group + 0x24, first};
}

bool mission_walk_at_end(const MissionWalk& walk, Cc13SweepB8Host& host) {
    std::uint8_t* group = mission_group(host);
    std::uint8_t* last = at<std::uint8_t*>(group, 0x2C);
    std::uint8_t* container = group + 0x24;
    if (at<std::uint8_t*>(container, 4) > last) host.crt_invalid_parameter_noinfo_00bf6713();
    if (walk.container != container) host.crt_invalid_parameter_noinfo_00bf6713();
    return walk.node == last;
}

void mission_walk_advance(MissionWalk& walk, Cc13SweepB8Host& host) {
    if (walk.node >= at<std::uint8_t*>(walk.container, 8)) host.crt_invalid_parameter_noinfo_00bf6713();
    walk.node += 0x434;
}

// "<prefix><index><suffix>" built as the image does: two literal
// temporaries (suffix first), 004263B0 then 004261A0, the child looked up
// under the group widget +110h, then the four strings released newest first
// (concat, int concat, prefix, suffix).
void* find_numbered_child(void* screen, const char* prefix, std::uint32_t prefix_length, std::int32_t index,
                          Cc13SweepB8Host& host) {
    Cc13SweepBNativeString suffix;
    construct_literal(suffix, "_Icon", 5, host);  // 00CED1BC
    Cc13SweepBNativeString head;
    construct_literal(head, prefix, prefix_length, host);
    Cc13SweepBNativeString numbered = {0, nullptr};
    void* with_index = host.native_string_concat_int_004263b0(&head, &numbered, index);
    Cc13SweepBNativeString name = {0, nullptr};
    void* full = host.native_string_concat_004261a0(with_index, &name, &suffix);
    void* child = host.widget_find_child_00aa7e00(at<void*>(screen, 0x110), full, 1);
    release_string(name, host);
    release_string(numbered, host);
    release_string(head, host);
    release_string(suffix, host);
    return child;
}

}  // namespace

// ---------------------------------------------------------------------------
// 00597870 BSP_MainMenuScreen_BuildMissionListPage
// __thiscall(ECX = main menu screen, int page), RET 4; EH frame 00C7129E
// (temporaries). No draw from the shared RNG 00BD2F10.
// Evidence 00597870..0059879B (x87: DC C1 at 00598741 = FADD ST1,ST0):
// - byte +56Ch = 0, [00E08874] = page, 00683790(+1D4h, 0), +110h = 0, +19Ch
//   = 0.0f, byte +1A0h = 0, 00AAC870(+33Ch) when 00AAC8E0(+33Ch).
// - [00E08874] re-read: 4 "FE.ijn_campaign", 00518B90(1), group 1, +110h =
//   +314h, list 0, bytes +564h/+565h = 0/0; 5 "FE.usn_campaign", 00518B90(0),
//   group 2, +310h, list 1, 1/0; 8 "FE.training_grounds", no flag call,
//   group 0, +320h, list 2, bytes untouched; 6 "FE.main_ijn_dlc_title",
//   00518B90(1), group 3, +31Ch, list 3, 0/1; 7 "FE.main_usn_dlc_title",
//   00518B90(0), group 4, +318h, list 4, 1/1 (its title temporary goes
//   through 0041DD20 instead of the inline release); any other page keeps
//   group, +110h and list 0. Each title: 00518D60(10, 0, 0, title).
// - Visibility: +324h, +2F0h on; +314h, +310h, +320h off; +328h, +32Ch on,
//   +32Ch vtable +88h(1, 0, 1.0f); +110h on; +344h off. 0054A0C0([00E1930C],
//   ""); +3DCh, +1B8h, +41Ch off; 00583E50; 00A9AC40(+1B8h, 0), 00A9BEC0.
// - 00E194E0 = 00E194DC = [00E08878] unless it is -1 (then 0). list = +134h
//   + 10h * list index (a vector of 0Ch-byte points); empty = its count == 0.
// - Row n = 1, 2, .. over the group's missions: unlocked = 007FC820(profile,
//   record+78h), done = 005C2F70(record); row = clone of +238h; its text is
//   record+8h when unlocked, else a temporary of the C string [00E08870];
//   row +77h = !unlocked, +D8h = n - 1, appended to +1B8h;
//   "mission_mapflag_<n>_Icon" under +110h shown as done (the image pushes
//   the dword holding the byte); "mission_mappoint_<n>_Icon" shown and,
//   when the list was empty, its resolved position minus +124h/+128h/+12Ch
//   pushed onto it (004215D0); done and [00E08878] == -1 select n.
// - [00E08878] == -1: when 00E194DC >= the list count (unsigned) both
//   selections drop by one; 00E194DC is clamped to [0, missions - 1]
//   (signed). Byte +1B0h: 00581CE0(screen, list index), byte cleared.
// - Every "mission_mapflag_<n>_Icon" gets vtable +4Ch(n == 00E194DC ? 1.0f
//   : 0.4f) (no null test). Then 00594B60; 005C35D0(tree, 005806A0());
//   +2FCh and +300h vtable +34h(007F8D60(profile, 005806A0())) (the vtable
//   slot address is read before the calls); +300h text "FE.main_checkpoint"
//   when 007FC370(profile, 005806A0()) else "FE.main_checkpoint_dlc";
//   +2FCh vtable +58h({+300h+114h / 960 + +2FCh+10Ch + +2FCh+110h,
//   size(+2FCh).y}); +1B8h: 00A9C7C0(00E194DC), listener screen+8h, vtable
//   +80h(1), +60h(1), +34h(1), bytes +149h/+14Ah = 0, position {+558h +
//   0.0, 0.0 + +55Ch, +560h + 1000.0}, 00AA6BC0(0, 0); 005830A0.
void cc13_sweepB8_main_menu_build_mission_list_00597870(void* screen, std::int32_t page, Cc13SweepB8Host& host) {
    at<std::uint8_t>(screen, 0x56C) = 0;
    image_dword(host, 0x00E08874) = static_cast<std::uint32_t>(page);
    host.gui_scroller_attach_content_00683790(&at<std::uint8_t>(screen, 0x1D4), 0);
    at<void*>(screen, 0x110) = nullptr;
    at<float>(screen, 0x19C) = 0.0f;
    at<std::uint8_t>(screen, 0x1A0) = 0;
    if (host.gui_movie_is_playing_00aac8e0(at<void*>(screen, 0x33C))) host.gui_movie_stop_00aac870(at<void*>(screen, 0x33C));

    std::int32_t list_index = 0;
    const std::uint32_t current = image_dword(host, 0x00E08874);
    auto select_title = [&](const char* title, bool inline_release) {
        Cc13SweepBNativeString text = {0, nullptr};
        host.native_string_assign_0041e870(&text, title);
        host.frontend_frame_select_00518d60(10, 0, 0, &text);
        if (inline_release) {
            release_string(text, host);
        } else {
            host.native_string_destroy_0041dd20(&text);
        }
    };
    if (current == 4) {
        select_title("FE.ijn_campaign", true);  // 00CEFE5C
        host.frontend_frame_set_flag_00518b90(1);
        image_dword(host, 0x00E194D8) = 1;
        at<void*>(screen, 0x110) = at<void*>(screen, 0x314);
        at<std::uint8_t>(screen, 0x564) = 0;
        at<std::uint8_t>(screen, 0x565) = 0;
    } else if (current == 5) {
        select_title("FE.usn_campaign", true);  // 00CEFE4C
        list_index = 1;
        host.frontend_frame_set_flag_00518b90(0);
        image_dword(host, 0x00E194D8) = 2;
        void* group_widget = at<void*>(screen, 0x310);
        at<std::uint8_t>(screen, 0x565) = 0;
        at<std::uint8_t>(screen, 0x564) = 1;
        at<void*>(screen, 0x110) = group_widget;
    } else if (current == 8) {
        select_title("FE.training_grounds", true);  // 00CEFE38
        list_index = 2;
        image_dword(host, 0x00E194D8) = 0;
        at<void*>(screen, 0x110) = at<void*>(screen, 0x320);
    } else if (current == 6) {
        select_title("FE.main_ijn_dlc_title", true);  // 00CEFE20
        host.frontend_frame_set_flag_00518b90(1);
        image_dword(host, 0x00E194D8) = 3;
        at<void*>(screen, 0x110) = at<void*>(screen, 0x31C);
        list_index = 3;
        at<std::uint8_t>(screen, 0x564) = 0;
        at<std::uint8_t>(screen, 0x565) = 1;
    } else if (current == 7) {
        select_title("FE.main_usn_dlc_title", false);  // 00CEFE08
        host.frontend_frame_set_flag_00518b90(0);
        image_dword(host, 0x00E194D8) = 4;
        void* group_widget = at<void*>(screen, 0x318);
        list_index = 4;
        at<std::uint8_t>(screen, 0x565) = 1;
        at<std::uint8_t>(screen, 0x564) = 1;
        at<void*>(screen, 0x110) = group_widget;
    }

    host.widget_vcall_34_set_visible(at<void*>(screen, 0x324), 1);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x2F0), 1);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x314), 0);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x310), 0);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x320), 0);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x328), 1);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x32C), 1);
    host.widget_vcall_88(at<void*>(screen, 0x32C), 1, 0, 1.0f);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x110), 1);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x344), 0);
    {
        Cc13SweepBNativeString empty;
        construct_literal(empty, "", 0, host);  // 00CE3A0C
        host.main_menu_command_bar_set_help_line_0054a0c0(*static_cast<void**>(host.image_global(0x00E1930C)), &empty);
        release_string(empty, host);
    }
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x3DC), 0);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x1B8), 0);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x41C), 0);
    host.main_menu_clear_objective_rows_00583e50(screen);
    host.gui_listbox_set_listener_00a9ac40(at<void*>(screen, 0x1B8), nullptr);
    host.gui_listbox_clear_rows_00a9bec0(at<void*>(screen, 0x1B8));

    const std::uint32_t preset = image_dword(host, 0x00E08878);
    if (preset != 0xFFFFFFFFu) {
        image_dword(host, 0x00E194E0) = preset;
        image_dword(host, 0x00E194DC) = preset;
    } else {
        image_dword(host, 0x00E194E0) = 0;
        image_dword(host, 0x00E194DC) = 0;
    }
    void* points = &at<std::uint8_t>(screen, 0x134 + 0x10 * static_cast<std::size_t>(list_index));
    auto point_count = [&]() {
        std::uint8_t* first = at<std::uint8_t*>(points, 4);
        return first == nullptr ? 0 : static_cast<std::int32_t>(at<std::uint8_t*>(points, 8) - first) / 12;
    };
    const bool points_empty = point_count() == 0;

    std::int32_t n = 1;
    MissionWalk walk = mission_walk_begin(host);
    while (!mission_walk_at_end(walk, host)) {
        if (walk.node >= at<std::uint8_t*>(walk.container, 8)) host.crt_invalid_parameter_noinfo_00bf6713();
        std::uint8_t* record = walk.node;
        void* profile = static_cast<std::uint8_t*>(host.global_00e188a8()) + 0x650;
        const bool unlocked = host.profile_test_unlock_007fc820(profile, record + 0x78);
        const bool done = host.mission_record_is_completed_005c2f70(record);
        void* row = host.widget_clone_subtree_00aab4c0(at<void*>(screen, 0x238), nullptr);
        if (unlocked) {
            host.gui_text_set_localised_source_00abaed0(row, record + 8, 1);
        } else {
            const char* locked = *static_cast<const char**>(host.image_global(0x00E08870));
            Cc13SweepBNativeString text;
            construct_literal(text, locked, static_cast<std::uint32_t>(std::strlen(locked)), host);
            host.gui_text_set_localised_source_00abaed0(row, &text, 1);
            release_string(text, host);
        }
        at<std::uint8_t>(row, 0x77) = unlocked ? 0 : 1;
        at<std::int32_t>(row, 0xD8) = n - 1;
        host.gui_listbox_insert_row_00a9d750(at<void*>(screen, 0x1B8), row, nullptr, 0);

        void* flag = find_numbered_child(screen, "mission_mapflag_", 0x10, n, host);  // 00CEFB90
        if (flag != nullptr) host.widget_vcall_34_set_visible(flag, done ? 1 : 0);
        void* point = find_numbered_child(screen, "mission_mappoint_", 0x11, n, host);  // 00CEFBA4
        if (point != nullptr) {
            host.widget_vcall_34_set_visible(point, 1);
            if (points_empty) {
                float resolved[3];
                const float* p = host.widget_get_resolved_position_00aa6750(point, resolved);
                const float offset[3] = {p[0] - at<float>(screen, 0x124), p[1] - at<float>(screen, 0x128),
                                         p[2] - at<float>(screen, 0x12C)};
                host.vector12_push_back_004215d0(points, offset);
            }
            if (done && image_dword(host, 0x00E08878) == 0xFFFFFFFFu) {
                image_dword(host, 0x00E194E0) = static_cast<std::uint32_t>(n);
                image_dword(host, 0x00E194DC) = static_cast<std::uint32_t>(n);
            }
        }
        mission_walk_advance(walk, host);
        ++n;
    }

    if (image_dword(host, 0x00E08878) == 0xFFFFFFFFu) {
        if (image_dword(host, 0x00E194DC) >= static_cast<std::uint32_t>(point_count())) {
            image_dword(host, 0x00E194DC) -= 1;
            image_dword(host, 0x00E194E0) -= 1;
        }
        std::uint8_t* group = mission_group(host);
        std::uint8_t* first = at<std::uint8_t*>(group, 0x28);
        const std::int32_t selected = static_cast<std::int32_t>(image_dword(host, 0x00E194DC));
        const std::int32_t missions =
            (first == nullptr ? 0 : static_cast<std::int32_t>(at<std::uint8_t*>(group, 0x2C) - first) / 0x434) - 1;
        image_dword(host, 0x00E194DC) =
            static_cast<std::uint32_t>(selected < 0 ? 0 : (selected > missions ? missions : selected));
    }
    if (at<std::uint8_t>(screen, 0x1B0) != 0) {
        host.main_menu_layout_mission_row_00581ce0(screen, list_index);
        at<std::uint8_t>(screen, 0x1B0) = 0;
    }

    n = 1;
    walk = mission_walk_begin(host);
    while (!mission_walk_at_end(walk, host)) {
        void* flag = find_numbered_child(screen, "mission_mapflag_", 0x10, n, host);
        const float alpha = image_dword(host, 0x00E194DC) == static_cast<std::uint32_t>(n) ? 1.0f
                                                                                           : f32_bits(0x3ECCCCCDu);  // 00CE7804
        host.widget_vcall_4c_set_alpha(flag, alpha);
        mission_walk_advance(walk, host);
        ++n;
    }

    host.main_menu_update_mission_medal_00594b60(screen);
    {
        void* mission = host.main_menu_selected_mission_005806a0(screen);
        host.mission_tree_select_by_id_005c35d0(at<void*>(*static_cast<void**>(host.image_global(0x00E198AC)), 0x5C),
                                                mission);
    }
    for (std::size_t slot : {std::size_t{0x2FC}, std::size_t{0x300}}) {
        void* profile = static_cast<std::uint8_t*>(host.global_00e188a8()) + 0x650;
        const std::int32_t value =
            host.profile_lookup_007f8d60(profile, host.main_menu_selected_mission_005806a0(screen));
        host.widget_vcall_34_set_visible(at<void*>(screen, slot), value);
    }
    {
        void* profile = static_cast<std::uint8_t*>(host.global_00e188a8()) + 0x650;
        const bool valid =
            host.profile_checkpoint_vehicle_class_valid_007fc370(profile, host.main_menu_selected_mission_005806a0(screen));
        host.gui_text_set_localised_cstring_00abbe50(at<void*>(screen, 0x300),
                                                     valid ? "FE.main_checkpoint" : "FE.main_checkpoint_dlc", 1);
    }
    const float height = host.widget_get_size_00aa6740(at<void*>(screen, 0x2FC))[1];
    void* checkpoint = at<void*>(screen, 0x2FC);
    const float scaled = static_cast<float>(at<float>(at<void*>(screen, 0x300), 0x114) / f64_bits(0x408E000000000000ull));
    const float xy[2] = {static_cast<float>(static_cast<double>(scaled) + at<float>(checkpoint, 0x10C) +
                                            at<float>(checkpoint, 0x110)),
                         height};
    host.widget_vcall_58_set_position(checkpoint, xy);

    void* list = at<void*>(screen, 0x1B8);
    host.listbox_select_index_00a9c7c0(list, static_cast<std::int32_t>(image_dword(host, 0x00E194DC)));
    host.gui_listbox_set_listener_00a9ac40(at<void*>(screen, 0x1B8), &at<std::uint8_t>(screen, 8));
    host.widget_vcall_80(at<void*>(screen, 0x1B8), 1);
    host.widget_vcall_60(at<void*>(screen, 0x1B8), 1);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x1B8), 1);
    at<std::uint8_t>(at<void*>(screen, 0x1B8), 0x149) = 0;
    at<std::uint8_t>(at<void*>(screen, 0x1B8), 0x14A) = 0;
    const float position[3] = {static_cast<float>(static_cast<double>(at<float>(screen, 0x558)) + 0.0),
                               static_cast<float>(0.0 + at<float>(screen, 0x55C)),
                               static_cast<float>(at<float>(screen, 0x560) + f64_bits(0x408F400000000000ull))};
    host.widget_set_resolved_position_00aa8240(at<void*>(screen, 0x1B8), position);
    host.gui_widget_set_listener_00aa6bc0(at<void*>(screen, 0x1B8), nullptr, 0);
    host.main_menu_release_mission_preview_005830a0(screen);
}

// ---------------------------------------------------------------------------
// 005E7B10 BSP_MultiMainMenuScreen_BindLayout
// __fastcall(ECX = screen), RET; EH frame 00C75382 (temporaries, the empty
// string and the award record). Vtable 00CF2708 slot +14h, no static caller.
// No draw from the shared RNG 00BD2F10.
// Evidence 005E7B10..005E8B12: pages through 00AA5840(name, 1, 0), children
// through 00AA7E00(name, 1), every name an inlined literal temporary
// released after the call; listeners 00AA6BC0(screen+0Ch, 0) on +60h, +64h,
// +CCh. Scroller +6Ch: 00683790(+68h) and 00683380(+CCh, size(slider_bg_Icon)
// .y - size(+CCh).y, +60h, +64h). +38h and +3Ch hidden. An empty string
// (literal "", kept to the end) prefixes each texture name. +D8h is a second
// achievement_Group lookup under +C8h; +D4h select_FrameBox takes +DCh's
// local position (00AA67F0 -> 00AA7DC0), local z -100.0f, hidden. For i <
// 006B8B10([00E19900]) (re-read each pass) from 006B8CC0's iterator (the
// dereference checks call 00BF6713): the node+14h award record is copied
// (0050A210); when its byte +24h is set, a clone of +DCh under +D8h gets
// vtable +2Ch(node+0Ch), +34h(1), 00AB44B0(0, "" + record+14h, {0, 0, 1,
// 1}) and the listener, and becomes +E8h; the copy is destroyed (00501FA0),
// the iterator advanced (004FBE20).
void cc13_sweepB8_multi_main_menu_bind_layout_005e7b10(void* screen, Cc13SweepB8Host& host) {
    void* listener = &at<std::uint8_t>(screen, 0xC);
    at<void*>(screen, 0x18) = load_page("FE_multimenu_listbox", 0x14, host);
    at<void*>(screen, 0x20) = find_child(at<void*>(screen, 0x18), "Ranked_Listbox", 0xE, host);
    at<void*>(screen, 0x24) = find_child(at<void*>(screen, 0x18), "Main_Listbox", 0xC, host);
    at<void*>(screen, 0x1C) = load_page("FE_multimenu", 0xC, host);
    at<void*>(screen, 0xD0) = find_child(at<void*>(screen, 0x1C), "achievement_FrameBox", 0x14, host);
    at<void*>(screen, 0xC8) = find_child(at<void*>(screen, 0x1C), "achievement_Clipbox", 0x13, host);
    at<void*>(screen, 0x68) = find_child(at<void*>(screen, 0xC8), "achievement_Group", 0x11, host);
    at<void*>(screen, 0x60) = find_child(at<void*>(screen, 0x1C), "arrow_top_Icon", 0xE, host);
    host.gui_widget_set_listener_00aa6bc0(at<void*>(screen, 0x60), listener, 0);
    at<void*>(screen, 0x64) = find_child(at<void*>(screen, 0x1C), "arrow_bottom_Icon", 0x11, host);
    host.gui_widget_set_listener_00aa6bc0(at<void*>(screen, 0x64), listener, 0);
    at<void*>(screen, 0xCC) = find_child(at<void*>(screen, 0x1C), "slider_FrameBox", 0xF, host);
    host.gui_widget_set_listener_00aa6bc0(at<void*>(screen, 0xCC), listener, 0);

    void* scroller = &at<std::uint8_t>(screen, 0x6C);
    host.gui_scroller_attach_content_00683790(
        scroller, static_cast<std::int32_t>(reinterpret_cast<std::intptr_t>(at<void*>(screen, 0x68))));
    {
        Cc13SweepBNativeString text;
        construct_literal(text, "slider_bg_Icon", 0xE, host);
        void* background = host.widget_find_child_00aa7e00(at<void*>(screen, 0x1C), &text, 1);
        const float track = host.widget_get_size_00aa6740(background)[1];
        const float frame = host.widget_get_size_00aa6740(at<void*>(screen, 0xCC))[1];
        const float length = track - frame;
        host.gui_scroller_attach_scroll_bar_00683380(scroller, at<void*>(screen, 0xCC), length, at<void*>(screen, 0x60),
                                                     at<void*>(screen, 0x64));
        release_string(text, host);
    }

    at<void*>(screen, 0x28) = find_child(at<void*>(screen, 0x1C), "text_Group", 0xA, host);
    at<void*>(screen, 0x4C) = find_child(at<void*>(screen, 0x28), "achievement_description_Text", 0x1C, host);
    at<void*>(screen, 0x48) = find_child(at<void*>(screen, 0x28), "achievement_name_Text", 0x15, host);
    at<void*>(screen, 0x2C) = find_child(at<void*>(screen, 0x1C), "player_Group", 0xC, host);
    at<void*>(screen, 0x38) = find_child(at<void*>(screen, 0x2C), "multiscore_Text", 0xF, host);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x38), 0);
    at<void*>(screen, 0x3C) = find_child(at<void*>(screen, 0x2C), "multiscore_var_Text", 0x13, host);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x3C), 0);
    at<void*>(screen, 0x30) = find_child(at<void*>(screen, 0x2C), "playername_Text", 0xF, host);
    at<void*>(screen, 0x34) = find_child(at<void*>(screen, 0x2C), "playername_var_Text", 0x13, host);
    at<void*>(screen, 0x40) = find_child(at<void*>(screen, 0x2C), "rank_Text", 9, host);
    at<void*>(screen, 0x44) = find_child(at<void*>(screen, 0x2C), "rank_var_Text", 0xD, host);
    at<void*>(screen, 0x50) = find_child(at<void*>(screen, 0x1C), "rank_Icon", 9, host);
    at<void*>(screen, 0x54) = find_child(at<void*>(screen, 0x2C), "rank_prog_Icon", 0xE, host);
    at<void*>(screen, 0x58) = find_child(at<void*>(screen, 0x2C), "rank_prog_new_Icon", 0x12, host);
    at<void*>(screen, 0x5C) = find_child(at<void*>(screen, 0x2C), "rankgraf_FrameBox", 0x11, host);

    Cc13SweepBNativeString empty;
    construct_literal(empty, "", 0, host);  // 00CE3A0C
    at<void*>(screen, 0xD8) = find_child(at<void*>(screen, 0xC8), "achievement_Group", 0x11, host);
    at<void*>(screen, 0xD4) = find_child(at<void*>(screen, 0xD8), "select_FrameBox", 0xF, host);
    at<void*>(screen, 0xDC) = find_child(at<void*>(screen, 0xD8), "achievement_0_Icon", 0x12, host);
    {
        const float* local = host.gui_widget_get_local_position_00aa67f0(at<void*>(screen, 0xDC));
        const float position[3] = {local[0], local[1], local[2]};
        host.gui_widget_set_local_position_00aa7dc0(at<void*>(screen, 0xD4), position);
        host.widget_set_local_z_00aa7910(at<void*>(screen, 0xD4), f32_bits(0xC2C80000u));  // 00CE65D8
        host.widget_vcall_34_set_visible(at<void*>(screen, 0xD4), 0);
    }

    void* iterator[2] = {nullptr, nullptr};
    host.achievement_list_begin_006b8cc0(*static_cast<void**>(host.image_global(0x00E19900)), iterator);
    for (std::int32_t i = 0; i < host.achievement_count_006b8b10(*static_cast<void**>(host.image_global(0x00E19900)));
         ++i) {
        check_iterator_dereference(iterator, host);
        alignas(4) std::uint8_t record[0x54];
        host.award_record_copy_construct_0050a210(record, static_cast<std::uint8_t*>(iterator[1]) + 0x14);
        if (record[0x24] != 0) {
            void* icon = host.widget_clone_subtree_00aab4c0(at<void*>(screen, 0xDC), at<void*>(screen, 0xD8));
            check_iterator_dereference(iterator, host);
            host.widget_vcall_2c(icon, static_cast<std::uint8_t*>(iterator[1]) + 0xC);
            host.widget_vcall_34_set_visible(icon, 1);
            Cc13SweepBNativeString texture = {0, nullptr};
            void* name = host.native_string_concat_004261a0(&empty, &texture, record + 0x14);
            const float uv[4] = {0.0f, 0.0f, 1.0f, 1.0f};
            host.gui_set_state_texture_00ab44b0(icon, 0, name, uv);
            release_string(texture, host);
            host.gui_widget_set_listener_00aa6bc0(icon, listener, 0);
            at<void*>(screen, 0xE8) = icon;
        }
        host.award_record_destroy_00501fa0(record);
        host.tree_iterator_increment_004fbe20(iterator);
    }
    release_string(empty, host);
}

}  // namespace bsp
