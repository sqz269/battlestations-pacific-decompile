// cc13_big1: the large rows deferred by cc13-small2. See include/bsp/cc13_big1.hpp
// and docs/CC13_BIG1.md. Every name is a hypothesis, not a recovered symbol.

#include "bsp/cc13_big1.hpp"

#include <cmath>
#include <cstdint>
#include <cstring>

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

namespace {

struct NativeStr {
    std::uint32_t length;
    char* data;
};

// The inlined NativeString-from-C-string, append and release sequence of 005E159A..005E1611
// (and 005E164A..005E16C1): {0, 0}, strlen, 0041DD40(length, 1), memcpy(length + 1) when the
// resize produced data, 00450540, then the temporary's block back to the sized pool.
void append_lua_string(void* vector, const char* text, Big1Host& host) {
    NativeStr tmp{0, nullptr};
    std::uint32_t n = 0;
    while (text[n] != '\0') ++n;
    host.native_string_resize_0041dd40(&tmp, n, 1);
    char* const data = tmp.data;
    const std::uint32_t length = tmp.length;
    if (data != nullptr) host.crt_memcpy_00bf7680(data, text, length + 1);
    host.native_string_vector_append_00450540(vector, &tmp);
    if (data != nullptr)
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), data, length + 1, 1);
}

} // namespace

namespace {

// 00669A77..00669B6D: {0, 0} resized to 15 and filled with "InferiorFailure" (00CF0B74), then
// object vtable[198h](&name, 0, &skip); the string's block goes back to the sized pool after
// the call (length and data re-read there).
void report_inferior_failure(void* object, std::uint8_t* skip, Big1Host& host) {
    NativeStr name{0, nullptr};
    host.native_string_resize_0041dd40(&name, 0xF, 1);
    if (name.data != nullptr) host.crt_memcpy_00bf7680(name.data, "InferiorFailure", name.length + 1);
    host.object_vtable198(object, &name, 0, skip);
    if (name.data != nullptr)
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), name.data, name.length + 1, 1);
}

// The inlined list teardown: unlink the head, zero the size, free every node and the head.
void free_list_nodes(NativeContainer& list, Big1Host& host) {
    void* node = at<void*>(list.head, 0);
    at<void*>(list.head, 0) = list.head;
    at<void*>(list.head, 4) = list.head;
    list.size = 0;
    while (node != list.head) {
        void* const next = at<void*>(node, 0);
        host.crt_free_00bf65ac(node);
        node = next;
    }
    host.crt_free_00bf65ac(list.head);
}

} // namespace

// 006697E0 __fastcall(ECX = support manager screen), RET; EBP frame aligned to 8, EH handler
// 00C7BA78. team = [[game + 18CCh + [game+18ECh]*4] + 28h]. The checked iterator at
// screen+350h/354h is dereferenced (00BF6713 on a null owner or the end node); its value is
// {+8h dword, +0Ch a unit list, +18h dword}, copied (005AAAF0) and its list copied again, the
// first copy then freed by 005A0B80. List +370h is emptied inline, +4Ch = 0 and the map +50h
// cleared (00659400 plus the head reset). For each unit of the list copy whose +54h is the team,
// or whose +344h object's +54h is: when the map +40h holds the unit's word +174h (0065B160), a
// linear scan from begin finds the node with that word key, +4Ch = its +10h and, unless it is
// the same object, map +50h = its +14h map (006641B0 erase of begin..end, then 0065CE20). Then,
// when unit+3Ch is set, vtable[5Ch](45h) or else (46h) reports "InferiorFailure" through
// vtable[198h], whose bool out skips the rest. Otherwise 0084D560(unit; &list, team, 1) and
// each of its 12-byte values is appended to +370h (00659AA0, 00662A70, then the two links), and
// the temporary list is freed inline. The loop runs while the counter is below the copy's
// current size; the copy is freed inline at the end.
void hud_support_manager_collect_catalog_006697e0(void* screen, Big1Host& host) {
    std::uint8_t* const s = static_cast<std::uint8_t*>(screen);
    void* const game = host.game_00e188a8();
    const std::int32_t team =
        at<std::int32_t>(at<void*>(game, 0x18CC + at<std::int32_t>(game, 0x18EC) * 4), 0x28);  // [B+1Ch]
    if (ptr(s, 0x350) == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();  // 0066982A
    if (ptr(s, 0x354) == at<void*>(ptr(s, 0x350), 4)) host.crt_invalid_parameter_noinfo_00bf6713();  // 00669840
    std::uint8_t* const current = static_cast<std::uint8_t*>(ptr(s, 0x354));
    std::uint32_t value_head = at<std::uint32_t>(current, 0x8);              // [B+74h]
    NativeContainer value_list{};                                           // [B+78h]
    host.list_copy_005aaaf0(&value_list, current + 0xC);                    // 0066985D
    std::uint32_t value_tail = at<std::uint32_t>(current, 0x18);             // [B+84h]
    static_cast<void>(value_head);
    static_cast<void>(value_tail);
    NativeContainer units{};                                                // [B+5Ch]
    host.list_copy_005aaaf0(&units, &value_list);                           // 00669880
    host.list_tidy_005a0b80(&value_list);                                   // 00669891

    // Inlined clear of list +370h (00669896..006698D8); the units' begin is read in between.
    void* node = at<void*>(ptr(s, 0x374), 0);
    void* it = at<void*>(units.head, 0);                                    // EDI, [B+40h]
    at<void*>(ptr(s, 0x374), 0) = ptr(s, 0x374);
    at<void*>(ptr(s, 0x374), 4) = ptr(s, 0x374);
    at<std::uint32_t>(s, 0x378) = 0;
    while (node != ptr(s, 0x374)) {
        void* const next = at<void*>(node, 0);
        host.crt_free_00bf65ac(node);                                       // 006698C8
        node = next;
    }
    at<std::uint32_t>(s, 0x4C) = 0;                                         // 006698DA
    host.support_tree_erase_subtree_00659400(s + 0x50, at<void*>(ptr(s, 0x54), 4));  // 006698ED
    at<void*>(ptr(s, 0x54), 4) = ptr(s, 0x54);
    at<std::uint32_t>(s, 0x58) = 0;
    at<void*>(ptr(s, 0x54), 0) = ptr(s, 0x54);
    at<void*>(ptr(s, 0x54), 8) = ptr(s, 0x54);

    std::uint32_t count = 0;                                                // [B+18h]
    if (units.size != 0) {                                                  // 00669910 JBE
        for (;;) {
            if (it == units.head) host.crt_invalid_parameter_noinfo_00bf6713();  // 0066992E
            bool take = at<std::int32_t>(ptr(it, 8), 0x54) == team;         // 00669936
            if (!take) {
                if (it == units.head) host.crt_invalid_parameter_noinfo_00bf6713();  // 00669941
                if (ptr(ptr(it, 8), 0x344) != nullptr) {                    // 00669949
                    if (it == units.head) host.crt_invalid_parameter_noinfo_00bf6713();  // 0066995C
                    take = at<std::int32_t>(ptr(ptr(it, 8), 0x344), 0x54) == team;  // 0066996A
                }
            }
            if (take) {
                if (it == units.head) host.crt_invalid_parameter_noinfo_00bf6713();  // 00669979
                void* const unit = ptr(it, 8);                              // EBX
                std::uint32_t key = at<std::uint16_t>(unit, 0x174);         // [B+20h]
                void* const tree = s + 0x40;                                // ESI
                void* const end = ptr(s, 0x44);                             // EBP
                void* found[2];                                             // [B+4Ch]
                void** const r = static_cast<void**>(host.support_tree_find_0065b160(tree, found, &key));  // 0066999E
                if (r[0] == nullptr || r[0] != tree) host.crt_invalid_parameter_noinfo_00bf6713();  // 006699AF
                if (r[1] != end) {                                          // 006699B4
                    void* scan[2] = {tree, at<void*>(ptr(s, 0x44), 0)};     // [B+34h] {owner, node}
                    bool hit = false;
                    for (;;) {
                        void* const head = ptr(s, 0x44);                    // [B+48h]
                        if (scan[0] == nullptr || scan[0] != tree)
                            host.crt_invalid_parameter_noinfo_00bf6713();   // 006699DF
                        if (scan[1] == head) break;                         // 006699E8
                        if (scan[0] == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();  // 006699EE
                        if (scan[1] == at<void*>(scan[0], 4)) host.crt_invalid_parameter_noinfo_00bf6713();  // 006699F8
                        if (at<std::uint16_t>(scan[1], 0xC) == at<std::uint16_t>(unit, 0x174)) {  // 00669A04
                            hit = true;
                            break;
                        }
                        host.support_tree_iterator_increment_00654f20(scan);  // 00669A0E
                    }
                    if (hit) {
                        if (scan[1] == at<void*>(scan[0], 4)) host.crt_invalid_parameter_noinfo_00bf6713();  // 00669A22
                        at<std::uint32_t>(s, 0x4C) = at<std::uint32_t>(scan[1], 0x10);  // 00669A2E
                        std::uint8_t* const dst = s + 0x50;
                        std::uint8_t* const src = static_cast<std::uint8_t*>(scan[1]) + 0x14;
                        if (dst != src) {                                   // 00669A37
                            void* out[2];                                   // [B+54h]
                            host.support_tree_erase_range_006641b0(dst, out, dst, at<void*>(ptr(dst, 4), 0), dst,
                                                                   ptr(dst, 4));  // 00669A4B
                            host.support_tree_copy_0065ce20(dst, src);      // 00669A53
                        }
                    }
                }
                bool skip_supplies = false;
                if (ptr(unit, 0x3C) != nullptr) {                           // 00669A58
                    std::uint8_t skip = 0;                                  // [B+13h]
                    if (host.unit_has_flag_vtable5c(ptr(unit, 0x3C), 0x45))  // 00669A71
                        report_inferior_failure(ptr(unit, 0x3C), &skip, host);  // [B+24h] string
                    else if (host.unit_has_flag_vtable5c(ptr(unit, 0x3C), 0x46))  // 00669AF0
                        report_inferior_failure(ptr(unit, 0x3C), &skip, host);  // [B+2Ch] string
                    skip_supplies = skip != 0;                              // 00669B72
                }
                if (!skip_supplies) {
                    NativeContainer supplies{};                             // [B+68h]
                    host.support_site_build_supply_class_list_0084d560(unit, &supplies, team, 1);  // 00669B8B
                    for (void* entry = at<void*>(supplies.head, 0); entry != supplies.head;) {
                        void* const list = s + 0x370;
                        void* const head = ptr(s, 0x374);
                        void* const added = host.support_list_buy_node_00659aa0(
                            list, head, at<void*>(head, 4), static_cast<std::uint8_t*>(entry) + 8);  // 00669BBF
                        host.support_list_incsize_00662a70(list, 1);        // 00669BCA
                        at<void*>(head, 4) = added;                         // 00669BCF
                        at<void*>(at<void*>(added, 4), 0) = added;          // 00669BD5
                        if (entry == supplies.head) host.crt_invalid_parameter_noinfo_00bf6713();  // 00669BDF
                        entry = at<void*>(entry, 0);
                    }
                    free_list_nodes(supplies, host);                        // 00669BED..00669C26
                    supplies.head = nullptr;                                // 00669C2E
                }
            }
            ++count;                                                        // 00669C3A
            if (it == units.head) host.crt_invalid_parameter_noinfo_00bf6713();  // 00669C45
            it = at<void*>(it, 0);
            if (!(count < units.size)) break;                               // 00669C58 JC
        }
    }
    free_list_nodes(units, host);                                           // 00669C60..00669C96
}

namespace {

// The inlined clear of a screen list {proxy, head, size} at s+off (head links reset, size 0,
// every node freed; the head stays).
void clear_screen_list(std::uint8_t* s, std::size_t off, Big1Host& host) {
    void* node = at<void*>(ptr(s, off + 4), 0);
    at<void*>(ptr(s, off + 4), 0) = ptr(s, off + 4);
    at<void*>(ptr(s, off + 4), 4) = ptr(s, off + 4);
    at<std::uint32_t>(s, off + 8) = 0;
    while (node != ptr(s, off + 4)) {
        void* const next = at<void*>(node, 0);
        host.crt_free_00bf65ac(node);
        node = next;
    }
}

// The inlined push_back of a list<void*> {proxy, head, size}: 004C2220(head, head->prev, value),
// 004CEE30(1), then head->prev = node and node->prev->next = node.
void unit_list_push_back(void* list, void* const* value, Big1Host& host) {
    void* const head = at<void*>(list, 4);
    void* const node = host.unit_list_buy_node_004c2220(list, head, at<void*>(head, 4), value);
    host.unit_list_incsize_004cee30(list, 1);
    at<void*>(head, 4) = node;
    at<void*>(at<void*>(node, 4), 0) = node;
}

} // namespace

// 0066A8A0 __fastcall(ECX = support manager screen), RET; EH handler 00C7BAE0. Lists (each
// {proxy, head, size}): A +3E0h (14h-byte entries), B +3ACh, C +3B8h, D +3C4h and E +3ECh (units
// or entries). A and B are emptied inline. A temporary list<void*> T (004C1630) gathers, for each
// unit of [[game+19CCh]+58h] (node +4 next, +8 unit) whose 009FFD20 slot is the current player
// ([game+18ECh], read before the call) and that is not IsKindOf(9): an IsKindOf(6) unit with +5Dh
// clear goes onto T and an entry {class +538h, -1, +28Ch, byte 1, unit} is pushed at A's front
// (inlined 00659D40 / 006632D0); an IsKindOf(0Fh) unit with +5Dh clear whose leader (+9D4h) word
// +174h differs from the last one taken puts the leader onto T and pushes {class, unit+C54h,
// leader+28Ch, byte 1, leader} at A's front through 006652C0. B is then T reversed. Each record
// of [00F89B3C] whose +80h is the current player puts {obj, Equipment+0Ch or -1, rec+7Ch, byte 0,
// 0} at A's front (obj and a copy of the name string from the record's first vector element;
// the property bag is that element's +0Ch, searched only when obj vtable[18h](0Fh)), counting
// them. When (+2CCh clear and +3F8h < count) or A's size > +3F4h: +3D5h = 1, +154h = 0, the
// scroll +2C8h from +2C4h/+324h (zeroed for game modes 4..7), and +324h = +2C4h = 0. When C and B
// are non-empty and C is larger, D = the units of C whose word +174h is not in B. Then C = B
// (006681D0), +3F8h = count, E = A (00668350), and T is freed inline. The C/B scan's owner tests
// compare an address with itself in the image (CMP EAX,EAX) and cannot fire; they are omitted.
// The bytes of the 14h-byte entry beyond those written are left as the stack had them.
void hud_support_manager_rebuild_lists_0066a8a0(void* screen, Big1Host& host) {
    std::uint8_t* const s = static_cast<std::uint8_t*>(screen);
    void* const list_a = s + 0x3E0;
    void* const list_b = s + 0x3AC;
    std::int32_t count = 0;                                                  // [B+1Ch]
    clear_screen_list(s, 0x3E0, host);                                       // 0066A8BC..0066A900
    std::uint32_t last_leader_id = 0;                                        // [B+20h]
    clear_screen_list(s, 0x3AC, host);                                       // 0066A902..0066A934
    NativeContainer t{};                                                     // [B+3Ch]
    t.head = host.unit_list_buy_head_node_004c1630(&t);                      // 0066A93A
    t.size = 0;
    alignas(4) std::uint8_t entry[0x14];                                     // [B+48h]

    void* const game = host.game_00e188a8();
    for (void* node = ptr(ptr(game, 0x19CC), 0x58); node != nullptr; node = ptr(node, 4)) {
        void* unit = ptr(node, 8);                                           // [B+24h]
        const std::int32_t player = at<std::int32_t>(host.game_00e188a8(), 0x18EC);
        if (host.unit_loss_counting_slot_009ffd20(unit) != player) continue;  // 0066A985
        if (host.unit_has_flag_vtable5c(unit, 9)) continue;                  // 0066A99B
        if (host.unit_has_flag_vtable5c(unit, 6) && at<std::uint8_t>(unit, 0x5D) == 0) {  // 0066A9AE
            unit_list_push_back(&t, &unit, host);                            // 0066A9D7..0066A9EF
            at<void*>(entry, 0x0) = ptr(unit, 0x538);
            at<std::int32_t>(entry, 0x4) = -1;
            at<std::uint32_t>(entry, 0x8) = at<std::uint32_t>(unit, 0x28C);
            at<std::uint8_t>(entry, 0xC) = 1;
            at<void*>(entry, 0x10) = unit;
            void* const begin = at<void*>(ptr(list_a, 4), 0);
            void* const added =
                host.entry_list_buy_node_00659d40(list_a, begin, at<void*>(begin, 4), entry);  // 0066AA27
            host.entry_list_incsize_006632d0(list_a, 1);                     // 0066AA32
            at<void*>(begin, 4) = added;
            at<void*>(at<void*>(added, 4), 0) = added;
        }
        if (host.unit_has_flag_vtable5c(unit, 0xF) && at<std::uint8_t>(unit, 0x5D) == 0) {  // 0066AA4C
            void* const leader = ptr(unit, 0x9D4);
            const std::uint16_t id = at<std::uint16_t>(leader, 0x174);       // [B+34h]
            if (id != static_cast<std::uint16_t>(last_leader_id)) {          // 0066AA6D
                void* pushed = leader;                                       // [B+24h]
                unit_list_push_back(&t, &pushed, host);                      // 0066AA91..0066AAA8
                at<void*>(entry, 0x0) = ptr(unit, 0x538);
                at<std::uint32_t>(entry, 0x4) = at<std::uint32_t>(unit, 0xC54);
                at<std::uint32_t>(entry, 0x8) = at<std::uint32_t>(leader, 0x28C);
                at<std::uint8_t>(entry, 0xC) = 1;
                at<void*>(entry, 0x10) = leader;
                host.entry_list_push_front_006652c0(list_a, entry);          // 0066AADA
                last_leader_id = id;                                         // 0066AAEC
            }
        }
    }

    // B = T reversed (0066AAFF..0066AB55).
    for (void* it = t.head; it != at<void*>(t.head, 0);) {
        it = at<void*>(it, 4);
        if (it == t.head) host.crt_invalid_parameter_noinfo_00bf6713();      // 0066AB12
        if (it == t.head) host.crt_invalid_parameter_noinfo_00bf6713();      // 0066AB1D
        unit_list_push_back(list_b, static_cast<void* const*>(static_cast<void*>(static_cast<std::uint8_t*>(it) + 8)),
                            host);                                           // 0066AB3D..0066AB53
    }

    void* const records = host.global_00f89b3c();
    if (at<std::uint32_t>(records, 8) != 0) {                                // 0066AB5D
        void* owner = records;                                               // [B+34h]
        void* it = at<void*>(ptr(records, 4), 0);                            // EBP
        for (;;) {
            void* const g = host.global_00f89b3c();
            if (owner == nullptr || owner != g) host.crt_invalid_parameter_noinfo_00bf6713();  // 0066AB7D
            if (it == ptr(g, 4)) break;                                      // 0066AB84
            if (owner == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();  // 0066AB8E
            if (it == ptr(owner, 4)) host.crt_invalid_parameter_noinfo_00bf6713();  // 0066AB98
            if (at<std::int32_t>(ptr(it, 8), 0x80) == at<std::int32_t>(host.game_00e188a8(), 0x18EC)) {  // 0066ABAC
                if (it == ptr(owner, 4)) host.crt_invalid_parameter_noinfo_00bf6713();  // 0066ABC1
                void* const record = ptr(it, 8);
                std::uint8_t* const first = at<std::uint8_t*>(record, 4);
                if (first > at<std::uint8_t*>(record, 8)) host.crt_invalid_parameter_noinfo_00bf6713();  // 0066ABD1
                if (!(at<std::uint8_t*>(record, 4) < at<std::uint8_t*>(record, 8)))
                    host.crt_invalid_parameter_noinfo_00bf6713();            // 0066ABDB
                void* const obj = at<void*>(first, 0);                       // [B+24h]
                NativeStr name{0, nullptr};                                  // [B+28h]
                const NativeStr* const source = reinterpret_cast<const NativeStr*>(first + 4);
                char* data = nullptr;                                        // EBX
                if (&name != source) {                                       // 0066ABEB (never equal)
                    host.native_string_resize_0041dd40(&name, source->length, 1);  // 0066AC02
                    data = name.data;
                    if (source->length != 0) host.crt_memcpy_00bf7680(data, source->data, name.length);  // 0066AC1A
                }
                void* const bag = at<void*>(first, 0xC);                     // [B+30h]
                at<void*>(entry, 0x0) = obj;                                 // 0066AC39
                if (host.object_vtable18(obj, 0xF))                          // 0066AC3D
                    at<std::uint32_t>(entry, 0x4) =
                        // 0066AC4A (00CF69AC)
                        at<std::uint32_t>(host.scene_property_bag_find_008f2260(bag, "Equipment"), 0xC);
                else
                    at<std::int32_t>(entry, 0x4) = -1;                       // 0066AC58
                if (it == ptr(owner, 4)) host.crt_invalid_parameter_noinfo_00bf6713();  // 0066AC69
                at<std::uint32_t>(entry, 0x8) = at<std::uint32_t>(ptr(it, 8), 0x7C);  // 0066AC74
                at<std::uint8_t>(entry, 0xC) = 0;                            // 0066AC8B
                at<void*>(entry, 0x10) = nullptr;                            // 0066AC90
                void* const begin = at<void*>(ptr(list_a, 4), 0);
                void* const added =
                    host.entry_list_buy_node_00659d40(list_a, begin, at<void*>(begin, 4), entry);  // 0066AC98
                host.entry_list_incsize_006632d0(list_a, 1);                 // 0066ACA5
                ++count;                                                     // 0066ACAA
                at<void*>(begin, 4) = added;
                at<void*>(at<void*>(added, 4), 0) = added;
                if (data != nullptr)
                    host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), data, name.length + 1,
                                                          1);                // 0066ACD2
            }
            if (it == ptr(owner, 4)) host.crt_invalid_parameter_noinfo_00bf6713();  // 0066ACE0
            it = at<void*>(it, 0);
        }
    }

    if ((at<std::uint8_t>(s, 0x2CC) == 0 && at<std::int32_t>(s, 0x3F8) < count) ||   // 0066AD0E JL
        at<std::uint32_t>(s, 0x3E8) > at<std::uint32_t>(s, 0x3F4)) {              // 0066AD1C JBE
        const std::int32_t top = at<std::int32_t>(s, 0x324);
        const std::int32_t selection = at<std::int32_t>(s, 0x2C4);
        at<std::uint8_t>(s, 0x3D5) = 1;
        at<std::uint8_t>(s, 0x154) = 0;
        if (top > selection)                                                 // 0066AD3E JLE
            at<std::int32_t>(s, 0x2C8) = at<std::int32_t>(s, 0x3E8) - top + selection;
        else
            at<std::int32_t>(s, 0x2C8) = selection - top + 1;
        const std::int32_t mode = host.game_effective_mode_004bca50(host.game_00e188a8());  // 0066AD63
        if (at<std::int32_t>(s, 0x2C8) != 0 && (mode == 4 || mode == 5 || mode == 6 || mode == 7))
            at<std::int32_t>(s, 0x2C8) = 0;                                  // 0066AD85
        at<std::int32_t>(s, 0x324) = 0;
        at<std::int32_t>(s, 0x2C4) = 0;
    }

    const std::uint32_t c_size = at<std::uint32_t>(s, 0x3C0);
    // 0066ADAB..0066ADC5
    if (c_size != 0 && at<std::uint32_t>(s, 0x3B4) != 0 && c_size > at<std::uint32_t>(s, 0x3B4)) {
        clear_screen_list(s, 0x3C4, host);                                   // 0066ADCF..0066AE01
        for (void* c = at<void*>(ptr(s, 0x3BC), 0); c != ptr(s, 0x3BC);) {  // 0066AE25
            bool found = false;
            for (void* b = at<void*>(ptr(s, 0x3B0), 0);;) {
                if (b == ptr(s, 0x3B0)) break;                               // 0066AE54
                if (b == ptr(s, 0x3B0)) host.crt_invalid_parameter_noinfo_00bf6713();  // 0066AE5B
                const std::uint16_t key = at<std::uint16_t>(ptr(b, 8), 0x174);
                if (c == ptr(s, 0x3BC)) host.crt_invalid_parameter_noinfo_00bf6713();  // 0066AE7C
                if (key == at<std::uint16_t>(ptr(c, 8), 0x174)) {            // 0066AE89
                    found = true;
                    break;
                }
                if (b == ptr(s, 0x3B0)) host.crt_invalid_parameter_noinfo_00bf6713();  // 0066AE97
                b = at<void*>(b, 0);
            }
            if (!found) {
                if (c == ptr(s, 0x3BC)) host.crt_invalid_parameter_noinfo_00bf6713();  // 0066AEAE
                unit_list_push_back(s + 0x3C4,
                                    static_cast<void* const*>(static_cast<void*>(static_cast<std::uint8_t*>(c) + 8)),
                                    host);                                   // 0066AEC1..0066AED7
            }
            if (c == ptr(s, 0x3BC)) host.crt_invalid_parameter_noinfo_00bf6713();  // 0066AEE7
            c = at<void*>(c, 0);
        }
    }

    host.unit_list_assign_006681d0(s + 0x3B8, list_b, at<void*>(ptr(list_b, 4), 0), list_b, ptr(list_b, 4),
                                   0);                                       // 0066AF19 (tag stale)
    at<std::int32_t>(s, 0x3F8) = count;                                      // 0066AF26
    host.entry_list_assign_00668350(s + 0x3EC, list_a, at<void*>(ptr(list_a, 4), 0), list_a, ptr(list_a, 4),
                                    0);                                      // 0066AF44 (tag stale)
    free_list_nodes(t, host);                                                // 0066AF49..0066AF7E
}

namespace {

// The checked dereference of the iterator {owner, node} at screen+off: 00BF6713 on a null owner
// or on the end node (the owner's +4h).
void check_iterator(std::uint8_t* s, std::size_t off, Big1Host& host) {
    if (ptr(s, off) == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();
    if (ptr(s, off + 4) == at<void*>(ptr(s, off), 4)) host.crt_invalid_parameter_noinfo_00bf6713();
}

// A NativeString's block back to the sized pool, when it has data (the inlined destructor).
void release_str(const NativeStr& str, Big1Host& host) {
    if (str.data != nullptr)
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), str.data, str.length + 1, 1);
}

// _ftol2 (00BF7420): ST0 truncated to a 64-bit integer, of which EAX is the low dword. Outside
// the int64 range (and for a NaN) the store gives 8000000000000000h, whose low dword is 0.
std::int32_t ftol2_low_00bf7420(double v) {
    if (!(v >= -9223372036854775808.0 && v < 9223372036854775808.0)) return 0;
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(
        static_cast<std::uint64_t>(static_cast<std::int64_t>(v))));
}

