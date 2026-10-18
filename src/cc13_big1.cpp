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

} // namespace bsp::cc13_big1
