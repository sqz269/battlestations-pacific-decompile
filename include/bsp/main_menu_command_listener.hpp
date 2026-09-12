#pragma once
#include "bsp/main_menu_widget_listener.hpp"
#include "bsp/frontend_prompts.hpp"
#include "bsp/mission_tree_data.hpp"
#include "bsp/profile_reset.hpp"

namespace bsp {
class GuiListboxRuntime;

// The stack temporary produced by 7FC490 -> 437490: three words followed by
// one owned8h native string. This is a returned value, not another profile.
// No inferred defaults: the actual copy-producing service writes every field.
struct MainMenuCommandCheckpointValue {
    std::uint32_t words[3];
    NativeString text_0c;
};
#if INTPTR_MAX == INT32_MAX
static_assert(sizeof(MainMenuCommandCheckpointValue) == 0x14);
#endif

// Required bindings to the actual screen/provider. Address names deliberately
// retain incomplete callee contracts; no success/no-op defaults are supplied.
// Each page method operates on the SAME screen as MainMenuWidgetListenerBindings.
// The report distinguishes complete caller sequencing from these dependencies.
struct MainMenuCommandServices {
    virtual ~MainMenuCommandServices() = default;
    virtual GuiListboxRuntime& listbox_runtime(GuiWidgetOwner&) = 0;
    // Existing canonical model/transform service for AA8240's two publications.
    virtual GuiWidgetTransformHost& transform_host(GuiWidgetOwner&) = 0;
    // Fresh actual global owner reads; capture profile BEFORE selected mission.
    virtual ProfileResetState& profile_00e188a8_650() = 0;
    virtual const MissionRecordData& selected_mission_005806a0() = 0;
    // Profile+94 checkpoint map lookup by record.screen.name, RET4 Boolean.
    virtual bool call_007f8d60(ProfileResetState&, const MissionRecordData&) = 0;
    // RET8: output14h then mission-name pointer; deep-copy string via437490.
    virtual void call_007fc490(ProfileResetState&, MainMenuCommandCheckpointValue&,
        const MissionRecordData&, NativeStringStorage&) = 0;
    // Map entry exists and value+8==-1 or globals.VehicleClass[value+8] is table.
    virtual bool call_007fc370(ProfileResetState&, const MissionRecordData&) = 0;
    // RET8; first Boolean stack operand is unread, second is vehicle-class id.
    // -1 succeeds; otherwise VehicleClass/Unlock/UnlockID/profile predicate.
    virtual bool call_00584750(bool first_argument, std::int32_t vehicle_class) = 0;
    virtual FrontEndPromptScreen& prompt_00425d10() = 0;
    virtual FrontEndPromptHost& prompt_host(FrontEndPromptScreen&) = 0;
    virtual void call_00588a80() = 0;
    // CEFC48+04 =598B60, ECX screen+8, RET8. BOTH arguments are retained.
    virtual void call_00598b60(GuiWidgetOwner* selected, GuiWidgetOwner& listbox) = 0;
    virtual void call_00584ae0(bool select_first) = 0; // RET4
    virtual void call_00584f50() = 0;
    virtual void call_0058c010() = 0;
    virtual void call_005922f0() = 0;
    virtual void call_00594bf0() = 0; // complete caller in main_menu_objective_rows
    virtual void call_00599340() = 0;
    virtual void call_005885d0() = 0;
    virtual void call_005886c0() = 0;
    // Actual [E198AC]+70 owner fields, stored together at5999BD..5999CD.
    // Provider writes +9C=mission,+A0=side,+A4=flag on the SAME current owner.
    virtual void tactical_selection_fields(const MissionRecordData&, std::int32_t side,
        bool flag) = 0;
};

struct MainMenuCommandListenerBindings {
    MainMenuWidgetListenerBindings& widget;
    MainMenuCommandServices& services;
    NativeStringStorage& strings;
    FrontEndScrollerHost& scroller_host;
    std::uint8_t& mission_flag_5c;
    volatile std::uint32_t& selected_00e194dc;
    volatile std::uint32_t& selected_00e08878;
    const volatile float& zoom_out_00ce69cc;
    const volatile float& zoom_in_00ce54a0;
    const volatile float& scroll_step_00cec178;
    MainMenuCommandWidgetListView widgets_280{widget.layout.descriptions_280.command_view()};
    MainMenuCommandWidgetListView widgets_298{widget.layout.descriptions_298.command_view()};
    MainMenuCommandWidgetListView widgets_2a4{widget.layout.descriptions_2a4.command_view()};
};

// Complete normal5993A0..599D57 caller control flow, with required addressed
// page/profile/navigation providers above. Native ECX screen+40, widget stack,
// RET4; FLD1/current4C precedes the fresh type5C read for EVERY widget.
// Caller keeps all borrowed screen/widget/list nodes and providers alive across
// callbacks, including callbacks which rebind layout slots. No native SEH/vtable
// replacement, no complete screen provider or gameplay claim.
void main_menu_listener_current04_005993a0(MainMenuCommandListenerBindings&,
    GuiWidgetOwner& widget);
} // namespace bsp
