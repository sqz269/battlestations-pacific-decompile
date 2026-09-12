#pragma once
#include "bsp/main_menu_command_listener.hpp"
#include "bsp/main_menu_command_owner.hpp"
#include "bsp/gui_text_type_dispatch.hpp"

namespace bsp {

// Required providers over the SAME live screen/resource domain. Address names
// retain the callee boundaries which still lack a canonical implementation.
// No method has a successful/no-op default. Resolver methods only resolve
// existing identity; they do not construct another screen, table or GUI tree.
struct MainMenuSelectionServices {
    virtual ~MainMenuSelectionServices() = default;
    virtual MissionTreeTables& mission_tree_00e198ac_5c() = 0;
    virtual MainMenuCommandOwner& command_owner_00e1930c() = 0;
    virtual MainMenuCommandBarHost& command_host(MainMenuCommandOwner&) = 0;
    virtual const MainMenuCommandBarEnvironment& command_environment(MainMenuCommandOwner&) = 0;
    // Mission reader5C75CC resolves picture into retained texture+A0 and UV+A4.
    // extra.picture is its authored NAME, not the resulting texture identity.
    virtual void* mission_picture_texture_0a0(const MissionRecordData&) = 0;
    // ECX Icon; state, retained texture pointer, UV pointer; RET0C.
    virtual void call_00ab2690(GuiWidgetOwner&, std::uint32_t state,
        void* texture, const GuiUvRect&) = 0;
    // ECX Icon; caller-owned output then state index; RET8, returns output.
    virtual void call_00ab27a0(GuiWidgetOwner&, GuiWidgetSize& output,
        std::uint32_t state) = 0;
    // ECX source widget, EDX destination widget, float stack, RET4. This
    // computes size/position using AC06C0; neither register may be omitted.
    virtual void call_00ac0820(GuiWidgetOwner& source, GuiWidgetOwner& destination,
        float width) = 0;
    // ECX output native8h header; EDX=date+60; stack date+64,+68; RET8.
    // Language-sensitive builder. The provider constructs output, including
    // all native temporary allocations, rather than returning formatted text.
    virtual void call_0043bc30(NativeString& output, std::uint32_t date_60,
        std::uint32_t date_64, std::uint32_t date_68, NativeStringStorage&) = 0;
    // Full normal callees remain required: actual highlight-background294
    // placement/size (RET4), and profile mission-score medal2F8 update (RET).
    virtual void call_00580820(GuiWidgetOwner& selected_objective) = 0;
    virtual void call_00594b60() = 0;
    // SAME screen-owned vector at +2CC/+2DC, through519DC0; payload is the
    // actual widget whose +4C node is used by B6DA70. No synthetic node map.
    virtual GuiWidgetOwner& call_00519dc0(std::uint32_t screen_vector_offset,
        std::uint32_t index) = 0;
    virtual void call_00b6da70(NativeNodeBinding&, float factor, bool recurse) = 0;
    // Actual FrameBox current58 (ACF120), including geometry rebuild. Native
    // caller captures its implementation before passing the size pair.
    virtual void framebox_current58(GuiWidgetOwner&, const GuiWidgetSize&) = 0;
};

// These references add no menu state. Bind to the SAME screen fields/global
// publication slots used by its producers and MainMenuCommandListenerBindings.
// +64/+68/+6C are authored primary/secondary Group counts and hidden objective
// count, written by58F5B0 through make_main_menu_objective_bindings.
struct MainMenuSelectionListenerBindings {
    MainMenuCommandListenerBindings& command;
    MainMenuSelectionServices& services;
    GuiNativeNameCompare compare_names_00bf7fbf;
    std::int32_t& field_60;
    volatile std::int32_t& field_64;
    volatile std::int32_t& field_68;
    volatile std::int32_t& field_6c;
    GuiLayoutWidget*& active_mission_group_110;
    std::uint8_t& arrow_bottom_enabled_1c4;
    std::uint8_t& arrow_top_enabled_1c5;
    volatile std::uint32_t& selected_00e194c4;
    volatile std::uint32_t& selected_00e194c8;
    volatile std::uint32_t& selected_00e194cc;
    const volatile std::uint32_t& group_00e194d8;
    volatile std::int32_t& previous_00e194e0;
    const volatile std::uint8_t& glyph_mode_00f88a30;
    const volatile float& objective_factor_00ce3e18;
    const volatile float& map_flag_alpha_00ce7804;
    const volatile float& arrow_duration_00d7a2f0;
    const volatile double& width_divisor_00cec380;
    const volatile double& text_padding_00ceed60;
    // Borrow the real pointer tables; valid selected indices are the domain.
    // Native performs no bound check, including on -1 for these help pages.
    const char* const volatile* help_00e087d4;
    const char* const volatile* help_00e08808;
    const char* const volatile* help_00e08828;
    const char* const volatile* help_00e0884c;
    const char* const volatile& objective_help_00e08868;
    const char* const volatile& objective_help_00e0886c;
};

// Complete normal caller sequence005966F0..0059786C; original ECX=screen+8,
// CEFC48+08, selected-row/Listbox stack operands, RET8. BOTH operands are
// unread: the first A9C990 resolves the screen's CURRENT +1B8 instead. Keep
// both operands in the public listener contract; a rebound screen1B8 is valid.
// Providers above remain partial; no original SEH/debug-iterator/native ABI,
// complete menu provider, rendering or gameplay equivalence is claimed.
// All borrowed owners, actual type profiles, record containers and list nodes
// must survive callbacks. Pending Text work requires the caller's retained
// continuation; this ordinary completed-content interface cannot resume it.
void main_menu_listener_current08_005966f0(MainMenuSelectionListenerBindings&,
    GuiWidgetOwner* selected_row, GuiWidgetOwner& callback_listbox);

// Full normal native string helper bodies in the new NativeString interface;
// 4260B0 ECX output,int stack,RET4;4263B0 ECX prefix,output/int stack,RET8.
// Output must be a fresh header; both routines clear it before constructing.
NativeString& native_string_from_int_004260b0(NativeString& output,
    std::int32_t value, NativeStringStorage&);
NativeString& native_string_concat_int_004263b0(const NativeString& prefix,
    NativeString& output, std::int32_t value, NativeStringStorage&);
//5C35D0 ECX same mission-tree owner,key stack,RET4. Existing last-match find;
// unknown key preserves both current selection fields.
void select_mission_by_id_005c35d0(MissionTreeTables&, const MissionRecordData&);
} // namespace bsp
