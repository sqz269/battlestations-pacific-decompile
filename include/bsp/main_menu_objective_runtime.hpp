#pragma once
#include "bsp/frontend_entry.hpp"
#include "bsp/gui_text_type_dispatch.hpp"
#include "bsp/mission_tree_data.hpp"
#include <map>

namespace bsp {

// The ONE map owned by the briefing screen at+12C. Values borrow the actual
// page roots returned by GuiPageRegistry; neither another page nor GUI tree is
// created here. This is a C++ container projection, not the native tree ABI.
struct BriefingObjectivePageLess {
    GuiNativeNameCompare compare_00bf7fbf;
    bool operator()(const std::string&, const std::string&) const;
};
class BriefingObjectivePageMap {
public:
    explicit BriefingObjectivePageMap(GuiNativeNameCompare);
    // Complete ordinary51CC90 map operator[]: absent keys insert a null value.
    // ECX native map, key stack, RET4; EAX points to the SAME stored value.
    GuiLayoutWidget*& index_0051cc90(const std::string& key);
    std::size_t size() const noexcept { return pages_.size(); }
private:
    std::map<std::string, GuiLayoutWidget*, BriefingObjectivePageLess> pages_;
};

// Complete normal5C5B40: append without clearing/deduplicating. Groups first,
// choosing side0 when enabled, else side1; then both sides of every multi row.
// Only nonempty briefingGuiLayer strings are appended. Native ECX mission
// tree, output vector stack, RET4. Native string/vector allocators are replaced.
void append_briefing_layer_names_005c5b40(const MissionTreeTables&,
    std::vector<std::string>& output);

struct BriefingObjectivePageLoadBindings {
    BriefingObjectivePageMap& pages_12c;
    const MissionTreeTables& mission_tree_00e198ac_5c;
    GuiPageRegistry& gui_pages;
    GuiLayoutHost& page_loader;
    const volatile std::uint8_t& force_ijn05_00e18d91;
    LoadingScreen* const volatile& loading_screen_00e194b4;
    const LoadingProgressCrtAccess& loading_progress;
    const volatile double& progress_base_00cec8f0;
    const volatile double& progress_step_00cec8f8;
};
// Complete normal51DDA0 (ECX briefing screen, RET): only an empty map starts
// loading. Each collected occurrence inserts/finds its original key BEFORE
// loading briefings/<key> (or briefings/IJN05), stores the actual root, and
// reports native x87 progress. A partial map after an exception is retained;
// a later call consequently skips it. Page loading/pending domains belong to
// the existing loader and must not be treated as successful completion here.
void load_briefing_objective_pages_0051dda0(BriefingObjectivePageLoadBindings&);

struct MainMenuObjectiveProviders {
    virtual ~MainMenuObjectiveProviders() = default;
    virtual const MissionRecordData& selected_mission_005806a0() = 0;
    virtual BriefingObjectivePageMap& briefing_pages_00e198ac_64() = 0;
};
struct MainMenuObjectiveBindings {
    GuiWidgetOwnerRuntime& owners;
    NativeStringStorage& strings;
    GuiNativeNameCompare compare_names_00bf7fbf;
    MainMenuObjectiveProviders& providers;
    volatile std::int32_t& primary_count_64;
    volatile std::int32_t& secondary_count_68;
    volatile std::int32_t& hidden_count_6c;
    GuiLayoutWidget*& page_2c8;
    std::vector<GuiLayoutWidget*>& objective_groups_2cc;
    std::vector<GuiLayoutWidget*>& companion_groups_2dc;
    GuiLayoutWidget*& background_2ec;
};
// Complete normal58F5B0 (ECX main-menu screen, RET): clear borrowed vectors,
// bind current mission-side authored page, then append its contiguous primary
// and secondary direct-child Group pairs and hide each pair. Count fields and
// page slots reference the SAME screen used by5966F0. No Text or Group clones
// occur in this function;594BF0 separately builds its text/list rows.
// Native strings are used for child lookup and generated names. Borrowed
// widgets/mission/map survive callbacks; lookup reloads CURRENT page_2c8.
void bind_main_menu_objectives_0058f5b0(MainMenuObjectiveBindings&);

// Complete ordinary STL_inst_00519dc0, preserved library name: native ECX
// vector, unsigned index stack, RET4, EAX=begin+4*index POINTER TO ELEMENT.
// This new C++ interface throws out_of_range at the native BF6713 failure.
// It may return a null payload; only the consumer's dereference rejects null.
GuiLayoutWidget*& objective_vector_element_00519dc0(
    std::vector<GuiLayoutWidget*>&, std::uint32_t index);
GuiWidgetOwner& main_menu_objective_widget_00519dc0(
    MainMenuObjectiveBindings&, std::uint32_t screen_vector_offset,
    std::uint32_t index);
} // namespace bsp
