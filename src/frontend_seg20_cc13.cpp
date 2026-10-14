// cc13-seg1b, segment 20 (docs/CC13_SEG20.md). Evidence per function in
// include/bsp/frontend_seg20_cc13.hpp.
#include "bsp/frontend_seg20_cc13.hpp"

#include "bsp/frontend_screens_cc13.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Segment 20 reconstruction reads Win32 image storage.
#endif

namespace bsp::frontend_seg20_cc13 {

namespace {

constexpr double kTextWidthScale = 960.0;            // 00CEC380
constexpr double kTextHeightPad = 0.011111111111111112;  // 00CEED60
constexpr double kTextLeftPad = 0.13333333333333333;   // 00CF0DF0
constexpr double kHalf = 0.5;                          // 00D7A280
constexpr std::uint32_t kVehicleClassMapSize = 0x800;   // 005926BE

}  // namespace

void main_menu_layout_mission_row_00581ce0(void* screen, std::uint32_t list, Seg20Host& host) {
    const Raw s = raw(screen);
    const std::uint32_t index = host.global_u32(0x00E194DC);         // 00581CE8
    const Raw points = raw(s.p + 0x134 + list * 0x10);                // 00581CF5
    const auto* begin = static_cast<const std::uint8_t*>(points.ptr(4));
    const auto* end = static_cast<const std::uint8_t*>(points.ptr(8));
    // 00581CFC..00581D1C: begin null or index >= (end - begin) / 12.
    if (begin == nullptr || index >= static_cast<std::uint32_t>((end - begin) / 12)) {
        host.invalid_parameter_00bf6713();
    }
    const float* point = reinterpret_cast<const float*>(begin + index * 12);
    const float zoom = s.at<float>(0x130);                             // 00581D24 copied as float
    // 00581D31..00581D55: x*zoom, y*zoom, zoom*z, each stored to float.
    s.at<float>(0x184) = static_cast<float>(static_cast<double>(point[0]) * zoom);
    s.at<float>(0x188) = static_cast<float>(static_cast<double>(point[1]) * zoom);
    s.at<float>(0x18C) = static_cast<float>(static_cast<double>(zoom) * point[2]);
    s.at<float>(0x18C) = 0.0f;                                         // 00581D77 MOVSS XMM0 (XORPS)
    const float sx = static_cast<float>(static_cast<double>(s.at<float>(0x1A4)) + s.at<float>(0x190));
    const float sy = static_cast<float>(static_cast<double>(s.at<float>(0x1A8)) + s.at<float>(0x194));
    const float sz = static_cast<float>(static_cast<double>(s.at<float>(0x1AC)) + s.at<float>(0x198));
    s.at<float>(0x184) = static_cast<float>(static_cast<double>(sx) + s.at<float>(0x184));  // 00581DB9
    s.at<float>(0x188) = static_cast<float>(static_cast<double>(sy) + s.at<float>(0x188));  // 00581DC9
    s.at<float>(0x18C) = static_cast<float>(static_cast<double>(s.at<float>(0x18C)) + sz);  // 00581DD9
}

void main_menu_listener_current_1c_00588280(void* screen, Seg20Host& host) {
    const Raw s = raw(screen);
    if (s.ptr(0x2E8) != nullptr) {                                     // 0058828D
        const std::array<float, 3> p = host.widget_resolved_position_00aa6750(s.ptr(0x2E8));
        s.at<float>(0xE4) = p[0];
        s.at<float>(0xE8) = p[1];
        s.at<float>(0xEC) = p[2];
    }
    if (s.ptr(0x178) != nullptr) {                                     // 005882BD
        const std::array<float, 3> p = host.widget_resolved_position_00aa6750(s.ptr(0x178));
        s.at<float>(0x518) = p[0];
        s.at<float>(0x51C) = p[1];
        s.at<float>(0x520) = p[2];
    }
    // 005882E5..00588346: five vectors {proxy, begin, end} 10h apart from +F4h.
    // erase(begin, end): 00419330 moves the empty range [end, end) to begin and
    // returns begin, which becomes the new end. The checked-iterator tests
    // (begin > end, a null vector) cannot fire on a valid vector.
    for (std::size_t i = 0; i < 5; ++i) {
        const std::size_t off = 0xF4 + i * 0x10;
        if (s.ptr(off + 4) != s.ptr(off + 8)) s.ptr(off + 8) = s.ptr(off + 4);
    }
    host.scroller_recapture_006833f0(s.p + 0x194);                     // 0058834E
    host.scroller_recapture_006833f0(s.p + 0x314);                     // 00588360 JMP
}

void* main_menu_screen_scalar_deleting_dtor_00590d60(void* screen, std::uint8_t flags, Seg20Host& host) {
    host.main_menu_screen_destruct_00590600(screen);                   // 00590D63
    if ((flags & 1) != 0) host.free_00bf65ac(screen);                  // 00590D70
    return screen;
}

void main_menu_on_play_mission_prompt_result_00592640(void* screen, std::int32_t result, Seg20Host& host) {
    const Raw s = raw(screen);
    if (result != 1) {
        s.at<std::uint8_t>(0x5C) = 0;                                  // 0059271E
        host.build_mission_detail_page_0058c010(screen);               // 00592722
        return;
    }
    s.at<std::uint8_t>(0x5C) = 1;                                      // 00592667
    // 00592677..00592684: 007FC490(ECX = [00E188A8]+650h, &record, 005806A0(screen)).
    const cc13_seg49_64_78::ProfileCheckpointRecord record =
        host.checkpoint_get_copy_007fc490(host.selected_mission_005806a0(screen));
    const Raw game = raw(reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(0x00E188A8))));
    game.at<std::uint32_t>(0x6AC) = record.words[0];                  // 0059269B
    const Raw registry = raw(host.vehicle_class_registry_00437f50());  // 005926A1
    for (std::uint32_t i = 0; i < kVehicleClassMapSize; ++i) {        // 005926B0..005926C4
        registry.at<std::uint32_t>(0x10 + i * 4) = i;
        registry.at<std::uint32_t>(0x2010 + i * 4) = i;
    }
    const Raw again = raw(host.vehicle_class_registry_00437f50());     // 005926C6
    again.at<std::uint32_t>(0x10 + record.words[1] * 4) = record.words[2];    // 005926D3
    again.at<std::uint32_t>(0x2010 + record.words[2] * 4) = record.words[1];  // 005926D7
    host.play_selected_mission_005922f0(screen);                       // 005926E0
}   // 005926E5..00592707: the record's NativeString is released (std::string here)

