#include "bsp/main_menu_command_bar.hpp"

namespace bsp {
namespace {
// 00CEE008..00CEE068, indexed by compacted count. Unused cells are -1 in
// the native table; no reconstruction path indexes those cells.
constexpr std::array<std::array<std::uint8_t, 5>, 5> kSlots{{
    {{0, 0, 0, 0, 0}}, {{1, 2, 0, 0, 0}}, {{1, 0, 2, 0, 0}},
    {{3, 1, 2, 4, 0}}, {{3, 1, 0, 2, 4}},
}};

std::uint16_t field(std::uint16_t base, std::size_t slot) noexcept {
    return static_cast<std::uint16_t>(base + slot * 4);
}

bool equals_ascii_key(std::string_view text, std::string_view key) noexcept {
    text = text.substr(0, text.find('\0'));
    if (text.size() != key.size()) return false;
    for (std::size_t i = 0; i < key.size(); ++i) {
        char value = text[i];
        if (value >= 'A' && value <= 'Z') value = static_cast<char>(value + ('a' - 'A'));
        if (value != key[i]) return false;
    }
    return true;
}

void enter_if_needed(MainMenuCommandBarState& state, MainMenuCommandBarHost& host) {
    if (state.entered_05 != 0) return;
    state.visible_04 = 1;
    state.entered_05 = 1;
    host.commit_visibility_004f83b0();
    host.enter_virtual_18();
}
}

MainMenuCommandPlan plan_main_menu_commands(
    const std::array<MainMenuCommandArgument, 5>& arguments) {
    MainMenuCommandPlan result;
    for (const auto& argument : arguments) {
        if (argument.command == 0) continue;
        auto& entry = result.entries[result.count++];
        entry.command = argument.command;
        entry.label = argument.label;
    }
    std::size_t ordinal = 0;
    for (const auto& argument : arguments) {
        if (argument.command == 0) continue;
        result.entries[ordinal].slot = static_cast<std::uint8_t>(argument.placement == 0 ? 3 :
            argument.placement == 2 ? 4 : kSlots[result.count - 1][ordinal]);
        ++ordinal;
    }
    return result;
}

std::string main_menu_command_glyph_bytes(std::uint8_t command) {
    if (command == 9) return std::string("\xb4/\xb6", 3);
    if (command == 1) return std::string("\xb7/\xb8", 3);
    return std::string(1, static_cast<char>(command));
}

bool main_menu_command_is_navigation_label(std::string_view label) noexcept {
    return equals_ascii_key(label, "globals.navigate") ||
        equals_ascii_key(label, "globals.scroll_menu") ||
        equals_ascii_key(label, "globals.change");
}

GuiWidgetSize main_menu_command_background_size(float text_width,
    float existing_height) noexcept {
    const float normalized = static_cast<float>(static_cast<double>(text_width) / 960.0);
    // Exact double bytes at 00CE4D68: 00 00 00 40 E1 7A A4 3F.
    constexpr double padding = 0.03999999910593033;
    return {static_cast<float>(static_cast<double>(normalized) + padding), existing_height};
}

void hide_main_menu_glyph_widgets_005495a0(MainMenuCommandResetHost& host) {
    for (std::size_t i = 0; i < 5; ++i) {
        host.set_widget_visible(field(0x34, i), false); // 005495BA
        host.set_widget_visible(field(0x48, i), false); // 005495C5
    }
}

void hide_main_menu_alternate_widgets_005495e0(MainMenuCommandResetHost& host) {
    for (std::size_t i = 0; i < 5; ++i) {
        host.set_widget_visible(field(0x84, i), false); // 005495FA
        host.set_widget_visible(field(0x98, i), false); // 00549605
    }
}

void rebuild_main_menu_command_bar_0054b530(MainMenuCommandBarState& state,
    const MainMenuCommandBarEnvironment& environment,
    const std::array<MainMenuCommandArgument, 5>& arguments,
    MainMenuCommandBarHost& host) {
    host.hide_alternate_widgets_005495e0();
    host.hide_glyph_widgets_005495a0();
    void* frame_box = host.find_frame_box();
    host.set_frame_box_visible(frame_box, environment.frame_box_source_e198c4 != 0);
    // These duplicate visibility calls follow both native reset helpers.
    for (std::size_t i = 0; i < 5; ++i) {
        host.set_widget_visible(field(0x34, i), false); // 0054B5FA
        host.set_widget_visible(field(0x48, i), false); // 0054B604
        host.clear_glyph_children(field(0x48, i)); // 0054B608
    }
    const auto plan = plan_main_menu_commands(arguments);
    if (plan.count == 0) return;
    enter_if_needed(state, host); // 0054B908..0054B925
    const bool glyph_mode = environment.glyph_mode_f88a30 != 0; // 0054B933
    constexpr std::array<float, 4> color{{0.0f, 0.0f, 0.0f, 1.0f}};
    for (std::size_t i = 0; i < plan.count; ++i) {
        const auto& entry = plan.entries[i];
        const auto glyph = field(0x48, entry.slot);
        host.set_widget_visible(glyph, true); // 0054B9D7
        const auto glyph_bytes = main_menu_command_glyph_bytes(entry.command);
        host.clear_glyph_source(glyph); // 0054BC12, before either mode branch
        if (!glyph_mode) {
            host.set_widget_visible(glyph, false); // 0054BD2B
            if (main_menu_command_is_navigation_label(entry.label)) continue;
        }
        host.configure_glyph_auxiliary(glyph, true, 0, 0.05f, color);
        host.set_localised_source(glyph, glyph_bytes);
        const auto label = field(glyph_mode ? 0x34 : 0x84, entry.slot);
        host.set_widget_visible(label, true);
        host.set_ellipsized_label(label, entry.label, -1.0f, true);
        state.cached_labels_5c[entry.slot] = entry.label; // 0054BCE9 / 0054BED3
        if (!glyph_mode) {
            const auto background = field(0x98, entry.slot);
            host.set_widget_visible(background, true); // 0054BF0D
            const auto old_size = host.widget_size(background); // 0054BF16
            const float text_width = host.text_width_114(label); // 0054BF22
            host.set_widget_size(background,
                main_menu_command_background_size(text_width, old_size.height));
        }
    }
}

void set_main_menu_help_line_0054a0c0(MainMenuCommandBarState& state,
    std::string_view source, MainMenuCommandBarHost& host) {
    enter_if_needed(state, host); // 0054A0C4..0054A0E2
    host.set_localised_source(0x2c, source); // 0054A0ED
    host.set_localised_source(0x30, source); // 0054A0F7
    const std::uint8_t mode = state.help_variant_20; // 0054A0FC, after both setters
    host.set_widget_visible(0x2c, mode == 0); // 0054A111
    host.set_widget_visible(0x30, mode != 0); // 0054A120
    state.help_variant_20 = mode; // 0054A123 restores even after a reentrant call
}
}
