#include "bsp/main_menu_selection_listener.hpp"
#include "bsp/gui_text_runtime_factory.hpp"
#include "bsp/gui_type_dispatch.hpp"
#include "bsp/gui_widget_attach.hpp"
#include "bsp/native_physical_file_date.hpp"
#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
std::int32_t signed_word(std::uint32_t value) noexcept {
    std::int32_t result; std::memcpy(&result, &value, 4); return result;
}
std::int32_t add(std::int32_t a, std::int32_t b) noexcept {
    return signed_word(static_cast<std::uint32_t>(a) + static_cast<std::uint32_t>(b));
}
std::int32_t sub(std::int32_t a, std::int32_t b) noexcept {
    return signed_word(static_cast<std::uint32_t>(a) - static_cast<std::uint32_t>(b));
}
__declspec(noinline) float spill(float value) noexcept {
    float result;
    __asm {
        fld value
        fstp result
    }
    return result;
}
// Exact caller stores: float(width/divisor), float(left), float(right), then
// left+normalized+right in x87 and one float result at597513.
__declspec(noinline) float checkpoint_width(float width, float left, float right,
    const volatile double& divisor) noexcept {
    const volatile double* address = &divisor;
    float normalized, l, r, result;
    __asm {
        mov eax, address
        fld width
        fdiv qword ptr [eax]
        fstp normalized
        fld left
        fstp l
        fld right
        fstp r
        fld l
        fadd normalized
        fadd r
        fstp result
    }
    return result;
}
__declspec(noinline) double total_height(float first, float second,
    const volatile double& padding) noexcept {
    const volatile double* address = &padding;
    double first_spill, result;
    __asm {
        fld first
        fstp first_spill
        mov eax, address
        fld second
        fadd first_spill
        fadd qword ptr [eax]
        fstp result
    }
    return result;
}
__declspec(noinline) float scroll_range(double height, float clip) noexcept {
    float result;
    __asm {
        fld clip
        fsubr height
        fstp result
        fld result
        fstp result
    }
    return result;
}
void clear_header(NativeString& value) noexcept {
    static_assert(sizeof(NativeString) == 8, "NativeString helper requires Win32");
    const std::uint32_t zero[2]{};
    std::memcpy(&value, zero, sizeof(zero));
}
struct StringTemporary {
    NativeString value;
    NativeStringStorage& storage;
    bool live{};
    explicit StringTemporary(NativeStringStorage& input) : storage(input) {}
    ~StringTemporary() { destroy(); }
    void construct(const char* text) { value.assign_0041e870(storage, text); live = true; }
    void destroy() noexcept {
        if (live) { live = false; destroy_native_string_header_0041dd20(&value, storage); }
    }
};
std::string_view c_text(const NativeString& value) {
    return value.data() ? std::string_view(value.data()) : std::string_view();
}
GuiWidgetOwner& owner(MainMenuSelectionListenerBindings& b, GuiLayoutWidget* widget) {
    require(widget != nullptr, "main-menu selection requires its actual bound widget");
    return b.command.widget.owners.owner(*widget);
}
GuiListboxRuntime& listbox(MainMenuSelectionListenerBindings& b) {
    auto& actual_owner = owner(b, b.command.widget.layout.main_listbox_1b8);
    auto& actual = b.command.services.listbox_runtime(actual_owner);
    require(&actual.owner() == &actual_owner, "main-menu selection requires the SAME Listbox");
    return actual;
}
GuiTextRuntimeImplementation& text(GuiWidgetOwner& value) {
    auto* actual = dynamic_cast<GuiTextRuntimeImplementation*>(&value.implementation());
    require(actual && value.text_lifetime() == &actual->lifetime(),
        "main-menu selection requires the actual Text lifetime");
    return *actual;
}
GuiIconRuntime& icon(GuiWidgetOwner& value) {
    auto* actual = dynamic_cast<GuiIconTypeImplementation*>(&value.implementation());
    require(actual != nullptr, "main-menu selection requires its actual Icon companion");
    return actual->runtime();
}
GuiFrameBoxTypeImplementation& framebox(GuiWidgetOwner& value) {
    auto* actual = dynamic_cast<GuiFrameBoxTypeImplementation*>(&value.implementation());
    require(actual != nullptr, "main-menu selection requires its actual FrameBox companion");
    return *actual;
}
void alpha(GuiWidgetOwner& widget, float value) {
    widget.implementation().set_alpha4c(widget, value);
}
void help(MainMenuSelectionListenerBindings& b, std::string_view source) {
    auto& command = b.services.command_owner_00e1930c();
    auto& host = b.services.command_host(command);
    set_main_menu_help_line_0054a0c0(command.bar, source, host);
}
void temporary_help(MainMenuSelectionListenerBindings& b, const char* source) {
    StringTemporary temporary(b.command.strings);
    temporary.construct(source);
    help(b, c_text(temporary.value));
}
MainMenuCommandWidgetListNode* first(const MainMenuCommandWidgetListView& list) {
    auto* head = list.head_04;
    require(head != nullptr, "main-menu objective list requires its actual sentinel");
    return head->next;
}
GuiLayoutWidget* payload(const MainMenuCommandWidgetListView& list,
    MainMenuCommandWidgetListNode* node) {
    require(node && node != list.head_04 && node->widget,
        "main-menu objective iterator is outside its valid native domain");
    return node->widget;
}
void color50(MainMenuSelectionListenerBindings& b, GuiLayoutWidget* widget,
    const GuiTextColor& color) {
    // MSVC Win32 carrier view of the SAME four contiguous screen floats.
    // In particular AB6B50 reloads alpha after its base/material callbacks.
    static_assert(sizeof(GuiTextColor) == 16 && offsetof(GuiTextColor, a) == 12);
    const auto& lanes = *reinterpret_cast<const float (*)[4]>(&color);
    text(owner(b, widget)).set_color50_00ab6b50(lanes);
}
void reset_objective_colors(MainMenuSelectionListenerBindings& b,
    const MainMenuCommandWidgetListView& list) {
    if (!list.count_08) return;
    auto* node = first(list);
    for (std::uint32_t index = 0; index < list.count_08; ++index) {
        color50(b, payload(list, node), b.command.widget.layout.dest2_color_260);
        node = node->next;
    }
}
void objective_factor(MainMenuSelectionListenerBindings& b,
    std::uint32_t vector_offset, std::int32_t index, const volatile float& factor) {
    auto& widget = b.services.call_00519dc0(vector_offset, static_cast<std::uint32_t>(index));
    auto* node = widget.node_binding();
    require(node != nullptr, "main-menu objective requires its actual node+4C");
    b.services.call_00b6da70(*node, spill(factor), true);
}
void objectives(MainMenuSelectionListenerBindings& b, std::int32_t selected) {
    auto& c = b.command;
    reset_objective_colors(b, c.widgets_280);
    reset_objective_colors(b, c.widgets_298);
    reset_objective_colors(b, c.widgets_2a4);
    const MainMenuCommandWidgetListView* active = nullptr;
    MainMenuCommandWidgetListNode* node = nullptr;
    const auto count64 = b.field_64;
    if (selected < count64 && c.widgets_280.count_08) {
        active = &c.widgets_280; node = first(*active);
        for (auto remaining = selected; remaining > 0; --remaining) node = node->next;
    } else if (sub(selected, count64) < b.field_68 && c.widgets_298.count_08) {
        active = &c.widgets_298; node = first(*active);
        if (sub(selected, count64) > 0) {
            std::int32_t index = 0;
            do { node = node->next; index = add(index, 1); }
            while (index < sub(selected, b.field_64));
        }
    } else if (sub(sub(selected, b.field_68), count64) < b.field_6c && c.widgets_2a4.count_08) {
        active = &c.widgets_2a4; node = first(*active);
        if (sub(sub(selected, b.field_68), count64) > 0) {
            std::int32_t index = 0;
            do { node = node->next; index = add(index, 1); }
            while (index < sub(sub(selected, b.field_64), b.field_68));
        }
    }
    if (active) {
        (void)payload(*active, node); // Native performs three dereferences.
        b.services.call_00580820(owner(b, payload(*active, node)));
        color50(b, payload(*active, node), c.widget.layout.dest_color_270);
    }
    if (selected == add(add(b.field_68, b.field_64), b.field_6c)) {
        temporary_help(b, "globals.back");
    } else {
        if (selected < add(b.field_68, b.field_64)) {
            objective_factor(b, 0x2cc, b.field_60, b.objective_factor_00ce3e18);
            objective_factor(b, 0x2dc, b.field_60, b.objective_factor_00ce3e18);
            const float literal_one = 1.0f;
            objective_factor(b, 0x2cc, selected, literal_one);
            objective_factor(b, 0x2dc, selected, literal_one);
            b.field_60 = selected;
        }
        const auto current64 = b.field_64;
        if (selected < current64 && c.widgets_280.count_08) {
            temporary_help(b, b.objective_help_00e08868);
        } else if (sub(selected, current64) < b.field_68 && c.widgets_298.count_08) {
            temporary_help(b, b.objective_help_00e0886c);
        } else if (sub(sub(selected, b.field_68), current64) < b.field_6c && c.widgets_2a4.count_08) {
            const auto& mission = c.services.selected_mission_005806a0();
            const auto side = mission_side_index(mission.screen);
            const auto index = static_cast<std::uint32_t>(sub(sub(selected, b.field_64), b.field_68));
            const auto& hints = mission.sides[side].extra.hidden_hints;
            if (index < hints.size()) help(b, hints[index]);
            else temporary_help(b, "Hidden objective hint, pls fill it in Missiontree.lua");
            if (b.field_60 < add(b.field_64, b.field_68)) {
                objective_factor(b, 0x2cc, b.field_60, c.widget.dim_alpha_00ce3800);
                objective_factor(b, 0x2dc, b.field_60, c.widget.dim_alpha_00ce3800);
            }
        }
    }
    set_gui_widget_listener_00aa6bc0(owner(b, c.widget.layout.main_listbox_1b8), nullptr, 0);
}
const MissionGroupData& current_group(MainMenuSelectionListenerBindings& b) {
    //580650..580662 capture E198AC+5C and its group vector BEFORE E194D8.
    const auto& groups = b.services.mission_tree_00e198ac_5c().groups;
    const auto index = b.group_00e194d8;
    require(index < groups.size(), "main-menu mission group is outside its valid native domain");
    return groups[index];
}
struct MapName {
    StringTemporary suffix, prefix, numbered, combined;
    MapName(NativeStringStorage& strings, const char* base, std::int32_t index)
        : suffix(strings), prefix(strings), numbered(strings), combined(strings) {
        suffix.construct("_Icon");
        prefix.construct(base);
        native_string_concat_int_004263b0(prefix.value, numbered.value, index, strings);
        numbered.live = true;
        concatenate_native_string_headers_004261a0(&numbered.value, &combined.value,
            &suffix.value, strings);
        combined.live = true;
    }
    void destroy() noexcept { combined.destroy(); numbered.destroy(); prefix.destroy(); suffix.destroy(); }
};
GuiLayoutWidget* find_map_widget(MainMenuSelectionListenerBindings& b,
    const NativeString& name, std::uint32_t unused) {
    require(b.active_mission_group_110 != nullptr,
        "main-menu selection requires current mission-group110");
    require(b.compare_names_00bf7fbf != nullptr, "main-menu selection requires native name comparison");
    return find_child_by_name_00aa7e00(b.command.widget.owners,
        *b.active_mission_group_110, name, unused, b.compare_names_00bf7fbf);
}
void rebuild_commands(MainMenuSelectionListenerBindings& b) {
    auto& strings = b.command.strings;
    const bool scrollable = b.command.widget.layout.mission_scroller_1d4.scrollable;
    const std::uint8_t scroll_code = scrollable ? 0xb2 : 0;
    StringTemporary scroll(strings), back(strings), navigate(strings), zoom(strings), select(strings);
    scroll.construct(scrollable ? "globals.scroll_menu" : "");
    const bool glyph = b.glyph_mode_00f88a30 != 0;
    back.construct("globals.back"); navigate.construct("globals.navigate");
    zoom.construct(glyph ? "FE.zoom" : ""); select.construct("globals.select");
    const std::array<MainMenuCommandArgument, 5> arguments{{
        {0xa2, c_text(select.value), 0},
        {static_cast<std::uint8_t>(glyph ? 9 : 0), c_text(zoom.value), 1},
        {0xb3, c_text(navigate.value), 1}, {scroll_code, c_text(scroll.value), 1},
        {0xa3, c_text(back.value), 2}}};
    auto& command = b.services.command_owner_00e1930c();
    auto& host = b.services.command_host(command);
    rebuild_main_menu_command_bar_0054b530(command.bar,
        b.services.command_environment(command), arguments, host);
    select.destroy(); zoom.destroy(); navigate.destroy(); back.destroy(); scroll.destroy();
}
void mission_page(MainMenuSelectionListenerBindings& b, std::int32_t selected) {
    auto& c = b.command; auto& layout = c.widget.layout;
    if (selected == -1) selected = 0;
    const auto& first_group = current_group(b);
    const auto index = static_cast<std::uint32_t>(selected);
    require(index < first_group.missions.size(), "main-menu mission index is outside its valid native domain");
    const auto& mission = first_group.missions[index];
    // A0 is the retained texture produced by the reader, NOT the picture key.
    void* texture = b.services.mission_picture_texture_0a0(mission);
    const float one = c.widget.one_00d7a24c;
    const GuiUvRect uv{0.0f, 0.0f, one, one};
    b.services.call_00ab2690(owner(b, layout.mission_picture_2f4), 0, texture, uv);
    (void)icon(owner(b, layout.mission_picture_2f4)); // capture/check current58 profile
    GuiWidgetSize picture_size;
    b.services.call_00ab27a0(owner(b, layout.mission_picture_2f4), picture_size, 0);
    auto& picture = owner(b, layout.mission_picture_2f4);
    icon(picture).set_size58_00ab1ef0(picture, picture_size);
    text(owner(b, layout.mission_name_304)).submit_source_00abaed0(mission.screen.title, true);
    auto& name_background = owner(b, layout.mission_name_background_334); // EDX first
    auto& name = owner(b, layout.mission_name_304);
    b.services.call_00ac0820(name, name_background, spill(0.0f));
    owner(b, layout.mission_name_background_334).set_visible34(true);
    StringTemporary date(c.strings);
    const auto date68 = mission.screen.briefing_word2;
    const auto date64 = mission.screen.briefing_word1;
    const auto date60 = mission.screen.briefing_word0;
    b.services.call_0043bc30(date.value, date60, date64, date68, c.strings);
    date.live = true;
    text(owner(b, layout.mission_date_308)).submit_source_00abaed0(std::string(c_text(date.value)), true);
    date.destroy();
    text(owner(b, layout.mission_content_30c)).submit_source_00abaed0(mission.extra.helpline, true);
    b.active_mission_group_110 = nullptr;
    switch (c.widget.page_00e08874) {
    case 4: case 6: b.active_mission_group_110 = layout.missions_jp_314; break;
    case 5: case 7: b.active_mission_group_110 = layout.missions_us_310; break;
    case 8: b.active_mission_group_110 = layout.training_320; break;
    default: break;
    }
    if (selected == 0) {
        b.arrow_top_enabled_1c5 = 0;
        icon(owner(b, layout.arrow_top_1bc)).select_state_00ab1710(0, 0, spill(1.0f));
        alpha(owner(b, layout.arrow_top_1bc), spill(c.zoom_in_00ce54a0));
    } else {
        b.arrow_top_enabled_1c5 = 1;
        alpha(owner(b, layout.arrow_top_1bc), spill(1.0f));
    }
    auto& current_list = listbox(b);
    const auto last = signed_word(current_list.row_count_104() - 1u);
    bool disable_down = last == selected;
    if (!disable_down && selected < last) {
        auto* next = current_list.row_at_00a9be00(add(selected, 1));
        require(next != nullptr, "main-menu next-row hidden-byte read requires an actual row");
        disable_down = next->scene_flags().hidden;
    }
    if (disable_down) {
        b.arrow_bottom_enabled_1c4 = 0;
        icon(owner(b, layout.arrow_bottom_1c0)).select_state_00ab1710(0, 0, spill(1.0f));
        alpha(owner(b, layout.arrow_bottom_1c0), spill(c.zoom_in_00ce54a0));
    } else {
        b.arrow_bottom_enabled_1c4 = 1;
        alpha(owner(b, layout.arrow_bottom_1c0), spill(1.0f));
    }
    {
        MapName selected_name(c.strings, "mission_mappoint_", add(selected, 1));
        layout.selected_map_point_330 = find_map_widget(b, selected_name.combined.value, 0);
        selected_name.destroy();
    }
    const auto& captured_group = current_group(b);
    auto iterator = captured_group.missions.begin();
    std::int32_t ordinal = 1;
    for (;;) {
        // The native loop reloads E198AC+5C BEFORE E194D8, as does580650.
        const auto& current_tables = b.services.mission_tree_00e198ac_5c();
        const auto current_index = b.group_00e194d8;
        require(current_index < current_tables.groups.size(), "main-menu current group is invalid");
        const auto& checked_group = current_tables.groups[current_index];
        require(&captured_group == &checked_group, "main-menu mission iterator changed owner during callback");
        if (iterator == checked_group.missions.end()) break;
        GuiLayoutWidget* map_point;
        {
            MapName point_name(c.strings, "mission_mappoint_", ordinal);
            map_point = find_map_widget(b, point_name.combined.value, 1);
            point_name.destroy();
        }
        const bool is_selected = map_point == layout.selected_map_point_330;
        const auto state = static_cast<std::int16_t>((iterator->extra.side_mission ? 2 : 0) + (is_selected ? 1 : 0));
        icon(owner(b, map_point)).select_state_00ab1710(state, 0, spill(1.0f));
        const float flag_alpha = selected == sub(ordinal, 1) ? c.widget.one_00d7a24c : b.map_flag_alpha_00ce7804;
        MapName flag_name(c.strings, "mission_mapflag_", ordinal);
        alpha(owner(b, find_map_widget(b, flag_name.combined.value, 1)), spill(flag_alpha));
        flag_name.destroy();
        ++iterator; ordinal = add(ordinal, 1);
    }
    c.selected_00e194dc = static_cast<std::uint32_t>(selected);
    b.services.call_00594b60();
    const auto& fresh_mission = c.services.selected_mission_005806a0();
    auto& mission_tree = b.services.mission_tree_00e198ac_5c();
    select_mission_by_id_005c35d0(mission_tree, fresh_mission);
    // Mission key is the earlier EBP record+0, not fresh_mission or its title.
    auto& before_background = owner(b, layout.checkpoint_background_2fc);
    (void)framebox(before_background);
    auto& profile1 = c.services.profile_00e188a8_650();
    const bool checkpoint1 = c.services.call_007f8d60(profile1, mission);
    owner(b, layout.checkpoint_background_2fc).set_visible_00aa8530(checkpoint1);
    (void)text(owner(b, layout.checkpoint_text_300));
    auto& profile2 = c.services.profile_00e188a8_650();
    const bool checkpoint2 = c.services.call_007f8d60(profile2, mission);
    owner(b, layout.checkpoint_text_300).set_visible_00aa8530(checkpoint2);
    auto& profile3 = c.services.profile_00e188a8_650();
    const bool available = c.services.call_007fc370(profile3, mission);
    // ABBE50 captures ECX before its C-string equality gate/constructor. Keep
    // that SAME Text even if a string-allocation callback rebinds layout300.
    auto& checkpoint_text = text(owner(b, layout.checkpoint_text_300));
    const char* label = available ? "FE.main_checkpoint" : "FE.main_checkpoint_dlc";
    const auto& cached_source = checkpoint_text.lifetime().text().source;
    if (cached_source.empty() || b.compare_names_00bf7fbf(cached_source.c_str(), label) != 0) {
        StringTemporary checkpoint_label(c.strings);
        checkpoint_label.construct(label);
        checkpoint_text.submit_source_00abaed0(std::string(c_text(checkpoint_label.value)), true);
        checkpoint_label.destroy();
    }
    const auto& size = owner(b, layout.checkpoint_background_2fc).layout().transform.size;
    const float width = text(owner(b, layout.checkpoint_text_300)).lifetime().text().measured_width;
    auto& background = owner(b, layout.checkpoint_background_2fc);
    auto& frames = framebox(background).state().frame_sizes_x;
    const float height = size.height;
    const GuiWidgetSize checkpoint_size{checkpoint_width(width, frames[0], frames[1], b.width_divisor_00cec380), height};
    b.services.framebox_current58(background, checkpoint_size);
    const auto previous = b.previous_00e194e0;
    if (previous < selected && b.arrow_bottom_enabled_1c4) {
        icon(owner(b, layout.arrow_bottom_1c0)).select_temporary84_00ab1110(2, 0, spill(b.arrow_duration_00d7a2f0));
    } else if (selected < previous && b.arrow_top_enabled_1c5) {
        icon(owner(b, layout.arrow_top_1bc)).select_temporary84_00ab1110(2, 0, spill(b.arrow_duration_00d7a2f0));
    }
    b.previous_00e194e0 = selected;
    set_gui_widget_listener_00aa6bc0(owner(b, layout.main_listbox_1b8), nullptr, 0);
    attach_content(layout.mission_scroller_1d4, nullptr, c.scroller_host);
    const float content_height = text(owner(b, layout.mission_content_30c)).normalized_height_00ab6bd0();
    const float date_height = text(owner(b, layout.mission_date_308)).normalized_height_00ab6bd0();
    const double total = total_height(content_height, date_height, b.text_padding_00ceed60);
    const float clip_height = owner(b, layout.clipbox_230).layout().transform.size.height;
    set_scroll_range(layout.mission_scroller_1d4, scroll_range(total, clip_height), c.scroller_host);
    rebuild_commands(b);
}
} // namespace