void narrative_set_portrait_005b5c10(void* screen, void* texture, Seg20Host& host) {
    if (texture == nullptr) return;                                    // 005B5C16
    host.widget_vslot88(raw(screen).ptr(0x40), texture, 0, 1.0f);       // 005B5C2C, FLD1
}

void narrative_query_text_005b5d20(void* screen, Seg20Host& host) {
    std::uint8_t local[16];
    host.widget_vslot54(raw(screen).ptr(0xB0), local);                 // 005B5D32
}

void narrative_show_text_005b7390(void* screen, std::uint32_t source, float* x, float* y, float* w,
                                  Seg20Host& host) {
    const Raw s = raw(screen);
    host.widget_vslot34(s.ptr(0xAC), 1);                               // 005B73A3
    host.text_set_localised_cstring_00abbe50(s.ptr(0xB0), "", 1);       // 005B73B2, 00CE3A0C ""
    host.text_set_localised_source_00abaed0(s.ptr(0xB0), source, 1);    // 005B73C4
    void* text = s.ptr(0xB0);
    // 005B73CF..005B73DB: stored to float in the argument slot.
    const float t = static_cast<float>(static_cast<double>(raw(text).at<float>(0x114)) / kTextWidthScale);
    const double height = host.text_normalized_height_00ab6bd0(text);  // 005B73DF
    float xy[2];
    const float y_pad = static_cast<float>(height + kTextHeightPad);   // 005B73F5
    xy[0] = static_cast<float>(static_cast<double>(t) + kTextLeftPad);  // 005B7403
    xy[1] = y_pad;                                                     // 005B740B
    host.widget_vslot58(s.ptr(0xB4), xy);                              // 005B7414
    std::uint8_t local[16];
    host.widget_vslot54(s.ptr(0xB0), local);                           // 005B7426, dropped
    const Raw gui = raw(raw(reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(0x00E198C4)))).ptr(0xA4));
    const Raw text_now = raw(s.ptr(0xB0));
    *x = static_cast<float>(static_cast<double>(gui.at<float>(0x78)) * kHalf);           // 005B7448
    *y = static_cast<float>(static_cast<double>(text_now.at<std::int32_t>(0xEC)) * gui.at<float>(0x7C) +
                            gui.at<float>(0x80));                                       // 005B745D
    *w = gui.at<float>(0x78);                                                          // 005B7466
}

