// cc13_big1: the large rows deferred by cc13-small2. See include/bsp/cc13_big1.hpp
// and docs/CC13_BIG1.md. Every name is a hypothesis, not a recovered symbol.

#include "bsp/cc13_big1.hpp"

#include <cmath>

namespace bsp::cc13_big1 {

namespace {

// x87 helpers. The listing's sqrt (00BF7030, CRT _CIsqrt) takes and returns ST0;
// the input here is always a float store, so the float overload gives the same
// rounding as the 24-bit FSTP that follows it.
float sqrt_00bf7030(float v) { return std::sqrt(v); }

// The reciprocal pattern at 004C7C2B and 004C7D09: FLDZ; FCOMI len, 0; JBE zero.
// JBE is taken for below, equal or unordered, so a NaN length gives 0.
float reciprocal_or_zero(float len) { return len > 0.0f ? 1.0f / len : 0.0f; }

} // namespace

// 004C7AA0 __thiscall(ECX = world; BorderZoneApproach* out, const float* position, float radius,
// int side) RET 10h. ECX is not read here: it is passed through to 004C7730. Precision: modelled
// at 24 bits (every x87 op rounded to float, in the listing's order). The only caller found is
// 0094BFF0 (cc13_large61); if it runs on a fixed-step job at 53 bits, the unstored intermediates
// (the x^2 + 0 and y^2 + x^2 partial sums and the 0.25 / 2.0 products before their FSTPs) would
// round once at the store instead (uncertain which thread applies).
void world_border_zone_approach_004c7aa0(void* world, BorderZoneApproach* out, const float* position,
                                         float radius, std::int32_t side, Big1Host& host) {
    float point[3];     // [ESP+34h]
    float direction[3]; // [ESP+1Ch]
    void* const zone =
        host.world_find_closest_border_zone_004c7730(world, position, side, point, direction);  // 004C7AB8
    if (zone == nullptr) {
        out->zone = nullptr;                                                   // 004C7DBE
        return;
    }
    float normal[3];    // [ESP+28h], reused for the inner point
    const float* const n = host.vector3f_normalize_00419510(normal, direction);  // 004C7ACF
    const float t0 = n[0] * radius;                                            // 004C7ADC
    const float t1 = n[1] * radius;                                            // 004C7AE7
    const float t2 = radius * n[2];                                            // 004C7AED
    const float inner[3] = {point[0] - t0, point[1] - t1, point[2] - t2};     // 004C7AF4..004C7B14

    // Corner average in x/z: A, B, C, D at +10h, +1Ch, +28h, +34h, each summed onto 0.
    float sx = at<float>(zone, 0x10) + 0.0f;                                   // 004C7B2C
    float sz = at<float>(zone, 0x18) + 0.0f;                                   // 004C7B38
    sx = at<float>(zone, 0x1C) + sx;                                           // 004C7B50
    sz = at<float>(zone, 0x24) + sz;                                           // 004C7B5C
    sx = at<float>(zone, 0x28) + sx;                                           // 004C7B76
    sz = at<float>(zone, 0x30) + sz;                                           // 004C7B82
    sx = at<float>(zone, 0x34) + sx;                                           // 004C7B9C
    sz = at<float>(zone, 0x3C) + sz;                                           // 004C7BA8
    const float cx = static_cast<float>(sx * 0.25);                           // 004C7BBA (00D7A348)
    const float cz = static_cast<float>(0.25 * sz);                           // 004C7BC2
    const float ux = cx - inner[0];                                            // 004C7BCE
    const float uz = cz - inner[2];                                            // 004C7BDA

    // Length with the y term taken from the FLDZ left on the stack at 004C7B2A (0 * 0).
    const float ux2 = ux * ux;                                                 // 004C7BEC
    const float uy2 = 0.0f * 0.0f;                                             // 004C7BFA
    const float uz2 = uz * uz;                                                 // 004C7C08
    const float inv = reciprocal_or_zero(sqrt_00bf7030((ux2 + uy2) + uz2));    // 004C7C06..004C7C46
    const float vx = ux * inv;                                                 // 004C7C56
    const float vy = static_cast<float>(0.0 * inv);                           // 004C7C64 (00D7A258)
    const float vz = inv * uz;                                                 // 004C7C6A

    const float w0 = static_cast<float>(direction[0] * 2.0) + vx;             // 004C7C7C, 004C7C9A
    const float w1 = static_cast<float>(direction[1] * 2.0) + vy;             // 004C7C88, 004C7CA6
    const float w2 = static_cast<float>(2.0 * direction[2]) + vz;             // 004C7C8E, 004C7CB2
    const float wy2 = w1 * w1;                                                 // 004C7CC8
    const float wx2 = w0 * w0;                                                 // 004C7CD6
    const float wz2 = w2 * w2;                                                 // 004C7CE4
    const float inv2 = reciprocal_or_zero(sqrt_00bf7030((wy2 + wx2) + wz2));   // 004C7CDE..004C7D24

    out->direction[0] = direction[0];                                          // 004C7D3E
    out->direction[1] = direction[1];                                          // 004C7D4B
    out->direction[2] = direction[2];                                          // 004C7D5C
    out->inner_point[0] = inner[0];                                            // 004C7D6D
    out->inner_point[1] = inner[1];                                            // 004C7D78
    out->inner_point[2] = inner[2];                                            // 004C7D87
    out->heading[0] = w0 * inv2;                                               // 004C7D49, 004C7D96
    out->zone = zone;                                                          // 004C7DA5
    out->heading[1] = w1 * inv2;                                               // 004C7D6B, 004C7DAB
    out->heading[2] = inv2 * w2;                                               // 004C7D8C, 004C7DB0
}

// 005AAC00 __thiscall(ECX = icon; template_widget) RET 4, EBP frame, EH handler 00C71BB8 (states
// 0 set, 1 + outer list, 2 + inner list, -1 before the set's teardown). icon+4 is the unit.
// Builds a std::set<unsigned> of HUD icon indices: for each support site in a copy of the unit's
// +784h list, 0084D560 lists the classes it supplies to the current player's team ([game +
// 18CCh + [game+18ECh]*4]+28h, flag 1); each class value goes through 00964790(DL 1) and
// 0059CB90(unit null), and the index is inserted when find() misses. With a template widget,
// one clone per index: vtable[34h](0), vtable[88h](index as a word, 0, 1.0f), pushed onto
// icon+B8h; a null clone is skipped but the iterator still advances. Returns the set's size,
// read before the set is erased (005A6420 over begin..end) and its head freed. The list
// teardowns (free each node, then the head) are inlined in the image and are done here the
// same way. Every checked-iterator test calls 00BF6713 at the binary's points.
std::uint32_t hud_map_icon_refresh_carried_unit_icons_005aac00(void* icon, void* template_widget,
                                                               Big1Host& host) {
    NativeContainer set{};   // [ESP+20h] (value-initialised here; the image leaves the proxy unset)
    void* iterator[2];     // [ESP+18h] {owner, node}
    std::uint32_t key = 0; // [ESP+10h]; the image reuses it as the clone slot
    void* insert_out[2];   // [ESP+44h] pair<iterator, bool> (only the address is passed)
    NativeContainer outer{}; // [ESP+38h] the list copy
    NativeContainer inner{}; // [ESP+2Ch] 0084D560's list

    void* head = host.set_buy_head_node_005a0300(&set);                        // 005AAC2B
    set.head = head;
    at<std::uint8_t>(set.head, 0x11) = 1;                                        // 005AAC34
    at<void*>(set.head, 0x4) = set.head;
    at<void*>(set.head, 0x0) = set.head;
    at<void*>(set.head, 0x8) = set.head;
    set.size = 0;                                                          // 005AAC4E
    host.set_erase_subtree_0059fe60(&set, at<void*>(set.head, 0x4));             // 005AAC62 (clear())
    at<void*>(set.head, 0x4) = set.head;
    set.size = 0;
    at<void*>(set.head, 0x0) = set.head;
    at<void*>(set.head, 0x8) = set.head;

    void* const unit = ptr(icon, 0x4);
    if (host.unit_has_flag_vtable5c(unit, 0x1C)) {                             // 005AAC89
        host.list_copy_005aaaf0(&outer, static_cast<std::uint8_t*>(ptr(icon, 0x4)) + 0x784);  // 005AACA0
        for (void* site_node = at<void*>(outer.head, 0); site_node != outer.head;) {  // 005AACB0
            void* const game = host.game_00e188a8();
            void* const player = at<void*>(game, 0x18CC + at<std::int32_t>(game, 0x18EC) * 4);
            host.support_site_build_supply_class_list_0084d560(
                at<void*>(site_node, 0x8), &inner, at<std::int32_t>(player, 0x28), 1);  // 005AACD8
            for (void* class_node = at<void*>(inner.head, 0); class_node != inner.head;) {  // 005AACE8
                void* const vehicle_class =
                    host.vehicle_class_get_or_create_00964790(at<void*>(class_node, 0x8), true);  // 005AACF1
                key = host.unit_hud_icon_index_0059cb90(nullptr, vehicle_class);  // 005AACFA
                host.set_find_005a18e0(&set, iterator, &key);                   // 005AAD11
                void* const end = set.head;
                if (iterator[0] == nullptr || iterator[0] != &set)
                    host.crt_invalid_parameter_noinfo_00bf6713();              // 005AAD2A
                if (iterator[1] == end) host.set_insert_005a59c0(&set, insert_out, &key);  // 005AAD43
                if (class_node == inner.head) host.crt_invalid_parameter_noinfo_00bf6713();  // 005AAD50
                class_node = at<void*>(class_node, 0);
            }
            // Inlined list teardown (005AAD5F..005AADA4).
            void* node = at<void*>(inner.head, 0);
            at<void*>(inner.head, 0) = inner.head;
            at<void*>(inner.head, 4) = inner.head;
            inner.size = 0;
            while (node != inner.head) {
                void* const next = at<void*>(node, 0);
                host.crt_free_00bf65ac(node);                                  // 005AAD83
                node = next;
            }
            host.crt_free_00bf65ac(inner.head);                                  // 005AAD96
            inner.head = nullptr;                                                // 005AADA4
            if (site_node == outer.head) host.crt_invalid_parameter_noinfo_00bf6713();  // 005AADAA
            site_node = at<void*>(site_node, 0);
        }
        if (template_widget != nullptr) {                                     // 005AADBA
            iterator[1] = at<void*>(set.head, 0);                                // begin
            iterator[0] = &set;
            for (;;) {
                if (iterator[0] == nullptr || iterator[0] != &set)
                    host.crt_invalid_parameter_noinfo_00bf6713();              // 005AADE5
                if (iterator[1] == set.head) break;                              // 005AADEC
                void* const clone = host.gui_widget_clone_subtree_00aab4c0(template_widget, nullptr);  // 005AADF3
                void* clone_slot = clone;                                      // [ESP+10h]
                if (clone != nullptr) {
                    host.widget_vtable34(clone, 0);                            // 005AAE0B
                    if (iterator[0] == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();  // 005AAE11
                    if (iterator[1] == at<void*>(iterator[0], 0x4))
                        host.crt_invalid_parameter_noinfo_00bf6713();          // 005AAE1B
                    host.widget_vtable88(clone, at<std::uint16_t>(iterator[1], 0xC), 0, 1.0f);  // 005AAE37
                    host.vector_push_back_0043dca0(static_cast<std::uint8_t*>(icon) + 0xB8,
                                                   &clone_slot);               // 005AAE48
                }
                host.set_iterator_increment_0059f0e0(iterator);                // 005AAE51
            }
        }
        // Inlined list teardown (005AAE69..005AAE96).
        void* node = at<void*>(outer.head, 0);
        at<void*>(outer.head, 0) = outer.head;
        at<void*>(outer.head, 4) = outer.head;
        outer.size = 0;
        while (node != outer.head) {
            void* const next = at<void*>(node, 0);
            host.crt_free_00bf65ac(node);                                      // 005AAE83
            node = next;
        }
        host.crt_free_00bf65ac(outer.head);                                      // 005AAE96
    }
    const std::uint32_t count = set.size;
    host.set_erase_range_005a6420(&set, iterator, &set, at<void*>(set.head, 0), &set, set.head);  // 005AAEC1
    host.crt_free_00bf65ac(set.head);                                            // 005AAECB
    return count;                                                              // 005AAED8
}

// 0064AE20 __fastcall(ECX = ship screen 45), RET; EH handler 00C7A978. screen+184h is the unit,
// [unit+538h] its class record. Lua: VehicleClass[class+70h].NoRepairGUI; the damage widget
// (+40h) is shown unless that is a boolean equal to 1. +44h = clamp((unit+980h + 0.5) / 1.5, 0,
// 1) (24-bit: the sum and the quotient each round to float; a NaN passes both clamps), the
// three gauges' +18h bytes are cleared, +128h = 0064AAD0(), +12Ch = 0.0f, 0064A9F0(0.0f). Flags:
// +105h = [class+C8h] > 0 (signed), +107h = 00818300() != 0, +106h = !+105h && 0059CB90(unit,
// null) == 16h. Widgets +54h, +68h, +6Ch shown when +105h or +106h; +54h's frame is +106h != 0.
// With neither flag and unit vtable[5Ch](8), +104h = 00852350() != 0 drives +54h (frame 2),
// +68h and +6Ch. +58h, +70h and +74h follow +107h; then 0064A960. +104h is not written on the
// other paths. The pushed bytes for vtable[34h] carry stale upper EAX bits in the image (only the
// low byte is modelled; the callee is assumed to read a bool).
void hud_ship_screen45_apply_unit_0064ae20(void* screen, Big1Host& host) {
    alignas(4) std::uint8_t globals[0x14];       // [ESP+48h]
    alignas(4) std::uint8_t vehicle_classes[0x14]; // [ESP+34h]
    alignas(4) std::uint8_t class_entry[0x14];   // [ESP+20h]
    alignas(4) std::uint8_t entry_temp[0x14];    // [ESP+5Ch]
    alignas(4) std::uint8_t no_repair[0x14];     // [ESP+0Ch]

    void* const owner = static_cast<std::uint8_t*>(host.game_00e188a8()) + 0x1A0C;
    void* const g = host.lua_state_owner_get_globals_00b67980(owner, globals);  // 0064AE4D
    host.lua_object_get_by_name_00b67800(g, vehicle_classes, "VehicleClass");  // 0064AE67 (00CE5880)
    host.lua_object_destruct_00b67700(globals);                                // 0064AE75
    host.lua_object_construct_00b65f50(class_entry);                           // 0064AE7E
    void* const unit = ptr(screen, 0x184);
    void* const entry = host.lua_object_get_by_index_00b67720(
        vehicle_classes, entry_temp, at<std::int32_t>(ptr(unit, 0x538), 0x70));  // 0064AEA4
    host.lua_object_assign_00b67690(class_entry, entry);                       // 0064AEB3
    host.lua_object_destruct_00b67700(entry_temp);                             // 0064AEC1
    host.lua_object_get_by_name_00b67800(class_entry, no_repair, "NoRepairGUI");  // 0064AED4 (00CF5C88)
    std::uint8_t show_damage = 1;
    if (host.lua_object_is_boolean_00b66000(no_repair) &&                      // 0064AEE2
        host.lua_object_get_boolean_00b66250(no_repair) == 1)                  // 0064AEEF
        show_damage = 0;
    host.widget_vtable34(ptr(screen, 0x40), show_damage);                     // 0064AF05

    const float speed = at<float>(ptr(screen, 0x184), 0x980);                  // 0064AF0D
    const float sum = static_cast<float>(speed + 0.5);                        // 0064AF1B (00D7A280)
    float throttle = static_cast<float>(sum / 1.5);                           // 0064AF21 (00CE3D78)
    if (0.0f > throttle) {                                                     // 0064AF31 FCOMIP / JBE
        throttle = 0.0f;
    } else if (throttle > 1.0f) {                                              // 0064AF4A COMISS / JBE (00D7A24C)
        throttle = 1.0f;
    }
    at<float>(screen, 0x44) = throttle;                                        // 0064AF55
    at<std::uint8_t>(ptr(screen, 0x78), 0x18) = 0;                            // 0064AF5A
    at<std::uint8_t>(ptr(screen, 0x7C), 0x18) = 0;
    at<std::uint8_t>(ptr(screen, 0x80), 0x18) = 0;                            // 0064AF6B
    at<float>(screen, 0x128) = host.hud_ship_screen45_update_dir_icon_0064aad0(screen);  // 0064AF6E
    at<float>(screen, 0x12C) = 0.0f;                                           // 0064AF84
    host.hud_ship_screen45_update_throttle_indicator_0064a9f0(screen, 0.0f);   // 0064AF8C

    at<std::uint8_t>(screen, 0x105) =
        at<std::int32_t>(ptr(ptr(screen, 0x184), 0x538), 0xC8) > 0 ? 1 : 0;    // 0064AFA3 SETG
    at<std::uint8_t>(screen, 0x107) =
        host.unit_torpedo_barrel_count_00818300(ptr(screen, 0x184)) != 0 ? 1 : 0;  // 0064AFAC
    std::uint8_t icon_16h = 0;
    if (at<std::uint8_t>(screen, 0x105) == 0 &&
        host.unit_hud_icon_index_0059cb90(ptr(screen, 0x184), nullptr) == 0x16)  // 0064AFCC
        icon_16h = 1;
    at<std::uint8_t>(screen, 0x106) = icon_16h;                            // 0064AFE5

    const auto either = [&] {
        return at<std::uint8_t>(screen, 0x105) != 0 || at<std::uint8_t>(screen, 0x106) != 0;
    };
    host.widget_vtable34(ptr(screen, 0x54), either() ? 1 : 0);                 // 0064B003
    host.widget_vtable88(ptr(screen, 0x54), at<std::uint8_t>(screen, 0x106) != 0 ? 1 : 0, 0,
                         1.0f);                                                // 0064B023
    host.widget_vtable34(ptr(screen, 0x68), either() ? 1 : 0);                 // 0064B047
    host.widget_vtable34(ptr(screen, 0x6C), either() ? 1 : 0);                 // 0064B06B
    if (!either() && host.unit_has_flag_vtable5c(ptr(screen, 0x184), 8)) {     // 0064B08A
        const std::uint8_t bombs =
            host.unit_count_bomb_platform_children_00852350(ptr(screen, 0x184)) != 0 ? 1 : 0;  // 0064B096
        at<std::uint8_t>(screen, 0x104) = bombs;                               // 0064B0A3
        host.widget_vtable34(ptr(screen, 0x54), bombs);                        // 0064B0AF
        host.widget_vtable88(ptr(screen, 0x54), 2, 0, 1.0f);                   // 0064B0C5
        host.widget_vtable34(ptr(screen, 0x68), at<std::uint8_t>(screen, 0x104));  // 0064B0D7
        host.widget_vtable34(ptr(screen, 0x6C), at<std::uint8_t>(screen, 0x104));  // 0064B0E9
    }
    host.widget_vtable34(ptr(screen, 0x58), at<std::uint8_t>(screen, 0x107));  // 0064B0FB
    host.widget_vtable34(ptr(screen, 0x70), at<std::uint8_t>(screen, 0x107));  // 0064B10D
    host.widget_vtable34(ptr(screen, 0x74), at<std::uint8_t>(screen, 0x107));  // 0064B11F
    host.hud_ship_screen45_update_formation_icon_0064a960(screen);             // 0064B123
    host.lua_object_destruct_00b67700(no_repair);                              // 0064B131
    host.lua_object_destruct_00b67700(class_entry);                            // 0064B13F
    host.lua_object_destruct_00b67700(vehicle_classes);                        // 0064B150
}

} // namespace bsp::cc13_big1
