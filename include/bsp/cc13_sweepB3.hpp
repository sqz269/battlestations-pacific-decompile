#pragma once

// cc13_sweepB3: third pass of the segment 15-51 leftover sweep (packet
// cc13-sweepB3, docs/CC13_SWEEPB3.md). Every name is a hypothesis, not a
// recovered symbol. These are new C++ interfaces over the native records, not
// drop-in replacements of the original ABI. Nothing here is wired into the game.
//
// Same conventions as cc13_sweepB / cc13_sweepB2: native objects are raw byte
// blocks touched only at the offsets the image reads or writes, and every call
// the image makes goes through a named host method at the same point with the
// same arguments. Cc13SweepB3Host extends Cc13SweepB2Host, so the sweepB and
// sweepB2 hooks (image_global, the CRT hooks, the sized pool, ...) are reused.

#include "bsp/cc13_sweepB2.hpp"

#include <cstdint>

namespace bsp {

class Cc13SweepB3Host : public Cc13SweepB2Host {
public:
    // ---- GUI widgets ----
    // 00AA6640 BSP_GuiWidget_CreateWithSceneNode: ECX = widget kind, EDX = name
    // (native string*), one stack argument, RET 4; EAX = the new widget.
    virtual void* gui_widget_create_with_scene_node_00aa6640(std::int32_t kind, const void* name,
                                                             std::int32_t arg) = 0;
    // 00AA7D40 BSP_GuiWidget_SetLocalZAndBounds: ECX = widget, float, RET 4.
    virtual void gui_widget_set_local_z_00aa7d40(void* widget, float z) = 0;
    // 00AAA5A0 BSP_GuiWidget_AttachChild: ECX = parent, (child, int), RET 8.
    virtual void gui_widget_attach_child_00aaa5a0(void* parent, void* child, std::int32_t arg) = 0;
    // 00AA67F0 BSP_GuiWidget_GetLocalPosition: ECX = widget, RET; EAX = float[3].
    virtual const float* gui_widget_get_local_position_00aa67f0(void* widget) = 0;
    // 00AA7DC0 BSP_GuiWidget_SetLocalPositionAndBounds: ECX = widget, (const float*), RET 4.
    virtual void gui_widget_set_local_position_00aa7dc0(void* widget, const float* position) = 0;
    // 00AA6BC0: ECX = widget, (listener, int), RET 8 (sets the widget's listener).
    virtual void gui_widget_set_listener_00aa6bc0(void* widget, void* listener, std::int32_t arg) = 0;
    // Widget vtable +38h, no arguments: AL (visible).
    virtual bool widget_vcall_38_is_visible(void* widget) = 0;
    // Widget vtable +44h, one float argument.
    virtual void widget_vcall_44_set_float(void* widget, float value) = 0;
    // Widget vtable +4Ch, one float argument.
    virtual void widget_vcall_4c_set_float(void* widget, float value) = 0;
    // Widget (GUI curve) vtable +50h, one dword argument.
    virtual void widget_vcall_50(void* widget, std::int32_t value) = 0;

    // ---- GUI curves ----
    // 00ABC920 BSP_GuiCurve_ClearPoints: ECX = curve, RET.
    virtual void gui_curve_clear_points_00abc920(void* curve) = 0;
    // 00ABC970 BSP_GuiCurve_AddPoint: ECX = curve, (const float* xy, float width), RET 8.
    virtual void gui_curve_add_point_00abc970(void* curve, const float* point, float width) = 0;
    // 00ABCA10 BSP_GuiCurve_RebuildControlPoints: ECX = curve, RET.
    virtual void gui_curve_rebuild_00abca10(void* curve) = 0;
    // 00ABDD80 BSP_GuiCurve_SetTexture: ECX = curve, (native string*), RET 4.
    virtual void gui_curve_set_texture_00abdd80(void* curve, const void* name) = 0;

    // ---- GUI scroller ----
    // 00683610 BSP_GuiScroller_Construct: ECX = scroller, RET.
    virtual void gui_scroller_construct_00683610(void* scroller) = 0;
    // 00683790 BSP_GuiScroller_AttachContent: ECX = scroller, (int), RET 4.
    virtual void gui_scroller_attach_content_00683790(void* scroller, std::int32_t arg) = 0;
    // 006834A0 BSP_GuiScroller_SetScrollRange: ECX = scroller, (float), RET 4.
    virtual void gui_scroller_set_scroll_range_006834a0(void* scroller, float range) = 0;

