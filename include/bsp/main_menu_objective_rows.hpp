#pragma once
#include "bsp/main_menu_selection_services.hpp"
#include "bsp/gui_text_child_lifetime.hpp"
#include "bsp/gui_text_resource_names.hpp"
#include "bsp/gui_listbox_runtime.hpp"

namespace bsp {
// Complete ordinary5896F0 arithmetic and length_error decision, with its
// existing library name preserved in Ghidra. Native ECX list, increment stack,
// RET4; this new interface aliases the original list+8 count only.
void grow_main_menu_objective_list_count_005896f0(volatile std::uint32_t& count,
    std::uint32_t increment);
// Mutable aliases of the SAME +280/+298/+2A4 lists exposed by the command
// listener. No second list, GUI tree, ownership index, or widget allocation.
class MainMenuObjectiveWidgetList final {
public:
    //582280 successful node allocation;590428/2B stores sole head/count0.
    MainMenuObjectiveWidgetList();
    ~MainMenuObjectiveWidgetList(); // C++ node ownership only, borrowed widgets.
    MainMenuObjectiveWidgetList(const MainMenuObjectiveWidgetList&) = delete;
    MainMenuObjectiveWidgetList& operator=(const MainMenuObjectiveWidgetList&) = delete;
    MainMenuCommandWidgetListView command_view() const noexcept { return {head_04_, count_08_}; }
    //5823B0's three words,5896F0's checked count, then594BF0's two link stores.
    void append_005823b0_005896f0(GuiLayoutWidget&);
private:
    MainMenuCommandWidgetListNode* head_04_;
    std::uint32_t count_08_{};
};
struct MainMenuObjectiveRowsProviders {
    virtual ~MainMenuObjectiveRowsProviders() = default;
    virtual MissionProgress& progress_00e188a8_6b4() = 0;
    // 518D60: ECX transition, EDX second word, stack third word/name-header,
    // RET8. Its frame/subtitle children are not implemented by this caller.
    virtual void call_00518d60(std::uint32_t ecx, std::uint32_t edx,
        std::uint32_t stack_word, const std::string& mission_title_08) = 0;
};
struct MainMenuObjectiveRowsBindings {
    MainMenuSelectionListenerBindings& selection;
    MainMenuObjectiveProviders& objective_providers;
    MainMenuObjectiveRowsProviders& providers;
    GuiWidgetSceneHost& clone_host;
    MainMenuLayoutNativeCalls& layout_native;
    MovieWidgetHost& movies;
    GuiTextResourceNameServices& text_names;
    GuiTextChildDeletion& deletion;
    void* screen_plus_40;
    volatile std::int32_t& page_00e08874;
    std::uint8_t& field_2c4;
    const volatile float& field_2b0;
    const volatile float& field_2b4;
    GuiLayoutWidget*& primary_header_2b8;
    GuiLayoutWidget*& secondary_header_2bc;
    GuiLayoutWidget*& hidden_header_2c0;
    MainMenuObjectiveWidgetList& primary_280;
    MainMenuObjectiveWidgetList& secondary_298;
    MainMenuObjectiveWidgetList& hidden_2a4;
    const volatile double& depth_00ce47a0;
};

// Complete ordinary synchronous caller00594BF0..005966E4, including the
// original post-free epilogue missing from Ghidra's stored function body.
// Native ECX main-menu, RET, no stack arguments. New C++ ABI only.
// Uses the SAME screen counts/layout/node lists and canonical Text/Listbox
// companions. All allocation, Text content, cloning, scene and deletion calls
// must complete synchronously. A missing/pending dependency stops at its
// actual call; resuming that child alone does not resume this outer operation.
// Borrowed screen, mission, score record, providers and currently traversed
// container nodes survive callbacks. Native STL/SEH/allocator reentry and
// corrupt iterators are outside this domain. See report for dependencies.
void build_main_menu_objective_rows_00594bf0(MainMenuObjectiveRowsBindings&);
} // namespace bsp