void hud_map_icon_observer_on_released_005a5920(void* screen, std::uint32_t key, Seg20Host& host) {
    const Raw s = raw(screen);
    void* map = s.p + 0x490;
    void* node = host.hud_map_icon_find_005a1800(map, key);           // 005A5943
    void* head = s.ptr(0x494);                                         // 005A594E, read after the find
    // 005A594C..005A595B / 005A596A..005A5978: checked-iterator tests, not modelled.
    if (node != head) {
        const Raw n = raw(node);
        if (n.ptr(0x10) != nullptr) {
            host.delete_object_vslot00(n.ptr(0x10), 1);                // 005A598A
            n.ptr(0x10) = nullptr;                                     // 005A598C
        }
        host.hud_map_icon_erase_00537f20(map, node);                   // 005A599C
    }
    s.at<std::uint8_t>(0x534) = 1;                                     // 005A59A3
}

void minimap_icon_observer_on_released_005bfdf0(void* screen, std::uint32_t key, Seg20Host& host) {
    const Raw s = raw(screen);
    void* map = s.p + 0xF0;
    void* iter[2] = {nullptr, nullptr};
    cc13_fe1::hud_minimap_icon_map_find_005be110(map, iter, &key);     // 005BFE11, iter = {container, node}
    void* node = iter[1];
    void* head = s.ptr(0xF4);                                          // 005BFE1C
    if (node != head) {
        const Raw n = raw(node);
        if (n.ptr(0x10) != nullptr) host.delete_object_vslot00(n.ptr(0x10), 1);  // 005BFE52
        host.minimap_icon_erase_005bf810(map, node);                   // 005BFE5D
    }
}

