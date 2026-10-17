#pragma once

// cc13_big5ad: 005ADDA0, the HUD map per-icon update, reconstructed as a whole
// body (packet cc13-big5ad, docs/CC13_BIG5AD.md). Every name is a hypothesis,
// not a recovered symbol. This is a new C++ interface over the native records,
// not a drop-in replacement of the original ABI. Nothing here is wired into
// the game.
//
// Same conventions as cc13_sweepB..B9 and cc13_bigB2: native objects are raw
// byte blocks touched only at the offsets the image reads or writes, and every
// call the image makes goes through a named host method at the same point with
// the same arguments. Cc13Big5adHost extends Cc13BigB2Host, so the earlier
// hooks are reused as they are; the hooks below are the callees no earlier
// cc13 host declared.

#include "bsp/cc13_bigB2.hpp"

#include <cstdint>

namespace bsp {

class Cc13Big5adHost : public Cc13BigB2Host {
public:
    // ---- HUD map helpers (segment 20) ----
    // 0059A840 BSP_HudMap_WorldToMapX: __thiscall(ECX = screen; float x), RET 4;
    // the result in ST0.
    virtual float hud_map_world_to_map_x_0059a840(void* screen, float x) = 0;
    // 0059A860 BSP_HudMap_WorldToMapY: __thiscall(ECX = screen; float z), RET 4;
    // the result in ST0.
    virtual float hud_map_world_to_map_y_0059a860(void* screen, float z) = 0;
    // 0059A9B0 BSP_HudMapIcon_SetColor: __thiscall(ECX = icon; const float rgba[4]), RET 4.
    virtual void hud_map_icon_set_color_0059a9b0(void* icon, const float* rgba) = 0;
    // 0059ADC0 BSP_Color_StaticWhite_0059adc0: no arguments, RET; EAX = the
    // static white colour. The caller keeps EDX live across it.
    virtual const float* color_static_white_0059adc0() = 0;
    // 004B6BC0 BSP_Game_HasLocalPlayerSlot: __thiscall(ECX = game), RET; AL.
    virtual bool game_has_local_player_slot_004b6bc0(void* game) = 0;
    // 004FB9E0 BSP_GuiWidget_SetUniformScale: __thiscall(ECX = widget; float), RET 4.
    virtual void widget_set_uniform_scale_004fb9e0(void* widget, float scale) = 0;
    // 004F9950 TMPL_OwnedPtr_DeleteReset_004f9950: __thiscall(ECX = the owning
    // pointer slot), RET.
    virtual void owned_ptr_delete_reset_004f9950(void** slot) = 0;
    // 006F1F90 BSP_CommandBuilding_CaptureRingFill: __thiscall(ECX = unit), RET;
    // the fill in ST0.
    virtual float command_building_capture_ring_fill_006f1f90(void* unit) = 0;
    // 00415690 BSP_Math_ClampInPlace: __fastcall(ECX = float* value, EDX =
    // const float* low; const float* high), RET 4.
    virtual void math_clamp_in_place_00415690(float* value, const float* low, const float* high) = 0;
    // 0059CEF0 BSP_HudMapIcon_UpdateSizeAndVisibility: __thiscall(ECX = icon;
    // bool invert), RET 4; AL.
    virtual bool hud_map_icon_update_size_and_visibility_0059cef0(void* icon, bool invert) = 0;
    // 0059D150 BSP_HudMapIcon_QueryUnitStatusFlags: __thiscall(ECX = icon; four
    // byte*), RET 10h; clears all four before setting them.
    virtual void hud_map_icon_query_unit_status_0059d150(void* icon, std::uint8_t* first, std::uint8_t* second,
                                                         std::uint8_t* third, std::uint8_t* fourth) = 0;
    // 0059D410 BSP_HudMap_IsPointInsideMapArea: __stdcall(const float* point),
    // RET 4; AL. The caller leaves the screen in ECX; it is not read.
    virtual bool hud_map_is_point_inside_map_area_0059d410(const float* point) = 0;
    // 0059D4A0 BSP_HudMapIcon_PlaceWidget: __thiscall(ECX = screen; icon), RET 4.
    virtual void hud_map_icon_place_widget_0059d4a0(void* screen, void* icon) = 0;
    // 0059D6D0 BSP_HudMapIcon_UpdateOffscreenEdge: __thiscall(ECX = screen;
    // icon), RET 4; AL.
    virtual bool hud_map_icon_update_offscreen_edge_0059d6d0(void* screen, void* icon) = 0;
    // 0059E0C0 (tagged STL_inst_0059e0c0; it is the HUD grid size: cols = rows =
    // (int)sqrt(count), each grown by one while the product is short):
    // __stdcall(int* cols, int* rows, int count), RET 0Ch; the caller leaves the
    // screen in ECX, unread.
    virtual void hud_map_grid_dims_0059e0c0(void* screen, std::int32_t* cols, std::int32_t* rows,
                                            std::uint32_t count) = 0;
    // 0059F7C0 BSP_HudMapScreen_WorldToClampedMapPoint: __thiscall(ECX =
    // screen; const float* world, float* out, float* angle), RET 0Ch.
    virtual void hud_map_world_to_clamped_map_point_0059f7c0(void* screen, const float* world, float* out,
                                                             float* angle) = 0;
    // 005AAC00 BSP_HudMapIcon_RefreshCarriedUnitIcons: __thiscall(ECX = icon;
    // template widget), RET 4.
    virtual void hud_map_icon_refresh_carried_unit_icons_005aac00(void* icon, void* template_widget) = 0;
    // 0059AC50 BSP_Unit_GetHudHealth: __thiscall(ECX = screen; unit), RET 4; ST0.
    virtual float unit_hud_health_0059ac50(void* screen, void* unit) = 0;
    // 0059CB90 BSP_Unit_GetHudIconIndex: __fastcall(ECX = unit, EDX = class
    // descriptor), RET; EAX.
    virtual std::int32_t unit_hud_icon_index_0059cb90(void* unit, void* descriptor) = 0;
    // 0077A2F0 BSP_Unit_HealthColour: __thiscall(ECX = float out[4]; float
    // fraction), RET 4; EAX = out.
    virtual const float* unit_health_colour_0077a2f0(float* out, float fraction) = 0;
    // 00B64470 BSP_Matrix_ComposeEulerScaleTranslation: __fastcall(ECX = out
    // float[16], EDX = translation; const float* euler, const float* scale), RET 8.
    virtual void matrix_compose_euler_scale_translation_00b64470(float* out, const float* translation,
                                                                 const float* euler, const float* scale) = 0;
    // 004391A0 STL_inst_004391a0 (std::vector<void*> checked subscript):
    // __thiscall(ECX = vector; unsigned index), RET 4; EAX = &element.
    virtual void** pointer_vector_at_004391a0(void* vector, std::uint32_t index) = 0;
    // 005A0B20 STL_inst_005a0b20 (std::vector<void*>::erase(first, last)):
    // __thiscall(ECX = vector; iterator* result, first {container, pointer},
    // last {container, pointer}), RET 14h.
    virtual void pointer_vector_erase_range_005a0b20(void* vector, void* result, void* first_container,
                                                     void* first, void* last_container, void* last) = 0;
    // 00BF7420 (_ftol2-style truncation): ST0 in, EAX out.
    virtual std::int32_t crt_ftol_00bf7420(double value) = 0;
    // 00BF9940 (acos): ST0 in and out.
    virtual double crt_acos_00bf9940(double value) = 0;
    // Unit vtable +198h with its AL result read: __thiscall(ECX = unit; const
    // NativeString* name, 0, byte* out), RET 0Ch.
    virtual bool unit_vcall_198_query_result(void* unit, const void* name, std::int32_t zero,
                                             std::uint8_t* out) = 0;
    // 00645160 BSP_Unit_IsSelectableForCurrentPlayer: __fastcall(ECX = unit,
    // DL = flag), RET; AL (as cc13_sweepB10 declares it; that host is not in
    // this chain).
    virtual bool unit_selectable_00645160(void* unit, std::uint8_t flag) = 0;
    // Widget vtable +30h with a float pair: __thiscall(ECX = widget; const
    // float[2]), RET 4.
    virtual void widget_vcall_30_pair(void* widget, const float* pair) = 0;
};

// 005ADDA0 BSP_HudMapScreen_UpdateUnitIcon: __thiscall(ECX = the HUD map
// screen; icon record), RET 4. The icon's unit is icon+4h.
void cc13_big5ad_hud_map_screen_update_unit_icon_005adda0(void* screen, void* icon, Cc13Big5adHost& host);

}  // namespace bsp
