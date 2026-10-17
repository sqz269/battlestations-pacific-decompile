#pragma once

// cc13_sweepB10: tenth pass of the segment 15-51 leftover sweep (packet
// cc13-sweepB10, docs/CC13_SWEEPB10.md). Every name is a hypothesis, not a
// recovered symbol. These are new C++ interfaces over the native records, not
// drop-in replacements of the original ABI. Nothing here is wired into the
// game.
//
// Same conventions as cc13_sweepB..B9 and cc13_pkt642: native objects are raw
// byte blocks touched only at the offsets the image reads or writes, and every
// call the image makes goes through a named host method at the same point with
// the same arguments. Cc13SweepB10Host extends Cc13SweepB9Host, so the earlier
// hooks are reused as they are.

#include "bsp/cc13_sweepB9.hpp"

#include <cstdint>

namespace bsp {

class Cc13SweepB10Host : public Cc13SweepB9Host {
public:
    // ---- formation screen register (segment 15, 00538F80) ----
    // 0093CCC0 BSP_Simple3dScene_Init, __fastcall(ECX = scene), RET.
    virtual void simple_3d_scene_init_0093ccc0(void* scene) = 0;
    // 00B6FE10 BSP_Camera_SetClearFlags, __thiscall(ECX = camera; flags), RET 4.
    virtual void camera_set_clear_flags_00b6fe10(void* camera, std::uint32_t flags) = 0;
    // 0093DE10 BSP_MapScene_BuildQuad, __thiscall(ECX = scene; mode, rgba[4]
    // by value, NativeString effect by value (the callee owns it), y_offset,
    // width, height, alpha_ref), RET 2Ch; EAX = the quad's model.
    virtual void* map_scene_build_quad_0093de10(void* scene, std::int32_t mode, const float* rgba,
                                                Cc13SweepBNativeString effect, float y_offset, float width,
                                                float height, std::int32_t alpha_ref) = 0;
    // 004C1400 BSP_ResourceManager_GetSingleton; EAX.
    virtual void* resource_manager_get_004c1400() = 0;
    // 00B80D70 BSP_NativeResourceManager_LoadDefaultFactory, __thiscall(ECX =
    // manager; const NativeString* name), RET 4; EAX = referenced factory.
    virtual void* resource_load_default_factory_00b80d70(void* manager, const void* name) = 0;
    // Node vtable +10h, __thiscall(ECX = node; a, b), RET 8; EAX.
    virtual void* node_vcall_10(void* node, std::int32_t a, std::int32_t b) = 0;
    // 0093D2D0 BSP_MapScene_SetupOceanMaterials, __thiscall(ECX = scene; node,
    // flag), RET 8.
    virtual void map_scene_setup_ocean_materials_0093d2d0(void* scene, void* node, std::int32_t flag) = 0;
    // 00535880 / 005357E0 (std::list<void*> _Buynode of the screen lists +194h
    // / +1A0h), __thiscall(ECX = list; next, prev, const value*), RET 0Ch; EAX
    // = node. 00537E80 / 00537B80 (_Incsize: length check, then size += n),
    // __thiscall(ECX = list; n), RET 4.
    virtual void* formation_list_buy_node_00535880(void* list, void* next, void* prev, void* const* value) = 0;
    virtual void formation_list_incsize_00537e80(void* list, std::uint32_t count) = 0;
    virtual void* formation_list_buy_node_005357e0(void* list, void* next, void* prev, void* const* value) = 0;
    virtual void formation_list_incsize_00537b80(void* list, std::uint32_t count) = 0;
    // 00B6E680 BSP_Node_SetParent, __thiscall(ECX = node; parent), RET 4.
    virtual void node_set_parent_00b6e680(void* node, void* parent) = 0;