namespace {

constexpr double kIconPad = 52.0;            // 00CEFFC0
constexpr double kScreenHeight = 720.0;      // 00CEF1B8
constexpr double kStatusThreshold = 0.8999999761581421;  // 00D7A390 (0.9f as double)
constexpr double kLabelDepth = 10.0;         // 00CE3DC0

float global_f32(Seg20Host& host, std::uint32_t address) {
    const std::uint32_t bits = host.global_u32(address);
    float value;
    static_assert(sizeof value == sizeof bits);
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

// The checked `widgets[0]` of the 4-byte vector at +ECh/+F0h (00BF6713 when
// the vector is null or empty), then the element.
void* first_widget(const Raw& icon, Seg20Host& host) {
    const auto* begin = static_cast<void* const*>(icon.ptr(0xEC));
    const auto* end = static_cast<void* const*>(icon.ptr(0xF0));
    if (begin == nullptr || end - begin == 0) host.invalid_parameter_00bf6713();
    return begin[0];
}

// Truncate (CVTTSS2SI) and keep the even part: CDQ / SUB / SAR 1 / ADD.
float even_truncated(float value) {
    const std::int32_t n = static_cast<std::int32_t>(value);
    return static_cast<float>((n / 2) * 2);
}

}  // namespace

bool hud_map_icon_update_size_and_visibility_0059cef0(void* icon, bool invert, Seg20Host& host) {
    const Raw s = raw(icon);
    float t = 1.0f;                                                    // 0059CEF3 [00D7A24C]
    bool at_rest = false;
    const float a = s.at<float>(0x110);
    const float g = global_f32(host, 0x00F876A4);
    if (g >= a) {                                                      // 0059CF1B FCOMI / JC
        at_rest = true;
        s.at<float>(0x100) = s.at<float>(0xF8);
        s.at<float>(0x104) = s.at<float>(0xFC);
    } else {
        const float b = s.at<float>(0x114);
        const float num = static_cast<float>(static_cast<double>(g) - b);   // 0059CF56
        const float den = static_cast<float>(static_cast<double>(a) - b);   // 0059CF62
        t = static_cast<float>(static_cast<double>(num) / den);             // 0059CF6A
        const float dx = static_cast<float>(static_cast<double>(s.at<float>(0xF8)) - s.at<float>(0x108));
        const float dy = static_cast<float>(static_cast<double>(s.at<float>(0xFC)) - s.at<float>(0x10C));
        const float px = static_cast<float>(static_cast<double>(dx) * t);   // 0059CF9C
        const float py = static_cast<float>(static_cast<double>(t) * dy);   // 0059CFA4
        s.at<float>(0x100) = static_cast<float>(static_cast<double>(s.at<float>(0x108)) + px);
        s.at<float>(0x104) = static_cast<float>(static_cast<double>(s.at<float>(0x10C)) + py);
        const float ex = even_truncated(s.at<float>(0x100));               // 0059CFD8
        const float ey = even_truncated(s.at<float>(0x104));               // 0059CFEB
        s.at<float>(0x100) = ex;
        s.at<float>(0x104) = ey;
    }
    void* widget = first_widget(s, host);                               // 0059D00E..0059D030
    const std::array<float, 2> size = host.widget_size_00aa6740(widget);  // 0059D032
    float n[2];
    n[0] = static_cast<float>((static_cast<double>(s.at<float>(0x100)) + kIconPad) / kTextWidthScale);
    n[1] = static_cast<float>((kIconPad + s.at<float>(0x104)) / kScreenHeight);
    if (size[0] != n[0] || size[1] != n[1]) {                          // 0059D07A / 0059D08C FUCOMIP
        host.widget_vslot58(first_widget(s, host), n);                  // 0059D0C4
    }
    host.widget_vslot34(first_widget(s, host), s.at<std::uint8_t>(0x2A));  // 0059D0F4
    // 0059D0FB..0059D131: FLD t; invert -> 1 - t; stored as double, then float.
    const double alpha = invert ? 1.0 - static_cast<double>(t) : static_cast<double>(t);
    host.widget_vslot4c(first_widget(s, host), static_cast<float>(alpha));  // 0059D142
    return at_rest;                                                    // 0059D145 MOV AL,BL
}

void hud_map_icon_query_unit_status_0059d150(void* icon, std::uint8_t* b, std::uint8_t* c, std::uint8_t* d, std::uint8_t* a,
                                             Seg20Host& host) {
    *a = 0;                                                            // 0059D163
    *d = 0;
    *c = 0;
    *b = 0;                                                            // 0059D16F
    void* unit = raw(icon).ptr(4);
    if (host.is_a_vslot5c(unit, 8)) {                                  // 0059D17D
        *a = raw(raw(icon).ptr(4)).at<std::int32_t>(0x122C) == 2 ? 1 : 0;  // SETZ
    } else {
        *a = raw(raw(icon).ptr(4)).at<std::uint8_t>(0x9E5);            // the byte is copied as is
    }
    for (void* m = raw(raw(icon).ptr(4)).ptr(0x48); m != nullptr; m = raw(m).ptr(0x44)) {
        if (host.is_a_vslot5c(m, 0x20) && raw(m).at<std::uint8_t>(0x5D) != 0) {  // 0059D1B9 / 0059D1BF
            *d = 1;
            break;
        }
    }
    // 00939F80 / 00939F70 on unit+A20h are FLD [ECX+34h] / [ECX+38h]; RET:
    // inlined here as the field reads they are.
    if (static_cast<double>(raw(raw(icon).ptr(4)).at<float>(0xA20 + 0x34)) > kStatusThreshold) *b = 1;
    if (static_cast<double>(raw(raw(icon).ptr(4)).at<float>(0xA20 + 0x38)) > kStatusThreshold) *c = 1;
}

void hud_map_label_center_on_widget_0059d350(void* label, Seg20Host& host) {
    const Raw s = raw(label);
    const float w = static_cast<float>(static_cast<double>(raw(s.ptr(0x124)).at<float>(0x114)) / kTextWidthScale);
    const float h = static_cast<float>(host.text_normalized_height_00ab6bd0(s.ptr(0x124)));  // 0059D36C
    const std::array<float, 2> size = host.widget_size_00aa6740(s.ptr(0x120));             // 0059D37B
    const float dx = static_cast<float>((static_cast<double>(size[0]) - w) * kHalf);       // 0059D39A
    const std::array<float, 2> size2 = host.widget_size_00aa6740(s.ptr(0x120));            // 0059D39E
    const float dy = static_cast<float>((static_cast<double>(size2[1]) - h) * kHalf);      // 0059D3BB
    const std::array<float, 3> p = host.widget_resolved_position_00aa6750(s.ptr(0x120));   // 0059D3BF
    float xyz[3];
    xyz[1] = static_cast<float>(static_cast<double>(p[1]) + dy);                         // 0059D3D6
    xyz[2] = static_cast<float>(static_cast<double>(p[2]) - kLabelDepth);                // 0059D3E3
    xyz[0] = static_cast<float>(static_cast<double>(p[0]) + dx);                         // 0059D3ED
    host.widget_set_resolved_position_00aa8240(s.ptr(0x124), xyz);                       // 0059D401
}

namespace {

constexpr double kEdgePad = 60.0;   // 00CE3D68
constexpr double kZero = 0.0;       // 00D7A258

Raw game_of(Seg20Host& host) {
    return raw(reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(0x00E188A8))));
}

