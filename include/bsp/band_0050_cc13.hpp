#pragma once

// Non-gameplay helpers from 00500000-0059FFFF reconstructed by cc13-sweep50:
// the cheat-menu debug text list (render, selection, paging, input poll, line
// load and row teardown), the HUD counter widgets' hide/restore flag loops, the
// formation session message's type filter, the map screen's world-to-map
// conversions, a linear remap and the administrator-membership check. Names
// are hypotheses, not recovered symbols. Evidence, ABIs and open questions are
// in docs/CC13_BAND_0050_0059.md. None is a drop-in binary replacement: these
// are C++ interfaces over models of the native data.
//
// x87 model: the image runs with 53-bit precision; every FSTP to a float slot is
// a float cast and unspilled intermediates are doubles.

#include <cstdint>
#include <string>
#include <vector>

namespace bsp::cc13_band0050 {

// ---------------------------------------------------------------------------
// Debug text list (the cheat menu's scrolling line list). Native object, as the
// routines below read it:
//   +04h  row count (number of row widgets)
//   +0Ch  line vector data (8-byte NativeString {length, char*}), +10h count
//   +18h  frame widget (vslot +34h set-visible)
//   +20h  row widget pointer array (row 0 header, last used row footer)
//   +24h  first visible line (scroll top)
//   +28h  selected line
//   +2Ch  byte: rows shown
//   +30h  normal row colour, +40h highlight colour (passed by address to the
//         row widget's vslot +50h)
// +00h, +08h, +14h, +1Ch are not touched by these routines.
enum class DebugRowColour : std::uint8_t { untouched, normal_30, highlight_40 };

struct DebugTextRow {
    std::string text;               // last BSP_GuiText_SetLocalisedCString(text, 0)
    bool visible = false;           // last vslot +34h argument (header/footer only)
    bool visibility_written = false;
    DebugRowColour colour = DebugRowColour::untouched;   // last vslot +50h argument
};

struct DebugTextList {
    std::vector<DebugTextRow> rows;     // +20h / +04h
    std::vector<std::string> lines;     // +0Ch / +10h; a null char* reads as ""
    bool frame_visible = false;         // +18h vslot +34h
    std::int32_t top_24 = 0;
    std::int32_t selection_28 = 0;
    bool shown_2c = false;

    std::int32_t row_count() const { return static_cast<std::int32_t>(rows.size()); }
    std::int32_t line_count() const { return static_cast<std::int32_t>(lines.size()); }

    // 00542380, ECX list, RET. Header row 0: "===========" when top <= 0, else
    // "---- ^ ----"; then lines top.. into rows 1..rows-2 with the normal
    // colour until the lines run out; the next row gets "---- v ----" when
    // lines remain below it, else "==========="; the remaining rows get "".
    // Header and footer visibility follow +2Ch. With lines and a non-negative
    // selection the selected row gets the highlight colour. The frame is shown.
    void render_00542380();

    // 005424F0, ECX list, stack int, RET 4. Wraps the selection into
    // [0, line count) (no wrap when there are no lines), keeps it inside the
    // window top..top+rows-3, then renders.
    void set_selection_005424f0(std::int32_t selection);

    // 005425B0, ECX list, stack (up, down, page_up, page_down) bytes, RET 10h.
    // First set flag wins: up -1, down +1, page_down +(rows-2)/2,
    // page_up -(rows-2)/2. Nothing set: no call, no render.
    void move_selection_005425b0(bool up, bool down, bool page_up, bool page_down);

    // 00544540, ECX list, stack (const NativeStringVector*, bool reset), RET 8.
    // Copies the lines (BSP_NativeStringVector_CopyContents on +0Ch), zeroes
    // top and selection when reset is set, then renders.
    void set_lines_00544540(const std::vector<std::string>& source, bool reset);

    // 00543E00, ECX list, RET. Deletes every row widget (vslot 1, flag 1),
    // frees the row array and empties the lines. The image only does the row
    // part when +20h is non-null.
    void destroy_rows_00543e00();
};

// Front-end action query used by 00542DB0 (BSP_FrontEnd_ActionEdgeOrRepeat on
// the front-end manager [00E188A8]).
class FrontEndActionSource {
public:
    virtual ~FrontEndActionSource() = default;
    virtual bool action_edge_or_repeat(std::int32_t action) = 0;
};

// 00542DB0, ECX list, RET. Queries actions 49h, 48h, 47h, 46h in that order and
// passes them to 005425B0 as (46h up, 47h down, 48h page up, 49h page down).
void poll_arrows_00542db0(DebugTextList& list, FrontEndActionSource& actions);

// ---------------------------------------------------------------------------
// HUD counter entry list (+20h entry pointer array, +24h count). Each entry's
// +0 is its widget (vslot +38h visible, +34h set-visible); bytes +A5h..+A8h
// record why the counter was hidden.
struct HudCounterEntryFlags {
    bool widget_visible = false;
    bool hidden_a5 = false;   // under the powerup/support manager panel
    bool hidden_a6 = false;   // everything hidden
    bool hidden_a7 = false;   // under the screen at HUD root +58h
    bool hidden_a8 = false;   // under the map screen
};

// 00529060: re-show entries hidden by A5h and not by A6h, clearing A5h.
void restore_hidden_a5_00529060(std::vector<HudCounterEntryFlags>& entries);
// 005290D0: hide every visible entry and set A6h.
void hide_all_a6_005290d0(std::vector<HudCounterEntryFlags>& entries);
// 005292E0: re-show entries hidden by A7h and not by A6h, clearing A7h.
void restore_hidden_a7_005292e0(std::vector<HudCounterEntryFlags>& entries);
// 00529350: re-show entries hidden by A6h, clearing A6h.
void restore_hidden_a6_00529350(std::vector<HudCounterEntryFlags>& entries);
// 00529490: re-show entries hidden by A8h, clearing A8h.
void restore_hidden_a8_00529490(std::vector<HudCounterEntryFlags>& entries);

// ---------------------------------------------------------------------------
// 005347F0, vtable 00CED548 slot after the scalar deleting dtor, ECX unused,
// stack int, RET 4, returns EAX 1 or 0. True for session message types 78h,
// 49h and 46h.
bool formation_message_matches_type_005347f0(std::int32_t type);

// Map screen fields read by 0059A840/0059A860 (ECX map screen).
struct HudMapProjection {
    float scale_x_30 = 1.0f;
    float scale_y_34 = 1.0f;
    float origin_x_54c = 0.0f;
    float origin_y_550 = 0.0f;
    float offset_x_e08890 = 0.0f;   // global DAT_00E08890
    float offset_y_e08894 = 0.0f;   // global DAT_00E08894
};

// 0059A840, ECX map, stack float, RET 4, result in ST0 rounded to float.
float world_to_map_x_0059a840(const HudMapProjection& map, float x);
// 0059A860, ECX map, stack float, RET 4. Y axis is flipped: (+550h - y).
float world_to_map_y_0059a860(const HudMapProjection& map, float y);

// 00569E40, __fastcall (ECX x0*, EDX x1*, stack x*, y0*, y1*), RET 0Ch, ST0.
// y0 + (y1 - y0) * t with t = (x - x0) / (x1 - x0) rounded to float.
float remap_linear_00569e40(float x0, float x1, float x, float y0, float y1);

// 00551CD0, no arguments, returns AL. Membership of the process token in
// BUILTIN\Administrators (SECURITY_NT_AUTHORITY, RIDs 20h/220h); false when the
// SID cannot be built or the check fails. Windows only.
bool is_user_administrator_00551cd0();

}  // namespace bsp::cc13_band0050