// 00675352..00675408 and 0067548F..0067554B: the device icon of widget +158h. Each read goes
// through 009552E0(unit; id) and 00443490([[[r+4]+8]+4], DL = 0); the texture is
// [[[c+74h]+34h]+7Ch] of the first, the UV the four dwords at [[c+74h]+34h]+90h of the second.
// The id is re-read for each call (from screen+334h, or from unit+134h).
void set_device_icon(std::uint8_t* s, void* unit, const std::uint8_t* id_base, std::size_t id_off,
                     Big1Host& host) {
    auto device = [&]() {
        void* const r = host.object_class_device_set_009552e0(unit, at<std::uint32_t>(id_base, id_off));
        return host.device_class_resolve_00443490(at<std::uint32_t>(ptr(ptr(r, 4), 8), 4), 0);
    };
    void* const texture = ptr(ptr(ptr(device(), 0x74), 0x34), 0x7C);
    void* const record = ptr(ptr(device(), 0x74), 0x34);
    const std::uint32_t uv[4] = {at<std::uint32_t>(record, 0x90), at<std::uint32_t>(record, 0x94),
                                 at<std::uint32_t>(record, 0x98), at<std::uint32_t>(record, 0x9C)};  // [B+4Ch]
    host.gui_set_state_texture_uv_00ab2690(ptr(s, 0x158), 3, texture, uv);
    host.widget_vtable88(ptr(s, 0x158), 0, 0, 1.0f);
    host.widget_vtable88(ptr(s, 0x158), 3, 0, 1.0f);
}

// 00674F58..006750B3: the Enter with an available unit at the iterator +300h.
void enter_unit_arm(std::uint8_t* s, Big1Host& host) {
    if (at<std::int32_t>(s, 0x1F4) == 1) {                                  // 00674F5E
        host.support_build_entries_0066a130(s);                              // 00674F65
        if (at<std::uint32_t>(s, 0x384) == 0 && at<std::uint32_t>(s, 0x2EC) == 0) {
            at<std::int32_t>(s, 0x5C) = 1;                                   // 00674F80
            host.widget_vtable34(ptr(s, 0xF0), 1);
            host.widget_vtable34(ptr(s, 0xF4), 0);
        } else {
            at<std::int32_t>(s, 0x5C) = 2;                                   // 00674F9E
            host.widget_vtable34(ptr(s, 0x12C), 0);
            host.widget_vtable34(ptr(s, 0x130), 0);
            host.widget_vtable34(ptr(s, 0x134), 0);
        }
    }
    check_iterator(s, 0x300, host);                                          // 00674FC9
    host.gui_text_set_localised_cstring_00abbe50(
        ptr(s, 0x170), host.object_vtable14_name(at<void*>(ptr(s, 0x304), 8)), 1);  // 00674FF7
    host.support_layout_name_label_0065bbc0(s);                              // 00674FFE
    check_iterator(s, 0x300, host);                                          // 00675003
    if (host.unit_has_flag_vtable5c(at<void*>(ptr(s, 0x304), 8), 0x46)) {    // 00675028
        host.widget_vtable88(ptr(s, 0x1CC), 0, 0, 1.0f);
        return;
    }
    if (host.unit_has_flag_vtable5c(*host.support_unit_iterator_value_00658a60(s + 0x300), 9)) {  // 0067505B
        host.widget_vtable88(ptr(s, 0x1CC), 1, 0, 1.0f);
        return;
    }
    if (host.unit_has_flag_vtable5c(*host.support_unit_iterator_value_00658a60(s + 0x300), 0x45))  // 0067508F
        host.widget_vtable88(ptr(s, 0x1CC), 2, 0, 1.0f);
}

// 006750CF..0067557F: the Enter on the catalog page (+34Ch == 1).
void enter_catalog_arm(std::uint8_t* s, Big1Host& host) {
    host.list_clear_004e7a40(s + 0x370);                                     // 006750D5
    void* const game = host.game_00e188a8();
    const std::int32_t team =
        at<std::int32_t>(at<void*>(game, 0x18CC + at<std::int32_t>(game, 0x18EC) * 4), 0x28);  // [B+14h]
    check_iterator(s, 0x350, host);                                          // 006750FB / 00675111
    std::uint8_t* const row = static_cast<std::uint8_t*>(ptr(s, 0x354));
    NativeContainer value_list{};                                           // [B+60h] of {[B+5Ch], list, [B+6Ch]}
    host.list_copy_005aaaf0(&value_list, row + 0xC);                        // 0067512E
    NativeContainer units{};                                                // [B+40h]
    host.list_copy_005aaaf0(&units, &value_list);                           // 0067514E
    host.list_tidy_005a0b80(&value_list);                                   // 0067515C

    for (void* it = at<void*>(units.head, 0); it != units.head;) {          // 00675167
        NativeContainer supplies{};                                         // [B+34h]
        host.support_site_build_supply_class_list_0084d560(ptr(it, 8), &supplies, team, 1);  // 00675184
        for (void* entry = at<void*>(supplies.head, 0); entry != supplies.head;) {
            void* const list = s + 0x370;
            void* const head = ptr(s, 0x374);
            void* const added = host.support_list_buy_node_00659aa0(
                list, head, at<void*>(head, 4), static_cast<std::uint8_t*>(entry) + 8);  // 006751AD
            host.support_list_incsize_00662a70(list, 1);                    // 006751BC
            at<void*>(head, 4) = added;                                     // 006751C1
            at<void*>(at<void*>(added, 4), 0) = added;                      // 006751C7
            if (entry == supplies.head) host.crt_invalid_parameter_noinfo_00bf6713();  // 006751D1
            entry = at<void*>(entry, 0);
        }
        free_list_nodes(supplies, host);                                    // 006751E2..0067521D
        supplies.head = nullptr;                                            // 00675224
        if (it == units.head) host.crt_invalid_parameter_noinfo_00bf6713(); // 0067522E
        it = at<void*>(it, 0);
    }

    host.support_build_entries_0066a130(s);                                  // 0067523F
    if (at<std::uint32_t>(s, 0x384) == 0 && at<std::uint32_t>(s, 0x2EC) == 0) {
        at<std::int32_t>(s, 0x5C) = 1;                                       // 0067525A
        host.widget_vtable34(ptr(s, 0xF0), 1);
        host.widget_vtable34(ptr(s, 0xF4), 0);                               // 00675561
    } else {
        at<std::int32_t>(s, 0x5C) = 2;                                       // 00675276
        host.widget_vtable34(ptr(s, 0x12C), 0);
        host.widget_vtable34(ptr(s, 0x130), 0);
        host.widget_vtable34(ptr(s, 0x134), 0);
        bool device = false;
        if (at<std::uint32_t>(s, 0x384) != 0) {                              // 006752A1
            if (at<std::uint32_t>(s, 0x58) != 0) {
                host.support_refresh_selection_0066d020(s);                  // 006752B4
            } else {
                void* const head = ptr(s, 0x380);
                void* const first = at<void*>(head, 0);
                if (first == head) host.crt_invalid_parameter_noinfo_00bf6713();  // 006752C7
                at<void*>(s, 0x3A4) = at<void*>(first, 8);                   // 006752CF
            }
            if (host.object_vtable18(ptr(s, 0x3A4), 0xF)) {                  // 006752E8
                device = true;
                void* const unit = ptr(s, 0x3A4);                            // EDI
                void* const map = s + 0x50;
                void* const end = ptr(s, 0x54);                              // [B+38h]
                void* found[2];                                              // [B+2Ch]
                void** const r =
                    static_cast<void**>(host.support_map_find_0065b0f0(map, found, s + 0x3A4));  // 00675307
                if (r[0] == nullptr || r[0] != map) host.crt_invalid_parameter_noinfo_00bf6713();  // 0067531A
                if (r[1] != end)
                    at<std::uint32_t>(s, 0x334) =
                        *static_cast<std::uint32_t*>(host.support_map_subscript_00667d60(map, s + 0x3A4));  // 0067532F
                else
                    at<std::uint32_t>(s, 0x334) = at<std::uint32_t>(unit, 0x134);  // 0067533E
                if (at<std::uint32_t>(s, 0x334) != 0) {
                    set_device_icon(s, unit, s, 0x334, host);                // 0067535B..00675433
                } else {
                    const bool kind17 = host.object_vtable18(unit, 0x17);    // 00675411
                    host.widget_vtable88(ptr(s, 0x158), kind17 ? 1 : 0, 0, 1.0f);  // 00675433
                }
            }
        } else if (at<std::uint32_t>(s, 0x2EC) != 0) {                       // 0067544B
            void* const head = ptr(s, 0x2E8);
            void* const first = at<void*>(head, 0);
            if (first == head) host.crt_invalid_parameter_noinfo_00bf6713();  // 00675463
            at<void*>(s, 0x2A0) = at<void*>(first, 8);                       // 0067546B
            host.support_select_weapon_value_00669170(s);                    // 00675473
            if (host.object_vtable18(ptr(s, 0x2A0), 0xF)) {                  // 00675485
                device = true;
                void* const unit = ptr(s, 0x2A0);                            // EDI
                set_device_icon(s, unit, static_cast<std::uint8_t*>(unit), 0x134, host);  // 0067549E..00675433
            }
        } else {
            host.list_tidy_005a0b80(&units);                                 // 0067556E
            return;
        }
        if (device) {
            host.widget_vtable34(ptr(s, 0xF0), 0);                           // 00675442
            host.widget_vtable34(ptr(s, 0xF4), 1);
        } else {
            host.widget_vtable34(ptr(s, 0xF0), 1);                           // 0067555D
            host.widget_vtable34(ptr(s, 0xF4), 0);
        }
    }
    host.list_tidy_005a0b80(&units);                                         // 0067557A
}

