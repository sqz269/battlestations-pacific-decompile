// Non-gameplay helpers from 00500000-0059FFFF (cc13-sweep50). Evidence and
// open questions: docs/CC13_BAND_0050_0059.md. Names are hypotheses.

#include "bsp/band_0050_cc13.hpp"

#include <cassert>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace bsp::cc13_band0050 {
namespace {

// String constants read by 00542380.
constexpr const char* kRuleText = "===========";      // 00CEDD28
constexpr const char* kMoreAbove = "---- ^ ----";     // 00CEDD34
constexpr const char* kMoreBelow = "---- v ----";     // 00CEDD1C
constexpr const char* kEmptyText = "";                // 00CE3A0C (zero bytes)

void set_row_visible(DebugTextRow& row, bool visible) {
    row.visible = visible;
    row.visibility_written = true;
}

}  // namespace

void DebugTextList::render_00542380() {
    const std::int32_t rows_n = row_count();
    assert(rows_n >= 1);
    // 00542383..005423A1: header text into row 0.
    rows[0].text = top_24 <= 0 ? kRuleText : kMoreAbove;
    set_row_visible(rows[0], shown_2c);                       // 005423A6

    // 005423CA..0054242B: line rows. Line index = row + top - 1; a null
    // NativeString data pointer is replaced by DAT_00E192F0 (an empty string).
    std::int32_t row = 1;
    while (row < rows_n - 1) {
        const std::int32_t line = row + top_24 - 1;
        if (line >= line_count()) break;                       // 005423EC
        rows[row].text = lines[static_cast<std::size_t>(line)];
        rows[row].colour = DebugRowColour::normal_30;          // 0054241E (&+30h)
        ++row;
    }

    // 0054242D..0054247C: footer row. Its index is not range-checked by the
    // image (row_count 1 would write past the array); the model asserts.
    assert(row < rows_n);
    rows[row].text = row - 1 + top_24 < line_count() ? kMoreBelow : kRuleText;
    set_row_visible(rows[row], shown_2c);
    ++row;

    // 00542481..0054249E: clear the rest.
    for (; row < rows_n; ++row) rows[row].text = kEmptyText;

    // 005424A0..005424C3: highlight the selected row (index sel - top + 1,
    // not range-checked by the image).
    if (line_count() > 0 && selection_28 >= 0) {
        const std::int32_t hi = selection_28 - top_24 + 1;
        assert(hi >= 0 && hi < rows_n);
        rows[static_cast<std::size_t>(hi)].colour = DebugRowColour::highlight_40;
    }
    frame_visible = true;                                      // 005424C5
}

void DebugTextList::set_selection_005424f0(std::int32_t selection) {
    const std::int32_t n = line_count();
    selection_28 = selection;                                  // 005424FB
    if (n > 0) {
        // 00542500: negative values are lifted by whole multiples first. IDIV
        // truncates toward zero, as C++ division does.
        if (selection < 0) selection_28 = (1 - selection / n) * n + selection;
        selection_28 %= n;                                     // 0054251C
        if (selection_28 < top_24) top_24 = selection_28;      // 00542526
        if (selection_28 - top_24 > row_count() - 3)           // 00542534
            top_24 = selection_28 - row_count() + 3;
    }
    render_00542380();
}

void DebugTextList::move_selection_005425b0(bool up, bool down, bool page_up, bool page_down) {
    // (rows - 2) / 2 is a signed halving (CDQ; SUB; SAR) in the image.
    const std::int32_t page = (row_count() - 2) / 2;
    if (up) {
        set_selection_005424f0(selection_28 - 1);
    } else if (down) {
        set_selection_005424f0(selection_28 + 1);
    } else if (page_down) {
        set_selection_005424f0(page + selection_28);
    } else if (page_up) {
        set_selection_005424f0(selection_28 - page);
    }
}

void DebugTextList::set_lines_00544540(const std::vector<std::string>& source, bool reset) {
    lines = source;                                            // 0054454B
    if (reset) {
        selection_28 = 0;
        top_24 = 0;
    }
    render_00542380();
}