    // ---- HUD map ----
    // 005A2C30 BSP_HudMap_CreateSimpleColorModel: one float stack argument, RET 4
    // (ECX is overwritten before use); EAX = the model.
    virtual void* hud_map_create_simple_color_model_005a2c30(float size) = 0;
    // Model vtable +10h with (6, 0): EAX = a new model (clone).
    virtual void* model_vcall_10_clone(void* model, std::int32_t a, std::int32_t b) = 0;
    // Model vtable +38h with one argument: the 4x4 local frame.
    virtual void model_vcall_38_set_frame(void* model, const float* frame) = 0;
    // 00413920 BSP_Matrix_Multiply4x4: ECX = left, (out, right), RET 8; EAX = out.
    virtual float* matrix_multiply_00413920(const float* left, float* out, const float* right) = 0;
    // 00B74640 BSP_GeneratedModel_GetGeometry: ECX = model, (index), RET 4.
    virtual void* generated_model_geometry_00b74640(void* model, std::int32_t index) = 0;
    // 00B732C0 BSP_MeshObject_GetElement: ECX = geometry, (index), RET 4.
    virtual void* mesh_element_00b732c0(void* geometry, std::int32_t index) = 0;
    // 00B17310 BSP_Material_SetAlphaRef: ECX = material, (value), RET 4.
    virtual void material_set_alpha_ref_00b17310(void* material, std::uint32_t value) = 0;
    // 00506DD0 checked dword vector push_back: ECX = vector, (const dword*), RET 4.
    virtual void dword_vector_push_back_00506dd0(void* vector, const void* value) = 0;
    // 00505D50 checked dword vector insert: ECX = vector, (out iterator*,
    // iterator {container, pointer} by value, const dword*), RET 10h.
    virtual void dword_vector_insert_00505d50(void* vector, void* out_iterator, void* iterator_container,
                                              void* iterator_pointer, const void* value) = 0;

    // ---- sound ----
    // 00A83FD0 BSP_SoundSampleCache_AcquireByName: ECX = cache 00F8BBE8, (out ref*, name), RET 8;
    // EAX = out.
    virtual void** sound_sample_cache_acquire_00a83fd0(void* cache, void** out, void* name) = 0;
    // 00A7ACF0 BSP_SoundManager_FindClassByName: ECX = manager, (native string*), RET 4; EAX.
    virtual std::int32_t sound_find_class_00a7acf0(void* manager, const void* name) = 0;
    // 00A7B0A0 BSP_SoundManager_FindTypeByName: ECX = manager, (native string*), RET 4;
    // EAX (EDX unused by the callers here).
    virtual std::int32_t sound_find_type_00a7b0a0(void* manager, const void* name) = 0;
    // 00A7E490 BSP_SoundManager_CreateTrackedSound: ECX = manager,
    // (out ref*, sample, class, type, flag), RET 14h; EAX = out.
    virtual void** sound_create_tracked_00a7e490(void* manager, void** out, void* sample, std::int32_t sound_class,
                                                 std::int32_t type, std::uint8_t flag) = 0;

    // ---- multiplayer lobby ----
    // 00576B10 BSP_MultiLobbyOptionRegistry_GetSingleton, RET; EAX.
    virtual void* lobby_option_registry_00576b10() = 0;
    // 00575E80 map subscript: ECX = map, (const int* key), RET 4; EAX = value record.
    virtual void* lobby_map_subscript_00575e80(void* map, const std::int32_t* key) = 0;
    // 005C6850 int tree subscript: ECX = map, (const int* key), RET 4; EAX = inner map.
    virtual void* lobby_tree_subscript_005c6850(void* map, const std::int32_t* key) = 0;
    // 005C49C0 map subscript: ECX = map, (const uint* key), RET 4; EAX = byte*.
    virtual std::uint8_t* lobby_flag_subscript_005c49c0(void* map, const std::uint32_t* key) = 0;
    // 005D7070: ECX = the dword 00E19594, RET; EAX = player record index.
    virtual std::uint32_t lobby_player_index_005d7070(std::uint32_t id) = 0;
    // 005E9BE0 BSP_MultiMainMenuScreen_UpdateSessionList: ECX = screen, RET.
    virtual void multi_main_update_session_list_005e9be0(void* screen) = 0;