// One by-value copy of the selected row's unit list (00658B50 + 4, 005AAAF0) and its first
// element through 0059FB60; the copy is freed by the caller.
void* first_row_element(std::uint8_t* s, NativeContainer& copy, Big1Host& host) {
    void* const value = host.support_map_iterator_value_00658b50(s + 0x350);
    host.list_copy_005aaaf0(&copy, static_cast<std::uint8_t*>(value) + 4);
    void* iterator[2] = {&copy, at<void*>(copy.head, 0)};                    // [B+1Ch]
    return *host.list_iterator_value_0059fb60(iterator);
}

// 00675584..006758DA: the Enter on a map page (+34Ch >= 2), or the "-" name of an empty one.
void enter_map_arm(std::uint8_t* s, Big1Host& host) {
    if (at<std::int32_t>(s, 0x34C) == 0) {                                   // 006750C0
        host.gui_text_set_localised_cstring_00abbe50(ptr(s, 0x170), "-", 1);     // 006758B7, 00CEED40
        host.support_layout_name_label_0065bbc0(s);
        host.widget_vtable88(ptr(s, 0x1CC), 1, 0, 1.0f);                     // 006758DA
        return;
    }
    if (at<std::int32_t>(s, 0x34C) == 1) {
        enter_catalog_arm(s, host);
        return;
    }

    bool has_object = false;                                                // [B+13h]
    if (at<std::int32_t>(s, 0x34C) != 0) {                                   // 00675586
        check_iterator(s, 0x350, host);
        std::uint8_t* const row = static_cast<std::uint8_t*>(ptr(s, 0x354));
        void* const object = at<void*>(row, 8);                              // [B+5Ch]
        NativeContainer copy{};
        host.list_copy_005aaaf0(&copy, row + 0xC);                          // 006755C3
        has_object = object != nullptr;
        host.list_tidy_005a0b80(&copy);                                      // 006755F7
    }

    if (has_object) {
        check_iterator(s, 0x350, host);                                      // 00675603
        std::uint8_t* const row = static_cast<std::uint8_t*>(ptr(s, 0x354));
        void* const object = at<void*>(row, 8);
        NativeContainer copy{};
        host.list_copy_005aaaf0(&copy, row + 0xC);                          // 0067563E
        host.gui_text_set_localised_cstring_00abbe50(ptr(s, 0x170), host.object_vtable14_name(object), 1);  // 00675669
        host.list_tidy_005a0b80(&copy);                                      // 00675676
    } else {
        bool kind9 = false;                                                  // [B+13h]
        NativeContainer first_copy{};                                        // [B+4Ch], flag 2
        NativeContainer second_copy{};                                       // [B+40h], flag 4
        bool have_first = false;
        bool have_second = false;
        if (at<std::int32_t>(s, 0x34C) != 0) {                               // 00675680
            have_first = true;
            void* const element = first_row_element(s, first_copy, host);    // 00675696..006756C6
            if (ptr(element, 0x3C) != nullptr) {                             // 006756CD
                have_second = true;
                void* const second = first_row_element(s, second_copy, host);  // 006756D4..00675704
                kind9 = host.unit_has_flag_vtable5c(ptr(second, 0x3C), 9);   // 00675715
            }
        }
        if (have_second) host.list_tidy_005a0b80(&second_copy);              // 00675735
        if (have_first) host.list_tidy_005a0b80(&first_copy);                // 00675749

        if (kind9) {
            NativeContainer copy{};                                          // [B+4Ch]
            void* const element = first_row_element(s, copy, host);          // 0067575B..00675786
            host.gui_text_set_localised_cstring_00abbe50(
                ptr(s, 0x170), host.object_vtable14_name(ptr(element, 0x3C)), 1);  // 006757A0
            host.list_tidy_005a0b80(&copy);                                  // 006757AD
        } else if (at<std::int32_t>(s, 0x34C) != 0) {                        // 006757B7
            NativeContainer copy{};
            void* const element = first_row_element(s, copy, host);          // 006757CB..006757EE
            const bool has_class = at<std::uint32_t>(element, 0x15C) > 0;    // 00675800 SETA
            host.list_tidy_005a0b80(&copy);                                  // 00675803
            if (has_class) {
                NativeContainer copy2{};
                void* const element2 = first_row_element(s, copy2, host);    // 0067580E..00675839
                host.gui_text_set_localised_cstring_00abbe50(
                    ptr(s, 0x170), host.object_vtable14_name(element2), 1);      // 00675850
                host.list_tidy_005a0b80(&copy2);                             // 00675861
            } else {
                host.gui_text_set_localised_cstring_00abbe50(ptr(s, 0x170), "globals.unitclass_spawnpoint",
                                                             1);  // 00675888, 00CF6B24
            }
        } else {
            void* const unit = *host.support_unit_iterator_value_00658a60(s + 0x300);  // 00675877
            host.gui_text_set_localised_cstring_00abbe50(ptr(s, 0x170), host.object_vtable14_name(unit),
                                                         1);  // 0067588E
        }
    }
    host.support_layout_name_label_0065bbc0(s);                              // 00675895
    host.widget_vtable88(ptr(s, 0x1CC), 1, 0, 1.0f);                         // 006758DA
}

} // namespace

// 00674B80 __fastcall(ECX = support manager screen), RET; EBP frame aligned to 8, EH handler
// 00C7C5CA. The screen's Enter. Resets the panel (counters 00529A30, the powerup prompts, +0Ch =
// 2.0 in modes 4..7, the slot rebuilds), shows the "SM1STUSE" hint once through the award tracker,
// observes [00E188D8], resets the widgets and fields, clears list +3B8h inline and, on a map page,
// takes the selected row's +8h, unit list (00665D20 assign) and +18h. Then one of three arms sets
// the name text +170h and the icon +1CCh: a unit at +300h (when +1F4h is set and the unit is
// non-null with +5Dh clear), the catalog page (+34Ch == 1: list +370h rebuilt from every listed
// unit's 0084D560 supply classes, the device icon +158h), or a map page. The tail refreshes the
// selection (on the catalog page or +1F4h == 1) or writes the points text +1A8h, then shows
// +F0h/+F4h, +16Ch, and plays the open sound.
void hud_support_manager_screen_enter_00674b80(void* screen, Big1Host& host) {
    std::uint8_t* const s = static_cast<std::uint8_t*>(screen);
    host.hud_counters_relayout_00529a30(ptr(host.hud_00e198c4(), 0xC8));     // 00674BB5
    at<std::uint8_t>(s, 0xA) = 0;
    host.widget_vtable34(ptr(s, 0x178), 0);
    host.widget_vtable34(ptr(s, 0x1C4), 0);
    host.widget_vtable34(ptr(s, 0x1C8), 1);
    at<std::uint32_t>(s, 0x3F8) = 0;                                         // 00674BEC
    at<std::uint8_t>(s, 0x71) = 0;
    host.powerup_screen_refresh_prompt_widgets_00612da0(ptr(host.hud_00e198c4(), 0xC0));  // 00674C01
    const std::int32_t mode = host.game_effective_mode_004bca50(host.game_00e188a8());  // 00674C0C
    at<float>(s, 0xC) = (mode == 4 || mode == 5 || mode == 6 || mode == 7) ? 2.0f : 0.0f;  // [00CE3958]
    at<std::uint8_t>(s, 0x30) = host.global_00e188d8() == nullptr ? 1 : 0;   // 00674C3F
    at<std::uint8_t>(s, 0x2CC) = 1;
    at<std::uint8_t>(s, 0x2B1) = 1;
    host.support_layout_widget_1c0_006567d0(s);                              // 00674C53
    host.support_delete_slot_clones_00652a60(s);                             // 00674C5A
    host.support_rebind_slots_0065d220(s);                                   // 00674C61

    NativeStr hint{0, nullptr};                                              // [B+1Ch]
    host.native_string_resize_0041dd40(&hint, 8, 1);                         // 00674C75
    if (hint.data != nullptr) host.crt_memcpy_00bf7680(hint.data, "SM1STUSE", hint.length + 1);  // 00CF6B8C
    const bool shown = host.award_tracker_has_hint_been_shown_0068e600(
        host.award_tracker_get_singleton_004e1ca0(), &hint);                 // 00674C9B / 00674CA7
    release_str(hint, host);                                                 // 00674CC0..00674CD3
    if (!shown) {
        host.native_string_assign_0041e870(&hint, "SM1STUSE");               // 00674CE8
        host.award_tracker_trigger_hint_00690fd0(host.award_tracker_get_singleton_004e1ca0(), &hint, 0);  // 00674D01
        release_str(hint, host);                                             // 00674D0A..00674D23
    }

    void* const target = host.global_00e188d8();                             // 00674D28
    if (target != nullptr && ptr(s, 0x21C) == nullptr) host.support_set_tracked_unit_target_0065be80(s, target);
    at<std::uint8_t>(s, 0x1E4) = 1;                                          // 00674D47
    at<float>(s, 0x1E8) = 0.0f;
    host.widget_vtable34(ptr(s, 0x78), 1);
    host.widget_vtable34(ptr(s, 0x16C), 1);
    host.widget_vtable34(ptr(s, 0x190), 0);
    host.widget_vtable34(ptr(s, 0x1B0), 1);
    at<std::int32_t>(s, 0x5C) = 1;                                           // 00674D8A
    host.support_refresh_sites_0066fb00(s);                                  // 00674D8D
    at<float>(s, 0x1E0) = 0.35f;                                             // [00CF6560] = 3EB33333h
    at<std::uint32_t>(s, 0x3A4) = 0;
    at<std::uint32_t>(s, 0x2A0) = 0;
    at<std::uint8_t>(s, 0x1DC) = 1;
    at<std::uint8_t>(s, 0x1DD) = 1;
    at<std::uint32_t>(s, 0x308) = 0;
    at<std::uint32_t>(s, 0x30C) = 0;
    at<std::uint32_t>(s, 0x310) = 0;
    at<std::uint32_t>(s, 0x314) = 0;
    at<float>(s, 0x2B8) = 0.0f;
    at<float>(s, 0x2BC) = 0.0f;
    at<std::uint8_t>(s, 0x2C0) = 0;
    at<std::uint8_t>(s, 0x2C1) = 0;
    at<std::uint32_t>(s, 0x2C4) = 0;                                         // 00674DF5

    // Inlined clear of list +3B8h (00674DFB..00674E33); the head is re-read for each compare.
    void* node = at<void*>(ptr(s, 0x3BC), 0);
    at<void*>(ptr(s, 0x3BC), 0) = ptr(s, 0x3BC);
    at<void*>(ptr(s, 0x3BC), 4) = ptr(s, 0x3BC);
    at<std::uint32_t>(s, 0x3C0) = 0;
    while (node != ptr(s, 0x3BC)) {
        void* const next = at<void*>(node, 0);
        host.crt_free_00bf65ac(node);                                        // 00674E23
        node = next;
    }

    void* const icon = ptr(s, 0x158);                                        // 00674E35
    at<std::uint32_t>(s, 0x324) = 0;
    at<std::uint32_t>(s, 0x32C) = 0;
    at<std::uint32_t>(s, 0x334) = 0;
    host.widget_vtable88(icon, 0, 0, 1.0f);                                  // 00674E5D
    at<std::uint8_t>(s, 0x3A8) = 0;
    at<std::uint8_t>(s, 0x3DC) = 0;
    if (at<std::int32_t>(s, 0x34C) == 0) host.support_rebuild_00668770(s);   // 00674E77
    host.support_sync_cursor_iterators_0065a2b0(s);                          // 00674E7E
    if (at<std::int32_t>(s, 0x34C) != 0) {                                   // 00674E83
        check_iterator(s, 0x350, host);                                      // 00674E93 / 00674EA9
        std::uint8_t* const row = static_cast<std::uint8_t*>(ptr(s, 0x354));
        at<std::uint32_t>(s, 0x358) = at<std::uint32_t>(row, 8);             // 00674EBA
        std::uint8_t* const units = row + 0xC;
        if (s + 0x35C != units)                                              // 00674EC9
            host.support_list_assign_00665d20(s + 0x35C, units, at<void*>(ptr(units, 4), 0), units,
                                              ptr(units, 4), 0);             // 00674EE3 (tag [B+14h] = 0)
        at<std::uint32_t>(s, 0x368) = at<std::uint32_t>(row, 0x18);          // 00674EEE
    }

    bool unit_arm = false;
    if (at<std::int32_t>(s, 0x1F4) != 0) {                                   // 00674EFA
        check_iterator(s, 0x300, host);                                      // 00674F10 / 00674F1F
        if (at<void*>(ptr(s, 0x304), 8) != nullptr) {                        // 00674F27
            check_iterator(s, 0x300, host);                                  // 00674F34 / 00674F43
            unit_arm = at<std::uint8_t>(at<void*>(ptr(s, 0x304), 8), 0x5D) == 0;  // 00674F4E
        }
    }
    if (unit_arm)
        enter_unit_arm(s, host);
    else
        enter_map_arm(s, host);

    // 006758DC
    if (at<std::int32_t>(s, 0x34C) == 1 || at<std::int32_t>(s, 0x1F4) == 1) {
        host.support_refresh_selection_0066d020(s);                          // 00675BF8
        void* const selected = ptr(s, 0x2A0);
        if (selected != nullptr && host.object_vtable18(selected, 0xF)) {    // 00675C12
            host.widget_vtable34(ptr(s, 0xF0), 0);
            host.widget_vtable34(ptr(s, 0xF4), 1);
        } else {
            host.widget_vtable34(ptr(s, 0xF0), 1);                           // 006759F5
            host.widget_vtable34(ptr(s, 0xF4), 0);
        }
    } else {
        const std::int32_t mode2 = host.game_effective_mode_004bca50(host.game_00e188a8());  // 006758FC
        if (mode2 != 8) host.support_rebuild_lists_0066a8a0(s);              // 0067590A
        host.support_rebuild_00670020(s);                                    // 00675911
        if (at<std::int32_t>(s, 0x1F4) != 0 && mode2 == 8) {
            check_iterator(s, 0x300, host);                                  // 0067593B / 0067594A
            bool air = host.unit_has_flag_vtable5c(at<void*>(ptr(s, 0x304), 8), 9);  // 0067595C
            if (!air)
                air = host.unit_has_flag_vtable5c(*host.support_unit_iterator_value_00658a60(s + 0x300),
                                                  0x45);  // 00675976
            if (!air) {
                NativeStr prefix{};                                          // [B+14h]
                host.native_string_assign_0041e870(&prefix, "ingame.sm_slots|.: ");  // 00675989, 00CF6B10
                const std::int32_t idle = host.shipyard_count_idle_slots_008434d0(
                    *host.support_unit_iterator_value_00658a60(s + 0x300));  // 0067599B / 006759A2
                NativeStr text{};                                            // [B+1Ch]
                host.native_string_concat_int_004263b0(&prefix, &text, idle);  // 006759AF
                host.gui_text_set_localised_source_00abaed0(ptr(s, 0x1A8), &text, 1);  // 006759C5
                host.native_string_destroy_0041dd20(&text);                  // 006759D2
                host.native_string_destroy_0041dd20(&prefix);                // 006759E3
            } else {
                const bool carrier =
                    host.unit_has_flag_vtable5c(*host.support_unit_iterator_value_00658a60(s + 0x300),
                                                0x45);  // 00675A65
                void* const unit = *host.support_unit_iterator_value_00658a60(s + 0x300);  // 00675A6D / 00675A80
                void* const block = unit != nullptr ? static_cast<std::uint8_t*>(unit) + (carrier ? 0x72C : 0x1188)
                                                    : nullptr;
                NativeStr prefix{};                                          // [B+14h]
                host.native_string_assign_0041e870(&prefix, "ingame.sm_slots|.: ");  // 00675A9E
                const std::int32_t idle = host.air_ops_count_idle_slots_006bdc30(block);  // 00675AB0
                NativeStr text{};                                            // [B+1Ch]
                host.native_string_concat_int_004263b0(&prefix, &text, idle);  // 00675ABD
                host.gui_text_set_localised_source_00abaed0(ptr(s, 0x1A8), &text, 1);  // 00675AD3
                release_str(text, host);                                     // 00675AD8..00675AF6
                release_str(prefix, host);                                   // 00675AFB..00675BEC
            }
        } else {
            void* const game = host.game_00e188a8();
            const std::int32_t points =
                // 00675B24 / 00675B29
                ftol2_low_00bf7420(host.player_support_points_00946f00(at<std::uint32_t>(game, 0x18EC)));
            NativeStr number{};                                              // [B+34h]
            host.native_string_from_int_004260b0(&number, points);           // 00675B33
            NativeStr prefix{};                                              // [B+2Ch]
            host.native_string_assign_0041e870(&prefix, "ingame.sm_cp|.: "); // 00675B4E, 00CF69CC
            NativeStr text{};                                                // [B+24h]
            host.native_string_concat_004261a0(&prefix, &text, &number);     // 00675B64
            host.gui_text_set_localised_source_00abaed0(ptr(s, 0x1A8), &text, 1);  // 00675B7A
            release_str(text, host);                                         // 00675B7F..00675B9D
            release_str(prefix, host);                                       // 00675BA2..00675BC1
            release_str(number, host);                                       // 00675BC6..00675BEC
        }
        host.widget_vtable34(ptr(s, 0xF0), 1);                               // 006759F5
        host.widget_vtable34(ptr(s, 0xF4), 0);                               // 00675A04
    }
    host.widget_vtable34(ptr(s, 0x16C), 1);                                  // 00675A13
    if (at<std::uint8_t>(ptr(host.hud_00e198c4(), 0x54), 4) != 0) at<std::uint8_t>(s, 0x3DC) = 1;  // 00675A1E
    host.support_play_open_sound_0065f950(s);                                // 00675A2D
    host.hud_counters_hide_under_manager_panel_00528f70(ptr(host.hud_00e198c4(), 0xC8));  // 00675A3D
}

