#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "bsp/frontend_screen_animation.hpp"

namespace bsp {

// 0054B530: ECX = command-bar screen, RET 3Ch. Five source-order triples
// consume fifteen stack dwords. Only the low byte of command is inspected;
// label points to an eight-byte NativeString; placement is a full dword.
// This semantic projection is not the native layout or a replacement ABI.
struct MainMenuCommandArgument {
    std::uint8_t command{};
    std::string_view label;
    std::uint32_t placement{1}; // 0 -> slot 3; 2 -> slot 4; otherwise table
};

struct MainMenuCommandEntry {
    std::uint8_t command{};
    std::uint8_t slot{};
    std::string label;
};

struct MainMenuCommandPlan {
    std::array<MainMenuCommandEntry, 5> entries{};
    std::size_t count{};
};

// Compacts nonzero commands in source order before selecting slots. Duplicate
// forced slots are retained: later entries can overwrite earlier slot state.
MainMenuCommandPlan plan_main_menu_commands(
    const std::array<MainMenuCommandArgument, 5>& arguments);

// Byte strings, not Unicode or input-action lookups: 9 -> B4 2F B6,
// 1 -> B7 2F B8, everything else -> the command byte itself.
std::string main_menu_command_glyph_bytes(std::uint8_t command);

// Native _stricmp comparisons at 0054BD6A, 0054BDB1 and 0054BDF2, projected
// for ASCII localization keys. Native C strings stop at the first NUL.
bool main_menu_command_is_navigation_label(std::string_view label) noexcept;

// 0054BF22..0054BF53: float(round-to-float(text_width / 960.0) + padding).
// Preserve the intermediate float store; the original divides/adds as x87
// against doubles. Uses the existing two-float widget-size type.
GuiWidgetSize main_menu_command_background_size(float text_width,
    float existing_height) noexcept;

struct MainMenuCommandBarState {
    std::uint8_t visible_04{};
    std::uint8_t entered_05{};
    std::uint8_t help_variant_20{};
    std::array<std::string, 5> cached_labels_5c{};
};

struct MainMenuCommandBarEnvironment {
    std::uint32_t frame_box_source_e198c4{};
    std::uint8_t glyph_mode_f88a30{};
};

// Widget handles are represented by field offsets, including the five-element
// families +34 (glyph-mode labels), +48 (glyphs), +84 (alternate labels),
// and +98 (alternate backgrounds). No new full screen/widget type is invented.
struct MainMenuCommandResetHost {
    virtual ~MainMenuCommandResetHost() = default;
    virtual void set_widget_visible(std::uint16_t field_offset, bool visible) = 0;
};

// ECX-only routines with RET, 005495A0..005495D1 and 005495E0..00549611.
void hide_main_menu_glyph_widgets_005495a0(MainMenuCommandResetHost& host);
void hide_main_menu_alternate_widgets_005495e0(MainMenuCommandResetHost& host);

// One method invokes one native target; repeated sites keep their native order.
// NativeString allocation/copy/destruction and STL vector storage are projected
// into standard strings/arrays. They are not reimplemented as game operations.
struct MainMenuCommandBarHost : MainMenuCommandResetHost {
    virtual void hide_alternate_widgets_005495e0() = 0; // 0054B555
    virtual void hide_glyph_widgets_005495a0() = 0; // 0054B55C
    // 0054B5A7: 00AA7E00 on root +28, "button_FrameBox", required=1.
    virtual void* find_frame_box() = 0;
    virtual void set_frame_box_visible(void* widget, bool visible) = 0;
    virtual void clear_glyph_children(std::uint16_t field_offset) = 0; // 00AB80C0
    virtual void commit_visibility_004f83b0() = 0;
    virtual void enter_virtual_18() = 0;
    // 0054BC12 -> 00ABBE50, empty C string, flag=1.
    virtual void clear_glyph_source(std::uint16_t field_offset) = 0;
    // 0054BC69 / 0054BE4E -> 00AB6C30. Raw parameters are established;
    // the auxiliary object's precise rendering role remains a contract.
    virtual void configure_glyph_auxiliary(std::uint16_t field_offset,
        bool enabled, std::uint32_t value, float scalar,
        const std::array<float, 4>& color) = 0;
    // 00ABAED0, flag=1, 0054BC77 / 0054BE5C / help-line 0054A0ED/A0F7.
    virtual void set_localised_source(std::uint16_t field_offset,
        std::string_view source) = 0;
    // 0054BCC3 / 0054BEAC -> 00ABB000, width=-1.0f, localize=1.
    virtual void set_ellipsized_label(std::uint16_t field_offset,
        std::string_view source, float width, bool localize) = 0;
    virtual GuiWidgetSize widget_size(std::uint16_t field_offset) = 0; // 00AA6740
    // A native field read (+114h), kept explicit after label geometry updates.
    virtual float text_width_114(std::uint16_t field_offset) = 0;
    virtual void set_widget_size(std::uint16_t field_offset,
        const GuiWidgetSize& size) = 0; // widget virtual +58h
};

// 0054B530 normal path. Environment is read at the corresponding native points
// (frame source after lookup, glyph mode after enter). Only nonempty plans enter
// the screen. Empty plans still reset both families and clear all five glyphs.
// Native SEH/pool failures, upper bytes of bool argument dwords, and aliases
// into the original screen/string layout are outside this projection.
void rebuild_main_menu_command_bar_0054b530(MainMenuCommandBarState& state,
    const MainMenuCommandBarEnvironment& environment,
    const std::array<MainMenuCommandArgument, 5>& arguments,
    MainMenuCommandBarHost& host);

// 0054A0C0: ECX screen, one NativeString const* stack argument, RET 4.
// Updates both +2C/+30 texts, displays !help_variant / help_variant, then
// restores the captured +20 byte after the visibility calls.
void set_main_menu_help_line_0054a0c0(MainMenuCommandBarState& state,
    std::string_view source, MainMenuCommandBarHost& host);
}