void select_mission_by_id_005c35d0(MissionTreeTables& tree, const MissionRecordData& mission) {
    const auto found = find_mission_by_id_005c3470(tree.groups, mission.screen.name);
    if (found.mission != 0xffffffffu) {
        tree.selection.mission = found.mission;
        tree.selection.group = found.group;
    }
}
NativeString& native_string_from_int_004260b0(NativeString& output,
    std::int32_t value, NativeStringStorage& strings) {
    clear_header(output);
    char buffer[52];
    (void)std::snprintf(buffer, sizeof(buffer), "%d", value);
    NativeString temporary;
    temporary.resize_0041dd40(strings, static_cast<std::uint32_t>(std::strlen(buffer)), true);
    char* const data = temporary.data();
    const auto length = temporary.length();
    if (data) std::memcpy(data, buffer, static_cast<std::size_t>(length) + 1u);
    try {
        output.resize_0041dd40(strings, length, true);
        if (length) std::memcpy(output.data(), data, output.length());
    } catch (...) {
        if (data) strings.release(data, length + 1u);
        throw;
    }
    if (data) strings.release(data, length + 1u);
    return output;
}
NativeString& native_string_concat_int_004263b0(const NativeString& prefix,
    NativeString& output, std::int32_t value, NativeStringStorage& strings) {
    clear_header(output);
    if (&output != &prefix) {
        output.resize_0041dd40(strings, prefix.length(), true);
        if (prefix.length()) std::memcpy(output.data(), prefix.data(), output.length());
    }
    StringTemporary number(strings);
    native_string_from_int_004260b0(number.value, value, strings);
    number.live = true;
    const auto length = number.value.length();
    if (length) {
        const auto old_length = output.length();
        output.resize_0041dd40(strings, length + old_length, true);
        std::memcpy(output.data() + old_length, number.value.data(), length);
    }
    return output;
}
void main_menu_listener_current08_005966f0(MainMenuSelectionListenerBindings& b,
    GuiWidgetOwner* selected_row, GuiWidgetOwner& callback_listbox) {
    (void)selected_row; (void)callback_listbox; // RET8 operands are both unread.
    require(&b.arrow_top_enabled_1c5 == &b.command.widget.arrow_top_enabled_1c5 &&
        &b.arrow_bottom_enabled_1c4 == &b.command.widget.arrow_bottom_enabled_1c4,
        "main-menu selection requires the SAME command/widget arrow-enable fields");
    const auto selected_bits = listbox(b).selected_data_d8_00a9c990();
    const auto selected = signed_word(selected_bits);
    switch (b.command.widget.page_00e08874) {
    case 1: {
        const char* source = b.help_00e087d4[selected];
        b.selected_00e194c4 = selected_bits; temporary_help(b, source); return;
    }
    case 2: {
        const char* source = b.help_00e08808[selected];
        b.selected_00e194c8 = selected_bits; temporary_help(b, source); return;
    }
    case 3: {
        const char* source = b.help_00e08828[selected];
        b.selected_00e194cc = selected_bits; temporary_help(b, source); return;
    }
    case 11: temporary_help(b, b.help_00e0884c[selected]); return;
    case 12: objectives(b, selected); return;
    case 4: case 5: case 6: case 7: case 8: mission_page(b, selected); return;
    default: return;
    }
}
} // namespace bsp