namespace {

// The inlined NativeString assignment dst = *src (dst != src): 0041DD40(dst; src.length, 1),
// then, when src is non-empty, memcpy(dst.data, src.data, dst.length) (no terminator copied).
void assign_str(NativeStr& dst, const void* src, Big1Host& host) {
    if (static_cast<const void*>(&dst) == src) return;
    host.native_string_resize_0041dd40(&dst, at<std::uint32_t>(src, 0), 1);
    if (at<std::uint32_t>(src, 0) != 0) host.crt_memcpy_00bf7680(dst.data, at<char*>(src, 4), dst.length);
}

// The inlined by-value ref-pointer release: InterlockedDecrement(+4h), vtable +0h at zero.
void ref_release(void* object, Big1Host& host) {
    if (host.interlocked_decrement_00ce2220(&at<std::int32_t>(object, 4)) == 0)
        host.refcounted_vtable0_release(object);
}

// 0066EC0F..0066F2EB, eight times: a {0, 0} path resized to its length and filled (memcpy of
// length + 1), 00A83FD0([00F8BBE8]; &ref, &path), then the inlined ref-pointer assignment of
// the result into screen+field (unchanged -> nothing; else store, InterlockedIncrement on the
// new one, release of the old one), the temporary ref released and zeroed, the path freed.
void acquire_sample(std::uint8_t* s, std::size_t field, const char* name, std::uint32_t length,
                    Big1Host& host) {
    NativeStr path{0, nullptr};                                             // [B+20h]
    host.native_string_resize_0041dd40(&path, length, 1);
    if (path.data != nullptr) host.crt_memcpy_00bf7680(path.data, name, path.length + 1);
    void* sample;                                                           // [B+10h], written by 00A83FD0
    void* const value = *host.sound_sample_cache_acquire_00a83fd0(host.global_00f8bbe8(), &sample, &path);
    void* const old = ptr(s, field);
    if (old != value) {
        at<void*>(s, field) = value;
        if (value != nullptr) host.interlocked_increment_00ce221c(&at<std::int32_t>(value, 4));
        if (old != nullptr) ref_release(old, host);
    }
    if (sample != nullptr) {
        ref_release(sample, host);
        sample = nullptr;
    }
    release_str(path, host);
}

} // namespace

// 0066EA60 __fastcall(ECX = support manager screen), RET; EBP frame aligned to 8, EH handler
// 00C7BE50. Vtable +10h of HUD screen slot 4Eh. 004F71D0, the tuning floats, page
// "GUI_support_full" (00CF4D4C) into +78h, the layout virtual (vtable +14h), the widget state
// resets and copies, the eight environment samples into +10h..+2Ch, more resets, then the radar
// shot: the mission name ([[00E188A8]+5FCh]+90Ch) after its last '/' or '\', without its last
// four characters, cut to 20 characters, the mode suffix (jump table 0066FAE0: _ICS, _ICM, _ICL,
// _ICH, _D, _E, _S, _C for modes 0..7; none for 8 and 9; "_xxx" otherwise), and
// "interface/textures/gui/radarshots/" + name + ".dds" on widget +1C0h. Its size and UV rect
// follow from the map bounds [00E188A8]+711Ch..+7130h (copied to +460h..+474h) over a
// 30000-unit square centred on 0. Last: tree +408h cleared, iterator +300h = begin of list +1ECh,
// the observer at +208h released, 0066DF30(+338h, +2D0h).
void hud_support_manager_screen_register_0066ea60(void* screen, Big1Host& host) {
    std::uint8_t* const s = static_cast<std::uint8_t*>(screen);
    at<std::uint8_t>(s, 0xA) = 0;
    host.frontend_screen_register_004f71d0(s);                               // 0066EA89
    at<float>(s, 0x2A8) = 0.628f;                                            // [00CF6B04] = 3F20C49Ch
    at<float>(s, 0x2A4) = 1.57f;                                             // [00CF0B40] = 3FC8F5C3h
    at<float>(s, 0x290) = 0.0275f;                                           // [00CF6B00] = 3CE147AEh
    at<std::uint8_t>(s, 0x70) = 0;
    at<std::uint8_t>(s, 0x66) = 0;
    at<float>(s, 0x294) = 0.05f;                                             // [00CE7638] = 3D4CCCCDh
    at<std::uint8_t>(s, 0x2B0) = 0;

    NativeStr page{0, nullptr};                                              // [B+20h]
    host.native_string_resize_0041dd40(&page, 0x10, 1);                      // 0066EAEE
    if (page.data != nullptr) host.crt_memcpy_00bf7680(page.data, "GUI_support_full", page.length + 1);
    void* const manager = host.gui_manager_get_or_create_004c12b0();          // 0066EB14
    at<void*>(s, 0x78) = host.gui_manager_load_page_00aa5840(manager, &page, 1, 0);  // 0066EB22
    release_str(page, host);                                                 // 0066EB2A..0066EB4B

    host.screen_vtable14_layout(s);                                          // 0066EB57
    host.widget_vtable34(ptr(s, 0x1C4), 0);                                  // 0066EB65
    const float full_uv[4] = {0.0f, 0.0f, 1.0f, 1.0f};                       // [B+38h], 1.0 = [00D7A24C]
    host.gui_icon_add_state_00ab64c0(ptr(s, 0x158), nullptr, full_uv);       // 0066EB96
    host.widget_vtable34(ptr(s, 0x170), 0);                                  // 0066EBA7
    const float* const size_18c = host.gui_widget_get_size_00aa6740(ptr(s, 0x18C));  // 0066EBAF
    at<float>(s, 0x194) = size_18c[0];                                       // FLD/FSTP float copies
    at<float>(s, 0x198) = size_18c[1];
    float position[3];                                                       // [B+38h]
    const float* const pos_170 = host.gui_widget_get_resolved_position_00aa6750(ptr(s, 0x170), position);  // 0066EBD0
    at<float>(s, 0x19C) = pos_170[0];
    at<float>(s, 0x1A0) = pos_170[1];
    at<float>(s, 0x1A4) = pos_170[2];
    at<float>(s, 0x1AC) = host.gui_widget_get_size_00aa6740(ptr(s, 0x184))[0];  // 0066EBF5 (MOVSS)
    at<std::uint32_t>(s, 0x45C) = 0;
    at<std::int32_t>(s, 0x2B4) = 3;                                          // 0066EC1B (cc9: launch group)
    at<std::uint8_t>(s, 0x1E4) = 0;
    at<float>(s, 0x1E8) = 0.0f;
    at<std::uint8_t>(s, 0x30) = 0;
    at<std::int32_t>(s, 0x5C) = 1;
    at<std::int32_t>(s, 0x60) = 1;
    at<std::uint8_t>(s, 0x288) = 0;
    at<float>(s, 0x28C) = 0.0f;
    at<std::uint32_t>(s, 0x308) = 0;
    at<std::uint32_t>(s, 0x30C) = 0;
    at<std::uint32_t>(s, 0x310) = 0;
    at<std::uint32_t>(s, 0x314) = 0;

    acquire_sample(s, 0x10, "environment/environment_unavailable.fsb", 0x27, host);  // 00CF5E68
    acquire_sample(s, 0x14, "environment/sm_sp_to_unit.fsb", 0x1D, host);            // 00CF6AE0
    acquire_sample(s, 0x18, "environment/sm_change_weap.fsb", 0x1E, host);           // 00CF6AC0
    acquire_sample(s, 0x20, "environment/sm_launch_unit.fsb", 0x1E, host);           // 00CF6AA0
    acquire_sample(s, 0x1C, "environment/sm_unit_to_unit.fsb", 0x1F, host);          // 00CF6A80
    acquire_sample(s, 0x24, "environment/sm_left_right.fsb", 0x1D, host);            // 00CF6A60
    acquire_sample(s, 0x28, "environment/sm_open.fsb", 0x17, host);                  // 00CF6A48
    acquire_sample(s, 0x2C, "environment/sm_close.fsb", 0x18, host);                 // 00CF6A2C

    at<std::uint32_t>(s, 0x2C8) = 0;                                         // 0066F2F3
    at<std::uint8_t>(s, 0x154) = 0;
    at<std::uint8_t>(s, 0x3D5) = 0;
    at<std::uint8_t>(s, 0x2DC) = 0;
    at<std::uint8_t>(s, 0x2DD) = 0;
    at<float>(s, 0x2E0) = 0.0f;
    at<std::uint32_t>(s, 0x438) = 0;
    at<std::uint32_t>(s, 0x448) = 0;
    at<std::uint32_t>(s, 0x43C) = 0;
    at<std::uint32_t>(s, 0x44C) = 0;
    at<std::uint32_t>(s, 0x440) = 0;
    at<std::uint32_t>(s, 0x450) = 0;
    at<std::uint32_t>(s, 0x444) = 0;
    at<std::uint32_t>(s, 0x454) = 0;
    at<std::uint8_t>(s, 0x2D4) = 0;
    at<std::uint32_t>(s, 0x2D8) = 0;

    // The radar-shot name.
    NativeStr name{0, nullptr};                                              // [B+20h]
    assign_str(name, static_cast<std::uint8_t*>(ptr(host.game_00e188a8(), 0x5FC)) + 0x90C, host);  // 0066F355..0066F39C
    std::int32_t slash = host.native_string_rfind_any_004bcb80(&name, "/", 0x7FFFFFFF);  // 0066F3B5, 00CE7898
    if (slash == -1) slash = host.native_string_rfind_any_004bcb80(&name, "\\", 0x7FFFFFFF);  // 0066F3CD, 00CE7894
    if (slash != -1) {
        NativeStr part;                                                      // [B+38h]
        const void* const r = host.native_string_substring_00469840(
            &name, &part, static_cast<std::uint32_t>(slash) + 1,
            name.length - static_cast<std::uint32_t>(slash) - 5);            // 0066F3EC
        assign_str(name, r, host);                                           // 0066F3F3..0066F421
        release_str(part, host);                                             // 0066F424..0066F443
    }
    {
        NativeStr part;                                                      // [B+38h]
        const void* const r = host.native_string_substring_00469840(
            &name, &part, 0, name.length > 0x14 ? 0x14 : name.length);       // 0066F459 / 0066F498 (JBE)
        assign_str(name, r, host);                                           // 0066F45E..0066F4CD
        release_str(part, host);                                             // 0066F4D0..0066F4EF
    }
    const std::int32_t mode = host.game_effective_mode_004bca50(host.game_00e188a8());  // 0066F4FA
    if (mode != 8 && mode != 9) {
        if (static_cast<std::uint32_t>(mode) <= 7) {                         // 0066F514 JA, table 0066FAE0
            static const char* const kSuffix[8] = {"_ICS", "_ICM", "_ICL", "_ICH",  // 00CF1324 .. 00CF130C
                                                   "_D",   "_E",   "_S",   "_C"};   // 00CF1308 .. 00CF12FC
            NativeStr suffix;                                                // [B+18h]
            host.native_string_assign_0041e870(&suffix, kSuffix[mode]);
            host.native_string_append_00425e10(&name, &suffix);              // 0066F53D
            release_str(suffix, host);                                       // 0066F542..0066F67D
        } else {
            // 0066F61B: "_xxx" (00CF12F4), appended inline.
            NativeStr suffix;                                                // [B+38h]
            host.native_string_assign_0041e870(&suffix, "_xxx");
            const std::uint32_t n = suffix.length;                           // EDI
            char* const data = suffix.data;                                  // EBP
            if (n != 0) {
                const std::uint32_t old = name.length;
                host.native_string_resize_0041dd40(&name, old + n, 1);       // 0066F64C
                host.crt_memcpy_00bf7680(name.data + old, data, n);          // 0066F65E
            }
            if (data != nullptr)
                host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), data, n + 1, 1);  // 0066F676
        }
    }

    NativeStr path{0, nullptr};                                              // [B+28h]
    NativeStr ext{0, nullptr};                                               // [B+18h]
    host.native_string_resize_0041dd40(&ext, 4, 1);                          // 0066F69F
    char* const ext_data = ext.data;                                         // EBP
    if (ext_data != nullptr) host.crt_memcpy_00bf7680(ext_data, ".dds", ext.length + 1);  // 00CE9BC8
    NativeStr prefixed;                                                      // [B+10h]
    void* const p = host.native_string_prefixed_0043c130(&prefixed, "interface/textures/gui/radarshots/",
                                                         &name);             // 0066F6D5, 00CF12D0
    NativeStr joined;                                                        // [B+38h]
    const void* const j = host.native_string_concat_004261a0(p, &joined, &ext);  // 0066F6EB
    assign_str(path, j, host);                                               // 0066F6F0..0066F724
    release_str(joined, host);                                               // 0066F727..0066F746
    release_str(prefixed, host);                                             // 0066F74B..0066F76A
    if (ext_data != nullptr)
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), ext_data, ext.length + 1, 1);  // 0066F783
    host.gui_set_state_texture_00ab44b0(ptr(s, 0x1C0), 0, &path, full_uv);   // 0066F7C3, uv [B+48h]

    // 0066F7C8: the map bounds, then the radar shot's size and UV rect. Modelled at 24 bits (every
    // x87 result rounded to float, in the listing's order); the DC/DE register forms were read
    // from the bytes (tools/x87_dc_form_scan.py and the opcode bytes).
    for (std::size_t i = 0; i < 3; ++i)
        at<float>(s, 0x460 + 4 * i) = at<float>(host.game_00e188a8(), 0x711C + 4 * i);
    for (std::size_t i = 0; i < 3; ++i)
        at<float>(s, 0x46C + 4 * i) = at<float>(host.game_00e188a8(), 0x7128 + 4 * i);
    const float kSpan = 30000.0f;                                            // [00CEF1C8] (double)
    const float kLow = -15000.0f;                                            // [00CEF1D0] (double)
    const float* const shot_size = host.gui_widget_get_size_00aa6740(ptr(s, 0x1C0));  // 0066F81E
    const float a = (at<float>(s, 0x46C) - at<float>(s, 0x460)) / kSpan;     // DC F9 at 0066F83F
    const float size_x = shot_size[0];                                       // [B+38h]
    const float size_y = shot_size[1];                                       // [B+3Ch]
    const float b = (at<float>(s, 0x468) - at<float>(s, 0x474)) / kSpan;     // DE F1 at 0066F85E
    const float larger = a > b ? a : b;                                      // FCOMI/JBE: a NaN takes b
    const float r = 1.0f / larger;                                           // DE F1 at 0066F890
    const float scaled[2] = {(a * r) * size_x, (b * r) * size_y};            // [B+30h]
    host.widget_vtable58_set_size(ptr(s, 0x1C0), scaled);                    // 0066F8CC
    void* const shot = ptr(s, 0x1C0);                                        // 0066F8DA
    const float v0 = ((at<float>(s, 0x468) - kLow) / kSpan) * -1.0f + 1.0f;  // [B+3Ch]
    const float v1 = ((at<float>(s, 0x474) - kLow) / kSpan) * -1.0f + 1.0f;  // [B+44h]
    const float u0 = ((at<float>(s, 0x460) - kLow) / kSpan) * 1.0f + 0.0f;   // [B+38h]
    at<float>(shot, 0x118) = u0;                                             // 0066F94A
    const float h = (at<float>(s, 0x46C) - kLow) / kSpan;                    // DE E3 / DE F3
    at<float>(shot, 0x11C) = v0;                                             // 0066F952
    const float u1 = 0.0f + h * 1.0f;                                        // FMUL, then FADDP onto the 0
    at<float>(shot, 0x120) = u1;                                             // 0066F974
    at<float>(shot, 0x124) = v1;                                             // 0066F97C
    host.widget_vtable8c(shot);                                              // 0066F988

    host.widget_vtable34(ptr(s, 0x1D8), 0);                                  // 0066F996
    host.widget_vtable88(ptr(s, 0x1D8), 1, 0, 1.0f);                         // 0066F9AF
    at<std::uint8_t>(s, 0x4) = 0;
    at<std::uint8_t>(s, 0x8) = 0;
    std::uint8_t* const tree = s + 0x408;
    host.support_tree_erase_0066cc80(tree, at<void*>(ptr(tree, 4), 4));      // 0066F9C9
    at<void*>(ptr(tree, 4), 4) = ptr(tree, 4);
    at<std::uint32_t>(tree, 8) = 0;
    at<void*>(ptr(tree, 4), 0) = ptr(tree, 4);
    at<void*>(ptr(tree, 4), 8) = ptr(tree, 4);
    at<std::uint8_t>(s, 0x9) = 0;
    void* const first = at<void*>(ptr(s, 0x1F0), 0);
    at<void*>(s, 0x300) = s + 0x1EC;                                         // 0066F9F9
    at<void*>(s, 0x304) = first;
    at<std::uint32_t>(s, 0x318) = 0;
    at<std::uint32_t>(s, 0x320) = 0;
    at<std::uint32_t>(s, 0x324) = 0;
    at<std::uint32_t>(s, 0x32C) = 0;
    at<std::uint32_t>(s, 0x330) = 0;
    at<std::uint32_t>(s, 0x334) = 0;
    if (ptr(s, 0x21C) != nullptr) {                                          // 0066FA2C
        host.observer_unregister_pair_006952a0(ptr(s, 0x21C), s + 0x208);    // 0066FA32
        at<void*>(s, 0x21C) = nullptr;
    }
    at<std::uint32_t>(s, 0x298) = 0;
    at<float>(s, 0x3D0) = 0.0f;
    at<std::uint8_t>(s, 0x3D4) = 0;
    at<std::uint8_t>(s, 0x204) = 0;
    at<std::uint32_t>(s, 0x3D8) = 0;
    at<std::uint8_t>(s, 0x3DC) = 1;
    host.support_sites_build_with_icons_0066df30(s + 0x338, &at<float>(s, 0x2D0));  // 0066FA70
    at<std::uint8_t>(s, 0x479) = 0;
    at<std::int32_t>(s, 0x47C) = -1;
    release_str(path, host);                                                 // 0066FA75..0066FAA3
    release_str(name, host);                                                 // 0066FAA8..0066FAC6
}

