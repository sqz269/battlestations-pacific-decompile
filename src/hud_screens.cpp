#include "bsp/hud_screens.hpp"

namespace bsp {
namespace {

// ASCII case-insensitive compare: the binary writes "GUI_markers", the
// installed file is gui_markers.lua, and the loader resolves the two.
char lower_ascii(char c) noexcept {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

bool equal_ignoring_case(const char* a, const char* b) noexcept {
    if (a == nullptr || b == nullptr) {
        return false;
    }
    while (*a != '\0' && *b != '\0' && lower_ascii(*a) == lower_ascii(*b)) {
        ++a;
        ++b;
    }
    return *a == '\0' && *b == '\0';
}

const char* const kPages44[] = {
    "GUI_powerups",
    "GUI_unit",
    "GUI_selector",
};
const HudScreenWidgetBinding kWidgets44[] = {
    {"puptemplate_Icon", HudScreenVirtual::Layout},
    {"circletemplate_Section", HudScreenVirtual::Layout},
    {"unit_name_Text", HudScreenVirtual::Layout},
    {"FlagJP_Icon", HudScreenVirtual::Layout},
    {"FlagUS_Icon", HudScreenVirtual::Layout},
    {"unit_payload_Icon", HudScreenVirtual::Layout},
    {"unit_HP_damage_Icon", HudScreenVirtual::Layout},
    {"unit_HP_healthy_Icon", HudScreenVirtual::Layout},
    {"unit_command_Icon", HudScreenVirtual::Layout},
    {"Units_Group", HudScreenVirtual::Layout},
    {"ClosedUnitHUD_Group", HudScreenVirtual::Layout},
    {"Medal_Icon", HudScreenVirtual::Layout},
    {"Medal_Text", HudScreenVirtual::Layout},
    {"WeaponInfo_Text", HudScreenVirtual::Layout},
};
const char* const kPages4D[] = {
    "GUI_markers",
};
const char* const kPages26[] = {
    "GUI_binoculars",
};
const HudScreenWidgetBinding kWidgets26[] = {
    {"Tavcso_Model", HudScreenVirtual::Enter},
};
const char* const kPages2E[] = {
    "GUI_cross_gunstate",
    "GUI_cross_ship",
};
const HudScreenWidgetBinding kWidgets2E[] = {
    {"ship_AA_Group", HudScreenVirtual::Layout},
    {"ship_art_Group", HudScreenVirtual::Layout},
    {"ship_rocket_Group", HudScreenVirtual::Layout},
    {"ship_torpedo_Group", HudScreenVirtual::Layout},
    {"ship_DC_Group", HudScreenVirtual::Layout},
    {"cross_botton_Icon", HudScreenVirtual::Layout},
    {"cross_left_Icon", HudScreenVirtual::Layout},
    {"cross_right_Icon", HudScreenVirtual::Layout},
    {"cross_F_Icon", HudScreenVirtual::Layout},
    {"cross_H_Icon", HudScreenVirtual::Layout},
    {"cross_HF_Icon", HudScreenVirtual::Layout},
    {"cross_L_Icon", HudScreenVirtual::Layout},
    {"cross_F2_Icon", HudScreenVirtual::Layout},
    {"CrosshairDisable_Icon", HudScreenVirtual::Layout},
    {"GunState_Icon", HudScreenVirtual::Layout},
    {"circle_Section", HudScreenVirtual::Layout},
};
const char* const kPages4C[] = {
    "GUI_map",
    "GUI_classicons",
};
const HudScreenWidgetBinding kWidgets4C[] = {
    {"All_Group", HudScreenVirtual::Enter},
    {"PlayerNameTemplate_Text", HudScreenVirtual::Enter},
    {"PlayerTalkingTemplate_Icon", HudScreenVirtual::Enter},
    {"radio_Icon", HudScreenVirtual::Enter},
    {"actunit_Icon", HudScreenVirtual::Enter},
    {"PupTemplate_Icon", HudScreenVirtual::Enter},
    {"unit_name_Text", HudScreenVirtual::Enter},
    {"WarningTemplate_Icon", HudScreenVirtual::Enter},
    {"HealthTemplate_Icon", HudScreenVirtual::Enter},
    {"JapCaptureTemplate_Icon", HudScreenVirtual::Enter},
    {"USCaptureTemplate_Icon", HudScreenVirtual::Enter},
    {"HealthTemplate2_Icon", HudScreenVirtual::Enter},
    {"distance_A_Icon", HudScreenVirtual::Enter},
    {"distance_B_Icon", HudScreenVirtual::Enter},
    {"map_corner_Icon", HudScreenVirtual::Enter},
    {"popup_FrameBox", HudScreenVirtual::Enter},
    {"select_Icon", HudScreenVirtual::Enter},
    {"target_Icon", HudScreenVirtual::Enter},
    {"mission_HL_Icon", HudScreenVirtual::Enter},
    {"filter_panel_Icon", HudScreenVirtual::Enter},
    {"filter_item_Icon", HudScreenVirtual::Enter},
    {"filter_panel2_Icon", HudScreenVirtual::Enter},
    {"filter_Text", HudScreenVirtual::Enter},
    {"map_class_Icon", HudScreenVirtual::Enter},
};
const char* const kPages35[] = {
    "GUI_minimap",
};
const HudScreenWidgetBinding kWidgets35[] = {
    {"minimap_islandmap_Icon", HudScreenVirtual::Register},
    {"capture_icon_Group", HudScreenVirtual::Register},
    {"icon_big_item_Icon", HudScreenVirtual::Register},
    {"icon_big_item_hit_Icon", HudScreenVirtual::Register},
    {"icon_big_Icon", HudScreenVirtual::Register},
    {"bluecapture_Section", HudScreenVirtual::Register},
    {"redcapture_Section", HudScreenVirtual::Register},
    {"capturebg_Section", HudScreenVirtual::Register},
    {"minimap_compass_Icon", HudScreenVirtual::Layout},
    {"minimap_units_white_Group", HudScreenVirtual::Layout},
    {"item_ship_Icon", HudScreenVirtual::Layout},
    {"item_plane_Icon", HudScreenVirtual::Layout},
    {"item_sub_Icon", HudScreenVirtual::Layout},
    {"item_building_Icon", HudScreenVirtual::Layout},
    {"item_marker_Icon", HudScreenVirtual::Layout},
    {"minimap_units_red_Group", HudScreenVirtual::Layout},
    {"item_ship_Icon", HudScreenVirtual::Layout},
    {"minimap_units_blue_Group", HudScreenVirtual::Layout},
    {"item_ship_Icon", HudScreenVirtual::Layout},
    {"minimap_units_grey_Group", HudScreenVirtual::Layout},
    {"item_ship_Icon", HudScreenVirtual::Layout},
    {"minimap_units_yellow_Group", HudScreenVirtual::Layout},
    {"item_ship_Icon", HudScreenVirtual::Layout},
    {"minimap_units_silver_Group", HudScreenVirtual::Layout},
    {"item_ship_Icon", HudScreenVirtual::Layout},
    {"minimap_dir_Icon", HudScreenVirtual::Layout},
    {"unit_marker_Group", HudScreenVirtual::Layout},
};
const char* const kPages2A[] = {
    "GUI_formation",
    "GUI_classicons",
    "GUI_unit",
    "GUI_formation_circle",
    "GUI_formationOvrly",
};
const HudScreenWidgetBinding kWidgets2A[] = {
    {"ship_plane_agressive_Icon", HudScreenVirtual::Register},
    {"ship_plane_defensive_Icon", HudScreenVirtual::Register},
    {"form_line_Icon", HudScreenVirtual::Register},
    {"form_side_Icon", HudScreenVirtual::Register},
    {"form_gem_Icon", HudScreenVirtual::Register},
    {"all_ship_leave_form_Icon", HudScreenVirtual::Register},
    {"cancel_Icon", HudScreenVirtual::Register},
    {"repair_hl_west_Icon", HudScreenVirtual::Register},
    {"repair_hl_east_Icon", HudScreenVirtual::Register},
    {"repair_hl_north_Icon", HudScreenVirtual::Register},
    {"repair_hl_south_Icon", HudScreenVirtual::Register},
    {"repair_hl_middle_Icon", HudScreenVirtual::Register},
    {"repair_glass_Icon", HudScreenVirtual::Register},
    {"repair_frame_Icon", HudScreenVirtual::Register},
    {"info_Text", HudScreenVirtual::Register},
    {"unit_name_Text", HudScreenVirtual::Register},
    {"FlagJP_Icon", HudScreenVirtual::Register},
    {"FlagUS_Icon", HudScreenVirtual::Register},
    {"unit_payload_Icon", HudScreenVirtual::Register},
    {"unit_HP_damage_Icon", HudScreenVirtual::Register},
    {"unit_HP_healthy_Icon", HudScreenVirtual::Register},
    {"unit_command_Icon", HudScreenVirtual::Register},
    {"circle_Group", HudScreenVirtual::Register},
    {"map_class_Icon", HudScreenVirtual::Register},
    {"Jobboldali_Listbox", HudScreenVirtual::Register},
    {"icon_Icon", HudScreenVirtual::Enter},
};
const char* const kPages39[] = {
    "GUI_objectives",
};
const HudScreenWidgetBinding kWidgets39[] = {
    {"Open_Group", HudScreenVirtual::Enter},
    {"obj_main_Icon", HudScreenVirtual::Enter},
    {"obj_text_top_Icon", HudScreenVirtual::Enter},
    {"obj_scale_Icon", HudScreenVirtual::Enter},
    {"obj_text_bottom_Icon", HudScreenVirtual::Enter},
    {"PrimaryNumbers_Text", HudScreenVirtual::Enter},
    {"SecondaryNumbers_Text", HudScreenVirtual::Enter},
    {"ObjectiveNumbers_Listbox", HudScreenVirtual::Enter},
    {"ObjectiveNumbersTemplate_Text", HudScreenVirtual::Enter},
    {"KisPocset_Icon", HudScreenVirtual::Enter},
    {"SelectedObjective_Text", HudScreenVirtual::Enter},
};
const char* const kPages3B[] = {
    "GUI_order",
};
const HudScreenWidgetBinding kWidgets3B[] = {
    {"ship_plane_agressive_Icon", HudScreenVirtual::Layout},
    {"ship_plane_defensive_Icon", HudScreenVirtual::Layout},
    {"plane_formation_Icon", HudScreenVirtual::Layout},
    {"ship_formation_Icon", HudScreenVirtual::Layout},
    {"ship_plane_move_Icon", HudScreenVirtual::Layout},
    {"plane_attack_Icon", HudScreenVirtual::Layout},
    {"ship_attack_Icon", HudScreenVirtual::Layout},
    {"ship_sell_Icon", HudScreenVirtual::Layout},
    {"ship_leave_form_Icon", HudScreenVirtual::Layout},
    {"plane_retreat_Icon", HudScreenVirtual::Layout},
    {"plane_landing_Icon", HudScreenVirtual::Layout},
    {"repair_glass_Icon", HudScreenVirtual::Layout},
    {"repair_frame_Icon", HudScreenVirtual::Layout},
    {"cancel_Icon", HudScreenVirtual::Layout},
    {"repair_hl_west_Icon", HudScreenVirtual::Layout},
    {"repair_hl_east_Icon", HudScreenVirtual::Layout},
    {"repair_hl_north_Icon", HudScreenVirtual::Layout},
    {"repair_hl_south_Icon", HudScreenVirtual::Layout},
    {"repair_hl_middle_Icon", HudScreenVirtual::Layout},
    {"info_Text", HudScreenVirtual::Layout},
};
const char* const kPages3E[] = {
    "GUI_cross_all",
    "GUI_plane_effects",
    "GUI_plane",
};
const HudScreenWidgetBinding kWidgets3E[] = {
    {"cross_tilt_Icon", HudScreenVirtual::Layout},
    {"plane_AA_Group", HudScreenVirtual::Layout},
    {"plane_rocket_Group", HudScreenVirtual::Layout},
    {"plane_bomb_Group", HudScreenVirtual::Layout},
    {"plane_torpedo_Group", HudScreenVirtual::Layout},
    {"plane_DC_Group", HudScreenVirtual::Layout},
    {"plane_parat_Group", HudScreenVirtual::Layout},
    {"cross_F_Icon", HudScreenVirtual::Layout},
    {"cross_H_Icon", HudScreenVirtual::Layout},
    {"cross_HF_Icon", HudScreenVirtual::Layout},
    {"cross_L_Icon", HudScreenVirtual::Layout},
    {"cross_F2_Icon", HudScreenVirtual::Layout},
    {"payload_template_Icon", HudScreenVirtual::Layout},
    {"payload_template_Section", HudScreenVirtual::Layout},
    {"AttackRun_Text", HudScreenVirtual::Layout},
    {"RepairZone_Text", HudScreenVirtual::Layout},
    {"VillanasFelso_Icon", HudScreenVirtual::Layout},
    {"VillanasAlso_Icon", HudScreenVirtual::Layout},
    {"VillanasBal_Icon", HudScreenVirtual::Layout},
    {"VillanasJobb_Icon", HudScreenVirtual::Layout},
    {"plane_stick_Icon", HudScreenVirtual::Layout},
    {"turbo_Group", HudScreenVirtual::Layout},
    {"plane_turbo_arrow_Icon", HudScreenVirtual::Layout},
    {"plane_turbo_warning_Icon", HudScreenVirtual::Layout},
    {"plane_arrow_blue_Icon", HudScreenVirtual::Layout},
    {"plane_arrow_red_Icon", HudScreenVirtual::Layout},
    {"plane_warning_Icon", HudScreenVirtual::Layout},
    {"ship_speed_num1_Icon", HudScreenVirtual::Layout},
    {"ship_speed_num2_Icon", HudScreenVirtual::Layout},
    {"ship_speed_num3_Icon", HudScreenVirtual::Layout},
    {"muhorizont_Model", HudScreenVirtual::Layout},
    {"plane_payload_Group", HudScreenVirtual::Layout},
    {"ship_payload_1_Icon", HudScreenVirtual::Layout},
    {"ship_payload_2_Icon", HudScreenVirtual::Layout},
    {"payload_type_Icon", HudScreenVirtual::Layout},
};
const char* const kPages25[] = {
    "GUI_bomber",
};
const HudScreenWidgetBinding kWidgets25[] = {
    {"role_Text", HudScreenVirtual::Update},
};
const char* const kPages41[] = {
    "GUI_plane_spawn",
};
const char* const kPages45[] = {
    "GUI_ship",
    "GUI_repair",
    "GUI_ship_effects",
    "GUI_ship_damage",
};
const HudScreenWidgetBinding kWidgets45[] = {
    {"ship_stick_Icon", HudScreenVirtual::Layout},
    {"ship_dir_Icon", HudScreenVirtual::Layout},
    {"ship_speed_num1_Icon", HudScreenVirtual::Layout},
    {"ship_speed_num2_Icon", HudScreenVirtual::Layout},
    {"ship_recon_num1_Icon", HudScreenVirtual::Layout},
    {"ship_recon_num2_Icon", HudScreenVirtual::Layout},
    {"ship_torpedo_num1_Icon", HudScreenVirtual::Layout},
    {"ship_torpedo_num2_Icon", HudScreenVirtual::Layout},
    {"ship_relation_Icon", HudScreenVirtual::Layout},
    {"ship_recon_Icon", HudScreenVirtual::Layout},
    {"ship_torpedo_Icon", HudScreenVirtual::Layout},
    {"repair_hl_west_Icon", HudScreenVirtual::Layout},
    {"repair_hl_east_Icon", HudScreenVirtual::Layout},
    {"repair_hl_north_Icon", HudScreenVirtual::Layout},
    {"repair_hl_south_Icon", HudScreenVirtual::Layout},
    {"repair_ikons_periscope_Icon", HudScreenVirtual::Layout},
    {"repair_ikons_engine_Icon", HudScreenVirtual::Layout},
    {"repair_warning_west_Icon", HudScreenVirtual::Layout},
    {"repair_warning_east_Icon", HudScreenVirtual::Layout},
    {"repair_warning_north_Icon", HudScreenVirtual::Layout},
    {"repair_warning_south_Icon", HudScreenVirtual::Layout},
    {"repair_hl_middle_Icon", HudScreenVirtual::Layout},
    {"repair_Text", HudScreenVirtual::Layout},
    {"Icon_1_Icon", HudScreenVirtual::Layout},
    {"Icon_2_Icon", HudScreenVirtual::Layout},
    {"Icon_3_Icon", HudScreenVirtual::Layout},
    {"Icon_4_Icon", HudScreenVirtual::Layout},
    {"Icon_5_Icon", HudScreenVirtual::Layout},
    {"Hl_1_Icon", HudScreenVirtual::Layout},
    {"Hl_2_Icon", HudScreenVirtual::Layout},
    {"Hl_3_Icon", HudScreenVirtual::Layout},
    {"Hl_4_Icon", HudScreenVirtual::Layout},
    {"circle_1_Section", HudScreenVirtual::Layout},
    {"circle_2_Section", HudScreenVirtual::Layout},
    {"circle_3_Section", HudScreenVirtual::Layout},
    {"circle_4_Section", HudScreenVirtual::Layout},
    {"VillanasFelso_Icon", HudScreenVirtual::Layout},
    {"VillanasAlso_Icon", HudScreenVirtual::Layout},
    {"VillanasBal_Icon", HudScreenVirtual::Layout},
    {"VillanasJobb_Icon", HudScreenVirtual::Layout},
};
const char* const kPages4A[] = {
    "GUI_binoculars",
};
const char* const kPages47[] = {
    "GUI_periscope",
};
const HudScreenWidgetBinding kWidgets47[] = {
    {"Periscope_Model", HudScreenVirtual::Layout},
    {"Tavcso_Model", HudScreenVirtual::Layout},
};
const char* const kPages48[] = {
    "GUI_sub",
};
const HudScreenWidgetBinding kWidgets48[] = {
    {"sub_depht_arrow_dest_Icon", HudScreenVirtual::Layout},
    {"sub_depth_arrow_Icon", HudScreenVirtual::Layout},
    {"sub_air_arrow_Icon", HudScreenVirtual::Layout},
    {"sub_air_warning_Icon", HudScreenVirtual::Layout},
};
const char* const kPages2B[] = {
    "GUI_freecam",
};
const HudScreenWidgetBinding kWidgets2B[] = {
    {"weapon_Text", HudScreenVirtual::Register},
};
const char* const kPages36[] = {
    "GUI_moviecamera_debug",
};
const HudScreenWidgetBinding kWidgets36[] = {
    {"TargetNames_Text", HudScreenVirtual::Enter},
    {"DeckPosName_Text", HudScreenVirtual::Enter},
    {"TempEditCursor_Icon", HudScreenVirtual::Enter},
    {"TempEditArrow_Icon", HudScreenVirtual::Enter},
};
const char* const kPages37[] = {
    "GUI_movie",
};
const HudScreenWidgetBinding kWidgets37[] = {
    {"up_Icon", HudScreenVirtual::Register},
    {"down_Icon", HudScreenVirtual::Register},
};
const char* const kPages38[] = {
    "GUI_movie",
};
const HudScreenWidgetBinding kWidgets38[] = {
    {"up_Icon", HudScreenVirtual::Register},
    {"down_Icon", HudScreenVirtual::Register},
};
const char* const kPages33[] = {
    "GUI_blackout",
    "GUI_subtitle",
    "GUI_narrative",
};
const HudScreenWidgetBinding kWidgets33[] = {
    {"Warnings_Group", HudScreenVirtual::Register},
    {"MessageBoxTemplate_Group", HudScreenVirtual::Register},
    {"textbox_main_Group", HudScreenVirtual::Register},
    {"textbox_main_Text", HudScreenVirtual::Register},
    {"textbox_main_Text", HudScreenVirtual::Register},
    {"textbox_main_FrameBox", HudScreenVirtual::Register},
    {"textbox_main_Icon", HudScreenVirtual::Register},
    {"textbox_main_FrameBox", HudScreenVirtual::Register},
    {"textbox_main_Text", HudScreenVirtual::Register},
    {"Blackout_Icon", HudScreenVirtual::Register},
    {"MissionInfoTitle_Group", HudScreenVirtual::Register},
    {"textbox_obj_Text", HudScreenVirtual::Register},
    {"textbox_obj_FrameBox", HudScreenVirtual::Register},
};
const char* const kPages34[] = {
    "GUI_hints",
};
const HudScreenWidgetBinding kWidgets34[] = {
    {"hints_pause_Group", HudScreenVirtual::Layout},
    {"hints_title_Text", HudScreenVirtual::Layout},
    {"hints_description_Text", HudScreenVirtual::Layout},
    {"hints_button_Text", HudScreenVirtual::Layout},
    {"hints_1_Icon", HudScreenVirtual::Layout},
    {"hints_2_Icon", HudScreenVirtual::Layout},
    {"hints_FrameBox", HudScreenVirtual::Layout},
    {"hints_normal_Group", HudScreenVirtual::Layout},
    {"hints_description_Text", HudScreenVirtual::Layout},
    {"hints_FrameBox", HudScreenVirtual::Layout},
    {"hints_1_Icon", HudScreenVirtual::Layout},
};
const char* const kPages5A[] = {
    "FE_sceneinit",
};
const HudScreenWidgetBinding kWidgets5A[] = {
    {"Message_Text", HudScreenVirtual::Register},
    {"hint_Text", HudScreenVirtual::Enter},
    {"bg_Group", HudScreenVirtual::Enter},
    {"title_Text", HudScreenVirtual::Enter},
    {"frameFlag_Icon", HudScreenVirtual::Enter},
    {"loadingLogo_FrameBox", HudScreenVirtual::Enter},
};
const char* const kPages3C[] = {
    "GUI_pause_single",
    "GUI_pause_tip",
    "GUI_pause_objectives",
};
const HudScreenWidgetBinding kWidgets3C[] = {
    {"title_Group", HudScreenVirtual::Register},
    {"hint_title_Text", HudScreenVirtual::Register},
    {"parent_Group", HudScreenVirtual::Register},
    {"child_Group", HudScreenVirtual::Register},
    {"HintTitle_Listbox", HudScreenVirtual::Register},
    {"slider_Group", HudScreenVirtual::Register},
    {"slider_FrameBox", HudScreenVirtual::Register},
    {"back_Icon", HudScreenVirtual::Register},
    {"info_Group", HudScreenVirtual::Register},
    {"info_Clipbox", HudScreenVirtual::Register},
    {"info_text_group", HudScreenVirtual::Register},
    {"info_Text", HudScreenVirtual::Register},
    {"silverline_FrameBox", HudScreenVirtual::Register},
    {"title_Text", HudScreenVirtual::Register},
    {"slider_Group", HudScreenVirtual::Register},
    {"slider_FrameBox", HudScreenVirtual::Register},
    {"ScrollUp_Icon", HudScreenVirtual::Register},
    {"ScrollDown_Icon", HudScreenVirtual::Register},
    {"back_Icon", HudScreenVirtual::Register},
    {"back_Icon", HudScreenVirtual::Register},
    {"GUI_pause_objectives_FrameBox", HudScreenVirtual::Register},
    {"objectives_listbox_Text", HudScreenVirtual::Register},
    {"Primary_Listbox", HudScreenVirtual::Register},
    {"Secondary_Listbox", HudScreenVirtual::Register},
    {"primary_objectives_Text", HudScreenVirtual::Register},
    {"secondary_objectives_Text", HudScreenVirtual::Register},
    {"Main_Listbox", HudScreenVirtual::Enter},
    {"felki_bal_Icon", HudScreenVirtual::Enter},
    {"felki_jobb_Icon", HudScreenVirtual::Enter},
    {"MainListbox_Text", HudScreenVirtual::Enter},
};
const char* const kPages3D[] = {
    "GUI_pause_cheat",
};
const char* const kPages5E[] = {
    "GUI_scoring",
};
const HudScreenWidgetBinding kWidgets5E[] = {
    {"Header_Group", HudScreenVirtual::Layout},
    {"Main_Listbox", HudScreenVirtual::Layout},
    {"Player_Group", HudScreenVirtual::Layout},
    {"Connection_Group", HudScreenVirtual::Layout},
    {"USN_Icon", HudScreenVirtual::Enter},
    {"USN_point_Text", HudScreenVirtual::Enter},
    {"USN_Text", HudScreenVirtual::Enter},
    {"Player_point_Text", HudScreenVirtual::Enter},
    {"Player_name_Text", HudScreenVirtual::Enter},
    {"Player_clan_Text", HudScreenVirtual::Enter},
    {"USN_Icon", HudScreenVirtual::Enter},
    {"USN_point_Text", HudScreenVirtual::Enter},
    {"USN_Text", HudScreenVirtual::Enter},
    {"Player_point_Text", HudScreenVirtual::Enter},
    {"Player_name_Text", HudScreenVirtual::Enter},
    {"Player_clan_Text", HudScreenVirtual::Enter},
    {"Player_point_Text", HudScreenVirtual::Update},
    {"mic_Icon", HudScreenVirtual::Update},
    {"Player_point_Text", HudScreenVirtual::Update},
    {"mic_Icon", HudScreenVirtual::Update},
    {"mic_Icon", HudScreenVirtual::Update},
    {"USN_Icon", HudScreenVirtual::Update},
    {"USN_point_Text", HudScreenVirtual::Update},
    {"USN_Icon", HudScreenVirtual::Update},
    {"USN_point_Text", HudScreenVirtual::Update},
};
const char* const kPages19[] = {
    "GUI_pause_multi",
};
const HudScreenWidgetBinding kWidgets19[] = {
    {"Main_Listbox", HudScreenVirtual::Enter},
    {"felki_bal_Icon", HudScreenVirtual::Enter},
    {"felki_jobb_Icon", HudScreenVirtual::Enter},
};
const char* const kPages32[] = {
    "GUI_limbo",
};
const HudScreenWidgetBinding kWidgets32[] = {
    {"Message_Text", HudScreenVirtual::Enter},
    {"Continue_Text", HudScreenVirtual::Enter},
};
const char* const kPages43[] = {
    "GUI_spectator",
};
const HudScreenWidgetBinding kWidgets43[] = {
    {"Score_Text", HudScreenVirtual::Enter},
    {"Spectator_Text", HudScreenVirtual::Enter},
};
const char* const kPages4E[] = {
    "GUI_support_full",
    "GUI_support_infopanel",
};
const HudScreenWidgetBinding kWidgets4E[] = {
    {"pum_info_Text", HudScreenVirtual::Layout},
    {"sm_close_Group", HudScreenVirtual::Layout},
    {"circle_full_Section", HudScreenVirtual::Layout},
    {"circle_left_Section", HudScreenVirtual::Layout},
    {"circle_right_Section", HudScreenVirtual::Layout},
    {"center_Group", HudScreenVirtual::Layout},
    {"nagykep_Icon", HudScreenVirtual::Layout},
    {"without_pload_Group", HudScreenVirtual::Layout},
    {"pload_Group", HudScreenVirtual::Layout},
    {"icon_L_Icon", HudScreenVirtual::Layout},
    {"up_1_Icon", HudScreenVirtual::Layout},
    {"up_4_Icon", HudScreenVirtual::Layout},
    {"up_2_Icon", HudScreenVirtual::Layout},
    {"up_3_Icon", HudScreenVirtual::Layout},
    {"circle_large_Group", HudScreenVirtual::Layout},
    {"unit_name_Text", HudScreenVirtual::Layout},
    {"resource_Text", HudScreenVirtual::Layout},
    {"piros_Icon", HudScreenVirtual::Layout},
    {"vilagos_Icon", HudScreenVirtual::Layout},
    {"sotet_Icon", HudScreenVirtual::Layout},
    {"keret_FrameBox", HudScreenVirtual::Layout},
    {"support_Icon", HudScreenVirtual::Layout},
    {"map_Group", HudScreenVirtual::Layout},
    {"minimap_map_Icon", HudScreenVirtual::Layout},
    {"SpawnThing_Icon", HudScreenVirtual::Layout},
    {"Unit_Icon", HudScreenVirtual::Layout},
    {"minimap_FrameBox", HudScreenVirtual::Layout},
    {"powerup_tip_FrameBox", HudScreenVirtual::Layout},
};
const char* const kPages4F[] = {
    "GUI_support_full",
    "GUI_powerups_info",
};
const HudScreenWidgetBinding kWidgets4F[] = {
    {"map_Group", HudScreenVirtual::Register},
    {"New_Group", HudScreenVirtual::Layout},
    {"text_Text", HudScreenVirtual::Layout},
    {"left_circle_Icon", HudScreenVirtual::Layout},
    {"right_circle_Icon", HudScreenVirtual::Layout},
    {"empty_circle_Icon", HudScreenVirtual::Layout},
    {"tile_Icon", HudScreenVirtual::Layout},
    {"icon_Icon", HudScreenVirtual::Layout},
    {"center_Group", HudScreenVirtual::Layout},
    {"power_up_Group", HudScreenVirtual::Layout},
    {"color_Icon", HudScreenVirtual::Layout},
    {"radar_Icon", HudScreenVirtual::Layout},
    {"sipp_Icon", HudScreenVirtual::Layout},
    {"szub_Icon", HudScreenVirtual::Layout},
    {"plane_Icon", HudScreenVirtual::Layout},
    {"command_Icon", HudScreenVirtual::Layout},
};
const char* const kPages50[] = {
    "GUI_Warning",
};
const HudScreenWidgetBinding kWidgets50[] = {
    {"first_Group", HudScreenVirtual::Layout},
    {"warning_text", HudScreenVirtual::Layout},
    {"warning_1_Icon", HudScreenVirtual::Layout},
    {"warning_2_Icon", HudScreenVirtual::Layout},
    {"second_Group", HudScreenVirtual::Layout},
    {"third_Group", HudScreenVirtual::Layout},
    {"warning_2_text", HudScreenVirtual::Layout},
};
const char* const kPages51[] = {
    "GUI_counters",
};

} // namespace

const HudScreenPage kHudScreenPages[kHudScreenPageCount] = {
    {"FE_sceneinit", "interface/fe_sceneinit.lua", 13},
    {"GUI_binoculars", "interface/gui_binoculars.lua", 1},
    {"GUI_blackout", "interface/gui_blackout.lua", 13},
    {"GUI_bomber", "interface/gui_bomber.lua", 1},
    {"GUI_classicons", "interface/gui_classicons.lua", 1},
    {"GUI_counters", "interface/gui_counters.lua", 12},
    {"GUI_cross_all", "interface/gui_cross_all.lua", 34},
    {"GUI_cross_gunstate", "interface/gui_cross_gunstate.lua", 3},
    {"GUI_cross_ship", "interface/gui_cross_ship.lua", 32},
    {"GUI_formation", "interface/gui_formation.lua", 1},
    {"GUI_formation_circle", "interface/gui_formation_circle.lua", 4},
    {"GUI_formationOvrly", "interface/gui_formationovrly.lua", 15},
    {"GUI_freecam", "interface/gui_freecam.lua", 1},
    {"GUI_hints", "interface/gui_hints.lua", 11},
    {"GUI_limbo", "interface/gui_limbo.lua", 2},
    {"GUI_map", "interface/gui_map.lua", 28},
    {"GUI_markers", "interface/gui_markers.lua", 23},
    {"GUI_minimap", "interface/gui_minimap.lua", 33},
    {"GUI_movie", "interface/gui_movie.lua", 2},
    {"GUI_moviecamera_debug", "interface/gui_moviecamera_debug.lua", 4},
    {"GUI_narrative", "interface/gui_narrative.lua", 3},
    {"GUI_objectives", "interface/gui_objectives.lua", 12},
    {"GUI_order", "interface/gui_order.lua", 21},
    {"GUI_pause_cheat", "interface/gui_pause_cheat.lua", 2},
    {"GUI_pause_multi", "interface/gui_pause_multi.lua", 4},
    {"GUI_pause_objectives", "interface/gui_pause_objectives.lua", 6},
    {"GUI_pause_single", "interface/gui_pause_single.lua", 4},
    {"GUI_pause_tip", "interface/gui_pause_tip.lua", 30},
    {"GUI_periscope", "interface/gui_periscope.lua", 2},
    {"GUI_plane", "interface/gui_plane.lua", 21},
    {"GUI_plane_effects", "interface/gui_plane_effects.lua", 8},
    {"GUI_plane_spawn", "interface/gui_plane_spawn.lua", 2},
    {"GUI_powerups", "interface/gui_powerups.lua", 2},
    {"GUI_powerups_info", "interface/gui_powerups_info.lua", 7},
    {"GUI_repair", "interface/gui_repair.lua", 15},
    {"GUI_scoring", "interface/gui_scoring.lua", 20},
    {"GUI_selector", "interface/gui_selector.lua", 9},
    {"GUI_ship", "interface/gui_ship.lua", 13},
    {"GUI_ship_damage", "interface/gui_ship_damage.lua", 17},
    {"GUI_ship_effects", "interface/gui_ship_effects.lua", 24},
    {"GUI_spectator", "interface/gui_spectator.lua", 2},
    {"GUI_sub", "interface/gui_sub.lua", 6},
    {"GUI_subtitle", "interface/gui_subtitle.lua", 4},
    {"GUI_support_full", "interface/gui_support_full.lua", 50},
    {"GUI_support_infopanel", "interface/gui_support_infopanel.lua", 7},
    {"GUI_unit", "interface/gui_unit.lua", 8},
    {"GUI_Warning", "interface/gui_warning.lua", 12},
};

const HudScreenLayout kHudScreenLayouts[kInGameHudScreenCount] = {
    {0x44, 0x40, 0x00645F10u, 0x006463E0u, kPages44, 3, kWidgets44, 14},
    {0x27, 0x44, 0x0067B3F0u, 0x004F7590u, nullptr, 0, nullptr, 0},
    {0x4D, 0x48, 0x0063E280u, 0x004F7590u, kPages4D, 1, nullptr, 0},
    {0x26, 0x4C, 0x0051ED60u, 0x004F7590u, kPages26, 1, kWidgets26, 1},
    {0x2E, 0x50, 0x005468B0u, 0x00546A20u, kPages2E, 2, kWidgets2E, 16},
    {0x4C, 0x54, 0x005AB190u, 0x004F7590u, kPages4C, 2, kWidgets4C, 24},
    {0x35, 0x58, 0x005BEC50u, 0x005BE240u, kPages35, 1, kWidgets35, 27},
    {0x2A, 0x5C, 0x00538F80u, 0x004F7590u, kPages2A, 5, kWidgets2A, 26},
    {0x39, 0x60, 0x005EE010u, 0x004F7590u, kPages39, 1, kWidgets39, 11},
    {0x3B, 0x64, 0x005F9FF0u, 0x005FA0F0u, kPages3B, 1, kWidgets3B, 20},
    {0x3E, 0x68, 0x00606A90u, 0x00606C60u, kPages3E, 3, kWidgets3E, 35},
    {0x3F, 0x6C, 0x00607BE0u, 0x004F7590u, nullptr, 0, nullptr, 0},
    {0x25, 0x70, 0x00519830u, 0x004F7590u, kPages25, 1, kWidgets25, 1},
    {0x41, 0x74, 0x0060D1C0u, 0x004F7590u, kPages41, 1, nullptr, 0},
    {0x45, 0x78, 0x0064BB90u, 0x0064C0F0u, kPages45, 4, kWidgets45, 40},
    {0x46, 0x7C, 0x0064A360u, 0x004F7590u, nullptr, 0, nullptr, 0},
    {0x4A, 0x80, 0x0067C430u, 0x004F7590u, kPages4A, 1, nullptr, 0},
    {0x47, 0x84, 0x00650E00u, 0x00651230u, kPages47, 1, kWidgets47, 2},
    {0x48, 0x88, 0x00650170u, 0x00650B90u, kPages48, 1, kWidgets48, 4},
    {0x24, 0x8C, 0x0067B390u, 0x004F7590u, nullptr, 0, nullptr, 0},
    {0x2B, 0x90, 0x00540FE0u, 0x004F7590u, kPages2B, 1, kWidgets2B, 1},
    {0x2C, 0x94, 0x0054ED00u, 0x004F7590u, nullptr, 0, nullptr, 0},
    {0x36, 0x98, 0x005CC770u, 0x004F7590u, kPages36, 1, kWidgets36, 4},
    {0x37, 0x9C, 0x005CCCB0u, 0x004F7590u, kPages37, 1, kWidgets37, 2},
    {0x38, 0xA0, 0x005CCE70u, 0x004F7590u, kPages38, 1, kWidgets38, 2},
    {0x33, 0xA4, 0x005BB130u, 0x004F7590u, kPages33, 3, kWidgets33, 13},
    {0x34, 0xA8, 0x0054D860u, 0x0054D980u, kPages34, 1, kWidgets34, 11},
    {0x23, 0xAC, 0x005176A0u, 0x004F7590u, nullptr, 0, nullptr, 0},
    {0x2F, 0xB0, 0x005211B0u, 0x004F7590u, nullptr, 0, nullptr, 0},
    {0x49, 0xB4, 0x0067BE80u, 0x0067BEA0u, nullptr, 0, nullptr, 0},
    {0x29, 0xCC, 0x00521670u, 0x004F7590u, nullptr, 0, nullptr, 0},
    {0x5A, 0xD4, 0x00636F30u, 0x004F7590u, kPages5A, 1, kWidgets5A, 6},
    {0x3C, 0xD8, 0x00602280u, 0x004F7590u, kPages3C, 3, kWidgets3C, 30},
    {0x3D, 0xDC, 0x00604890u, 0x004F7590u, kPages3D, 1, nullptr, 0},
    {0x5E, 0xE0, 0x0060D740u, 0x0060F210u, kPages5E, 1, kWidgets5E, 25},
    {0x19, 0xE4, 0x005D38A0u, 0x004F7590u, kPages19, 1, kWidgets19, 3},
    {0x32, 0xE8, 0x00565EE0u, 0x004F7590u, kPages32, 1, kWidgets32, 2},
    {0x43, 0xB8, 0x00643E80u, 0x004F7590u, kPages43, 1, kWidgets43, 2},
    {0x4E, 0xBC, 0x0066EA60u, 0x0065DAA0u, kPages4E, 2, kWidgets4E, 28},
    {0x4F, 0xC0, 0x00614170u, 0x00614600u, kPages4F, 2, kWidgets4F, 16},
    {0x50, 0xC4, 0x00682740u, 0x006823C0u, kPages50, 1, kWidgets50, 7},
    {0x51, 0xC8, 0x0052CBF0u, 0x00528C60u, kPages51, 1, nullptr, 0},
};

const HudScreenPage* hud_screen_page(const char* name) noexcept {
    for (const HudScreenPage& page : kHudScreenPages) {
        if (equal_ignoring_case(page.name, name)) {
            return &page;
        }
    }
    return nullptr;
}

const HudScreenLayout* hud_screen_layout_for_slot(int registry_slot) noexcept {
    for (const HudScreenLayout& layout : kHudScreenLayouts) {
        if (layout.registry_slot == registry_slot) {
            return &layout;
        }
    }
    return nullptr;
}

bool hud_screen_slot_raised_by_interface(int registry_slot, int interface_id) noexcept {
    const InGameInterfaceScreenSet set = in_game_interface_screen_set(interface_id);
    if (!set.sets_screen_set || set.screen_ids == nullptr) {
        return false;
    }
    for (std::size_t i = 0; i < set.screen_count; ++i) {
        if (set.screen_ids[i] == registry_slot) {
            return true;
        }
    }
    return false;
}

// 006488D0. Three widget virtual +4Ch calls with 0.0f, the effective game mode
// into +24h, the unit-list rebuild, then the update counter reset.
void hud_root_screen_enter(HudRootScreenState& state, HudScreenHost& host) {
    host.widget_apply_float(state.widget_3c, 0.0f);
    host.widget_apply_float(state.widget_5c, 0.0f);
    host.widget_apply_float(state.widget_60, 0.0f);
    state.effective_game_mode = host.game_effective_mode();
    host.hud_root_rebuild_unit_lists();
    state.update_countdown = 0;
}

// 00639480. movss from 00CEB698 into +88h, then from 00CE3800 into +44h and +48h.
void hud_markers_screen_enter(HudMarkersScreenState& state) noexcept {
    state.field_88 = kHudMarkersEnterField88;
    state.field_44 = kHudMarkersEnterHalf;
    state.field_48 = kHudMarkersEnterHalf;
}

// 005BD550. The layout virtual +14h, then three widget +34h(1) show calls.
void hud_minimap_screen_enter(const HudMinimapScreenState& state, HudScreenHost& host) {
    host.screen_load_layout();
    host.widget_set_shown(state.widget_48, true);
    host.widget_set_shown(state.widget_f0, true);
    host.widget_set_shown(state.widget_1c, true);
}

// 00649860..00649899. Both suppression bytes must be clear, and +F4h counts
// down; when it drops below 1 it reloads with 2 and the body runs.
bool hud_root_screen_update_should_run(HudRootScreenState& state, HudScreenHost& host) {
    if (host.game_hud_suppressed()) {
        return false;
    }
    state.update_countdown -= 1;
    if (state.update_countdown >= 1) {
        return false;
    }
    state.update_countdown = kHudRootUpdateInterval;
    return true;
}

} // namespace bsp
