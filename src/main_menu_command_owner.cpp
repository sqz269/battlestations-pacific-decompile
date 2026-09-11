#include "bsp/main_menu_command_owner.hpp"

#include <stdexcept>
#include <string>

namespace bsp {
namespace {
constexpr std::array<MainMenuCommandWidgetBinding, 22> bindings{{
    {0x2c, 0x24, "content_helpline_Text",  0x0054c24e},
    {0x30, 0x24, "content_helpline2_Text", 0x0054c2c4},
    {0x34, 0x28, "CenterText_Text",       0x0054c33a},
    {0x48, 0x28, "CenterIcon_Text",       0x0054c3b0},
    {0x38, 0x28, "Left1Text_Text",        0x0054c426},
    {0x4c, 0x28, "Left1Icon_Text",        0x0054c49c},
    {0x3c, 0x28, "Right1Text_Text",       0x0054c512},
    {0x50, 0x28, "Right1Icon_Text",       0x0054c588},
    {0x40, 0x28, "Left2Text_Text",        0x0054c5fe},
    {0x54, 0x28, "Left2Icon_Text",        0x0054c674},
    {0x44, 0x28, "Right2Text_Text",       0x0054c6ea},
    {0x58, 0x28, "Right2Icon_Text",       0x0054c760},
    {0x84, 0x28, "CenterText_pc_Text",    0x0054c7dc},
    {0x88, 0x28, "Left1Text_pc_Text",     0x0054c851},
    {0x8c, 0x28, "Right1Text_pc_Text",    0x0054c8ca},
    {0x90, 0x28, "Left2Text_pc_Text",     0x0054c943},
    {0x94, 0x28, "Right2Text_pc_Text",    0x0054c9bc},
    {0x98, 0x28, "Center_pc_FrameBox",    0x0054ca35},
    {0x9c, 0x28, "Left1_pc_FrameBox",     0x0054caae},
    {0xa0, 0x28, "Right1_pc_FrameBox",    0x0054cb27},
    {0xa4, 0x28, "Left2_pc_FrameBox",     0x0054cba0},
    {0xa8, 0x28, "Right2_pc_FrameBox",    0x0054cc19},
}};

GuiLayoutWidget& required_widget(GuiLayoutWidget* widget) {
    if (!widget) throw std::runtime_error("Command owner requires a loaded page/widget");
    return *widget;
}

std::uint16_t slot_offset(std::uint16_t base, std::uint16_t index) noexcept {
    return static_cast<std::uint16_t>(base + index * 4);
}
}

const std::array<MainMenuCommandWidgetBinding, 22>&
main_menu_command_widget_bindings() noexcept { return bindings; }

GuiLayoutWidget* MainMenuCommandOwner::widget_at(std::uint16_t offset) const noexcept {
    if (offset == 0x24) return help_page_24 ? help_page_24->root.get() : nullptr;
    if (offset == 0x28) return button_page_28 ? button_page_28->root.get() : nullptr;
    for (std::size_t i = 0; i != bindings.size(); ++i)
        if (bindings[i].field_offset == offset) return widgets[i];
    return nullptr;
}

std::int32_t main_menu_command_screen_id_0054a960() noexcept { return 0x58; }
bool main_menu_command_virtual04_0054a970() noexcept { return false; }
void exit_main_menu_command_owner_00549570() noexcept {}
void update_main_menu_command_owner_00549580(float) noexcept {}

void set_main_menu_help_variant_00549620(MainMenuCommandBarState& state,
    std::uint8_t variant, MainMenuCommandResetHost& host) {
    host.set_widget_visible(0x2c, variant == 0); // 00549636
    host.set_widget_visible(0x30, variant != 0); // 00549641
    state.help_variant_20 = variant;             // 00549643
}

void bind_main_menu_command_owner_0054c050(MainMenuCommandOwner& owner,
    const MainMenuCommandBarEnvironment& environment,
    MainMenuCommandOwnerHost& host, MainMenuCommandBarHost& command_host) {
    host.register_screen_004f71d0(owner); // 0054C077, id58h
    auto* manager = host.gui_manager_004c12b0();
    owner.help_page_24 = host.load_page_00aa5840(manager, "FE_helpline", 1, false);
    manager = host.gui_manager_004c12b0();
    owner.button_page_28 = host.load_page_00aa5840(manager, "_Buttonhelp", 1, false);
    host.set_visible_virtual34(required_widget(owner.widget_at(0x24)), false);
    host.set_visible_virtual34(required_widget(owner.widget_at(0x28)), false);
    auto* frame = host.find_child_00aa7e00(required_widget(owner.widget_at(0x28)),
        "button_FrameBox", 1);
    host.set_visible_virtual34(required_widget(frame), false);

    for (std::size_t i = 0; i != bindings.size(); ++i) {
        const auto& binding = bindings[i];
        owner.widgets[i] = host.find_child_00aa7e00(
            required_widget(owner.widget_at(binding.page_offset)), binding.name, 1);
    }

    for (std::uint16_t i = 0; i != 5; ++i) {
        // 0054CC4F: ESI starts owner+84h, [ESI-50h] is label+34h.
        for (const auto base : {0x34u, 0x84u}) {
            auto& widget = required_widget(owner.widget_at(
                static_cast<std::uint16_t>(base + i * 4)));
            auto& text = host.text_state(widget);
            text.multiline = false;
            text.font_scale = 0.75f; // float00CEE07C
            host.set_text_cstring_00abbe50(widget, " ", true);
        }
    }

    // 0054CCA7..0054CE57: byte concat then unsigned-byte widening004C5E60.
    constexpr std::u16string_view codes = u"\u00a2\u00a3\u00a5\u00a7\u00b4\u00b6\u00b7\u00b8\u00b1";
    for (std::uint16_t i = 0; i != 5; ++i) {
        auto& glyph = required_widget(owner.widget_at(slot_offset(0x48, i)));
        host.bind_text_codes_00531130(glyph, codes, owner, glyph);
        host.set_widget_callback_00aa6bc0(glyph, nullptr, 0);
        host.set_widget_callback_00aa6bc0(
            required_widget(owner.widget_at(slot_offset(0x98, i))), &owner, 0);
        host.set_widget_callback_00aa6bc0(
            required_widget(owner.widget_at(slot_offset(0x84, i))), &owner, 0);
    }

    // Five (0, empty, 1) triples, callee RET3Ch. This retains cached labels
    // and does not enter the screen. Do not substitute a help-text clear.
    rebuild_main_menu_command_bar_0054b530(owner.bar, environment, {}, command_host);
    owner.bar.help_variant_20 = 0; // 0054D37F, after the rebuild callbacks
}

void enter_main_menu_command_owner_00549fc0(MainMenuCommandOwner& owner,
    const std::uint8_t& style_61f, MainMenuCommandOwnerHost& host) {
    host.set_text_cstring_00abbe50(required_widget(owner.widget_at(0x2c)), "", false);
    host.set_text_cstring_00abbe50(required_widget(owner.widget_at(0x30)), "", false);
    for (std::uint16_t i = 0; i != 5; ++i) {
        auto& label = required_widget(owner.widget_at(slot_offset(0x34, i)));
        const float channel = style_61f != 0 ? 1.0f : 0.0f;
        host.set_text_color_virtual50(label, {channel, channel, channel, 1.0f});
        const auto shadow_enabled = style_61f; // reload after virtual50
        host.configure_text_shadow_00ab6c30(label, shadow_enabled, 0, 0.05f,
            {0.0f, 0.0f, 0.0f, shadow_enabled != 0 ? 1.0f : 0.0f});
        host.set_text_source_00abaed0(label, owner.bar.cached_labels_5c[i], true);
    }
}

void collect_main_menu_command_pages_0054b500(const MainMenuCommandOwner& owner,
    std::vector<GuiLayoutPage*>& output) {
    output.push_back(owner.help_page_24);
    output.push_back(owner.button_page_28);
}

void destroy_main_menu_command_owner_0054a990(MainMenuCommandOwner& owner,
    MainMenuCommandOwnerLifetimeHost& host) {
    auto* manager = host.gui_manager_004c12b0();
    host.remove_and_destroy_page_00aa31f0(manager, owner.help_page_24);
    manager = host.gui_manager_004c12b0();
    host.remove_and_destroy_page_00aa31f0(manager, owner.button_page_28);
    for (auto i = owner.bar.cached_labels_5c.size(); i != 0; --i)
        std::string{}.swap(owner.bar.cached_labels_5c[i - 1]);
    host.destroy_callback_owner_00695870(owner);
    host.unregister_screen_004f71a0(owner);
}

}