// 005E12A0 __thiscall(ECX = lobby screen), RET; EBP frame aligned to 8, EH handler 00C74D80.
// A private LuaStateOwner ([B+E8h], at most 4CCh bytes before the EH record) is opened with 4 and
// runs "SCRIPTS\datatables\Weather.lua" (00CE59DC, a 1Eh-byte NativeString released after the
// run). For i = 1.. while Weathers[i] is not nil (00CE59D0): when entry.UniqueID (00CF240C) is
// not nil, a 24h-byte entry with its two vectors' first/last/end zeroed (+8..+10h, +18h..+20h;
// +0, +4 and +14h are not written) is pushed onto screen+2E0h (005E11F0) and destroyed
// (005DACA0); the last element (checked subscript size-1, 00BF6713 on a null first or an out
// of range index; the store follows the check) gets +0 = the id. Then for j = 1.. while
// SubScenes[j] is not nil: SubScenes[j].ID appended to element+4 and .Text to element+14h. Each
// probe of t[i] is a temporary that is destroyed before the loop test. The owner is closed
// with 00B669A0.
void multi_game_lobby_load_weather_table_005e12a0(void* screen, Big1Host& host) {
    alignas(8) std::uint8_t owner[0x4CC];          // [B+E8h]
    alignas(4) std::uint8_t globals[0x14];         // [B+C0h]
    alignas(4) std::uint8_t weathers[0x14];        // [B+98h]
    alignas(4) std::uint8_t probe[0x14];           // [B+38h]
    alignas(4) std::uint8_t entry[0x14];           // [B+84h]
    alignas(4) std::uint8_t unique_id[0x14];       // [B+70h]
    alignas(4) std::uint8_t sub_scenes[0x14];      // [B+5Ch]
    alignas(4) std::uint8_t sub_scene[0x14];       // [B+ACh]
    alignas(4) std::uint8_t id[0x14];              // [B+38h] (reuses the probe slot)
    alignas(4) std::uint8_t text[0x14];            // [B+D4h]
    alignas(4) std::uint8_t weather[0x24];         // [B+38h] (the pushed entry)

    host.lua_state_owner_construct_00b66bd0(owner);                            // 005E12D0
    host.lua_state_owner_open_00b6a020(owner, 4);                              // 005E12E7
    NativeStr path{0, nullptr};                                                // [B+14h]
    host.native_string_resize_0041dd40(&path, 0x1E, 1);                        // 005E12FC
    if (path.data != nullptr)
        host.crt_memcpy_00bf7680(path.data, "SCRIPTS\\datatables\\Weather.lua", path.length + 1);  // 005E1317
    host.lua_state_owner_run_script_00b69d40(owner, &path, 0);                 // 005E1334
    if (path.data != nullptr)
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), path.data, path.length + 1,
                                              1);                              // 005E135B
    void* const g = host.lua_state_owner_get_globals_00b67980(owner, globals);  // 005E136F
    host.lua_object_get_by_name_00b67800(g, weathers, "Weathers");             // 005E138B
    host.lua_object_destruct_00b67700(globals);                                // 005E139F

    std::int32_t i = 1;                                                        // [B+24h]
    bool more = !host.lua_object_is_nil_00b65fb0(
        host.lua_object_get_by_index_00b67720(weathers, probe, i));            // 005E13BA / 005E13C9
    host.lua_object_destruct_00b67700(probe);                                  // 005E13DF
    while (more) {
        host.lua_object_get_by_index_00b67720(weathers, entry, i);             // 005E13FC
        host.lua_object_get_by_name_00b67800(entry, unique_id, "UniqueID");    // 005E141A
        if (!host.lua_object_is_nil_00b65fb0(unique_id)) {                     // 005E142B
            const std::int32_t value = host.lua_object_get_integer_00b66290(unique_id);  // 005E143C
            at<std::uint32_t>(weather, 0x08) = 0;                              // 005E1443..005E1457
            at<std::uint32_t>(weather, 0x0C) = 0;
            at<std::uint32_t>(weather, 0x10) = 0;
            at<std::uint32_t>(weather, 0x18) = 0;
            at<std::uint32_t>(weather, 0x1C) = 0;
            at<std::uint32_t>(weather, 0x20) = 0;
            std::uint8_t* const vector = static_cast<std::uint8_t*>(screen) + 0x2E0;
            host.weather_entry_vector_push_back_005e11f0(vector, weather);     // 005E1474
            host.lobby_map_entry_destruct_005daca0(weather);                   // 005E1485
            std::uint8_t* const first = at<std::uint8_t*>(vector, 0x4);
            const std::int32_t size =
                first == nullptr ? 0 : static_cast<std::int32_t>(at<std::uint8_t*>(vector, 0x8) - first) / 0x24;
            const std::uint32_t last = static_cast<std::uint32_t>(size - 1);  // 005E14AB
            if (at<std::uint8_t*>(vector, 0x4) == nullptr ||
                last >= static_cast<std::uint32_t>(
                            (at<std::uint8_t*>(vector, 0x8) - at<std::uint8_t*>(vector, 0x4)) / 0x24))
                host.crt_invalid_parameter_noinfo_00bf6713();                  // 005E14CF
            std::uint8_t* const element = at<std::uint8_t*>(vector, 0x4) + last * 0x24;  // 005E14DA
            at<std::int32_t>(element, 0) = value;                              // 005E14EE
            host.lua_object_get_by_name_00b67800(entry, sub_scenes, "SubScenes");  // 005E14F0 (00CE59B8)
            if (!host.lua_object_is_nil_00b65fb0(sub_scenes)) {                // 005E1501
                std::int32_t j = 1;                                            // EBP
                bool sub_more = !host.lua_object_is_nil_00b65fb0(
                    host.lua_object_get_by_index_00b67720(sub_scenes, probe, j));  // 005E151D / 005E152C
                host.lua_object_destruct_00b67700(probe);                      // 005E1542
                while (sub_more) {
                    host.lua_object_get_by_index_00b67720(sub_scenes, sub_scene, j);  // 005E156D
                    append_lua_string(element + 0x04,
                                      host.lua_object_get_string_00b662b0(
                                          host.lua_object_get_by_name_00b67800(sub_scene, id, "ID")),
                                      host);                                   // 005E158B..005E1611 (00CE59B4)
                    host.lua_object_destruct_00b67700(id);                     // 005E1622
                    append_lua_string(element + 0x14,
                                      host.lua_object_get_string_00b662b0(
                                          host.lua_object_get_by_name_00b67800(sub_scene, text, "Text")),
                                      host);                                   // 005E163B..005E16C1 (00CEC778)
                    host.lua_object_destruct_00b67700(text);                   // 005E16D5
                    host.lua_object_destruct_00b67700(sub_scene);              // 005E16E9
                    ++j;
                    sub_more = !host.lua_object_is_nil_00b65fb0(
                        host.lua_object_get_by_index_00b67720(sub_scenes, probe, j));  // 005E16FB / 005E170A
                    host.lua_object_destruct_00b67700(probe);                  // 005E1720
                }
            }
            host.lua_object_destruct_00b67700(sub_scenes);                     // 005E173B
        }
        host.lua_object_destruct_00b67700(unique_id);                          // 005E174C / 005E1773
        host.lua_object_destruct_00b67700(entry);                              // 005E1760 / 005E1787
        ++i;                                                                   // 005E178C
        more = !host.lua_object_is_nil_00b65fb0(
            host.lua_object_get_by_index_00b67720(weathers, probe, i));        // 005E17A0 / 005E17AF
        host.lua_object_destruct_00b67700(probe);                              // 005E17C5
    }
    host.lua_object_destruct_00b67700(weathers);                               // 005E17E1
    host.lua_state_owner_close_00b669a0(owner);                                // 005E17F8
}

namespace {

// 00641DF8..00641E06: a native string with data is returned to the sized pool (length + 1, 1).
void release_native_string(const Cc13SweepBNativeString& s, Cc13Pkt642Host& host) {
    if (s.data != nullptr)
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), s.data, s.length + 1, 1);
}

// [[game + 18CCh + [game+18ECh]*4] + 28h], the party table 00803DC0 takes in ECX.
void* current_party_table(Cc13Pkt642Host& host) {
    void* const game = host.global_00e188a8();
    return at<void*>(at<void*>(game, 0x18CC + at<std::int32_t>(game, 0x18EC) * 4), 0x28);
}

constexpr std::uint32_t kEmptyCString = 0x00CE3A0C;

} // namespace

// 00641910 __thiscall(ECX = markers; unit, is_self, is_friendly, is_target) RET 10h; EH handler
// 00C7A3AB (state 0 / 1 own the distance string, 2 the colour temporary). Frame base B = entry
// ESP - B0h; arguments at B+B4h..B+C0h. Only the low bytes of the three flags are tested, except
// that 0063D1E0 receives is_self whole as its flag.
//   Live check as in 00642040; record = 006407A0(markers, 0) ([B+30h]). The clip point is the
// unit position with y = float([class+A8h] + y) (class = unit+538h); for an IsKindOf(8) unit
// under 00852860 it is 0 when the [game+1ED4h] entity's refreshed +100h is > 0 (COMISS/JBE: NaN
// keeps it).
//   On screen (00638E50 true): 0063A6C0 bounds from the raw unit position; record+5 = collapsed;
// corners (min,min) (min,max) (max,max) (max,min) then the clip result; colour index 00639990;
// record+4 = !is_self unless is_target, and it is left unwritten when is_friendly and is_target
// are both set; distance 0063E5C0; colour 0077A2F0(1.0f); text "" (00CE3A0C) unless is_self or
// is_target, then 00803DC0; show = markers+1Ch set and unit vtable[140h]() equal to its
// vtable[140h](); 0063C620(record; corners, index, show, 0, text, unit, distance copy,
// float(00923BE0(unit)), colour, 0).
//   Off screen: the raw position (y = 0 under the same test, this time through 00427EB0 of the
// [game+1ED4h] entity), 00638E50 (result unused), 00638C60 for x, y and angle, index 00639990,
// distance, text by is_self alone, then 0063D1E0(record; unit, x, y, angle, index, colour, text,
// distance copy, float(00923BE0(unit)), is_self). The colour 0077A2F0 call comes after the
// distance copy and 00923BE0 on this arm.
void hud_markers_add_kind1c_marker_00641910(void* markers, void* unit, std::uint32_t is_self,
                                            std::uint32_t is_friendly, std::uint32_t is_target,
                                            Cc13Pkt642Host& host) {
    const auto low = [](std::uint32_t v) { return (v & 0xFFu) != 0; };
    if (unit == nullptr || at<std::uint8_t>(unit, 0x5C) == 0 || at<std::uint8_t>(unit, 0x5D) != 0 ||
        at<std::uint8_t>(unit, 0x60) != 0 || at<std::uint8_t>(unit, 0x5E) != 0)
        return;                                                                // 0064193A..00641962
    void* const record = host.hud_markers_acquire_record_006407a0(markers, 0);  // 0064196A
    if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);  // 00641980
    float clip_point[3] = {at<float>(unit, 0xFC), 0.0f, at<float>(unit, 0x104)};  // [B+24h]
    clip_point[1] = at<float>(ptr(unit, 0x538), 0xA8) + at<float>(unit, 0x100);  // 006419B7
    if (host.entity_vcall_5c_is_kind(unit, 8) &&
        host.entity_is_below_altitude_third_00852860(unit)) {                  // 006419C8 / 006419D0
        void* const player = ptr(host.global_00e188a8(), 0x1ED4);
        if (at<std::uint8_t>(player, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(player);
        if (at<float>(player, 0x100) > 0.0f) clip_point[1] = 0.0f;             // 006419FF COMISS / JBE
    }
    float screen[3];                                                           // [B+7Ch]
    Cc13SweepBNativeString distance{0, nullptr};
    if (host.hud_marker_project_and_clip_00638e50(clip_point, screen)) {       // 00641A12
        if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);  // 00641A45
        std::uint8_t collapsed;                                                // [B+13h]
        float min_x, min_y, max_x, max_y;                                      // [B+34h] [B+18h] [B+14h] [B+20h]
        host.hud_marker_project_unit_bounds_0063a6c0(markers, at<float>(unit, 0xFC), at<float>(unit, 0x100),
                                                     at<float>(unit, 0x104), unit, &collapsed, &min_x,
                                                     &min_y, &max_x, &max_y);  // 00641AB3
        float corners[10] = {min_x, min_y, min_x, max_y, max_x, max_y, max_x, min_y,
                             screen[0], screen[1]};                            // [B+54h]..[B+78h]
        at<std::uint8_t>(record, 5) = collapsed;                               // 00641B07
        const std::int32_t index = host.hud_markers_colour_index_00639990(unit);  // 00641B0A
        if (!(low(is_friendly) && low(is_target)))                             // 00641B1B..00641B31
            at<std::uint8_t>(record, 4) = (!low(is_target) && !low(is_self)) ? 1 : 0;  // 00641B43
        host.hud_marker_format_distance_0063e5c0(&distance, unit);             // 00641B5F
        float colour_out[4];                                                   // [B+38h]
        host.unit_health_colour_0077a2f0(colour_out, 1.0f);                    // 00641B6E (EAX unused)
        const char* text = static_cast<const char*>(host.image_global(kEmptyCString));
        if (low(is_self) || low(is_target))
            text = host.recon_display_name_for_party_00803dc0(current_party_table(host), unit);  // 00641BA7
        std::uint32_t show = 0;
        if (ptr(markers, 0x1C) != nullptr) {                                   // 00641BAE
            void* const mine = host.unit_vcall_140(unit);                      // 00641BC1
            void* const theirs = host.unit_vcall_140(ptr(markers, 0x1C));      // 00641BCF
            show = mine == theirs ? 1 : 0;
        }
        Cc13Pkt642Colour colour;
        for (int i = 0; i < 4; ++i) colour.rgba[i] = colour_out[i];            // 00641BDB..00641BFC
        const float scale = static_cast<float>(host.unit_health_00923be0(unit));  // 00641BFF
        Cc13SweepBNativeString copy;
        host.native_string_copy_construct_00426060(&copy, &distance);          // 00641C16
        cc13_pkt642_hud_marker_emit_box_marker_0063c620(record, corners, index, show, 0, text, unit, copy,
                                                        scale, colour, 0, host);  // 00641C31
    } else {
        if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);  // 00641C61
        float world[3] = {at<float>(unit, 0xFC), at<float>(unit, 0x100), at<float>(unit, 0x104)};  // [B+48h]
        if (host.entity_vcall_5c_is_kind(unit, 8) &&
            host.entity_is_below_altitude_third_00852860(unit)) {              // 00641C99 / 00641CA1
            const float* const p = host.entity_world_position_00427eb0(ptr(host.global_00e188a8(), 0x1ED4));
            if (p[1] > 0.0f) world[1] = 0.0f;                                  // 00641CC3 COMISS / JBE
        }
        float unused[3];                                                       // [B+88h]
        host.hud_marker_project_and_clip_00638e50(world, unused);              // 00641CD9
        float x, y, angle;                                                     // [B+20h] [B+18h] [B+14h]
        host.hud_marker_project_offscreen_arrow_00638c60(world, &x, &y, &angle);  // 00641CF0
        const std::int32_t index = host.hud_markers_colour_index_00639990(unit);  // 00641CF8
        host.hud_marker_format_distance_0063e5c0(&distance, unit);             // 00641D1C
        const char* text = static_cast<const char*>(host.image_global(kEmptyCString));
        if (low(is_self))
            text = host.recon_display_name_for_party_00803dc0(current_party_table(host), unit);  // 00641D45
        const float scale = static_cast<float>(host.unit_health_00923be0(unit));  // 00641D56
        Cc13SweepBNativeString copy;
        host.native_string_copy_construct_00426060(&copy, &distance);          // 00641D6D
        float colour_out[4];                                                   // [B+94h]
        const float* const colour_ptr = host.unit_health_colour_0077a2f0(colour_out, 1.0f);  // 00641D88
        Cc13Pkt642Colour colour;
        for (int i = 0; i < 4; ++i) colour.rgba[i] = colour_ptr[i];            // 00641D9E..00641DB3
        cc13_pkt642_hud_marker_write_unit_marker_widgets_0063d1e0(record, unit, x, y, angle, index, colour,
                                                                  text, copy, scale, is_self, host);  // 00641DDC
    }
    release_native_string(distance, host);                                     // 00641C36 / 00641DE1
}

