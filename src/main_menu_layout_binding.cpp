#include "bsp/main_menu_layout_binding.hpp"
#include "bsp/gui_type_dispatch.hpp"
#include <stdexcept>
#include <string>

namespace bsp {
namespace {
GuiLayoutWidget& child(GuiLayoutWidget& parent, std::string_view name) {
    auto* result = find_child_by_name_00aa7e00(parent, name);
    if (!result) throw std::runtime_error("main-menu layout: missing direct child " + std::string(name));
    const auto type = gui_widget_type_for_key_00aa2490(name);
    if (result->type != type) throw std::runtime_error("main-menu layout: incorrect widget type " + std::string(name));
    return *result;
}
GuiLayoutWidget* page(MainMenuLayoutEnvironment& e, const char* name) {
    auto* result = load_gui_page_00aa5840(e.pages, e.page_loader, name, 1, false);
    if (!result || !result->root) throw std::runtime_error("main-menu layout: missing page " + std::string(name));
    return result->root.get();
}
void hide(MainMenuLayoutEnvironment& e, GuiLayoutWidget* widget) {
    e.owners.owner(*widget).set_visible34(false);
}
void listener(MainMenuLayoutEnvironment& e, GuiLayoutWidget* widget) {
    //00AA6BC0/RET8, both stores, in the native order. This is the existing
    // retained owner's storage, not a second callback registry.
    auto& fields = e.owners.owner(*widget).extra_fields();
    fields.layout_listener_dc = e.screen_plus_40;
    fields.byte_79 = false;
}
} // namespace

void initialize_main_menu_list_00541b80(MainMenuListLayout& s,
    GuiLayoutWidget& group, std::string_view prototype, std::uint32_t count,
    std::int32_t spacing, const GuiTextColor& color0, const GuiTextColor& color1,
    MainMenuLayoutEnvironment& e) {
    s.group_18 = &group;
    s.prototype_1c = &child(group, prototype); //00541B95
    s.row_spacing_08 = spacing;
    s.row_count_04 = count;
    s.rows_20.resize(count);
    for (std::int32_t i = 0; i < static_cast<std::int32_t>(s.row_count_04); ++i) {
        auto* transform = clone_subtree(s.prototype_1c->transform, nullptr, e.clone_host); //00541BD7
        if (!transform) throw std::runtime_error("main-menu list clone allocation failed");
        auto& row = e.owners.owner(*transform);
        s.rows_20[static_cast<std::size_t>(i)] = &row.layout();
        //00541BE5 IMUL32, FILD/FDIV(double768)/FSTP(float), then three
        // separate adds/spills. It reads the prototype's LOCAL position.
        const auto product = static_cast<std::int32_t>(
            static_cast<std::uint32_t>(s.row_spacing_08) * static_cast<std::uint32_t>(i));
        const float step = static_cast<float>(static_cast<double>(product) / 768.0);
        const auto& base = s.prototype_1c->transform.position; //00AA67F0
        const GuiWidgetPoint pos{base.x + 0.0f,
            static_cast<float>(static_cast<double>(base.y) + step), base.z + 0.0f};
        row.set_position_00aa7dc0(pos); //00541C3B
        row.set_visible34(true); //00541C4D
    }
    s.color_30 = color0;
    s.color_40 = color1;
    s.field_2c = true;
    s.field_28 = 0;
    s.field_24 = 0;
}

void initialize_main_menu_text_00543710(MainMenuTextLayout& s,
    GuiLayoutWidget& group, std::string_view name, std::int32_t field04,
    const GuiTextColor& color0, const GuiTextColor& color1, const GuiTextColor& color2) {
    s.group_08 = &group;
    s.field_04 = field04;
    s.text_0c = &child(group, name); //00543759
    s.color_28 = color0;
    s.color_38 = color1;
    s.color_48 = color2;
    s.field_24 = true;
    s.field_20 = 0;
}

void bind_main_menu_layout_005861b0(MainMenuLayoutBindings& s, MainMenuLayoutEnvironment& e) {
    if (!e.background_completion_005861a0)
        throw std::invalid_argument("main-menu layout requires the real background-completion binding");
    s.main_listbox_page_470 = page(e, "FE_main_listbox"); //0058621C
    s.worldmap_2f0 = page(e, "FE_worldmap_historical"); //00586297
    hide(e, s.worldmap_2f0); //005862D1
    auto* main = page(e, "FE_main"); //0058631D, temporary EBX
    s.background_movie_340 = &child(*main, "bg_01_anim_Movie"); //0058638B
    set_movie_widget_completion_00aafee0(e.native.movie_state(*s.background_movie_340),
        e.background_completion_005861a0); //005863C9, current+7C
    s.briefing_grid_244 = page(e, "FE_briefing_grid"); //00586415
    hide(e, s.briefing_grid_244); //0058644F
    s.briefing_248 = page(e, "FE_briefing"); //0058649B
    hide(e, s.briefing_248); //005864D5
    auto& objectives = child(*s.briefing_248, "New_Group"); //0058651F, new EBX
    s.primary_text_24c = &child(objectives, "Primary_Text"); //0058658D
    hide(e, s.primary_text_24c); //005865C7
    s.number_text_250 = &child(objectives, "num_Text"); //0058660D
    hide(e, s.number_text_250); //00586647
    s.number2_text_254 = &child(objectives, "num_2_Text"); //0058668D
    hide(e, s.number2_text_254); //005866C7
    s.dest_text_258 = &child(objectives, "dest_Text"); //0058670D
    hide(e, s.dest_text_258); //00586747
    s.dest_color_270 = e.native.color54(*s.dest_text_258); //00586759
    s.dest2_text_25c = &child(objectives, "dest_2_Text"); //005867C2
    hide(e, s.dest2_text_25c); //005867FC
    s.dest2_color_260 = e.native.color54(*s.dest2_text_25c); //0058680E
    s.highlight_background_294 = &child(objectives, "hl_bg_FrameBox"); //00586877

    s.historical_group_344 = &child(*s.worldmap_2f0, "historical_Group"); //005868ED
    s.video_33c = &child(*s.historical_group_344, "video_Movie"); //00586963
    s.text_background_568 = &child(*s.historical_group_344, "bigtextbg_FrameBox"); //005869D9
    s.briefing_clipbox_3b0 = &child(*s.text_background_568, "test_Clipbox"); //00586A4F
    s.briefing_textbox_350 = &child(*s.briefing_clipbox_3b0, "textbox_Group"); //00586AC5
    attach_content(s.briefing_scroller_354, s.briefing_textbox_350, e.scroller_host); //00586B02
    s.briefing_content_3b8 = &child(*s.briefing_textbox_350, "content_main_Text"); //00586B4F
    auto& slider = child(*s.text_background_568, "slider_Group"); //00586BC9
    s.briefing_scroll_top_348 = &child(slider, "arrow_top_scroll_Icon"); //00586C3B
    listener(e, s.briefing_scroll_top_348); //00586C78
    s.briefing_scroll_bottom_34c = &child(slider, "arrow_botton_scroll_Icon"); //00586CC1
    listener(e, s.briefing_scroll_bottom_34c); //00586CFE
    s.briefing_scroll_thumb_3b4 = &child(slider, "textx_scroll_FrameBox"); //00586D47
    listener(e, s.briefing_scroll_thumb_3b4); //00586D84
    auto& track = child(slider, "textx_scroll_bg_Icon"); //00586DCD
    //00586DFD captures background height as double;00586E1D reads thumb.
    const double background_height = widget_size(track.transform).height;
    const float thumb_height = widget_size(s.briefing_scroll_thumb_3b4->transform).height;
    const float length = static_cast<float>(background_height - thumb_height);
    attach_scroll_bar(s.briefing_scroller_354, s.briefing_scroll_thumb_3b4, length,
        s.briefing_scroll_top_348, s.briefing_scroll_bottom_34c, e.scroller_host); //00586E3E

    s.mission_picture_group_324 = &child(*s.worldmap_2f0, "mission_pic_Group"); //00586E8B
    auto& detail = *s.mission_picture_group_324;
    s.text_background_568 = &child(detail, "textbackground_FrameBox"); //00586F04: overwrite SAME+568
    listener(e, s.text_background_568); //00586F3D
    s.selector_338 = &child(*s.worldmap_2f0, "selector_Icon"); //00586F8A
    s.map_point_template_114 = &child(*s.worldmap_2f0, "mission_mappoint_template_Icon"); //00587000
    hide(e, s.map_point_template_114); //0058703A
    s.map_flag_template_118 = &child(*s.worldmap_2f0, "mission_mapflag_template_Icon"); //00587084
    hide(e, s.map_flag_template_118); //005870BE
    s.mission_picture_2f4 = &child(detail, "missionpicture_Icon"); //00587108
    s.mission_name_304 = &child(detail, "mission_name_Text"); //0058717E
    s.mission_name_background_334 = &child(detail, "mission_name_backg_FrameBox"); //005871F4
    hide(e, s.mission_name_background_334); //0058722E
    s.clipbox_230 = &child(detail, "test_Clipbox"); //00587278
    s.textbox_1d0 = &child(*s.clipbox_230, "textbox_Group"); //005872EE
    attach_content(s.mission_scroller_1d4, s.textbox_1d0, e.scroller_host); //0058732B
    s.scroll_top_1c8 = &child(detail, "arrow_top_scroll_Icon"); //00587378
    s.scroll_bottom_1cc = &child(detail, "arrow_botton_scroll_Icon"); //005873EE
    s.scroll_thumb_234 = &child(detail, "textx_scroll_FrameBox"); //00587464
    listener(e, s.scroll_thumb_234); //0058749D
    attach_scroll_bar(s.mission_scroller_1d4, s.scroll_thumb_234, 0.10277777910232544f,
        s.scroll_top_1c8, s.scroll_bottom_1cc, e.scroller_host); //005874C3
    s.mission_content_30c = &child(*s.textbox_1d0, "content_main_Text"); //00587510
    s.mission_date_308 = &child(*s.textbox_1d0, "mission_date_Text"); //00587586
    s.checkpoint_text_300 = &child(detail, "checkpoint_Text"); //005875FC
    s.checkpoint_background_2fc = &child(detail, "checkpoint_FrameBox"); //00587672
    s.medal_2f8 = &child(detail, "medal1_Icon"); //005876E8
    s.missions_us_310 = &child(*s.worldmap_2f0, "missions_US_Group"); //0058775E
    s.missions_jp_314 = &child(*s.worldmap_2f0, "missions_JP_Group"); //005877D4
    s.missions_us_dlc_318 = &child(*s.worldmap_2f0, "missions_US_DLC_Group"); //0058784A
    s.missions_jp_dlc_31c = &child(*s.worldmap_2f0, "missions_JP_DLC_Group"); //005878C0
    s.training_320 = &child(*s.worldmap_2f0, "training_Group"); //00587936
    s.backdrop_328 = &child(*s.worldmap_2f0, "bg_01_Icon"); //005879AC
    auto& backdrop_owner = e.owners.owner(*s.backdrop_328);
    auto* backdrop_icon = dynamic_cast<GuiIconTypeImplementation*>(&backdrop_owner.implementation());
    if (!backdrop_icon) throw std::logic_error("main-menu backdrop requires its actual Icon implementation");
    backdrop_icon->runtime().set_scale48_00ab2820(backdrop_owner,
        {1.0000100135803223f, 1.0000100135803223f}); //005879FE, derived+48/raw00AB2820
    s.background_32c = &child(*s.worldmap_2f0, "background_Icon"); //00587A48
    s.backdrop_size_11c = widget_size(s.backdrop_328->transform); //00587A7C
    s.backdrop_position_124 = resolved_position(s.backdrop_328->transform); //00587A9D
    s.selected_map_point_330 = nullptr; //00587AC8

    auto& listpage = *s.main_listbox_page_470;
    s.listbox_group_3bc = &child(listpage, "ListBox_Group"); //00587B0A
    s.small_listbox_group_3c0 = &child(listpage, "SmallListBox_Group"); //00587B80
    s.sub_listbox_23c = &child(listpage, "Sub_Listbox"); //00587BF6
    e.native.call_00a9ac40(*s.sub_listbox_23c, e.screen_plus_8); //00587C2E
    s.sub_text_240 = &child(*s.sub_listbox_23c, "MainListbox_Text"); //00587C7B
    e.native.text_color50(*s.sub_text_240,
        {0.8039219975471497f, 0.5843139886856079f, 0.3294120132923126f, 1.0f}); //00587CF1
    s.main_listbox_1b8 = &child(listpage, "Main_Listbox"); //00587D3B
    e.native.call_00a9ac40(*s.main_listbox_1b8, e.screen_plus_8); //00587D70
    s.main_listbox_position_558 = resolved_position(s.main_listbox_1b8->transform); //00587D80
    s.main_text_238 = &child(*s.main_listbox_1b8, "MainListbox_Text"); //00587DE7
    s.arrow_top_1bc = &child(detail, "arrow_top_Icon"); //00587E5D
    listener(e, s.arrow_top_1bc); //00587E96
    s.arrow_bottom_1c0 = &child(detail, "arrow_botton_Icon"); //00587EE3
    listener(e, s.arrow_bottom_1c0); //00587F19
    listener(e, &child(detail, "arrow_botton_scroll_Icon")); //00587F68 then00587F6F
    listener(e, &child(detail, "arrow_top_scroll_Icon")); //00587FE1 then00587FE8
    e.native.text_state80(*s.main_text_238, 0); //0058801F
    initialize_main_menu_list_00541b80(s.list_layout_3c4, *s.listbox_group_3bc,
        "ListBox_Text", 24, 20, {0.5f, 0.5f, 0.5f, 1.0f}, {1, 1, 1, 1}, e); //005880F5
    auto& textgroup = child(listpage, "TextBox_Group"); //00588235
    initialize_main_menu_text_00543710(s.text_layout_414, textgroup, "TextBox_Text", 32,
        {0.5f, 0.5f, 0.5f, 1.0f}, {1, 1, 1, 1}, {0.2f, 0.2f, 0.2f, 1.0f}); //00588241
}
} // namespace bsp