void DebugTextList::destroy_rows_00543e00() {
    rows.clear();                                              // 00543E14..
    lines.clear();                                             // 00543E42 resize(0)
}

void poll_arrows_00542db0(DebugTextList& list, FrontEndActionSource& actions) {
    // Argument evaluation order is the image's: 49h first, 46h last.
    const bool page_down = actions.action_edge_or_repeat(0x49);
    const bool page_up = actions.action_edge_or_repeat(0x48);
    const bool down = actions.action_edge_or_repeat(0x47);
    const bool up = actions.action_edge_or_repeat(0x46);
    list.move_selection_005425b0(up, down, page_up, page_down);
}

void restore_hidden_a5_00529060(std::vector<HudCounterEntryFlags>& entries) {
    for (auto& e : entries) {
        if (!e.widget_visible && e.hidden_a5 && !e.hidden_a6) {
            e.widget_visible = true;
            e.hidden_a5 = false;
        }
    }
}

void hide_all_a6_005290d0(std::vector<HudCounterEntryFlags>& entries) {
    for (auto& e : entries) {
        if (e.widget_visible) {
            e.widget_visible = false;
            e.hidden_a6 = true;
        }
    }
}

void restore_hidden_a7_005292e0(std::vector<HudCounterEntryFlags>& entries) {
    for (auto& e : entries) {
        if (!e.widget_visible && !e.hidden_a6 && e.hidden_a7) {
            e.widget_visible = true;
            e.hidden_a7 = false;
        }
    }
}

void restore_hidden_a6_00529350(std::vector<HudCounterEntryFlags>& entries) {
    for (auto& e : entries) {
        if (!e.widget_visible && e.hidden_a6) {
            e.widget_visible = true;
            e.hidden_a6 = false;
        }
    }
}

void restore_hidden_a8_00529490(std::vector<HudCounterEntryFlags>& entries) {
    for (auto& e : entries) {
        if (!e.widget_visible && e.hidden_a8) {
            e.widget_visible = true;
            e.hidden_a8 = false;
        }
    }
}

bool formation_message_matches_type_005347f0(std::int32_t type) {
    return type == 0x78 || type == 0x49 || type == 0x46;
}

float world_to_map_x_0059a840(const HudMapProjection& map, float x) {
    // FLD x; FSUB +54Ch; FDIV +30h; FADD [00E08890]; FSTP float.
    const double v = (static_cast<double>(x) - map.origin_x_54c) / map.scale_x_30 + map.offset_x_e08890;
    return static_cast<float>(v);
}

float world_to_map_y_0059a860(const HudMapProjection& map, float y) {
    // FLD +550h; FSUB y; FDIV +34h; FADD [00E08894]; FSTP float.
    const double v = (static_cast<double>(map.origin_y_550) - y) / map.scale_y_34 + map.offset_y_e08894;
    return static_cast<float>(v);
}

float remap_linear_00569e40(float x0, float x1, float x, float y0, float y1) {
    // 00569E5B..00569E67: t = (x - x0) / (x1 - x0), stored as float.
    const float t = static_cast<float>((static_cast<double>(x) - x0) / (static_cast<double>(x1) - x0));
    // 00569E6B..00569E7D: y0 + (y1 - y0) * t, stored as float.
    return static_cast<float>(static_cast<double>(y0) + (static_cast<double>(y1) - y0) * t);
}

bool is_user_administrator_00551cd0() {
#ifdef _WIN32
    SID_IDENTIFIER_AUTHORITY nt_authority = SECURITY_NT_AUTHORITY;   // {0,0,0,0,0,5}
    PSID group = nullptr;
    if (!AllocateAndInitializeSid(&nt_authority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &group)) {
        return false;
    }
    BOOL member = FALSE;
    if (!CheckTokenMembership(nullptr, group, &member)) member = FALSE;
    FreeSid(group);
    return member != FALSE;
#else
    return false;
#endif
}

}  // namespace bsp::cc13_band0050