    // ---- HUD map update (segment 20, 005B45C0) ----
    // 0059AB10 (map zoom animation), __thiscall(ECX = screen; float dt), RET 4.
    virtual void hud_map_animate_zoom_0059ab10(void* screen, float dt) = 0;
    // 0068E5B0 (the award tracker's active hint name), __thiscall(ECX =
    // tracker; NativeString* out), RET 4; EAX = the NativeString filled.
    virtual void* award_tracker_copy_active_hint_0068e5b0(void* tracker, void* out) = 0;
    // Unit vtable +13Ch, __thiscall(ECX = unit), no arguments; EAX.
    virtual void* unit_vcall_13c(void* unit) = 0;
    // 0077C470, __thiscall(ECX = object; a, b), RET 8.
    virtual void unit_send_role_transfer_0077c470(void* object, std::int32_t a, std::int32_t b) = 0;
    // 0059ABC0 (the per-level map value), __thiscall(ECX = screen), RET;
    // result in ST0.
    virtual float hud_map_zoom_value_0059abc0(void* screen) = 0;
    // 0059D350 (re-centres the filter label), __thiscall(ECX = screen), RET.
    virtual void hud_map_label_center_0059d350(void* screen) = 0;
    // 00645160, register ABI: ECX = unit, DL = flag (only the low byte is
    // read further on); RET; AL.
    virtual bool unit_selectable_00645160(void* unit, std::uint8_t flag) = 0;
    // 00647300 BSP_InGameHudRoot_SetSpectatedUnit, __thiscall(ECX = hud root
    // [[00E198C4]+40h]; unit), RET 4.
    virtual void hud_set_spectated_unit_00647300(void* root, void* unit) = 0;
    // 006529E0, __thiscall(ECX = overlay): AL = byte overlay+4h.
    virtual bool overlay_busy_006529e0(void* overlay) = 0;
    // 004F7740 (the highest populated session level), no arguments; EAX.
    virtual std::int32_t top_populated_level_004f7740() = 0;
    // 00927880, __thiscall(ECX = unit), RET; EAX = controlled target or 0.
    virtual void* controlled_target_00927880(void* unit) = 0;
    // 009052E0 (adds dt to a player's timer), __thiscall(ECX = [game+21A0h];
    // index, float dt), RET 8.
    virtual void player_timer_add_009052e0(void* table, std::int32_t index, float dt) = 0;
    // 008E2EE0 (map ping), register ABI ECX = mode (1 or 2); float x, float
    // y by value; RET 8.
    virtual void map_ping_008e2ee0(std::int32_t mode, float x, float y) = 0;
    // 0082ADA0, __thiscall(ECX = object; index), RET 4; EAX.
    virtual void* nav_zone_group_0082ada0(void* object, std::int32_t index) = 0;
    // 004178F0, __thiscall(ECX = zone group; const float* point), RET 4; EAX.
    virtual std::int32_t avoid_zone_first_containing_004178f0(void* group, const float* point) = 0;
    // 00AA0E60, __thiscall(ECX = GUI manager; value), RET 4.
    virtual void gui_manager_set_pointer_state_00aa0e60(void* manager, std::int32_t value) = 0;

    // ---- HUD map pointer pick (segment 20, 005A3210) ----
    // 005A1310 BSP_HudMapScreen_PickNearestUnit, __thiscall(ECX = screen;
    // flag), RET 4 (its EAX is not used here).
    virtual void hud_map_pick_nearest_unit_005a1310(void* screen, std::int32_t flag) = 0;

    // ---- power-up bars (segment 25, 00617470) ----
    // 00617030 STL_Map_Subscript_00617030, __thiscall(ECX = map; const key*),
    // RET 4; EAX = the mapped record.
    virtual void* powerup_map_subscript_00617030(void* map, const void* key) = 0;
    // 008E4BB0 BSP_PowerupSlotRecord_GetCooldownFraction, __fastcall(ECX =
    // record), RET; result in ST0.
    virtual float powerup_record_cooldown_fraction_008e4bb0(void* record) = 0;
};

// 00538F80 BSP_HudFormationScreen_Register (vtable 00CED6FC slot +10h, HUD
// screen 2Ah), __fastcall(ECX = screen), RET. Chains 004F71D0, loads the five
// formation pages, binds 26 widgets, sets up the formation 3-D scene +8h
// (three quads, the terrain objects of [[00E188A8]+19CCh]+34Ch with their
// "mapzone" children faded out) and builds the quad model 00536310.
void cc13_sweepB10_formation_register_00538f80(void* screen, Cc13SweepB10Host& host);

// 005B45C0 BSP_HudMapScreen_Update (vtable 00CF0134 slot +20h),
// __thiscall(ECX = screen; float dt), RET 4. The whole GUI_map frame: zoom
// animation, the MAPPERMANENT hint, the controlled unit's role check, the
// cursor in world units, the zoom transition, the filter bar and its six
// slots, the pointer pick, the spectate shortcut, the filter cycle, the wheel
// zoom start, keyboard / stick panning, the cursor edge clamps, the followed
// unit, the icon units, the camera block, the shader values, the multiplayer
// timer, map pings and the pointer state.
void cc13_sweepB10_hud_map_update_005b45c0(void* screen, float dt, Cc13SweepB10Host& host);

// 005A3210 BSP_HudMapScreen_UpdatePointerPick, __fastcall(ECX = screen), RET;
// caller 005B45C0. Unless the in-game GUI gate is closed: 005A1310(screen, 0),
// then the stick (input +35F4h / +35C4h, thresholds +-0.25f) selects two of
// eight direction octants; the unit icon of the map +4F4h that lies in one of
// them, nearest the cursor, becomes the followed unit +524h (byte +528h = 1)
// and the cursor jumps to it.
void cc13_sweepB10_hud_map_update_pointer_pick_005a3210(void* screen, Cc13SweepB10Host& host);

// 00617470 BSP_HudPowerupManagerScreen_UpdatePowerupBars, __fastcall(ECX =
// carousel, the powerup screen + 100h), RET; callers 00619C50 and 0061BEA0.
// Fills four cooldown bars from the power-up records around the carousel
// position (+0h slot, +4h index into the support block's +148h vector, read
// through the per-player map [00F88C30] + slot * 0Ch + 14Ch), then shows,
// sizes (00ABE6E0) and fades the four bar widgets of the support block.
void cc13_sweepB10_powerup_update_bars_00617470(void* carousel, Cc13SweepB10Host& host);

}  // namespace bsp