// 0059D410 reads the layout byte [[0109CF04]+0Dh] itself on every call.
bool wide_layout(Seg20Host& host) {
    return raw(reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(0x0109CF04)))).at<std::uint8_t>(0x0D) != 0;
}

}  // namespace

bool hud_map_icon_update_offscreen_edge_0059d6d0(void* screen, void* icon, Seg20Host& host) {
    const Raw ic = raw(icon);
    const std::array<float, 3> p = host.widget_resolved_position_00aa6750(first_widget(ic, host));  // 0059D704
    const float nx = static_cast<float>((static_cast<double>(ic.at<float>(0xF8)) + kEdgePad) / kTextWidthScale);
    const float ny = static_cast<float>((kEdgePad + ic.at<float>(0xFC)) / kScreenHeight);
    float q[3];
    q[0] = static_cast<float>(static_cast<double>(p[0]) + nx);       // 0059D742
    q[1] = static_cast<float>(static_cast<double>(ny) + p[1]);       // 0059D74E
    q[2] = static_cast<float>(static_cast<double>(p[2]) + kZero);    // 0059D75C
    const float pp[3] = {p[0], p[1], p[2]};
    if (cc13_fe1::hud_map_is_point_inside_map_area_0059d410(pp, wide_layout(host)) &&   // 0059D760
        cc13_fe1::hud_map_is_point_inside_map_area_0059d410(q, wide_layout(host))) {   // 0059D770
        ic.at<std::int32_t>(0x120) = 0;                               // 0059D77A
        return true;
    }
    ic.at<std::int32_t>(0x120) =
        cc13_fe1::hud_map_classify_offscreen_edge_0059a910(icon, reinterpret_cast<const float*>(raw(screen).p + 0x54C),
                                                           host);    // 0059D796
    return false;
}

