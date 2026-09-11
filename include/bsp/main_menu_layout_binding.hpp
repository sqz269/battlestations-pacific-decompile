#pragma once
#include "bsp/frontend_screen_animation.hpp"
#include "bsp/gui_text.hpp"
#include "bsp/gui_widget_owner.hpp"
#include "bsp/movie_player.hpp"
#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

namespace bsp {

// Hypothesis names; normal-path projections, not native object layouts/ABIs.
// 00541B80 (RET30h): list helper embedded at main-menu+3C4h. The three
// leading/other constructor-owned fields are intentionally not modelled here.
struct MainMenuListLayout {
    std::uint32_t row_count_04{};
    std::int32_t row_spacing_08{};
    GuiLayoutWidget* group_18{};
    GuiLayoutWidget* prototype_1c{};
    std::vector<GuiLayoutWidget*> rows_20; // borrowed; native GUI owns the clones
    std::uint32_t field_24{}, field_28{};
    bool field_2c{};
    GuiTextColor color_30{}, color_40{};
};
// 00543710 (RET3Ch): text helper embedded at main-menu+414h.
struct MainMenuTextLayout {
    std::int32_t field_04{};
    GuiLayoutWidget* group_08{};
    GuiLayoutWidget* text_0c{};
    std::uint32_t field_20{};
    bool field_24{};
    GuiTextColor color_28{}, color_38{}, color_48{};
};

// Every stored widget is borrowed from the existing GUI page graph. Field
// suffixes are ORIGINAL screen offsets, not offsetof values in this projection.
// +568 is deliberately a single slot overwritten by the second lookup.
struct MainMenuLayoutBindings {
    GuiLayoutWidget *map_point_template_114{}, *map_flag_template_118{};
    GuiWidgetSize backdrop_size_11c{};
    GuiWidgetPoint backdrop_position_124{};
    GuiLayoutWidget *main_listbox_1b8{}, *arrow_top_1bc{}, *arrow_bottom_1c0{};
    GuiLayoutWidget *scroll_top_1c8{}, *scroll_bottom_1cc{}, *textbox_1d0{};
    FrontEndScreenScroller mission_scroller_1d4{};
    GuiLayoutWidget *clipbox_230{}, *scroll_thumb_234{}, *main_text_238{};
    GuiLayoutWidget *sub_listbox_23c{}, *sub_text_240{};
    GuiLayoutWidget *briefing_grid_244{}, *briefing_248{}, *primary_text_24c{};
    GuiLayoutWidget *number_text_250{}, *number2_text_254{}, *dest_text_258{}, *dest2_text_25c{};
    GuiTextColor dest2_color_260{}, dest_color_270{};
    GuiLayoutWidget *highlight_background_294{}, *worldmap_2f0{};
    GuiLayoutWidget *mission_picture_2f4{}, *medal_2f8{}, *checkpoint_background_2fc{};
    GuiLayoutWidget *checkpoint_text_300{}, *mission_name_304{}, *mission_date_308{}, *mission_content_30c{};
    GuiLayoutWidget *missions_us_310{}, *missions_jp_314{}, *missions_us_dlc_318{}, *missions_jp_dlc_31c{}, *training_320{};
    GuiLayoutWidget *mission_picture_group_324{}, *backdrop_328{}, *background_32c{};
    GuiLayoutWidget *selected_map_point_330{}, *mission_name_background_334{}, *selector_338{};
    GuiLayoutWidget *video_33c{}, *background_movie_340{}, *historical_group_344{};
    GuiLayoutWidget *briefing_scroll_top_348{}, *briefing_scroll_bottom_34c{}, *briefing_textbox_350{};
    FrontEndScreenScroller briefing_scroller_354{};
    GuiLayoutWidget *briefing_clipbox_3b0{}, *briefing_scroll_thumb_3b4{}, *briefing_content_3b8{};
    GuiLayoutWidget *listbox_group_3bc{}, *small_listbox_group_3c0{};
    MainMenuListLayout list_layout_3c4{};
    MainMenuTextLayout text_layout_414{};
    GuiLayoutWidget* main_listbox_page_470{};
    GuiWidgetPoint main_listbox_position_558{};
    GuiLayoutWidget* text_background_568{};
};

// Actual derived-widget boundaries. All are required and must resolve the
// supplied widget's SAME runtime object; none may fabricate a second GUI tree.
struct MainMenuLayoutNativeCalls {
    virtual ~MainMenuLayoutNativeCalls() = default;
    virtual MovieWidgetState& movie_state(GuiLayoutWidget&) = 0;
    //00A9AC40 writes Listbox+114h. Keep the actual screen+8 pointer.
    virtual void call_00a9ac40(GuiLayoutWidget&, void* screen_plus_8) = 0;
    // Current Text+54h /00AA68F0 reads material diffuse when present, else
    // widget color. Reading GuiLayoutWidget::color alone is not equivalent.
    virtual GuiTextColor color54(GuiLayoutWidget&) = 0;
    // Current Text+50h /00AB6B50 also updates shadow material alpha.
    virtual void text_color50(GuiLayoutWidget&, const GuiTextColor&) = 0;
    // Current Text+80h /00AB7200 chooses disabled/state color then calls+50h.
    virtual void text_state80(GuiLayoutWidget&, std::int32_t state) = 0;
};

struct MainMenuLayoutEnvironment {
    GuiPageRegistry& pages;
    GuiLayoutHost& page_loader;
    GuiWidgetOwnerRuntime& owners;
    GuiWidgetSceneHost& clone_host;
    FrontEndScrollerHost& scroller_host;
    MainMenuLayoutNativeCalls& native;
    void* screen_plus_8;  // actual navigation-owner subobject
    void* screen_plus_40; // actual widget-listener subobject
    // Reconstructed host binding for raw005861A0: load global manager+58h,
    // tail-jump005845D0. Never cast the original VA to a callable pointer.
    MovieCompletionCallback background_completion_005861a0;
};

// Complete normal-path sequencing of005861B0..0058827B, native __thiscall
// ECX=578h main-menu screen, no stack args, RET. Five page loads and 55 direct
// child lookups; every scalar/store/virtual/helper call retained. Native string
// allocator and SEH mechanics are replaced by existing C++ string ownership.
// Missing page/child/type/owner is an explicit failure, not a dummy binding.
// This is reconstructed source, not an ABI or game-validated replacement.
void bind_main_menu_layout_005861b0(MainMenuLayoutBindings&, MainMenuLayoutEnvironment&);

// The helper arguments are grouped as native by-value color quartets;
// RET30h and RET3Ch establish12/15 stack words, respectively.
void initialize_main_menu_list_00541b80(MainMenuListLayout&, GuiLayoutWidget& group,
    std::string_view prototype, std::uint32_t count, std::int32_t spacing,
    const GuiTextColor&, const GuiTextColor&, MainMenuLayoutEnvironment&);
void initialize_main_menu_text_00543710(MainMenuTextLayout&, GuiLayoutWidget& group,
    std::string_view text, std::int32_t field_04,
    const GuiTextColor&, const GuiTextColor&, const GuiTextColor&);
} // namespace bsp