// 00428420 __thiscall(ECX = projectile trace; float), RET 4; no frame pointer, 7Ch bytes of
// locals. The landing groups are the list [[00E188A8]+21D0h]+28h ({proxy, head, size}); each
// node's value +10h is a group whose box is +9Ch..+A8h and whose three member lists start at
// +B4h (0Ch apart). A list node holds {+8h type, +0Ch member, +34h state, +38h timer}. The
// trace's segments (18h bytes: start, end; count +C0h) are boxed (00427D10 from FLT_MAX /
// -FLT_MAX). For each group whose box overlaps (0085BF20), each member passing vtable[5Ch](3Ah)
// whose state is not 3 is tested against every segment: its world pose is refreshed when +C8h is
// clear (00414DB0 on the parent +3Ch, then +74h times the parent's +CCh, or +74h itself, copied
// to +CCh..+108h; +C8h = 1, +10Ch = 0), and 00428090(+FCh, start; 2.0, end) decides the hit.
// The first hit ends the search: state = 3, timer = 20.0, 00480BF0(member), a stream-0 draw
// (00BD2FC0(0) modulo the size of the type's 8-byte vector +60h..+64h) picks the death
// animation for 004B0F80(member; &anim, 0, 0), and a type with +39h clear forwards its +1Ch
// (Damage) to the group through 00480C20. No stack argument is read.
void projectile_trace_hit_landing_troops_00428420(void* trace, float unused, Big1Host& host) {
    static_cast<void>(unused);
    std::uint8_t* const t = static_cast<std::uint8_t*>(trace);              // [B+14h]
    std::uint8_t* const groups = static_cast<std::uint8_t*>(ptr(host.game_00e188a8(), 0x21D0)) + 0x28;  // EBX, [B+1Ch]
    if (at<std::uint32_t>(groups, 8) == 0) return;                            // 00428432
    if (at<std::uint32_t>(t, 0xC0) == 0) return;                              // 0042844A

    float box[6] = {3.4028235e38f, 3.4028235e38f, 3.4028235e38f,              // [00D7A248], [B+34h]
                    -3.4028235e38f, -3.4028235e38f, -3.4028235e38f};          // [00D7A244], [B+40h]
    std::uint8_t* const segments_end = t + at<std::uint32_t>(t, 0xC0) * 0x18;
    for (std::uint8_t* seg = t; seg != segments_end; seg += 0x18) {
        host.aabb_include_point_00427d10(box, reinterpret_cast<const float*>(seg));          // 00428499
        host.aabb_include_point_00427d10(box, reinterpret_cast<const float*>(seg + 0xC));    // 004284A6
    }

    void* it[2] = {groups, at<void*>(ptr(groups, 4), 0)};                     // [B+24h] owner, [B+28h] node
    for (;;) {
        void* const head = ptr(groups, 4);                                    // EDI, 004284C4
        if (it[0] == nullptr || it[0] != groups) host.crt_invalid_parameter_noinfo_00bf6713();  // 004284CD
        if (it[1] == head) return;                                            // 004284D4
        if (it[0] == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();   // 004284DE
        if (it[1] == at<void*>(it[0], 4)) host.crt_invalid_parameter_noinfo_00bf6713();  // 004284E8
        std::uint8_t* const group = static_cast<std::uint8_t*>(ptr(it[1], 0x10));  // [B+20h]
        if (host.aabb_overlaps_0085bf20(box, box + 3, reinterpret_cast<const float*>(group + 0x9C),
                                        reinterpret_cast<const float*>(group + 0xA8))) {  // 0042850A
            for (std::int32_t k = 0; k < 3; ++k) {                            // [B+18h]
                std::uint8_t* const list = group + 0xB4 + k * 0xC;            // EBP, [B+10h]; EDI, [B+2Ch]
                std::uint8_t* node = static_cast<std::uint8_t*>(at<void*>(ptr(list, 4), 0));  // EBX
                for (;;) {
                    if (list == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();  // 0042854B (owner == list)
                    if (node == ptr(list, 4)) break;                          // 00428552
                    if (list == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();  // 0042855C
                    if (node == ptr(list, 4)) host.crt_invalid_parameter_noinfo_00bf6713();  // 00428566
                    std::uint8_t* const member = static_cast<std::uint8_t*>(ptr(node, 0xC));  // ESI
                    if (host.unit_has_flag_vtable5c(member, 0x3A)) {          // 00428577
                        if (node == ptr(list, 4)) host.crt_invalid_parameter_noinfo_00bf6713();  // 00428586
                        if (at<std::int32_t>(node, 0x34) != 3) {              // 0042858B
                            for (std::uint32_t j = 0; j < at<std::uint32_t>(t, 0xC0); ++j) {  // EBP, JC
                                std::uint8_t* const seg = t + j * 0x18;       // EDI
                                if (at<std::uint8_t>(member, 0xC8) == 0) {    // 004285B0
                                    if (ptr(member, 0x3C) != nullptr)
                                        host.entity_pose_refresh_world_00414db0(ptr(member, 0x3C));  // 004285C4
                                    const float* src = reinterpret_cast<const float*>(member + 0x74);
                                    float product[16];                         // [B+4Ch]
                                    if (ptr(member, 0x3C) != nullptr)
                                        src = host.matrix_multiply_00413920(
                                            member + 0x74, product,
                                            static_cast<std::uint8_t*>(ptr(member, 0x3C)) + 0xCC);  // 004285DE
                                    for (std::size_t e = 0; e < 16; ++e)
                                        at<float>(member, 0xCC + 4 * e) = src[e];  // FLD/FSTP float copies
                                    at<std::uint8_t>(member, 0xC8) = 1;       // 00428677
                                    at<std::uint8_t>(member, 0x10C) = 0;
                                }
                                if (host.segment_intersects_sphere_00428090(
                                        reinterpret_cast<const float*>(member + 0xFC),
                                        reinterpret_cast<const float*>(seg), 2.0f,  // [00CE3958]
                                        reinterpret_cast<const float*>(seg + 0xC))) {  // 0042869B
                                    // 00428708: the hit.
                                    if (node == ptr(list, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
                                    at<std::int32_t>(node, 0x34) = 3;         // 00428716
                                    if (node == ptr(list, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
                                    at<float>(node, 0x38) = 20.0f;            // [00CE3930]
                                    if (node == ptr(list, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
                                    host.troop_member_kill_00480bf0(ptr(node, 0xC));  // 00428741
                                    if (node == ptr(list, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
                                    std::uint8_t* type = static_cast<std::uint8_t*>(ptr(node, 8));
                                    const std::uint32_t count =
                                        ptr(type, 0x60) == nullptr
                                            ? 0
                                            : static_cast<std::uint32_t>(at<std::uint8_t*>(type, 0x64) -
                                                                         at<std::uint8_t*>(type, 0x60)) >> 3;
                                    if (node == ptr(list, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
                                    // 0042877C DIV: a zero count faults (#DE) in the image.
                                    const std::uint32_t pick = host.random_threads_next_u32_00bd2fc0(0) % count;
                                    type = static_cast<std::uint8_t*>(ptr(node, 8));
                                    if (ptr(type, 0x60) == nullptr ||
                                        pick >= (static_cast<std::uint32_t>(at<std::uint8_t*>(type, 0x64) -
                                                                            at<std::uint8_t*>(type, 0x60)) >> 3))
                                        host.crt_invalid_parameter_noinfo_00bf6713();  // 00428799
                                    std::uint8_t* const anim = at<std::uint8_t*>(type, 0x60) + pick * 8;
                                    if (node == ptr(list, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
                                    host.soldier_start_animation_004b0f80(ptr(node, 0xC), anim, 0, 0);  // 004287B6
                                    if (node == ptr(list, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
                                    if (at<std::uint8_t>(ptr(node, 8), 0x39) == 0) {  // 004287C8
                                        if (node == ptr(list, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
                                        host.landing_group_apply_damage_00480c20(group,
                                                                                 at<float>(ptr(node, 8), 0x1C));  // 004287E6
                                    }
                                    return;
                                }
                            }
                        }
                    }
                    if (node == ptr(list, 4)) host.crt_invalid_parameter_noinfo_00bf6713();  // 004286C7
                    node = static_cast<std::uint8_t*>(at<void*>(node, 0));
                }
            }
        }
        host.group_iterator_increment_00427fe0(it);                          // 004286F6
    }
}

namespace {

// 004A0D20's member group: byte +38h clear gives 2, else (byte +39h != 0).
std::int32_t troop_member_group(const void* member) {
    if (at<std::uint8_t>(member, 0x38) == 0) return 2;
    return at<std::uint8_t>(member, 0x39) != 0 ? 1 : 0;
}

// FILD int; FMUL float: one 24-bit rounding of the product.
float int_times_float(std::int32_t n, float f) { return static_cast<float>(static_cast<double>(n) * f); }

// CVTTSS2SI: truncation; NaN and out-of-range values give 80000000h.
std::int32_t cvttss2si(float v) {
    const double d = v;
    if (!(d > -2147483649.0 && d < 2147483648.0)) return INT32_MIN;
    return static_cast<std::int32_t>(v);
}

// The new[] size: MUL; SETO; NEG; OR gives FFFFFFFFh when count * size overflows.
std::uint32_t array_new_size(std::uint32_t count, std::uint32_t size) {
    const std::uint64_t bytes = static_cast<std::uint64_t>(count) * size;
    return (bytes >> 32) != 0 ? 0xFFFFFFFFu : static_cast<std::uint32_t>(bytes);
}

// The vector's (end - begin) >> 2 as the image inlines it (0 for a null begin).
std::int32_t inline_point_count(const void* path) {
    if (at<const void*>(path, 8) == nullptr) return 0;
    return static_cast<std::int32_t>(at<const std::uint8_t*>(path, 0xC) - at<const std::uint8_t*>(path, 8)) >> 2;
}

// 004A1D7F..004A1E61 and 004A1F61..004A2030: the entity's cached world matrix +CCh, rebuilt while
// byte +C8h is clear (00414DB0 on the parent +3Ch, then local +74h times the parent's +CCh, or the
// local matrix alone, copied as 16 floats; +C8h = 1, +10Ch = 0).
const float* cached_world_matrix(std::uint8_t* e, Big1Host& host) {
    if (at<std::uint8_t>(e, 0xC8) == 0) {
        if (ptr(e, 0x3C) != nullptr) host.entity_pose_refresh_world_00414db0(ptr(e, 0x3C));
        const float* src = reinterpret_cast<const float*>(e + 0x74);
        float product[16];                                                    // [B+458h] / [B+494h]
        if (ptr(e, 0x3C) != nullptr)
            src = host.matrix_multiply_00413920(e + 0x74, product, static_cast<std::uint8_t*>(ptr(e, 0x3C)) + 0xCC);
        for (std::size_t k = 0; k < 16; ++k) at<float>(e, 0xCC + 4 * k) = src[k];
        at<std::uint8_t>(e, 0xC8) = 1;
        at<std::uint8_t>(e, 0x10C) = 0;
    }
    return reinterpret_cast<const float*>(e + 0xCC);
}

// 00B62D10 then the divide by w: inv = 1 / w (FLD1; FDIVRP), {x * inv, y * inv, inv * z}.
void project_point(const float* vector, const void* matrix, float* out, Big1Host& host) {
    float result[4];
    const float* const r = host.vector4f_transform_00b62d10(vector, result, matrix);
    const float x = r[0], y = r[1], z = r[2], w = r[3];
    const float inv = 1.0f / w;
    out[0] = x * inv;
    out[1] = y * inv;
    out[2] = inv * z;
}

// The checked `--end()` dereference: the list's last node (head->prev); 00BF6713 when that is the
// head, tested twice against the re-read head.
std::uint8_t* last_node(std::uint8_t* list, Big1Host& host) {
    void* const head = ptr(list, 4);
    std::uint8_t* const node = static_cast<std::uint8_t*>(at<void*>(head, 4));
    if (node == head) host.crt_invalid_parameter_noinfo_00bf6713();
    if (node == ptr(list, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
    return node;
}

void put_float(std::uint32_t* block, std::size_t index, float v) { std::memcpy(block + index, &v, 4); }

} // namespace

// 004A0D20 __thiscall(ECX = landing troop), RET. EBP frame aligned to 8, SUB ESP 4CCh, EH handler
// 00C639AE (state 0 only around 007B28C0, whose block the unwind frees; not modelled). B below is ESP
// after PUSH EDI. Precision: modelled at 24 bits (each x87 op rounded to float in the listing's order;
// double constants applied in double and rounded once). The caller 004A2FC0 is the landing-troop tick
// (vtable 00CE66E0 slot 1); which thread runs it is not established, so 53-bit is an uncertainty.
// Frame slots that persist across the member loop in the image (the rotated path points [B+358h],
// the long/short vectors [B+54h]/[B+6Ch] and the 40h-byte block [B+1B4h]) are function-scope here,
// zero before their first write where the image reads uninitialised stack.
void landing_troop_update_004a0d20(void* troop, Big1Host& host) {
    std::uint8_t* const t = static_cast<std::uint8_t*>(troop);              // EBX
    auto member = [&](std::int32_t i) { return at<std::uint8_t*>(ptr(t, 0x48), 4 * static_cast<std::size_t>(i)); };
    auto d8 = [&](std::size_t off) -> std::int32_t& { return at<std::int32_t>(ptr(t, 0xD8), off); };

    if (ptr(t, 0xDC) == nullptr) return;                                      // 004A0D4F
    if (at<std::uint8_t>(t, 0x20) != 0) return;                               // 004A0D59

    // [B+88h]: the three group counts. 004A0E15 indexes the store by the member index, so members
    // 3.. write the frame dwords after it ([B+94h] base x, [B+98h], [B+9Ch] base z, ...); each of
    // those is rewritten before it is read. The spill dwords stand for them (more than 16 members
    // would clobber further frame slots in the image; not modelled).
    std::int32_t counts[16] = {};
    std::int32_t total = 0;                                                   // [B+2Ch]
    bool distribute = false;
    if (at<std::int32_t>(t, 0x4C) > 0) {
        for (std::int32_t i = 0; i < at<std::int32_t>(t, 0x4C); ++i) {        // ESI
            if (at<std::uint8_t>(member(i), 0x38) != 0) {
                if ((at<std::int32_t>(t, 0xBC) == 0 && d8(0x10) > 0) ||
                    (at<std::int32_t>(t, 0xC8) == 0 && d8(0x14) > 0)) {       // 004A0D98..004A0DC0
                    total = ftol2_low_00bf7420(static_cast<float>(
                        int_times_float(at<std::int32_t>(member(i), 0x2C), at<float>(member(i), 0x14)) +
                        static_cast<double>(total)));                         // 004A0DC8 FILD/FMUL/FIADD
                    std::int32_t& c = counts[troop_member_group(member(i))];
                    c = ftol2_low_00bf7420(static_cast<float>(
                        int_times_float(at<std::int32_t>(member(i), 0x2C), at<float>(member(i), 0x14)) +
                        static_cast<double>(c)));                             // 004A0E01
                } else {
                    counts[i] = 0;                                            // 004A0E15
                }
            } else if (at<std::int32_t>(t, 0xD4) == 0) {
                const std::int32_t k = d8(0x10);                              // EBP
                if (k > 0) {
                    total = ftol2_low_00bf7420(static_cast<float>(
                        int_times_float(at<std::int32_t>(member(i), 0x2C), at<float>(member(i), 0x14)) +
                        static_cast<double>(total)));                         // 004A0E3E
                    std::int32_t& c = counts[troop_member_group(member(i))];
                    c = ftol2_low_00bf7420(static_cast<float>(
                        int_times_float(at<std::int32_t>(member(i), 0x2C), at<float>(member(i), 0x14)) +
                        static_cast<double>(c)));                             // 004A0E77
                    if (at<std::int32_t>(t, 0xBC) == 0 && k == 1) d8(0x10) += k;  // 004A0E9A
                }
            }
        }
        distribute = total > 0;                                               // 004A0EAF
    }
    if (!distribute) {
        if (d8(0x10) <= 0) return;                                            // 004A0EBE
        // 004A0EC4: every node of the list +B4h gets +38h = 10.0 ([00CE38B8]) and +34h = 6. The
        // iterator owner is the list itself (CMP EDI,EDI), so only the end tests can fail.
        std::uint8_t* const list0 = t + 0xB4;
        for (std::uint8_t* node = static_cast<std::uint8_t*>(at<void*>(ptr(list0, 4), 0));;
             node = static_cast<std::uint8_t*>(at<void*>(node, 0))) {
            if (node == ptr(list0, 4)) break;                                 // 004A0EDE
            if (node == ptr(list0, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
            at<float>(node, 0x38) = 10.0f;
            if (node == ptr(list0, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
            at<std::int32_t>(node, 0x34) = 6;
            if (node == ptr(list0, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
        }
        if (ptr(t, 0xDC) != nullptr) {                                        // 004A0F18
            for (std::int32_t i = 0; i < at<std::int32_t>(t, 0x4C); ++i) {
                std::uint8_t* const m = member(i);
                if (at<std::uint8_t>(m, 0x38) != 0 && at<std::uint8_t>(m, 0x39) == 0)
                    counts[troop_member_group(m)] = ftol2_low_00bf7420(
                        int_times_float(at<std::int32_t>(member(i), 0x2C), at<float>(member(i), 0x14)));  // 004A0F61
            }
        }
    }

    // 004A0F7B: [B+7Ch] the per-group weight sums, each replaced by a stream-0 draw in [0, sum].
    float sums[3] = {0.0f, 0.0f, 0.0f};
    {
        const std::int32_t n = at<std::int32_t>(t, 0x4C);                     // EBP, read once
        std::uint8_t* const* const members = static_cast<std::uint8_t* const*>(ptr(t, 0x48));
        for (std::int32_t i = 0; i < n; ++i) {
            const std::int32_t g = troop_member_group(members[i]);
            sums[g] = at<float>(members[i], 0x14) + sums[g];                  // 004A0FC0
        }
    }
    sums[0] = host.random_uniform_float_00bd2f10(0, 0.0f, sums[0]);           // 004A0FE0
    sums[1] = host.random_uniform_float_00bd2f10(0, 0.0f, sums[1]);           // 004A0FFE
    sums[2] = host.random_uniform_float_00bd2f10(0, 0.0f, sums[2]);           // 004A101F

    float points[5][3] = {};  // [B+358h]: 5 entries reach [B+394h]; the image's array is bounded by
                              // the 007AF800 buffer at [B+3A0h] (a 5-point path needs 4)
    float u[3] = {};          // [B+54h]: the longer of the two first spans, or the end direction
    float v[3] = {};          // [B+6Ch]: the shorter span (written on the straight path only)
    std::uint32_t block[16] = {};  // [B+1B4h]: the list value, later the identity matrix

    for (std::int32_t i = 0; i < at<std::int32_t>(t, 0x4C); ++i) {            // [B+30h]
        std::uint8_t* const m0 = member(i);                                   // ESI
        const std::int32_t g_list = troop_member_group(m0);                   // [B+78h]
        if (counts[troop_member_group(m0)] <= 0) continue;                    // 004A108C
        if (at<std::uint8_t>(m0, 0x39) != 0 && d8(0x14) < 1) continue;        // 004A10A3
        if (at<std::uint8_t>(m0, 0x39) == 0 && d8(0x10) < 1) continue;        // 004A10BA
        float& sum = sums[troop_member_group(m0)];
        sum = sum - at<float>(m0, 0x14);                                      // 004A10E3
        if (!(0.0f > sums[troop_member_group(m0)])) continue;                 // 004A10FC COMISS/JBE (NaN skips)
        if (at<std::uint8_t>(m0, 0x39) == 0 && d8(0x10) > 0) d8(0x10) -= 1;   // 004A111D
        if (at<std::uint8_t>(member(i), 0x39) != 0 && d8(0x14) > 0) d8(0x14) -= 1;  // 004A113B

        std::uint8_t* const data = static_cast<std::uint8_t*>(ptr(t, 0xD8));  // EAX
        std::uint8_t* const a_owner =
            static_cast<std::uint8_t*>(at<std::uint8_t>(member(i), 0x39) == 0 ? ptr(data, 0) : ptr(data, 8));
        std::uint8_t* const path_a = a_owner != nullptr ? a_owner + 0x1E4 : nullptr;  // [B+3Ch]
        std::uint8_t* b_owner = static_cast<std::uint8_t*>(ptr(data, 4));
        if (at<std::uint8_t>(member(i), 0x39) != 0 && ptr(data, 0xC) != nullptr)
            b_owner = static_cast<std::uint8_t*>(ptr(data, 0xC));
        std::uint8_t* const path_b = b_owner != nullptr ? b_owner + 0x1E4 : nullptr;  // [B+48h]
        if (path_a == nullptr || path_b == nullptr) continue;                 // 004A1196 / 004A119E

        // [B+1Bh]: set unless path b has exactly four points whose ends are under 10.0 apart.
        bool loose = true;
        if (ptr(path_b, 8) != nullptr && inline_point_count(path_b) == 4) {   // 004A11A9 / 004A11BA
            const std::int32_t n = inline_point_count(path_b);                // EBP
            float first[3], last[3];                                          // [B+3B8h], [B+3E8h]
            const float* const a = host.scene_path_point_to_world_007af800(path_b, first, 0);  // 004A11DE
            const float* const b = host.scene_path_point_to_world_007af800(path_b, last, n - 1);  // 004A11F3
            const float dx = b[0] - a[0], dy = b[1] - a[1], dz = b[2] - a[2];
            const float d2 = (dx * dx + dy * dy) + dz * dz;                   // 004A1214..004A1230
            if (static_cast<double>(d2) > 1e-10) {                            // [00CE3820], JBE: NaN clears
                const float len = sqrt_00bf7030(d2);                          // 004A1244
                loose = !(10.0f > len);                                       // [00CE38B8], COMISS/JA
            } else {
                loose = false;                                                // 004A1281
            }
        }
        const std::uint8_t m39 = at<std::uint8_t>(member(i), 0x39);           // [B+37h]

        // 004A1292: one 40h-byte matrix per point of path a, row 3 / row 2 sampled at its knots.
        float* const matrices = static_cast<float*>(host.crt_operator_new_00bf55be(
            array_new_size(static_cast<std::uint32_t>(inline_point_count(path_a) + 1), 0x40)));  // [B+40h]
        for (std::int32_t k = 0; k < host.path_point_count_00415870(path_a); ++k) {  // EBP
            float* const mtx = matrices + 16 * k;
            static constexpr float kIdentity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
            for (std::size_t e = 0; e < 16; ++e) mtx[e] = kIdentity[e];
            float knot = host.path_knot_time_007b05e0(path_a, static_cast<std::uint32_t>(k));  // [B+38h]
            const float end = at<float>(path_a, 0x1C);                        // [B+200h], [B+18Ch]
            if (knot >= end) knot = static_cast<float>(end - 0.01);           // FCOMIP/JC; [00CE6638]
            host.camera_path_sample_world_007b03c0(path_a, knot, mtx + 12, mtx + 8, 1);  // 004A138B
        }

        if (!loose) {
            // 004A13B0: append path b's first point as point 4, then rotate its points so the one
            // farthest from path a's end comes first. The search reads [B+358h] before this member
            // writes it (the previous member's rotation, or uninitialised stack: zero here).
            float first[3];                                                   // [B+3D0h]
            host.path_points_set_point_local_007ad180(
                path_b, 4, host.scene_path_point_to_world_007af800(path_b, first, 0));  // 004A13C0 / 004A13CA
            float end_point[3];                                               // [B+D4h], read from the buffer
            host.scene_path_point_to_world_007af800(path_a, end_point, inline_point_count(path_a) - 1);  // 004A13F0
            std::int32_t best = 0;                                            // EBP, [B+1Ch] = best * 0Ch
            if (host.path_point_count_00415870(path_b) - 2 > 0) {             // 004A1402
                std::int32_t k = 0;                                           // ESI
                do {
                    const float dx = points[k][0] - end_point[0];
                    const float dy = points[k][1] - end_point[1];
                    const float dz = points[k][2] - end_point[2];
                    const float dk = (dx * dx + dy * dy) + dz * dz;           // [B+44h]
                    const float bx = points[best][0] - end_point[0];
                    const float by = points[best][1] - end_point[1];
                    const float bz = points[best][2] - end_point[2];
                    const float db = (bx * bx + by * by) + bz * bz;
                    if (dk > db) best = k;                                    // FCOMIP/JBE: NaN keeps
                    ++k;
                } while (k < host.path_point_count_00415870(path_b) - 2);
            }
            const std::int32_t offset = best - 1;                             // [B+1Ch]
            std::int32_t j = 0;                                               // EDI = &points[j]
            for (std::int32_t k = best; k < host.path_point_count_00415870(path_b) + offset; ++k, ++j) {
                const std::int32_t index = k % (host.path_point_count_00415870(path_b) - 1);  // IDIV (#DE on 0)
                float point[3];                                               // [B+3A0h]
                const float* const q = host.scene_path_point_to_world_007af800(path_b, point, index);
                points[j][0] = q[0];
                points[j][1] = q[1];
                points[j][2] = q[2];
            }
            for (std::int32_t k = 0; k < host.path_point_count_00415870(path_b) - 1; ++k)
                host.path_points_set_point_local_007ad180(path_b, k, points[k]);  // 004A1584

            const float a[3] = {points[1][0] - points[0][0], points[1][1] - points[0][1],
                                points[1][2] - points[0][2]};                 // [B+54h] (+ [B+204h]...)
            const float b[3] = {points[2][0] - points[1][0], points[2][1] - points[1][1],
                                points[2][2] - points[1][2]};                 // [B+6Ch]
            const float la = (a[1] * a[1] + a[0] * a[0]) + a[2] * a[2];
            const float lb = (b[1] * b[1] + b[0] * b[0]) + b[2] * b[2];
            for (std::size_t e = 0; e < 3; ++e) {
                u[e] = lb > la ? b[e] : a[e];                                 // FCOMIP/JBE: NaN keeps a
                v[e] = lb > la ? a[e] : b[e];
            }
        }

        // 004A16D6: both lengths, computed on every path (the ratio is used on the straight one).
        const float lu2 = (u[1] * u[1] + u[0] * u[0]) + u[2] * u[2];          // [B+1Ch]
        const float len_u = static_cast<double>(lu2) > 1e-10 ? sqrt_00bf7030(lu2) : 0.0f;  // [B+A0h]
        const float lv2 = (v[1] * v[1] + v[0] * v[0]) + v[2] * v[2];          // [B+68h]
        const float len_v = static_cast<double>(lv2) > 1e-10 ? sqrt_00bf7030(lv2) : 0.0f;  // [B+C0h]
        const float ratio = len_u / len_v;                                    // [B+14h]

        float spacing;                                                        // [B+60h]
        if (loose) {
            // 004A17AB: u = the normalised last span of path b; spacing = the knot time of its
            // second-last point over (member +2Ch + 1).
            const std::int32_t n = inline_point_count(path_b);
            float q0b[3], q1b[3];                                             // [B+400h], [B+3DCh]
            const float* const q0 = host.scene_path_point_to_world_007af800(path_b, q0b, n - 2);
            const float* const q1 = host.scene_path_point_to_world_007af800(path_b, q1b, n - 1);
            const float dx = q1[0] - q0[0], dy = q1[1] - q0[1], dz = q1[2] - q0[2];
            const float len = sqrt_00bf7030((dx * dx + dy * dy) + dz * dz);   // 004A1846
            const float inv = reciprocal_or_zero(len);                        // [B+A8h]
            u[0] = inv * dx;
            u[1] = inv * dy;
            u[2] = inv * dz;
            if (at<std::uint8_t>(path_b, 0x18) == 0) host.camera_path_derive_knots_007af150(path_b);  // 004A18C1
            std::uint8_t* const* const slot = &at<std::uint8_t*>(ptr(t, 0x48), 4 * static_cast<std::size_t>(i));  // EDI
            const float knot = host.path_knot_time_007b05e0(path_b, static_cast<std::uint32_t>(n - 2));  // 004A18DA
            spacing = static_cast<float>(knot / static_cast<double>(at<std::int32_t>(*slot, 0x2C) + 1));  // FIDIV
        } else {
            const float lu = static_cast<double>(lu2) > 1e-10 ? sqrt_00bf7030(lu2) : 0.0f;  // [B+ACh]
            spacing = lu / ratio;                                             // 004A1934
        }

        std::int32_t segments = 0;                                            // [B+44h]
        std::int32_t steps = 0;                                               // [B+64h]
        std::int32_t row = 0;                                                 // [B+38h]
        std::int32_t slot = 0;                                                // [B+2Ch]
        std::int32_t row_left = at<std::int32_t>(member(i), 0x34);            // [B+28h]
        std::int32_t* const starts = static_cast<std::int32_t*>(host.crt_operator_new_00bf55be(
            array_new_size(static_cast<std::uint32_t>(at<std::int32_t>(member(i), 0x34) + 1), 4)));  // [B+1Ch]
        float base_x = -0.5f;                                                 // [B+94h]  [00CE69D0]
        float base_z = -0.2f;                                                 // [B+9Ch]  [00CE69CC]
        float step_x = 0.5f;                                                  // [B+164h] [00CE3800]
        float step_z = 0.3f;                                                  // [B+16Ch] [00CE69C8]
        if (ptr(t, 0xDC) != nullptr) {                                        // 004A19B0
            float p1b[3], p0b[3];                                             // [B+3F4h], [B+408h]
            const float* const p1 = host.unit_vtable24c_point(ptr(t, 0xDC), p1b, 1);  // 004A19F3
            base_x = p1[0];
            base_z = p1[2];
            const float* const p0 = host.unit_vtable24c_point(ptr(t, 0xDC), p0b, 0);  // 004A1A28
            const float p0x = p0[0];                                          // [B+300h]
            const float p0z = p0[2];                                          // [B+308h]
            std::uint8_t* const m = member(i);
            const std::int32_t g = troop_member_group(m);
            const float dxp = p0x - base_x;
            const float sx = static_cast<float>(dxp / static_cast<double>(at<std::int32_t>(m, 0x34)));  // 004A1A84
            const float dzp = p0z - base_z;
            const float q1 = static_cast<float>(counts[g] / static_cast<double>(at<float>(m, 0x14)));  // 004A1AAA
            const std::int32_t mean = (at<std::int32_t>(m, 0x30) + at<std::int32_t>(m, 0x34)) / 2;
            const float q2 = static_cast<float>(q1 / static_cast<double>(mean));  // 004A1AB6 FIDIV
            const float sz = dzp / q2;                                        // 004A1ABA
            step_x = sx;
            step_z = sz;
            base_z = static_cast<float>(sz * 0.5) + base_z;                   // [00D7A280] (DC C9)
            base_x = static_cast<float>(sx * 0.5) + base_x;
        }

        // 004A1AF8: one list node per soldier of the member.
        for (std::int32_t r = 0;; ++r) {                                      // [B+14h]
            if (!(r < at<std::int32_t>(member(i), 0x2C))) break;               // 004A1B17
            if (counts[troop_member_group(member(i))] <= 0) break;            // 004A1B3C
            float lo, hi;
            if (slot < at<std::int32_t>(member(i), 0x34) / 2) {
                lo = -0.0f - at<float>(t, 0x28);                              // [00D7A208] SUBSS
                hi = 0.0f;
            } else {
                lo = 0.0f;
                hi = at<float>(t, 0x28);
            }
            const float lateral = host.random_uniform_float_00bd2f10(0, lo, hi);  // [B+50h]
            std::uint8_t* const mr = member(i);                               // ESI
            const float speed = host.random_uniform_float_00bd2f10(0, at<float>(t, 0x30), at<float>(t, 0x34));  // [B+1E4h]
            const std::uintptr_t mr_bits = reinterpret_cast<std::uintptr_t>(mr);
            block[0] = static_cast<std::uint32_t>(mr_bits);
            block[1] = 0;
            put_float(block, 2, lateral);
            block[3] = 0;                                                     // [4] (+10h) left as is
            block[5] = 0;
            block[6] = 0;
            block[7] = 0;
            put_float(block, 8, 1.0f);                                        // [00D7A24C]
            block[9] = 0;
            block[10] = 0;
            block[11] = 0;
            put_float(block, 12, speed);                                      // [13] (+34h) left as is
            block[14] = 0;
            put_float(block, 15, 1.1f);                                       // [00CE6448]
            std::uint8_t* const list = t + 0xB4 + 0xC * static_cast<std::size_t>(g_list);  // ESI
            void* const head = ptr(list, 4);                                  // EDI
            void* const node = host.list_buy_node48_00488e30(list, head, at<void*>(head, 4), block);  // 004A1C64
            host.list_incsize_00490130(list, 1);                              // 004A1C6F
            at<void*>(head, 4) = node;
            at<void*>(ptr(node, 4), 0) = node;
            const float slot_f = static_cast<float>(slot);                    // [B+224h], [B+2E8h]
            const float row_f = static_cast<float>(row);                      // [B+154h], [B+2F0h]
            std::uint8_t* n = last_node(list, host);
            at<float>(n, 0x1C) = slot_f;
            at<float>(n, 0x20) = 0.0f;
            at<float>(n, 0x24) = row_f;
            void* const raw = host.crt_operator_new_00bf681b(0x2C);           // 004A1CEB
            void* component = nullptr;                                        // EBP
            if (raw != nullptr) component = host.path_component_construct_007b28c0(raw, ptr(path_a, 0x14));
            n = last_node(list, host);
            at<void*>(n, 0x18) = component;                                   // 004A1D3D

            for (std::int32_t j = 0; j < host.path_point_count_00415870(path_a); ++j) {  // [B+4Ch]
                const float* const mtx = matrices + 16 * j;                   // EBP - 8
                if (j < 1 && m39 == 0) {
                    // 004A1D72: two grid points in the +DCh unit's frame, the second 3.0 + |z offset|
                    // further along z.
                    std::uint8_t* dc = static_cast<std::uint8_t*>(ptr(t, 0xDC));  // EDI
                    const float x = slot_f * step_x + base_x;                 // [B+C8h]
                    const float zrel = row_f * step_z;                        // [B+20h] (FST double)
                    float z = zrel + base_z;                                  // [B+D0h]
                    const float* world = cached_world_matrix(dc, host);
                    const float v1[4] = {x, 0.0f, z, 1.0f};                   // [B+318h]
                    float p1[3];                                              // [B+2D0h]
                    project_point(v1, world, p1, host);                       // 004A1EB9
                    host.path_points_add_point_local_007ad330(ptr(last_node(list, host), 0x18), p1);  // 004A1F4E
                    dc = static_cast<std::uint8_t*>(ptr(t, 0xDC));
                    z = static_cast<float>(std::fabs(zrel) + 3.0) + z;        // [00D7A2B0]
                    world = cached_world_matrix(dc, host);
                    const float v2[4] = {x, 0.0f, z, 1.0f};                   // [B+338h]
                    float p2[3];                                              // [B+2DCh]
                    project_point(v2, world, p2, host);                       // 004A2088
                    host.path_points_add_point_local_007ad330(ptr(last_node(list, host), 0x18), p2);  // 004A263E
                } else {
                    // 004A211F: the point's matrix with row 1 replaced by the global vector.
                    float frame[16];                                          // [B+E0h]
                    for (std::size_t e = 0; e < 16; ++e) frame[e] = mtx[e];
                    const float* const up = host.global_vector_00f8758c();
                    frame[4] = up[0];
                    frame[5] = up[1];
                    frame[6] = up[2];
                    host.matrix_orthonormalize_0085dad0(frame);               // 004A2239
                    if (j <= 0 && m39 != 0) {
                        // 004A2254: first a point 2.0 back along path a's first span.
                        float a0b[3], a1b[3];                                 // [B+3ACh], [B+3C4h]
                        const float* const a0 = host.scene_path_point_to_world_007af800(path_a, a0b, 0);
                        const float* const a1 = host.scene_path_point_to_world_007af800(path_a, a1b, 1);
                        const float d[3] = {a1[0] - a0[0], a1[1] - a0[1], a1[2] - a0[2]};  // [B+B4h]
                        const float len = sqrt_00bf7030((d[1] * d[1] + d[0] * d[0]) + d[2] * d[2]);  // 004A22E8
                        const float inv = reciprocal_or_zero(len);            // [B+B0h]
                        const float nx = inv * d[0], ny = inv * d[1], nz = inv * d[2];  // [B+170h]
                        const float back[3] = {static_cast<float>(nx * 2.0), static_cast<float>(ny * 2.0),
                                               static_cast<float>(2.0 * nz)};  // [00D7A308]
                        const float q[4] = {frame[12] - back[0], frame[13] - back[1], frame[14] - back[2],
                                            1.0f};                            // [B+348h]
                        float p[3];                                           // [B+2BCh]
                        project_point(q, t + 0x58, p, host);                  // 004A2418
                        host.path_points_add_point_local_007ad330(ptr(last_node(list, host), 0x18), p);  // 004A24AD
                    } else {
                        // 004A24B7: jitter along row 0 by (draw(0, 0.2) + 0.9) * lateral.
                        const float jitter = host.random_uniform_float_00bd2f10(0, 0.0f, 0.2f);  // [00CE54A0]
                        const float s = static_cast<float>(jitter + 0.9) * lateral;  // [00CE69C0]
                        const float o[3] = {s * frame[0], s * frame[1], s * frame[2]};  // [B+208h]
                        frame[12] = o[0] + frame[12];
                        frame[13] = o[1] + frame[13];
                        frame[14] = o[2] + frame[14];
                    }
                    const float q[4] = {frame[12], frame[13], frame[14], 1.0f};  // [B+328h]
                    float p[3];                                               // [B+2F4h]
                    project_point(q, t + 0x58, p, host);                      // 004A25A9
                    host.path_points_add_point_local_007ad330(ptr(last_node(list, host), 0x18), p);  // 004A263E
                }
            }

            // 004A2662: steps along u at the spacing; past |u| a new segment starts.
            ++steps;
            const float wx = u[0] / spacing;                                  // [B+19Ch]
            const float wy = u[1] / spacing;                                  // [B+1A0h]
            const float wz = u[2] / spacing;                                  // [B+1A4h]
            const float kf = static_cast<float>(steps);                       // [B+20h]
            const float kw[3] = {wx * kf, kf * wy, kf * wz};                  // [B+238h]
            const float lu2b = (u[1] * u[1] + u[0] * u[0]) + u[2] * u[2];
            const float lk2 = (kw[0] * kw[0] + kw[1] * kw[1]) + kw[2] * kw[2];
            if (lk2 > lu2b) {                                                 // FCOMIP/JBE: NaN keeps
                ++segments;
                steps = 0;
            }
            if (!loose) {
                // 004A2766: a third point off the rotated first point, across along v and along u.
                const float lv_a = static_cast<double>(lv2) > 1e-10 ? sqrt_00bf7030(lv2) : 0.0f;  // [B+A4h]
                std::uint8_t* const* const mslot = &at<std::uint8_t*>(ptr(t, 0x48), 4 * static_cast<std::size_t>(i));  // EDI
                const float lv_b = static_cast<double>(lv2) > 1e-10 ? sqrt_00bf7030(lv2) : 0.0f;  // [B+C4h]
                std::uint8_t* const m = *mslot;
                const float a = static_cast<float>((lv_b / at<float>(m, 0x20)) * 0.5);  // [B+20h] (DC C9)
                const float dir[3] = {v[0] / a, v[1] / a, v[2] / a};          // [B+12Ch]
                const float hi1 = static_cast<float>((lv_a / at<float>(m, 0x24)) * 0.5);
                const float r1 = static_cast<float>(host.random_uniform_float_00bd2f10(0, 0.0f, hi1) +
                                                    static_cast<double>(segments));  // 004A2853, FIADD
                std::uint8_t* const m2 = member(i);
                const float o1[3] = {dir[0] * r1, r1 * dir[1], r1 * dir[2]};   // [B+158h]
                const float r2 = static_cast<float>(
                    host.random_uniform_float_00bd2f10(0, 0.0f, at<float>(m2, 0x24) - at<float>(m2, 0x20)) +
                    steps * 0.5);                                             // 004A28E6, [00D7A280]
                const float o2[3] = {wx * r2, r2 * wy, r2 * wz};              // [B+148h]
                const float s[3] = {o2[0] + o1[0], o2[1] + o1[1], o2[2] + o1[2]};  // [B+180h]
                const float p[3] = {s[0] + points[0][0], s[1] + points[0][1], s[2] + points[0][2]};  // [B+30Ch]
                host.path_points_add_point_local_007ad330(ptr(last_node(list, host), 0x18), p);  // 004A29D3
                void* const path = ptr(last_node(list, host), 0x18);
                if (at<std::uint8_t>(path, 0x18) == 0) host.camera_path_derive_knots_007af150(path);  // 004A29FA
            }

            if (at<std::uint32_t>(member(i), 0xC) != 0) {                     // 004A2A0B
                const std::uint32_t count = at<std::uint32_t>(member(i), 0xC);
                const std::uint32_t pick = host.random_threads_next_u32_00bd2fc0(0) % count;  // DIV
                std::uint8_t* const mm = member(i);
                std::uint8_t* const entry_lo = at<std::uint8_t**>(mm, 8)[pick];  // EBP
                std::uint8_t* const entry_hi = at<std::uint8_t**>(mm, 8)[pick];  // EDI
                std::uint8_t* const nd = last_node(list, host);               // [B+2B4h]
                at<float>(nd, 0x3C) =
                    host.random_uniform_float_00bd2f10(0, at<float>(entry_lo, 8), at<float>(entry_hi, 4));  // 004A2A89
            }
            std::uint8_t* const ms = member(i);
            float start;                                                      // [B+20h] (FSTP double)
            if (row == 0) {
                start = host.random_uniform_float_00bd2f10(0, 0.0f, at<float>(ms, 0x28));  // 004A2AC1
            } else {
                start = static_cast<float>(host.random_uniform_float_00bd2f10(0, 0.25f, at<float>(ms, 0x28)) +
                                           static_cast<double>(starts[slot]));  // [00CE3868], FIADD
            }
            at<float>(last_node(list, host), 0x40) = start;                   // 004A2B0B
            starts[slot] = cvttss2si(at<float>(last_node(list, host), 0x40));  // 004A2B2B
            --row_left;
            ++slot;
            if (row_left <= 0) {                                              // 004A2B4B
                std::uint8_t* const mw = member(i);
                const float whi = static_cast<float>(at<std::int32_t>(mw, 0x34) + 1);
                const float wlo = static_cast<float>(at<std::int32_t>(mw, 0x30));
                row_left = ftol2_low_00bf7420(host.random_uniform_float_00bd2f10(0, wlo, whi));  // 004A2B7B
                ++row;
                slot = 0;
            }
            std::uint8_t* const mc = member(i);
            std::int32_t& c = counts[troop_member_group(mc)];
            c = cvttss2si(static_cast<float>(c) - at<float>(mc, 0x14));       // 004A2BB5..004A2BCC
        }

        if (loose) {
            // 004A2BE2: sort the list by the float at value +8 (0047B9F0), then give each node with
            // state +34h == 0 a front point and a position sampled along path b.
            std::uint8_t* const list = t + 0xB4 + 0xC * static_cast<std::size_t>(g_list);  // EDI
            host.list_sort_0049f6a0(list, 0x0047B9F0);                        // 004A2BFC
            std::int32_t r = 0;                                               // [B+14h]
            for (std::uint8_t* node = static_cast<std::uint8_t*>(at<void*>(ptr(list, 4), 0));;
                 node = static_cast<std::uint8_t*>(at<void*>(node, 0))) {
                if (node == ptr(list, 4)) break;                              // 004A2C14
                if (node == ptr(list, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
                if (at<std::int32_t>(node, 0x34) == 0) {
                    static constexpr float kIdentity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
                    std::memcpy(block, kIdentity, sizeof block);              // [B+1B4h]
                    float frame[16];                                          // [B+270h]
                    host.matrix_copy_004134f0(frame, reinterpret_cast<const float*>(block));  // 004A2CD8
                    const float r1 = host.random_uniform_float_00bd2f10(0, 0.0f, spacing);  // [B+20h]
                    const float end_b = at<float>(path_b, 0x1C);              // [B+10h]
                    const float t0 = int_times_float(r, spacing) + r1;
                    const float time = end_b > t0 ? t0 : end_b - r1;          // FCOMI/JBE: NaN subtracts
                    host.camera_path_sample_world_linear_007afd60(path_b, time, frame + 12, frame + 8);  // 004A2D49
                    host.matrix_orthonormalize_0085dad0(frame);               // 004A2D55
                    const float r2 = host.random_uniform_float_00bd2f10(0, 0.0f, spacing);  // 004A2D6C
                    const float o[3] = {r2 * frame[0], r2 * frame[1], r2 * frame[2]};  // [B+190h]
                    frame[12] = o[0] + frame[12];
                    frame[13] = frame[13] + o[1];
                    frame[14] = frame[14] + o[2];
                    if (at<std::uint8_t>(member(i), 0x38) == 0) {             // 004A2DF4
                        const float h[3] = {static_cast<float>(u[0] * 2.0), static_cast<float>(u[1] * 2.0),
                                            static_cast<float>(u[2] * 2.0)};  // [B+1A8h], [00D7A308]
                        frame[12] = h[0] + frame[12];
                        frame[13] = frame[13] + h[1];
                        frame[14] = frame[14] + h[2];
                    }
                    const float f[3] = {static_cast<float>(u[0] * 2.0), static_cast<float>(u[1] * 2.0),
                                        static_cast<float>(2.0 * u[2])};      // [B+1F4h]
                    const float front[3] = {f[0] + frame[12], frame[13] + f[1], frame[14] + f[2]};  // [B+2B8h]
                    if (node == ptr(list, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
                    host.path_points_add_point_local_007ad330(ptr(node, 0x18), front);  // 004A2EED
                    if (node == ptr(list, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
                    host.path_points_add_point_local_007ad330(ptr(node, 0x18), frame + 12);  // 004A2F07
                    if (node == ptr(list, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
                    at<float>(node, 0x28) = frame[8];
                    at<float>(node, 0x2C) = frame[9];
                    at<float>(node, 0x30) = frame[10];
                    if (node == ptr(list, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
                    if (at<std::uint8_t>(ptr(node, 0x18), 0x18) == 0)
                        host.camera_path_derive_knots_007af150(ptr(node, 0x18));  // 004A2F53
                    ++r;
                }
                if (node == ptr(list, 4)) host.crt_invalid_parameter_noinfo_00bf6713();  // 004A2F5D
            }
        }
        host.crt_free_00bf6989(starts);                                       // 004A2F73
        host.crt_free_00bf6989(matrices);                                     // 004A2F7D
    }
}

// 0059D4A0 __thiscall(ECX = HUD map screen; icon), RET 4. Frame SUB ESP 34h; B below is ESP after
// PUSH EDI. Precision: modelled at 24 bits (each x87 op rounded to float; the double constants
// 0.02f / 0.03f / 0.0 applied in double and rounded at the store). The caller 005ADDA0 is the HUD
// map icon update (a UI path); 53-bit would only change the unstored sums before their FSTPs.
void hud_map_icon_place_widget_0059d4a0(void* screen, void* icon, Big1Host& host) {
    std::uint8_t* const s = static_cast<std::uint8_t*>(screen);               // EDI
    std::uint8_t* const ic = static_cast<std::uint8_t*>(icon);                // ESI
    const float depth = at<std::uint8_t>(ic, 0x28) != 0 ? -2.0f : -12.0f;     // [00CE7D7C] / [00CE781C], into [B+40h]
    if (ptr(ic, 0xEC) == nullptr || (at<std::uint8_t*>(ic, 0xF0) - at<std::uint8_t*>(ic, 0xEC)) >> 2 == 0)
        host.crt_invalid_parameter_noinfo_00bf6713();                         // 0059D4E0: front() of the widgets
    const float* const size = host.gui_widget_get_size_00aa6740(at<void*>(ptr(ic, 0xEC), 0));  // 0059D4ED
    const float w = size[0];                                                  // [B+10h]
    const float h = size[1];                                                  // [B+14h]

    // [B+18h] the position handed to 00AA8240. Anchor modes other than 0 and 1 leave all three
    // floats unset: the image adds the offsets below to stale stack (what 005ADDA0's earlier calls
    // at the same depth left there) and passes it on; they are zero here.
    float position[3] = {0.0f, 0.0f, 0.0f};
    const std::int32_t anchor = at<std::int32_t>(ic, 0x11C);
    if (anchor == 0) {
        // 0059D511: the fixed map point +554h / +558h against the origin +54Ch / +550h.
        const float* const offset = host.map_offset_00e08890();
        const float x = (at<float>(s, 0x554) - at<float>(s, 0x54C)) / at<float>(s, 0x30) + offset[0];  // [B+24h]
        const float y = (at<float>(s, 0x550) - at<float>(s, 0x558)) / at<float>(s, 0x34) + offset[1];  // [B+28h]
        position[0] = x;
        position[1] = y;
        position[2] = depth;                                                  // 0059D5C3
    } else if (anchor == 1) {
        // 0059D556: the unit's position, read twice through vtable +C4h.
        float first[3];                                                       // [B+24h]
        const float z = host.unit_vtablec4_position(ptr(ic, 4), first)[2];    // [B+8h]
        const float y = (at<float>(s, 0x550) - z) / at<float>(s, 0x34) + host.map_offset_00e08890()[1];  // [B+Ch]
        float second[3];                                                      // [B+30h]
        const float x0 = host.unit_vtablec4_position(ptr(ic, 4), second)[0];  // 0059D596
        position[0] = (x0 - at<float>(s, 0x54C)) / at<float>(s, 0x30) + host.map_offset_00e08890()[0];
        position[1] = y;
        position[2] = depth;
    }

    // 0059D5C9: the corner, by the jump table at 0059D6BC over +120h (above 3: no offset, no depth
    // step). The 0.02 / 0.03 pads are the doubles [00D7A2F8] / [00CEB690] (0.02f, 0.03f widened).
    const double pad_x = static_cast<double>(0.02f);
    const double pad_y = static_cast<double>(0.03f);
    const std::uint32_t corner = at<std::uint32_t>(ic, 0x120);
    if (corner <= 3) {
        switch (corner) {
        case 0:                                                               // 0059D5DF
            position[0] = static_cast<float>(position[0] + pad_x);
            position[1] = static_cast<float>(position[1] + pad_y);
            break;
        case 1: {                                                             // 0059D5F9
            const float left = -static_cast<float>(w + pad_x);                // [B+24h], FCHS
            position[0] = left + position[0];
            position[1] = static_cast<float>(position[1] + pad_y);
            break;
        }
        case 2: {                                                             // 0059D621
            const float down = -static_cast<float>(h + pad_y);                // [B+28h]
            position[0] = static_cast<float>(position[0] + pad_x);
            position[1] = down + position[1];
            break;
        }
        default: {                                                            // 3, 0059D63D
            const float left = -static_cast<float>(w + pad_x);
            const float down = -static_cast<float>(h + pad_y);
            position[0] = left + position[0];
            position[1] = down + position[1];
            break;
        }
        }
        position[2] = static_cast<float>(position[2] + 0.0);                  // 0059D679 [00D7A258]
    }
    if (ptr(ic, 0xEC) == nullptr || (at<std::uint8_t*>(ic, 0xF0) - at<std::uint8_t*>(ic, 0xEC)) >> 2 == 0)
        host.crt_invalid_parameter_noinfo_00bf6713();                         // 0059D69A
    host.gui_widget_set_resolved_position_00aa8240(at<void*>(ptr(ic, 0xEC), 0), position);  // 0059D6AC
}

} // namespace bsp::cc13_big1
