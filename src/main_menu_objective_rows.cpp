#include "bsp/main_menu_objective_rows.hpp"
#include "bsp/gui_text_runtime_factory.hpp"
#include "bsp/gui_widget_attach.hpp"
#include "bsp/native_physical_file_date.hpp"
#include <cstring>
#include <limits>
#include <stdexcept>

namespace bsp {
namespace {
void require(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
GuiWidgetOwner& owner(MainMenuObjectiveRowsBindings& b, GuiLayoutWidget* layout) {
    require(layout != nullptr, "594BF0 requires its actual authored widget");
    return b.selection.command.widget.owners.owner(*layout);
}
MainMenuLayoutBindings& layout(MainMenuObjectiveRowsBindings& b) {
    return b.selection.command.widget.layout;
}
GuiListboxRuntime& listbox(MainMenuObjectiveRowsBindings& b) {
    auto& widget = owner(b, layout(b).main_listbox_1b8);
    auto& list = b.selection.command.services.listbox_runtime(widget);
    require(&list.owner() == &widget, "594BF0 Listbox resolver changed owner identity");
    return list;
}
GuiTextRuntimeImplementation& text(GuiWidgetOwner& widget) {
    auto* actual = dynamic_cast<GuiTextRuntimeImplementation*>(&widget.implementation());
    require(actual && widget.text_lifetime() == &actual->lifetime(),
        "594BF0 requires its actual Text runtime companion");
    return *actual;
}
struct String {
    NativeString value;
    NativeStringStorage& storage;
    explicit String(NativeStringStorage& allocator) : storage(allocator) {}
    ~String() { value.release_to(storage); }
    void construct(const char* source) { value.assign_0041e870(storage, source); }
    std::string bytes() const {
        require(value.length() == 0 || value.data() != nullptr,
            "594BF0 native string allocation did not produce its buffer");
        return value.data() ? std::string(value.data(), value.length()) : std::string{};
    }
};
void source(GuiWidgetOwner& target, const std::string& value) {
    text(target).submit_source_00abaed0(value, true);
}
void cstring(MainMenuObjectiveRowsBindings& b, GuiWidgetOwner& target, const char* value) {
    auto& actual = text(target);
    if (b.selection.compare_names_00bf7fbf(actual.lifetime().text().source.c_str(), value) == 0)
        return; // ABBE50 outer C-string cache gate, before temporary allocation.
    String temporary(b.selection.command.strings);
    temporary.construct(value);
    source(target, temporary.bytes());
}
void numbered(MainMenuObjectiveRowsBindings& b, GuiWidgetOwner& target,
    std::uint32_t ordinal, const char* suffix, const std::vector<std::string>* objectives = nullptr) {
    auto& strings = b.selection.command.strings;
    String ending(strings), number(strings), prefix(strings), result(strings);
    ending.construct(suffix);
    std::int32_t next;
    const auto bits = ordinal + 1u;
    std::memcpy(&next, &bits, sizeof(next));
    native_string_from_int_004260b0(number.value, next, strings);
    concatenate_native_string_headers_004261a0(&number.value, &prefix.value, &ending.value, strings);
    if (!objectives) { source(target, prefix.bytes()); return; }
    // Native reloads vector bounds/data AFTER construction of the prefix.
    const auto& current = objectives->at(ordinal);
    String borrowed_transport(strings);
    borrowed_transport.construct(current.c_str());
    concatenate_native_string_headers_004261a0(&prefix.value, &result.value,
        &borrowed_transport.value, strings);
    source(target, result.bytes());
}
GuiWidgetOwner& clone(MainMenuObjectiveRowsBindings& b, GuiLayoutWidget* prototype) {
    auto& source_owner = owner(b, prototype);
    auto* cloned = clone_subtree(source_owner.layout().transform, nullptr, b.clone_host);
    require(cloned != nullptr, "594BF0 actual GUI clone allocation failed");
    return source_owner.runtime().owner(*cloned);
}
void data(GuiWidgetOwner& widget, std::uint32_t value) {
    widget.extra_fields().pointer_d8 = reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
void validate_list_alias(const MainMenuObjectiveWidgetList& list,
    const MainMenuCommandWidgetListView& view) {
    const auto mutable_list = list.command_view();
    require(&mutable_list.head_04 == &view.head_04 && &mutable_list.count_08 == &view.count_08,
        "594BF0 must append the SAME lists read by command/selection listeners");
}
float add(float first, float second) {
    float result;
    __asm {
        fld first
        fadd second
        fstp result
    }
    return result;
}
float advance(float height, const volatile float& spacing, float accumulator) {
    const volatile float* address = &spacing;
    float result;
    __asm {
        fld height
        mov eax, address
        fld dword ptr [eax]
        fadd accumulator
        faddp st(1), st(0)
        fstp result
    }
    return result;
}
void position(MainMenuObjectiveRowsBindings& b, GuiWidgetOwner& target,
    float offset, bool header) {
    // Three native accessor calls, Z then Y then X, reload live template slots.
    const auto z = resolved_position(owner(b, layout(b).dest_text_258).layout().transform).z;
    const auto y = resolved_position(owner(b, layout(b).dest_text_258).layout().transform).y;
    const auto x = resolved_position(owner(b, header ? layout(b).primary_text_24c :
        layout(b).dest_text_258).layout().transform).x;
    set_gui_widget_resolved_position_00aa8240(target, {x, add(y, offset), z});
}
void color(GuiWidgetOwner& widget, const GuiTextColor& value) {
    // SAME MSVC Win32 quartet: AB6BA5 reloads source alpha only after the
    // base/material callbacks. A temporary color would lose those writes.
    static_assert(sizeof(GuiTextColor) == 16 && offsetof(GuiTextColor, a) == 12);
    const auto& values = *reinterpret_cast<const float (*)[4]>(&value);
    text(widget).set_color50_00ab6b50(values);
}
void factor(MainMenuObjectiveRowsBindings& b, GuiWidgetOwner& widget, float value) {
    auto* node = widget.node_binding();
    require(node != nullptr, "594BF0 visibility factor requires its actual widget scene node");
    b.selection.command.widget.owners.set_node_visibility_factor_00b6da70(*node, value, true);
}
GuiWidgetOwner& row(MainMenuObjectiveRowsBindings& b, std::uint32_t index) {
    auto& result = clone(b, layout(b).main_text_238);
    data(result, index);
    result.layout().transform.mouse_hit = false;
    std::unique_ptr<GuiLayoutWidget> detached;
    listbox(b).append_row_00a9d750(result, detached); // All three calls position=null, after=0.
    return result;
}
void listener(MainMenuObjectiveRowsBindings& b, GuiWidgetOwner& description) {
    description.layout().transform.mouse_hit = true;
    set_gui_widget_listener_00aa6bc0(description, b.screen_plus_40, 0);
    data(description, listbox(b).row_count_104() - 1u);
}
void commands(MainMenuObjectiveRowsBindings& b) {
    auto& strings = b.selection.command.strings;
    String empty(strings), back(strings), navigate(strings), naval(strings), next(strings);
    empty.construct(""); back.construct("globals.back"); navigate.construct("globals.navigate");
    naval.construct("FE.naval"); next.construct("globals.continue");
    const auto next_text = next.bytes(), naval_text = naval.bytes(), navigate_text = navigate.bytes(),
        back_text = back.bytes(), empty_text = empty.bytes();
    const std::array<MainMenuCommandArgument, 5> args{{
        {0xa2, next_text, 0}, {0xa7, naval_text, 1}, {0xb3, navigate_text, 1},
        {0xa3, back_text, 2}, {0, empty_text, 1}}}; // 54B530 RET3C = 15 words.
    auto& s = b.selection.services;
    auto& command = s.command_owner_00e1930c();
    rebuild_main_menu_command_bar_0054b530(command.bar, s.command_environment(command),
        args, s.command_host(command));
}
} // namespace

void grow_main_menu_objective_list_count_005896f0(volatile std::uint32_t& count,
    std::uint32_t increment) {
    const auto captured = count;
    if (std::uint32_t{0x3fffffffu} - captured < increment)
        throw std::length_error("list<T> too long");
    count = captured + increment;
}

MainMenuObjectiveWidgetList::MainMenuObjectiveWidgetList()
    : head_04_(new MainMenuCommandWidgetListNode) {
    static_assert(sizeof(MainMenuCommandWidgetListNode) == 12, "BSP node transport requires Win32");
    head_04_->next = head_04_; //58228E
    head_04_->previous = head_04_; //582297; payload is constructor-unwritten.
}
MainMenuObjectiveWidgetList::~MainMenuObjectiveWidgetList() {
    auto* current = head_04_->next;
    while (current != head_04_) {
        auto* const next = current->next;
        delete current;
        current = next;
    }
    delete head_04_;
}
void MainMenuObjectiveWidgetList::append_005823b0_005896f0(GuiLayoutWidget& widget) {
    auto* const head = head_04_;
    auto node = std::make_unique<MainMenuCommandWidgetListNode>();
    node->next = head; //5823C2
    node->previous = head->previous; //5823CF
    node->widget = &widget; //5823DE
    grow_main_menu_objective_list_count_005896f0(count_08_, 1); //before either link store.
    head->previous = node.get();
    node->previous->next = node.get();
    static_cast<void>(node.release()); // The one intrusive list owns its nodes.
}

void build_main_menu_objective_rows_00594bf0(MainMenuObjectiveRowsBindings& b) {
    auto& selection = b.selection;
    auto& c = selection.command;
    require(&b.page_00e08874 == &c.widget.page_00e08874,
        "594BF0 page publication must alias the actual listener global");
    require(&b.text_names.widgets == &c.widget.owners,
        "594BF0 Text resource service must use the same widget runtime");
    require(b.screen_plus_40 != nullptr && selection.compare_names_00bf7fbf,
        "594BF0 requires the actual screen listener and native name comparator");
    validate_list_alias(b.primary_280, c.widgets_280);
    validate_list_alias(b.secondary_298, c.widgets_298);
    validate_list_alias(b.hidden_2a4, c.widgets_2a4);
    b.field_2c4 = 1;
    const auto& mission = c.services.selected_mission_005806a0();
    auto& score = mission_record_00594a70(b.providers.progress_00e188a8_6b4(), mission.screen.name);
    b.providers.call_00518d60(10, 0, 0, mission.screen.title);
    b.page_00e08874 = 12;
    selection.field_60 = 0;
    owner(b, layout(b).worldmap_2f0).set_visible34(false);
    owner(b, layout(b).briefing_grid_244).set_visible34(true);
    owner(b, layout(b).briefing_248).set_visible34(true);
    auto objective = make_main_menu_objective_bindings(selection, b.objective_providers);
    bind_main_menu_objectives_0058f5b0(objective);
    owner(b, layout(b).objective_background_2ec).set_visible34(true);
    const auto side_index = mission_side_index(mission.screen);
    const auto& side = mission.screen.sides[side_index];
    const auto& extra = mission.side_extras[side_index];
    if (movie_widget_is_playing_00aac8e0(b.layout_native.movie_state(
        owner(b, layout(b).video_33c).layout()), b.movies))
        stop_movie_widget_00aac870(b.layout_native.movie_state(
            owner(b, layout(b).video_33c).layout()), b.movies);
    for (std::size_t index = 0; index < layout(b).objective_groups_2cc.size(); ++index)
        factor(b, owner(b, layout(b).objective_groups_2cc.at(index)), c.widget.dim_alpha_00ce3800);
    for (std::size_t index = 0; index < layout(b).objective_companions_2dc.size(); ++index)
        factor(b, owner(b, layout(b).objective_companions_2dc.at(index)), c.widget.dim_alpha_00ce3800);
    main_menu_objective_widget_00519dc0(objective, 0x2cc,
        static_cast<std::uint32_t>(selection.field_60)).set_visible34(true);
    main_menu_objective_widget_00519dc0(objective, 0x2dc,
        static_cast<std::uint32_t>(selection.field_60)).set_visible34(true);
    factor(b, main_menu_objective_widget_00519dc0(objective, 0x2cc,
        static_cast<std::uint32_t>(selection.field_60)), 1.0f);
    factor(b, main_menu_objective_widget_00519dc0(objective, 0x2dc,
        static_cast<std::uint32_t>(selection.field_60)), 1.0f);
    commands(b);
    owner(b, layout(b).list_layout_3c4.group_18).set_visible34(false);
    owner(b, layout(b).main_listbox_1b8).set_visible34(false);
    owner(b, layout(b).text_layout_414.group_08).set_visible34(false);
    owner(b, layout(b).main_listbox_1b8).set_visible34(true);
    listbox(b).clear_rows_00a9bec0(b.deletion);
    auto& active_list = owner(b, layout(b).main_listbox_1b8);
    active_list.implementation().set_active60(active_list, true);
    b.primary_header_2b8 = &clone(b, layout(b).primary_text_24c).layout();
    {
        String shader(c.strings); shader.construct("GuiFontBilinear.mshd");
        set_gui_text_shader_name_00ab8e70(text(owner(b, b.primary_header_2b8)).lifetime(),
            shader.bytes(), b.text_names);
    }
    cstring(b, owner(b, b.primary_header_2b8), "globals.obj_pri");
    owner(b, b.primary_header_2b8).set_visible34(true);
    float offset = 0.0f;
    for (std::uint32_t i = 0; static_cast<std::int32_t>(i) < selection.field_64; ++i) {
        auto& item = row(b, i);
        auto& description = clone(b, layout(b).dest_text_258);
        description.set_visible34(true);
        listener(b, description);
        b.primary_280.append_005823b0_005896f0(description.layout());
        if (i < side.objectives_a.size()) {
            numbered(b, item, i, ". |", &side.objectives_a);
            numbered(b, description, i, ". |", &side.objectives_a);
        } else {
            numbered(b, item, i, ". primary Objective");
            cstring(b, description, "Primary Objective Placeholder");
        }
        position(b, description, offset, false);
        offset = advance(text(description).normalized_height_00ab6bd0(), b.field_2b4, offset);
        if (i == 0) {
            selection.services.call_00580820(description);
            color(description, layout(b).dest_color_270);
        } else color(description, layout(b).dest2_color_260);
    }
    offset = add(b.field_2b0, offset);
    b.secondary_header_2bc = &clone(b, layout(b).primary_text_24c).layout();
    position(b, owner(b, b.secondary_header_2bc), offset, true);
    cstring(b, owner(b, b.secondary_header_2bc), "globals.obj_sec");
    owner(b, b.secondary_header_2bc).set_visible34(true);
    offset = advance(text(owner(b, b.secondary_header_2bc)).normalized_height_00ab6bd0(), b.field_2b0, offset);
    for (std::uint32_t i = 0; static_cast<std::int32_t>(i) < selection.field_68; ++i) {
        auto& item = row(b, static_cast<std::uint32_t>(selection.field_64) + i);
        auto& description = clone(b, layout(b).dest_text_258);
        listener(b, description);
        description.set_visible34(true);
        color(description, layout(b).dest2_color_260);
        b.secondary_298.append_005823b0_005896f0(description.layout());
        if (i < side.objectives_b.size()) {
            numbered(b, item, i, ". |", &side.objectives_b);
            numbered(b, description, i, ". |", &side.objectives_b);
        } else {
            cstring(b, item, "Secondary Objective");
            numbered(b, description, i, ". secondary Objective");
        }
        position(b, description, offset, false);
        offset = advance(text(description).normalized_height_00ab6bd0(), b.field_2b4, offset);
    }
    offset = add(b.field_2b0, offset);
    b.hidden_header_2c0 = &clone(b, layout(b).primary_text_24c).layout();
    position(b, owner(b, b.hidden_header_2c0), offset, true);
    cstring(b, owner(b, b.hidden_header_2c0), "globals.obj_hid");
    owner(b, b.hidden_header_2c0).set_visible34(true);
    offset = advance(text(owner(b, b.hidden_header_2c0)).normalized_height_00ab6bd0(), b.field_2b0, offset);
    // Copy AFTER all header callbacks. Recompute side from captured mission.
    const auto hidden_scores = score.objectives_204[mission_side_index(mission.screen) == 0 ? 2 : 5];
    for (std::uint32_t i = 0; static_cast<std::int32_t>(i) < selection.field_6c; ++i) {
        auto& item = row(b, static_cast<std::uint32_t>(selection.field_68) + i +
            static_cast<std::uint32_t>(selection.field_64));
        auto& description = clone(b, layout(b).dest_text_258);
        description.set_visible34(true);
        listener(b, description);
        color(description, layout(b).dest2_color_260);
        b.hidden_2a4.append_005823b0_005896f0(description.layout());
        if (i < extra.hidden_objectives.size()) {
            source(item, extra.hidden_objectives.at(i));
            if (hidden_scores.empty() || hidden_scores.begin()->second.value == 0)
                numbered(b, description, i, ". |globals.obj_hid_mask");
            else numbered(b, description, i, ". |", &extra.hidden_objectives);
        } else {
            cstring(b, item, "Hidden objective");
            cstring(b, description, "Secondary Objective");
        }
        position(b, description, offset, false);
        offset = advance(text(description).normalized_height_00ab6bd0(), b.field_2b4, offset);
    }
    listbox(b).select_index_00a9c7c0(0);
    listbox(b).refresh80_00a9c220(true);
    const auto& base = layout(b).main_listbox_position_558;
    const auto x = add(base.x, 0.0f);
    auto& final_list = owner(b, layout(b).main_listbox_1b8);
    const auto y = add(0.0f, base.y);
    const volatile double* depth = &b.depth_00ce47a0;
    float z;
    const float original_z = base.z;
    __asm {
        mov eax, depth
        fld original_z
        fadd qword ptr [eax]
        fstp z
    }
    set_gui_widget_resolved_position_00aa8240(final_list, {x, y, z});
    set_gui_widget_listener_00aa6bc0(owner(b, layout(b).main_listbox_1b8), nullptr, 0);
}
} // namespace bsp