bool hud_map_is_unit_shown_at_zoom_0059d940(void* screen, void* unit, Seg20Host& host) {
    const Raw s = raw(screen);
    if (host.is_a_vslot5c(unit, 0x1C)) return true;                    // 0059D951
    {
        const Raw game = game_of(host);
        void* szurke = game.ptr(0x21A4 + game.at<std::int32_t>(0x18EC) * 4);  // 0059D969
        if (host.szurke_contains_unit_008ddf90(szurke, unit) && s.at<std::int32_t>(0x2C) >= 1) return true;
    }
    if (host.is_a_vslot5c(unit, 0x19) || host.is_a_vslot5c(unit, 0x1B) || host.is_a_vslot5c(unit, 0x1A) ||
        host.is_a_vslot5c(unit, 0x0F)) {
        return s.at<std::int32_t>(0x2C) >= 2;                          // 0059DA72
    }
    if (host.is_a_vslot5c(unit, 0x18)) return s.at<std::int32_t>(0x2C) < 2;  // 0059D9DD
    if (host.is_a_vslot5c(unit, 0x06) && host.unit_is_formation_follower_007788b0(unit)) {
        return s.at<std::int32_t>(0x2C) >= 1;                          // 0059DA05
    }
    if (!host.is_a_vslot5c(unit, 0x2B) && !host.is_a_vslot5c(unit, 0x34)) return true;  // 0059DA2F
    if (s.at<std::int32_t>(0x2C) < 2) return false;                    // 0059DA39
    const Raw game = game_of(host);
    const Raw side_record = raw(game.ptr(0x18CC + game.at<std::int32_t>(0x18EC) * 4));
    return host.unit_side_record_value_00521f10(raw(unit).p + 0x1E4, side_record.at<std::uint32_t>(0x28)) >= 2;
}

void* generated_model_find_material_by_effect_name_005a07b0(void* model, const char* name_data, Seg20Host& host) {
    for (std::uint32_t i = 0;
         i < host.mesh_count58_00b72b40(host.generated_model_geometry_00b74640(model, 0));  // 005A07BE / 005A081D
         ++i) {
        void* element = host.mesh_element_00b732c0(host.generated_model_geometry_00b74640(model, 0), i);  // 005A07DC
        void* material = raw(element).ptr(0x20);
        const char* effect = host.effect_name_data_00b172d0(raw(material).ptr(0x7C));  // 005A07E7
        // 005A07EE..005A0814: name data read first, then both null tests.
        const char* pattern = name_data;
        if (effect != nullptr && pattern != nullptr) {
            const char* hit = host.strstr_00bf9440(effect, pattern);   // 005A0802
            if (hit != nullptr && (hit - effect) != -1) return material;  // 005A0835 MOV EAX,EDI
        }
    }
    return nullptr;                                                    // 005A0830
}

void* hud_map_find_icon_carrying_unit_005a1220(void* screen, void* object, Seg20Host& host) {
    const Raw s = raw(screen);
    void* node = raw(s.ptr(0x4F8)).ptr(0);                             // 005A1232: head->left (begin)
    // The checked-iterator tests (00BF6713) only fire on a corrupt tree.
    while (node != s.ptr(0x4F8)) {                                     // 005A1242 / 005A1250
        void* unit = raw(raw(node).ptr(0x10)).ptr(4);
        void* found;
        if (host.is_a_vslot5c(unit, 0x46)) {                           // 005A1278
            found = host.shipyard_first_live_hangar_object_00845750(raw(raw(node).ptr(0x10)).ptr(4));
        } else if (host.is_a_vslot5c(raw(raw(node).ptr(0x10)).ptr(4), 0x45)) {  // 005A12AC
            found = host.airfield_find_usable_hangar_object_006d2730(raw(raw(node).ptr(0x10)).ptr(4));
        } else {
            node = host.tree_next_00535110(node);                      // 005A12D1
            continue;
        }
        if (found == object) return raw(node).ptr(0x10);              // 005A12C7 / 005A12ED
        node = host.tree_next_00535110(node);
    }
    return nullptr;                                                    // 005A12FD
}

