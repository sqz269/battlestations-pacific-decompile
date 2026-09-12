#include "bsp/main_menu_objective_runtime.hpp"
#include "bsp/main_menu_selection_listener.hpp"
#include "bsp/native_physical_file_date.hpp"
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
std::int32_t add_word(std::int32_t value, std::uint32_t increment) noexcept {
    const auto bits = static_cast<std::uint32_t>(value) + increment;
    std::int32_t result;
    std::memcpy(&result, &bits, sizeof result);
    return result;
}
struct Temporary {
    NativeString value;
    NativeStringStorage& storage;
    explicit Temporary(NativeStringStorage& s) : storage(s) {}
    Temporary(NativeStringStorage& s, const char* text) : storage(s) {
        value.assign_0041e870(s, text);
    }
    ~Temporary() { value.release_to(storage); }
};
GuiWidgetOwner& owner(MainMenuObjectiveBindings& b, GuiLayoutWidget* widget) {
    require(widget != nullptr, "main-menu objectives require their actual page/widget owner");
    return b.owners.owner(*widget);
}
GuiLayoutWidget* child(MainMenuObjectiveBindings& b, const NativeString& name,
    std::uint32_t unused) {
    return find_child_by_name_00aa7e00(b.owners, owner(b, b.page_2c8).layout(),
        name, unused, b.compare_names_00bf7fbf);
}
void numbered_name(NativeString& destination, NativeStringStorage& storage,
    const char* suffix, const volatile std::int32_t& live_count) {
    // Native suffix constructor precedes the CURRENT count read. Temporary
    // destruction is concatenation, integer text, suffix, before next name.
    Temporary tail(storage, suffix);
    Temporary number(storage);
    native_string_from_int_004260b0(number.value, live_count, storage);
    Temporary joined(storage);
    concatenate_native_string_headers_004261a0(&number.value, &joined.value,
        &tail.value, storage);
    destination.copy_from_00be0a30_fragment(storage, joined.value);
}
void collect_groups(MainMenuObjectiveBindings& b, NativeString& objective_name,
    NativeString& companion_name, volatile std::int32_t& count,
    const char* objective_suffix, const char* companion_suffix) {
    count = 1;
    auto* objective = child(b, objective_name, 0);
    while (objective) {
        auto* companion = child(b, companion_name, 1);
        // Publication precedes both virtual calls; a null companion reaches
        // the same invalid dereference domain after the first Group is hidden.
        b.objective_groups_2cc.push_back(objective);
        b.companion_groups_2dc.push_back(companion);
        owner(b, objective).set_visible34(false);
        owner(b, companion).set_visible34(false);
        count = add_word(count, 1);
        numbered_name(objective_name, b.strings, objective_suffix, count);
        numbered_name(companion_name, b.strings, companion_suffix, count);
        objective = child(b, objective_name, 0);
    }
    count = add_word(count, 0xffffffffu);
}
__declspec(noinline) float page_progress(std::int32_t index,
    const volatile double& step, const volatile double& base) noexcept {
    const volatile double* step_address = &step;
    const volatile double* base_address = &base;
    float result;
    __asm {
        fild index
        mov eax, step_address
        fmul qword ptr [eax]
        mov eax, base_address
        fadd qword ptr [eax]
        fstp result
        fld result
        fstp result
    }
    return result;
}
} // namespace

