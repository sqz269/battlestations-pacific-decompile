// Support-manager radial menus. Evidence: docs/CC13_RADIAL_MENUS.md.
#include "bsp/radial_menu_cc13.hpp"

namespace bsp::radial_cc13 {
namespace {

constexpr std::string_view kIconL = "icon_L_Icon";                  // 00CF4DDC
constexpr std::string_view kCircleHL = "circle_HL_Icon";            // 00CF4DC0
constexpr std::string_view kCircleL = "circle_L_Icon";              // 00CF670C
constexpr std::string_view kIconS = "icon_s_Icon";                  // 00CF6980
constexpr std::string_view kCircleS = "circle_s_Icon";              // 00CF698C
constexpr std::string_view kSmall01 = "circle_small_01_Group";      // 00CF4DA8
constexpr std::string_view kSmall02 = "circle_small_02_Group";      // 00CF4D90

constexpr double kScrollRate = 4.0;          // 00D7A328 (double)
constexpr double kSlotSpan = 5.0;            // 00D7A370 (double)
constexpr double kOne = 1.0;                 // 00D7A210 (double)
constexpr double kDimThreshold = 0.6000000238418579;  // 00CEFF98 (double of 0.6f)
constexpr float kDimAlpha = 0.6f;            // 00CE3D30
constexpr float kRemovalFade = 1.0f;         // 00D7A24C
constexpr double kDiagStepX = 0.04500000178813934;  // 00CF69A0 (double)
constexpr double kDiagStepY = 0.01899999938905239;  // 00CED088 (double)

// Record flags read through the record's vslot +18h.
constexpr std::int32_t kFlagHasLoadouts = 0x0F;
constexpr std::int32_t kFlagNoDefaultEntry = 0x10;

float f32(double value) { return static_cast<float>(value); }

// The scroll step shared by both rings: t - dt * 4.0 (double
// product, one rounding to float), clamped at 0 (00661B89..00661BAF,
// 00668D62..00668D8A).
void step_scroll(float& t, float dt) {
    t = f32(static_cast<double>(t) - static_cast<double>(dt) * kScrollRate);
    if (0.0f > t) {
        t = 0.0f;
    }
}

// The loadout ring builder shared by 00665F30 (record +3A4h) and 00666DC0
// (record +2A0h). The two listings differ only in the record field, the frame
// size and one repeated highlight call in 00665F30's first flag-10h entry
// (00666683..006666CB), which sets the same visibility again.
void build_loadout_ring(RadialScreen& screen, RadialHost& host, Record record, bool repeat_highlight) {
    clear_loadout_icons_0065abb0(screen, host);  // 00665F57
    if (!host.record_has_flag(record, kFlagHasLoadouts)) {  // 00665F67
        return;
    }
    if (host.record_loadout_count(record) == 0) {  // 00665F79
        return;
    }
    const std::int32_t quad = screen.quadrant_314;
    if (!host.record_has_flag(record, kFlagNoDefaultEntry)) {  // 00665F91
        // 00665FA0..00666244: the default entry (icon state 0, no loadout).
        screen.loadout_icons_15c.push_back(host.clone_subtree(screen.loadout_first_108[quad]));
        const Widget front = screen.loadout_icons_15c.front();
        host.set_icon_state(host.find_child(front, kIconS), 0, 0, 1.0f);
        host.set_alpha(host.find_child(front, kCircleHL), 0.0f);
        host.set_alpha(host.find_child(front, kCircleS), 0.0f);
        host.set_alpha(host.find_child(front, kIconS), 0.0f);
        host.set_visible(host.find_child(front, kCircleHL), screen.loadout_highlight_334 == 0);
        host.set_visible(front, true);
    }
    // 00666246..00666DA3: loadouts 1..record+128h, the count re-read each pass.
    for (std::int32_t i = 1; i <= host.record_loadout_count(record); ++i) {
        const bool no_default = host.record_has_flag(record, kFlagNoDefaultEntry);
        if (no_default && i == 1) {
            // 00666294..006666E4: loadout 1 takes the first-entry template.
            screen.loadout_icons_15c.push_back(host.clone_subtree(screen.loadout_first_108[quad]));
            void* const texture = host.loadout_small_texture(record, 1);
            const std::array<float, 4> uv = host.loadout_small_uv(record, 1);
            const Widget front = screen.loadout_icons_15c.front();
            host.icon_add_state(host.find_child(front, kIconS), texture, uv);
            host.set_icon_state(host.find_child(front, kIconS), 1, 0, 1.0f);
            host.set_alpha(host.find_child(front, kCircleHL), 0.0f);
            host.set_alpha(host.find_child(front, kCircleS), 0.0f);
            host.set_alpha(host.find_child(front, kIconS), 0.0f);
            host.set_visible(host.find_child(front, kCircleHL), screen.loadout_highlight_334 == 1);
            host.set_visible(front, true);
            if (repeat_highlight) {
                host.set_visible(host.find_child(front, kCircleHL), screen.loadout_highlight_334 == 1);
            }
            continue;
        }
        // 006666E9..00666D8B: later loadouts take the second template, laid
        // out along a line from the first template's resolved position.
        std::int32_t j = i;
        if (host.record_has_flag(record, kFlagNoDefaultEntry)) {
            j = i - 1;
        }
        screen.loadout_icons_15c.push_back(host.clone_subtree(screen.loadout_rest_118[quad]));
        const Widget back = screen.loadout_icons_15c.back();
        void* const texture = host.loadout_small_texture(record, i);
        const std::array<float, 4> uv = host.loadout_small_uv(record, i);
        host.icon_add_state(host.find_child(back, kIconS), texture, uv);
        host.set_icon_state(host.find_child(back, kIconS), 1, 0, 1.0f);
        const Vec3 origin = host.resolved_position(screen.loadout_first_108[quad]);  // 00666932
        const double k = f32(static_cast<double>(j - 1));  // FILD, FSTP float
        Vec3 at = origin;
        bool place = true;
        switch (quad) {
        case 0:  // 00666941
            at.x = f32(k * screen.line_step_x_290 + origin.x);
            at.y = f32(k * screen.line_step_y_294 + origin.y);
            break;
        case 1:  // 006669C4
            at.x = f32(k * kDiagStepX + origin.x);
            at.y = f32(k * kDiagStepY + origin.y);
            break;
        case 2:  // 00666A31
            at.x = f32(k * kDiagStepX + origin.x);
            at.y = f32(static_cast<double>(origin.y) - k * kDiagStepY);
            break;
        case 3:  // 00666A9F
            at.x = f32(k * screen.line_step_x_290 + origin.x);
            at.y = f32(static_cast<double>(origin.y) - k * screen.line_step_y_294);
            break;
        default:  // any other quadrant leaves the clone where the template is
            place = false;
            break;
        }
        if (place) {
            host.set_resolved_position(back, at);  // 00666B14
        }
        host.set_visible(back, true);
        host.set_visible(host.find_child(back, kCircleHL), i == screen.loadout_highlight_334);
        host.set_alpha(host.find_child(back, kCircleHL), 0.0f);
        host.set_alpha(host.find_child(back, kCircleS), 0.0f);
        host.set_alpha(host.find_child(back, kIconS), 0.0f);
    }
}

// The highlight pass of 00662020 (record +3A4h, loop bound: the icon count)
// and 00662210 (record +2A0h, loop bound: record+128h inclusive).
void highlight_icon(RadialScreen& screen, RadialHost& host, Record record, std::uint32_t index) {
    const Widget icon = screen.loadout_icons_15c[index];
    bool on;
    if (host.record_has_flag(record, kFlagNoDefaultEntry)) {
        on = index == static_cast<std::uint32_t>(screen.loadout_highlight_334 - 1);
    } else {
        on = index == static_cast<std::uint32_t>(screen.loadout_highlight_334);
    }
    host.set_visible(host.find_child(icon, kCircleHL), on);
}

}  // namespace

void set_large_icon_alpha_0065e870(RadialHost& host, Widget widget, float alpha) {
    host.set_alpha(host.find_child(widget, kIconL), alpha);
    host.set_alpha(host.find_child(widget, kCircleHL), alpha);
    host.set_alpha(host.find_child(widget, kCircleL), alpha);
}

void clear_loadout_icons_0065abb0(RadialScreen& screen, RadialHost& host) {
    for (const Widget icon : screen.loadout_icons_15c) {
        if (icon != 0) {
            host.destroy(icon);  // 0065ABE9
        }
    }
    screen.loadout_icons_15c.clear();  // 0065ABF9..0065AC23, _free per node
}

void prune_slots_006605c0(RadialScreen& screen, RadialHost& host) {
    if (screen.list_b_count_34c == 0) {  // 006605EE
        return;
    }
    std::array<std::int32_t, 4> kept{};
    std::size_t out = 0;
    bool removed = false;
    for (std::int32_t s = 0; s < 4; ++s) {
        const std::int32_t id = screen.slot_ids_438[s];
        if (removed) {  // 0066061E: after one removal the rest are kept as they are
            kept[out++] = id;
            continue;
        }
        screen.prune_pending_2d4 = false;
        if (id > 0) {  // 00660633, signed
            screen.prune_pending_2d4 = !host.entity_list_has_id(id);  // 00660650..0066068D
        }
        if (!screen.prune_pending_2d4) {
            kept[out++] = id;  // 00660AA5
            continue;
        }
        if (screen.removing_2dc) {
            continue;  // 006606AE: a stale id is dropped without touching the widgets
        }
        // 006606B4..00660AA3: remove slot s.
        screen.removed_slot_2d8 = s;
        if (screen.slot_group_ac[s] != 0) {
            host.destroy(screen.slot_group_ac[s]);
        }
        screen.removing_2dc = true;
        screen.removing_2dd = true;
        screen.removal_fade_2e0 = kRemovalFade;
        for (std::int32_t k = s; k < 3; ++k) {  // 00660703
            screen.slot_group_ac[k] = screen.slot_group_ac[k + 1];
            screen.slot_highlight_dc[k] = screen.slot_highlight_dc[k + 1];
            screen.slot_icon_bc[k] = screen.slot_icon_bc[k + 1];
        }
        if (screen.page_5c == 0) {  // 0066071C
            const std::int32_t count = static_cast<std::int32_t>(screen.list_c_count_3e8);
            if (count == 0) {
                screen.page_5c = 1;
                screen.selection_2c4 = 0;
                screen.slot_324 = 0;
            } else if (screen.removed_slot_2d8 == screen.slot_324) {
                if (screen.slot_324 != 0) {
                    --screen.slot_324;
                    screen.selection_2c4 = screen.selection_2c4 != 0 ? screen.selection_2c4 - 1 : count - 1;
                } else {
                    screen.selection_2c4 = screen.selection_2c4 == count - 1 ? 0 : screen.selection_2c4 + 1;
                }
                const Widget highlight = screen.slot_highlight_dc[screen.slot_324];  // 006607A5
                host.set_visible(highlight, true);
                host.set_alpha(highlight, 0.0f);
            } else {
                if (screen.removed_slot_2d8 < screen.slot_324) {  // 006607D7, signed
                    --screen.slot_324;
                }
                screen.selection_2c4 = screen.selection_2c4 != 0 ? screen.selection_2c4 - 1 : count - 1;
            }
        }
        removed = true;
        // 00660800..00660AA1: a fresh slot 3 from the template, scrolled in.
        const Widget group = host.clone_subtree(screen.slot_template_138);
        screen.slot_group_ac[3] = group;
        host.set_visible(group, true);
        screen.slot_icon_bc[3] = host.find_child(group, kIconL);
        host.set_icon_state(screen.slot_icon_bc[3], 3, 0, 1.0f);
        host.set_visible(host.find_child(group, kSmall01), false);
        host.set_visible(host.find_child(group, kSmall02), false);
        screen.slot_highlight_dc[3] = host.find_child(group, kCircleHL);
        host.set_visible(screen.slot_highlight_dc[3], false);
        const double span = static_cast<double>(screen.angle_step_2a8) * kSlotSpan;
        host.set_rotation(screen.slot_icon_bc[3], f32(span - screen.angle_base_2a4));
        host.set_rotation(group, f32(span + screen.angle_base_2a4));
        set_large_icon_alpha_0065e870(host, group, 0.0f);
        host.set_visible(screen.widget_fc, false);
    }
    screen.slot_ids_438 = kept;  // 00660AD3..00660AF5
}

void step_slot_ring_00661b50(RadialScreen& screen, RadialHost& host, float dt) {
    if (screen.slot_scroll_2bc == 0.0f) {  // 00661B83
        return;
    }
    step_scroll(screen.slot_scroll_2bc, dt);
    const double t = screen.slot_scroll_2bc;
    const double base = screen.angle_base_2a4;
    const double step = screen.angle_step_2a8;
    Widget incoming;
    double leaving;
    double offset;
    if (screen.scroll_reverse_154) {  // 00661BD5
        leaving = t * step;
        offset = (t + kOne) * step;
        incoming = screen.slot_group_ac[3];
    } else {  // 00661DB5
        leaving = (kSlotSpan - t) * step;
        offset = (kOne - t) * step;
        incoming = screen.slot_group_ac[0];
    }
    host.set_rotation(screen.slot_leaving_group_148, f32(leaving + base));
    host.set_rotation(screen.slot_leaving_icon_14c, f32(-base - leaving));
    set_large_icon_alpha_0065e870(host, screen.slot_leaving_group_148, screen.slot_scroll_2bc);
    float off = f32(offset);
    for (std::size_t k = 0; k < 4; ++k) {  // group rotates, its icon counter-rotates
        host.set_rotation(screen.slot_group_ac[k], f32(base + off));
        host.set_rotation(screen.slot_icon_bc[k], f32(-base - off));
        off = f32(static_cast<double>(off) + step);
    }
    set_large_icon_alpha_0065e870(host, incoming, f32(kOne - t));
    // 00661CDB..00661DA6: a record entry (not a live unit) under the cursor is dimmed.
    if (!host.list_c_entry_is_live_unit(screen.selection_2c4) && kOne - t > kDimThreshold) {
        host.set_alpha(host.find_child(incoming, kIconL), kDimAlpha);
    }
    if (screen.slot_scroll_2bc == 0.0f) {  // 00661FB3 (00D7A218)
        if (screen.slot_leaving_group_148 != 0) {
            host.destroy(screen.slot_leaving_group_148);
        }
        screen.slot_scrolling_2c1 = false;
        host.set_visible(screen.widget_f8, true);
        host.set_visible(screen.widget_fc, true);
    }
}

void highlight_loadout_00662020(RadialScreen& screen, RadialHost& host) {
    if (screen.loadout_icons_15c.empty()) {  // 0066204E
        host.play_unavailable_sound_0065f170();
        return;
    }
    const Record record = screen.catalog_record_3a4;
    for (std::uint32_t u = 0; u < screen.loadout_icons_15c.size(); ++u) {  // 006621EF, unsigned
        if (host.record_has_flag(record, kFlagNoDefaultEntry) &&
            u == static_cast<std::uint32_t>(host.record_loadout_count(record))) {
            return;
        }
        highlight_icon(screen, host, record, u);
    }
}

void highlight_hangar_loadout_00662210(RadialScreen& screen, RadialHost& host) {
    if (screen.loadout_icons_15c.empty()) {  // 0066223E
        host.play_unavailable_sound_0065f170();
        return;
    }
    const Record record = screen.hangar_record_2a0;
    if (host.record_loadout_count(record) < 0) {  // 0066226F
        return;
    }
    std::int32_t u = 0;
    do {
        if (host.record_has_flag(record, kFlagNoDefaultEntry) && u == host.record_loadout_count(record)) {
            return;
        }
        // The native walks the list without a size bound here; past the end it
        // reaches the checked-iterator failure 00BF6713. Stop instead.
        if (static_cast<std::size_t>(u) >= screen.loadout_icons_15c.size()) {
            return;
        }
        highlight_icon(screen, host, record, static_cast<std::uint32_t>(u));
        ++u;
    } while (u <= host.record_loadout_count(record));  // 006623F6, signed
}

void build_loadout_ring_00665f30(RadialScreen& screen, RadialHost& host) {
    build_loadout_ring(screen, host, screen.catalog_record_3a4, true);
}

void build_hangar_loadout_ring_00666dc0(RadialScreen& screen, RadialHost& host) {
    build_loadout_ring(screen, host, screen.hangar_record_2a0, false);
}

void step_catalog_ring_00668d40(RadialScreen& screen, RadialHost& host, float dt) {
    if (screen.catalog_scroll_2b8 == 0.0f) {  // 00668D5C
        return;
    }
    step_scroll(screen.catalog_scroll_2b8, dt);
    const double t = screen.catalog_scroll_2b8;
    const double base = screen.angle_base_2a4;
    const double step = screen.angle_step_2a8;
    Widget incoming;
    double offset;
    if (screen.scroll_reverse_154) {  // 00668EFA: the 4.0 is the constant left on the x87 stack
        const double leaving = (kOne - t) * step + kScrollRate * step;
        host.set_rotation(screen.catalog_leaving_group_13c, f32(base - leaving));
        host.set_rotation(screen.catalog_leaving_icon_140, f32(leaving - base));
        offset = (kOne - t) * step;
        incoming = screen.catalog_group_7c[0];
    } else {  // 00668DA8
        host.set_rotation(screen.catalog_leaving_group_13c, f32(base - t * step));
        host.set_rotation(screen.catalog_leaving_icon_140, f32(t * step - base));
        offset = (t + kOne) * step;
        incoming = screen.catalog_group_7c[3];
    }
    set_large_icon_alpha_0065e870(host, screen.catalog_leaving_group_13c, screen.catalog_scroll_2b8);
    float off = f32(offset);
    for (std::size_t k = 0; k < 4; ++k) {  // 00668E40 / 00668FB0
        host.set_rotation(screen.catalog_group_7c[k], f32(base - off));
        host.set_rotation(screen.loadout_first_108[k], f32(off - base));
        host.set_rotation(screen.loadout_rest_118[k], f32(off - base));
        host.set_rotation(screen.catalog_icon_8c[k], f32(off - base));
        off = f32(static_cast<double>(off) + step);
    }
    set_large_icon_alpha_0065e870(host, incoming, f32(kOne - t));
    if (screen.catalog_scroll_2b8 != 0.0f) {  // 00669082
        return;
    }
    if (screen.catalog_leaving_group_13c != 0) {
        host.destroy(screen.catalog_leaving_group_13c);
    }
    screen.catalog_scrolling_2c0 = false;
    host.set_visible(screen.widget_100, true);
    host.set_visible(screen.widget_104, true);
    if (screen.page_5c != 2) {  // 006690C0
        return;
    }
    // 006690CA..00669159: the unit at *(+304h)+8 picks the hangar ring for
    // kinds 9 and 45h; anything else, or an empty list +1ECh, the catalog ring.
    if (screen.list_a_count_1f4 != 0 &&
        (host.unit_is_kind(host.selected_unit_a(), 9) || host.unit_is_kind(host.selected_unit_a(), 0x45))) {
        build_hangar_loadout_ring_00666dc0(screen, host);
        return;
    }
    build_loadout_ring_00665f30(screen, host);
}

}  // namespace bsp::radial_cc13