void hud_map_icon_array_resize_0059ead0(void* slot, std::int32_t count, Seg20Host& host) {
    const Raw s = raw(slot);
    const std::int32_t current = s.ptr(0) != nullptr ? raw(s.ptr(0)).at<std::int32_t>(4) : 0;  // 0059EAD3..0059EADF
    if (count == current) return;                                      // 0059EAE7
    host.icon_array_reserve_0059e690(slot, count);                     // 0059EAEA
    raw(s.ptr(0)).at<std::int32_t>(4) = count;                         // 0059EAEF..0059EAF1
}

void* minimap_marker_construct_005bcac0(void* marker, void* unit, Seg20Host& host) {
    const Raw m = raw(marker);
    m.at<std::uint32_t>(0) = kMinimapMarkerVtable;                     // 005BCACA
    const auto layout = [&host]() {
        return raw(raw(reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(0x00E198C4)))).ptr(0x58));
    };
    std::size_t slot;
    if (unit == nullptr) {
        slot = 0x7C;                                                   // 005BCAD2
    } else if (host.is_a_vslot5c(unit, 8)) {
        slot = 0x74;                                                   // 005BCAEE
    } else if (host.is_a_vslot5c(unit, 6)) {
        slot = 0x6C;                                                   // 005BCB0B
    } else if (host.is_a_vslot5c(unit, 0x0F)) {
        slot = 0x70;                                                   // 005BCB27
    } else {
        slot = 0x78;                                                   // 005BCB35
    }
    m.ptr(4) = host.widget_clone_00aab4c0(layout().ptr(slot), 0);      // 005BCB42
    host.widget_add_child_00aaa5a0(layout().ptr(0xF0), m.ptr(4), 0);   // 005BCB5D
    host.widget_vslot34(m.ptr(4), 1);                                  // 005BCB6C
    return marker;
}

void* minimap_acquire_marker_005c0930(void* screen, void* unit, Seg20Host& host) {
    const Raw s = raw(screen);
    std::size_t k;
    if (unit == nullptr) {
        k = 0;
    } else if (host.is_a_vslot5c(unit, 8)) {                           // 005C0A0E
        k = 1;
    } else if (host.is_a_vslot5c(unit, 6)) {                           // 005C0A84
        k = 2;
    } else if (host.is_a_vslot5c(unit, 0x0F)) {                        // 005C0AD8
        k = 3;
    } else {
        k = 4;                                                         // 005C0B26
    }
    std::int32_t& counter = s.at<std::int32_t>(0x154 + k * 4);
    counter += 1;
    const std::uint32_t index = static_cast<std::uint32_t>(counter);
    const Raw vec = raw(s.p + 0x104 + k * 0x10);
    const auto* begin = static_cast<void* const*>(vec.ptr(4));
    const auto* end = static_cast<void* const*>(vec.ptr(8));
    if (begin != nullptr && index < static_cast<std::uint32_t>(end - begin)) {  // JC, unsigned
        if (k == 0) {
            // 005C09D6..005C09EE: the inline checked index, then the element.
            if (vec.ptr(4) == nullptr ||
                index >= static_cast<std::uint32_t>(static_cast<void* const*>(vec.ptr(8)) -
                                                    static_cast<void* const*>(vec.ptr(4)))) {
                host.invalid_parameter_00bf6713();
            }
            return static_cast<void* const*>(vec.ptr(4))[index];
        }
        return *host.pointer_vector_at_005bcbd0(vec.p, index);         // 005C0BA8
    }
    void* memory = host.operator_new_00bf681b(8);                      // 005C0981 / 005C0A47 / ...
    void* marker = memory != nullptr ? minimap_marker_construct_005bcac0(memory, k == 0 ? nullptr : unit, host)
                                     : nullptr;
    host.pointer_vector_push_back_005c0790(vec.p, &marker);            // 005C09BB / 005C0B8A
    return marker;
}

}  // namespace bsp::frontend_seg20_cc13