bool BriefingObjectivePageLess::operator()(const std::string& left,
    const std::string& right) const {
    if (left.empty()) return !right.empty();
    if (right.empty()) return false;
    return compare_00bf7fbf(left.c_str(), right.c_str()) < 0;
}
BriefingObjectivePageMap::BriefingObjectivePageMap(GuiNativeNameCompare compare)
    : pages_(BriefingObjectivePageLess{compare}) {
    require(compare != nullptr, "briefing page map requires actual CRT name comparison");
}
GuiLayoutWidget*& BriefingObjectivePageMap::index_0051cc90(const std::string& key) {
    return pages_[key];
}
void append_briefing_layer_names_005c5b40(const MissionTreeTables& tree,
    std::vector<std::string>& output) {
    for (const auto& group : tree.groups) {
        for (const auto& mission : group.missions) {
            const auto& key = mission.screen.sides[mission_side_index(mission.screen)].briefing_key;
            if (!key.empty()) output.push_back(key);
        }
    }
    for (const auto& mission : tree.multi) {
        for (const auto& side : mission.screen.sides) {
            if (!side.briefing_key.empty()) output.push_back(side.briefing_key);
        }
    }
}
void load_briefing_objective_pages_0051dda0(BriefingObjectivePageLoadBindings& b) {
    if (b.pages_12c.size() != 0) return;
    std::vector<std::string> names;
    append_briefing_layer_names_005c5b40(b.mission_tree_00e198ac_5c, names);
    for (std::size_t index = 0; index < names.size(); ++index) {
        const auto resource = std::string("briefings/") +
            (b.force_ijn05_00e18d91 ? std::string("IJN05") : names[index]);
        auto& publication = b.pages_12c.index_0051cc90(names[index]); //0051DF20
        auto* page = load_gui_page_00aa5840(b.gui_pages, b.page_loader, resource, 1, false);
        publication = page ? page->root.get() : nullptr; //0051DF3B, even null
        const auto fraction = page_progress(static_cast<std::int32_t>(index),
            b.progress_step_00cec8f8, b.progress_base_00cec8f0);
        report_loading_progress(b.loading_screen_00e194b4, fraction, b.loading_progress);
    }
}
void bind_main_menu_objectives_0058f5b0(MainMenuObjectiveBindings& b) {
    b.objective_groups_2cc.clear(); //51AEB0 does not delete widget payloads.
    b.companion_groups_2dc.clear();
    const auto& mission = b.providers.selected_mission_005806a0();
    const auto side = mission_side_index(mission.screen);
    auto& pages = b.providers.briefing_pages_00e198ac_64();
    b.page_2c8 = pages.index_0051cc90(mission.screen.sides[side].briefing_key);
    owner(b, b.page_2c8).set_visible34(true);
    {
        Temporary name(b.strings, "BG_01_Group");
        b.background_2ec = child(b, name.value, 1);
    }
    Temporary objective_name(b.strings, "1_pri_objective_Group");
    Temporary companion_name(b.strings, "1_pri_Group");
    collect_groups(b, objective_name.value, companion_name.value,
        b.primary_count_64, "_pri_objective_Group", "_pri_Group");
    // The native reuses these headers with resize(preserve=false), including
    // the equal-size no-op, then overwrites exactly their CURRENT lengths.
    objective_name.value.resize_0041dd40(b.strings, 21, false);
    if (objective_name.value.data())
        std::memcpy(objective_name.value.data(), "1_sec_objective_Group", objective_name.value.length());
    companion_name.value.resize_0041dd40(b.strings, 11, false);
    if (companion_name.value.data())
        std::memcpy(companion_name.value.data(), "1_sec_Group", companion_name.value.length());
    collect_groups(b, objective_name.value, companion_name.value,
        b.secondary_count_68, "_sec_objective_Group", "_sec_Group");
    b.hidden_count_6c = 0;
    const auto fresh_side = mission_side_index(mission.screen); //0058FE47
    const auto& hidden = mission.side_extras[fresh_side].hidden_objectives;
    for (std::uint32_t i = 0; i < hidden.size(); ++i)
        b.hidden_count_6c = add_word(b.hidden_count_6c, 1);
}
GuiLayoutWidget*& objective_vector_element_00519dc0(
    std::vector<GuiLayoutWidget*>& vector, std::uint32_t index) {
    if (vector.data() == nullptr || index >= vector.size())
        throw std::out_of_range("STL_inst_00519dc0: invalid vector subscript (native BF6713)");
    return vector[index];
}
GuiWidgetOwner& main_menu_objective_widget_00519dc0(MainMenuObjectiveBindings& b,
    std::uint32_t offset, std::uint32_t index) {
    if (offset == 0x2cc) return owner(b, objective_vector_element_00519dc0(b.objective_groups_2cc, index));
    if (offset == 0x2dc) return owner(b, objective_vector_element_00519dc0(b.companion_groups_2dc, index));
    throw std::invalid_argument("main-menu objective accessor requires vector+2CC or+2DC");
}
} // namespace bsp