    // ---- HUD submarine ----
    // 006501A0 BSP_HudSubmarineScreen_PulseAlpha: ECX = screen.
    virtual void hud_submarine_pulse_alpha_006501a0(void* screen) = 0;

    // ---- batch 2 ----
    // 005C0390 checked dword vector resize: ECX = vector, (new size, value), RET 8.
    virtual void dword_vector_resize_005c0390(void* vector, std::uint32_t size, std::uint32_t value) = 0;
    // 005CFB80 BSP_MultiChatScreen_ClearLogs: ECX = chat screen, RET.
    virtual void multi_chat_clear_logs_005cfb80(void* chat) = 0;
    // 00AA83F0 (an STL instantiation by its ledger name; clears a GUI list
    // widget's rows): ECX = widget, RET.
    virtual void gui_list_clear_00aa83f0(void* widget) = 0;
    // 00AA31F0 BSP_GuiManager_RemoveAndDestroyPage: ECX = manager, (page), RET 4.
    virtual void gui_manager_remove_page_00aa31f0(void* manager, void* page) = 0;
    // 005DACA0 BSP_LobbyMapEntry_Destruct: ECX = 24h-byte entry, RET.
    virtual void lobby_map_entry_destruct_005daca0(void* entry) = 0;
    // 004F71A0 BSP_FrontEndScreen_Destruct: ECX = screen, RET.
    virtual void frontend_screen_destruct_004f71a0(void* screen) = 0;
    // 0065E870 BSP_HudRadialIcon_SetLargeIconAlpha: ECX = support-manager
    // screen, (widget, float alpha), RET 8.
    virtual void hud_support_set_large_icon_alpha_0065e870(void* support, void* widget, float alpha) = 0;
    // 005A1C60 BSP_HudMapIcon_ReleaseWidgets: ECX = map icon, (int), RET 4.
    virtual void hud_map_icon_release_widgets_005a1c60(void* icon, std::int32_t arg) = 0;
    // 005AAAF0 (std::list copy constructor by its ledger name): ECX = the
    // destination {proxy, head, size}, (const source list*), RET 4.
    virtual void list_copy_005aaaf0(void* out, const void* source) = 0;
    // 0084D560 BSP_SupportSite_BuildSupplyClassList: ECX = node value,
    // (out list*, team, int), RET 0Ch.
    virtual void support_site_build_supply_class_list_0084d560(void* value, void* out, std::int32_t team,
                                                               std::int32_t arg) = 0;
    // Interface vtable +0h with no arguments, EAX tested (0068BC80's unit
    // sub-objects at +72Ch and +1188h).
    virtual std::int32_t interface_vcall_00_query(void* object) = 0;
    // 007ACA30 BSP_PathObject_GetEntity: ECX = path, RET; EAX.
    virtual void* path_object_get_entity_007aca30(void* path) = 0;
    // 007AF740 BSP_ScenePath_PointToWorldCached: ECX = path, (float* out,
    // index), RET 8; EAX = float[3].
    virtual const float* scene_path_point_to_world_007af740(void* path, float* out, std::int32_t index) = 0;

    // ---- batch 3 ----
    // 00A9BE00 (list box row by index; an STL instantiation by its ledger
    // name): ECX = list box, (index), RET 4; EAX = row widget.
    virtual void* gui_listbox_row_00a9be00(void* listbox, std::int32_t index) = 0;
    // 00ABBE50 BSP_GuiText_SetLocalisedCString: ECX = text widget, (const char*, int), RET 8.
    virtual void gui_text_set_localised_cstring_00abbe50(void* widget, const char* text, std::int32_t arg) = 0;
    // 00ABAED0 BSP_GuiText_SetLocalisedSource: ECX = text widget, (native string*, int), RET 8.
    virtual void gui_text_set_localised_source_00abaed0(void* widget, const void* text, std::int32_t arg) = 0;
    // 00AA6740 BSP_GuiWidget_GetSize: ECX = widget, RET; EAX = widget+20h (float[2]).
    virtual const float* gui_widget_get_size_00aa6740(void* widget) = 0;

    // ---- 005A2340 render objects ----
    // 00B74EB0 BSP_NativeModelPool_AllocateCanonicalSlot: replaces ECX with its
    // pool id and jumps to 00B74D00; EAX = the block (the caller's ECX = 184h
    // is overwritten).
    virtual void* generated_model_pool_alloc_00b74eb0() = 0;
    // 00B75030 BSP_GeneratedModel_Construct: ECX = block, (native string* name), RET 4; EAX.
    virtual void* generated_model_construct_00b75030(void* block, const void* name) = 0;
    // 00B73B60 BSP_MeshObject_AllocationContextEntry: replaces ECX with its pool
    // id and jumps to 00B73A10; EAX = the block (the caller's ECX = 0BCh is
    // overwritten).
    virtual void* mesh_object_pool_alloc_00b73b60() = 0;
    // 00B73D70 BSP_MeshObject_Construct: ECX = block, RET; EAX.
    virtual void* mesh_object_construct_00b73d70(void* block) = 0;
    // Renderer (*(00F8D394)) vtable +38h with (native string*): EAX = a
    // ref-counted resource.
    virtual void* renderer_vcall_38_load(void* renderer, const void* name) = 0;
    // Renderer vtable +5Ch with (int, int, resource): EAX = a ref-counted vertex stream.
    virtual void* renderer_vcall_5c_create_stream(void* renderer, std::int32_t a, std::int32_t b, void* resource) = 0;
    // 00B73BB0 BSP_Geometry_SetVertexStream: ECX = geometry, (int, stream), RET 8.
    virtual void geometry_set_vertex_stream_00b73bb0(void* geometry, std::int32_t index, void* stream) = 0;
    // Vertex stream vtable +10h with (0, 0, 0): EAX = float* (locked data).
    virtual float* stream_vcall_10_lock(void* stream, std::int32_t a, std::int32_t b, std::int32_t c) = 0;
    // Vertex stream vtable +14h, no arguments (unlock).
    virtual void stream_vcall_14_unlock(void* stream) = 0;
    // 00533FA0 BSP_DrawSection_Create, RET; EAX.
    virtual void* draw_section_create_00533fa0() = 0;
    // 00535320 BSP_Material_CreateForEffectName: ECX = native string*, RET; EAX.
    virtual void* material_create_for_effect_name_00535320(const void* name) = 0;
    // 00B864C0 BSP_DrawSection_SetMaterial: ECX = section, (material), RET 4.
    virtual void draw_section_set_material_00b864c0(void* section, void* material) = 0;
    // 00B865A0 BSP_DrawSection_RebuildVertexLayout: ECX = section, (geometry), RET 4.
    virtual void draw_section_rebuild_layout_00b865a0(void* section, void* geometry) = 0;
    // 00B73C60 BSP_Geometry_AppendDrawSection: ECX = geometry, (section).
    virtual void geometry_append_draw_section_00b73c60(void* geometry, void* section) = 0;
    // 00B75170 BSP_GeneratedModel_SetGeometry: ECX = model, (int, geometry,
    // float, float), RET 10h.
    virtual void generated_model_set_geometry_00b75170(void* model, std::int32_t index, void* geometry, float a,
                                                       float b) = 0;
};

// ---- segment 18 ----

// 0056D660 BSP_MultiSessionBrowserScreen_FillSessionSettings(screen).
void cc13_sweepB3_session_browser_fill_settings_0056d660(void* screen, Cc13SweepB3Host& host);

// ---- segment 19 ----

// 0056D9A0 BSP_MultiSessionBrowserScreen_BuildFilterRows(screen).
void cc13_sweepB3_session_browser_build_filter_rows_0056d9a0(void* screen, Cc13SweepB3Host& host);

// 00578580 BSP_SessionBrowserRow_UpdatePingIcons(row, ping).
void cc13_sweepB3_session_row_update_ping_icons_00578580(void* row, float ping, Cc13SweepB3Host& host);

// ---- segment 20 ----

// 00583E50 BSP_MainMenu_ClearObjectiveRows(screen).
void cc13_sweepB3_main_menu_clear_objective_rows_00583e50(void* screen, Cc13SweepB3Host& host);

// 00585C50 BSP_MainMenuScreen_PlayTestSpeech(screen, name).
void cc13_sweepB3_main_menu_play_test_speech_00585c50(void* screen, void* name, Cc13SweepB3Host& host);

// 005A2340 BSP_HudMap_CreateBuildingQuadModel(): returns a new 1Ch-byte block
// whose +0h holds the model.
void* cc13_sweepB3_hud_map_create_building_quad_model_005a2340(Cc13SweepB3Host& host);

// 005A3610 BSP_HudMapArrowWidget_Construct(widget): returns widget.
void* cc13_sweepB3_hud_map_arrow_widget_construct_005a3610(void* widget, Cc13SweepB3Host& host);

// 005A3830 BSP_HudMapArrowWidget_SetAiArrow(widget, screen, from, to, mode, style).
void cc13_sweepB3_hud_map_arrow_set_ai_arrow_005a3830(void* widget, void* screen, const float* from,
                                                      const float* to, std::int32_t mode, std::int32_t style,
                                                      Cc13SweepB3Host& host);

// 005A3BF0 BSP_HudMapObject_00cf00ac_Destruct(object).
void cc13_sweepB3_hud_map_object_destruct_005a3bf0(void* object, Cc13SweepB3Host& host);

// 005A8F50 BSP_HudMapScreen_AcquireLineModel(screen, frame, scale, layer).
void cc13_sweepB3_hud_map_acquire_line_model_005a8f50(void* screen, const float* frame, float scale,
                                                      std::uint32_t layer, Cc13SweepB3Host& host);

// 005C0BD0 BSP_HudMinimapScreen_HideUnusedMarkers(screen).
void cc13_sweepB3_hud_minimap_hide_unused_markers_005c0bd0(void* screen, Cc13SweepB3Host& host);

// ---- segment 23 ----

// 005E0DB0 BSP_MultiGameLobbyScreen_Destruct(screen).
void cc13_sweepB3_multi_game_lobby_destruct_005e0db0(void* screen, Cc13SweepB3Host& host);

// 005E3290 BSP_MultiGameLobbyScreen_Construct(screen): returns screen.
void* cc13_sweepB3_multi_game_lobby_construct_005e3290(void* screen, Cc13SweepB3Host& host);

// 005EA030 BSP_MultiMainMenuScreen_LayoutAchievementLocks(screen).
void cc13_sweepB3_multi_main_layout_achievement_locks_005ea030(void* screen, Cc13SweepB3Host& host);

// ---- segment 25 ----

// 006113A0 BSP_HudPowerupCarousel_AnimateScroll(carousel, dt).
void cc13_sweepB3_hud_powerup_carousel_animate_006113a0(void* carousel, float dt, Cc13SweepB3Host& host);

// 00611770 BSP_HudPowerupManagerScreen_TickSlide(screen, dt).
void cc13_sweepB3_hud_powerup_tick_slide_00611770(void* screen, float dt, Cc13SweepB3Host& host);

// 00614320 BSP_HudPowerupManagerScreen_BindSupportWidgets(screen).
void cc13_sweepB3_hud_powerup_bind_support_widgets_00614320(void* screen, Cc13SweepB3Host& host);

// ---- segment 28 ----

// 0068BC80 BSP_InGameInterface_BuildTacticalOverlayList(iface): AL. ECX is not read.
bool cc13_sweepB3_ingame_tactical_overlay_check_0068bc80(void* iface, Cc13SweepB3Host& host);

// ---- segment 46 ----

// 007A7200 BSP_MovCam_SetPathWalk(camera, path, start, end, a, flag, c, name, d).
void cc13_sweepB3_movcam_set_path_walk_007a7200(void* camera, void* path, std::int32_t start, std::int32_t end,
                                                float a, std::uint32_t flag, float c,
                                                const Cc13SweepBNativeString* name, std::int32_t d,
                                                Cc13SweepB3Host& host);

// ---- segment 26 ----

// 00650450 BSP_HudSubmarineScreen_Update(screen, unused).
void cc13_sweepB3_hud_submarine_update_00650450(void* screen, float unused, Cc13SweepB3Host& host);

}  // namespace bsp
