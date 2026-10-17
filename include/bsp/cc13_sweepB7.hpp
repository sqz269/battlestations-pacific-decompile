#pragma once

// cc13_sweepB7: seventh pass of the segment 15-51 leftover sweep (packet
// cc13-sweepB7, docs/CC13_SWEEPB7.md). Every name is a hypothesis, not a
// recovered symbol. These are new C++ interfaces over the native records, not
// drop-in replacements of the original ABI. Nothing here is wired into the
// game.
//
// Same conventions as cc13_sweepB..B6 and cc13_pkt642: native objects are raw
// byte blocks touched only at the offsets the image reads or writes, and every
// call the image makes goes through a named host method at the same point with
// the same arguments. Cc13SweepB7Host extends pkt642's Cc13Pkt642Host, so the
// earlier hooks are reused as they are.

#include "bsp/cc13_pkt642.hpp"

#include <cstdint>

namespace bsp {

class Cc13SweepB7Host : public Cc13Pkt642Host {
public:
    // ---- power-up manager screen (segment 25) ----
    // Widget vtable slot +98h, no stack arguments; the image keeps AX only
    // (MOVSX), so the hook returns the 16-bit value.
    virtual std::int16_t widget_vcall_98_short(void* widget) = 0;
    // 006170F0 STL_Map_Subscript_006170F0 (std::map<int, T*> operator[]
    // instantiation), __thiscall(map, const int* key), RET 4; returns the
    // value slot.
    virtual void** map_subscript_006170f0(void* map, const std::int32_t* key) = 0;
    // 00617870 STL_StringMapCI_Subscript (std::map<NativeString, T,
    // case-insensitive> operator[] instantiation), __thiscall(map, const
    // NativeString* key), RET 4; returns the value slot (node+14h).
    virtual void* string_map_ci_subscript_00617870(void* map, const void* key) = 0;

    // ---- network packet statistics overlay (segment 44) ----
    // 007884D0 BSP_NetworkPacketStatistics_WriteCsvRow, ECX = stats, RET.
    virtual void packet_stats_write_csv_row_007884d0(void* stats) = 0;
    // 00860030 BSP_DebugDraw_ScreenLineRgbaFloats, __thiscall(ECX = const
    // float rgba[4], x0, y0, x1, y1), RET 10h.
    virtual void debug_draw_screen_line_00860030(const float* rgba, float x0, float y0, float x1, float y1) = 0;
    // 00711370 BSP_NativeString_FromUnsigned, __thiscall(ECX = out native
    // string, unsigned value), RET 4; constructs out and returns it.
    virtual void* native_string_from_unsigned_00711370(void* out, std::uint32_t value) = 0;
    // 004C7070 BSP_DiagnosticPrintf, cdecl(sink, float x, float y, fmt, ...);
    // this caller passes no variadic arguments.
    virtual void diagnostic_printf_004c7070(void* sink, float x, float y, const char* fmt) = 0;

    // ---- HUD map unit marker (segment 20) ----
    // 006E40F0 BSP_Unit_BombRackLoadedDevice, ECX = unit, RET; EAX = device.
    virtual void* unit_bomb_rack_loaded_device_006e40f0(void* unit) = 0;
    // Device vtable slot +8h(int kind), AL (the IsKindOf test).
    virtual bool device_vcall_08_is_kind(void* device, std::int32_t kind) = 0;
    // 00818300 BSP_Unit_TorpedoBarrelCount, ECX = unit, RET; EAX.
    virtual std::int32_t unit_torpedo_barrel_count_00818300(void* unit) = 0;
    // 00852350 BSP_Unit_CountBombPlatformChildren, ECX = unit, RET; EAX.
    virtual std::int32_t unit_count_bomb_platform_children_00852350(void* unit) = 0;
    // 00803CE0 BSP_Recon_RelationIfIdentified, __fastcall(ECX = party, EDX =
    // unit), RET; the byte result, compared by this caller as the whole EAX.
    virtual std::uint32_t recon_relation_if_identified_00803ce0(void* party, void* unit) = 0;
    // 00B189F0 BSP_Material_SetTextureSlot, __thiscall(material, slot,
    // texture), RET 8.
    virtual void material_set_texture_slot_00b189f0(void* material, std::uint32_t slot, void* texture) = 0;
    // 00B72B40 BSP_MeshObject_GetCount58, ECX = geometry, RET; EAX.
    virtual std::uint32_t mesh_object_count58_00b72b40(void* geometry) = 0;
    // 007788B0 BSP_Unit_IsFormationFollower, ECX = unit, RET; AL.
    virtual bool unit_is_formation_follower_007788b0(void* unit) = 0;
    // 005A5A80 STL_Vector_Resize_005a5a80 (std::vector<T*>::resize
    // instantiation), __thiscall(vector, count, value), RET 8.
    virtual void pointer_vector_resize_005a5a80(void* vector, std::uint32_t count, std::uint32_t value) = 0;
    // Model node vtable slot +34h(const float matrix[16]) (set the transform).
    virtual void node_vcall_34_set_matrix(void* node, const float* matrix) = 0;

    // ---- support-manager map icons (segment 27) ----
    // 00652BA0 BSP_Hud_UnitIconFrame_00652BA0, one stack argument (the 4Ch
    // slot entry), RET 4; this caller loads ECX = screen. EAX = frame.
    virtual std::uint32_t hud_unit_icon_frame_00652ba0(void* screen, const void* entry) = 0;
    // 00658620 BSP_HudSupportManagerScreen_RecordIconFrame, __thiscall(screen,
    // record), RET 4; EAX (a char frame; the caller pushes the whole EAX).
    virtual std::uint32_t hud_support_record_icon_frame_00658620(void* screen, void* record) = 0;
    // 00658530 BSP_AirOpsSlotRecord_IsOut, __fastcall(ECX = record), AL.
    virtual bool air_ops_slot_is_out_00658530(void* record) = 0;
    // 00846FD0 (checked std::vector element copy, 4Ch-byte entries at
    // unit+794h), __thiscall(unit, out, index), RET 8; constructs out.
    virtual void* unit_slot_entry_copy_00846fd0(void* unit, void* out, std::uint32_t index) = 0;
    // 0065BB00 BSP_UnitSlotEntry_Destroy, ECX = entry.
    virtual void unit_slot_entry_destroy_0065bb00(void* entry) = 0;

    // ---- achievement table (segment 29) ----
    // 006B8C30 STL_Tree_BuyHeadNode_006b8c30 (std::map head-node
    // instantiation), ECX = map, EAX = node.
    virtual void* award_map_buy_head_node_006b8c30(void* map) = 0;
    // 006B9120 BSP_AwardRegistry_EraseActualSubtree, __thiscall(map, node), RET 4.
    virtual void award_map_erase_subtree_006b9120(void* map, void* node) = 0;
    // 00B66BD0 BSP_LuaStateOwner_Construct, ECX = owner.
    virtual void lua_state_owner_construct_00b66bd0(void* owner) = 0;
    // 00B6A020 BSP_LuaStateOwner_Open, __thiscall(owner, library mask), RET 4.
    virtual void lua_state_owner_open_00b6a020(void* owner, std::uint32_t mask) = 0;
    // 00B69D40 BSP_LuaStateOwner_RunScriptWithOverrides, __thiscall(owner,
    // const NativeString* path, int), RET 8.
    virtual void lua_state_owner_run_script_00b69d40(void* owner, const void* path, std::int32_t overrides) = 0;
    // 00B669A0 BSP_LuaStateOwner_Close, ECX = owner.
    virtual void lua_state_owner_close_00b669a0(void* owner) = 0;
    // 00B65F50 BSP_LuaObject_Construct, ECX = object (default constructor).
    virtual void lua_object_construct_00b65f50(void* object) = 0;
    // 00B67080 BSP_LuaObject_IterateFirst, __thiscall(table, key, value), RET 8.
    virtual void lua_object_iterate_first_00b67080(void* table, void* key, void* value) = 0;
    // 00B67190 BSP_LuaObject_IterateNext, __thiscall(table, key, value), RET 8.
    virtual void lua_object_iterate_next_00b67190(void* table, void* key, void* value) = 0;
    // 00B66420 BSP_LuaObject_IsUnbound, __thiscall(ECX ignored, const
    // LuaObject*), RET 4, AL; this caller loads ECX = the table.
    virtual bool lua_object_is_unbound_00b66420(void* ecx_table, const void* object) = 0;
    // 00B660A0 BSP_LuaObject_IsString / 00B65FB0 BSP_LuaObject_IsNil, ECX =
    // object, AL.
    virtual bool lua_object_is_string_00b660a0(void* object) = 0;
    virtual bool lua_object_is_nil_00b65fb0(void* object) = 0;
    // 00B66290 BSP_LuaObject_GetInteger, ECX = object, EAX.
    virtual std::int32_t lua_object_get_integer_00b66290(void* object) = 0;
    // 00B66250 BSP_LuaObject_GetBoolean, ECX = object, AL.
    virtual std::uint8_t lua_object_get_boolean_00b66250(void* object) = 0;
    // 00B66380 BSP_LuaReference_GetIntegerOrDefault, __thiscall(object, int
    // default), RET 4, EAX.
    virtual std::int32_t lua_object_get_integer_or_default_00b66380(void* object, std::int32_t fallback) = 0;
    // 00B67720 BSP_LuaObject_GetByIndex, __thiscall(table, LuaObject* out,
    // int index), RET 8, EAX = out.
    virtual void* lua_object_get_by_index_00b67720(void* table, void* out, std::int32_t index) = 0;
    // 00B67690 BSP_LuaObject_Assign, __thiscall(object, const LuaObject*), RET 4.
    virtual void lua_object_assign_00b67690(void* object, const void* source) = 0;
    // 00BD1120 BSP_SizedStoragePool_AllocateBlock, __thiscall(pool, bytes,
    // int), RET 8, EAX.
    virtual void* sized_pool_allocate_block_00bd1120(void* pool, std::uint32_t bytes, std::int32_t kind) = 0;
    // 004857F0 STL_List_BuyNode_004857f0 (std::list<int> node instantiation),
    // __thiscall(list, next, prev, const int*), RET 0Ch, EAX = node.
    virtual void* int_list_buy_node_004857f0(void* list, void* next, void* prev, const std::int32_t* value) = 0;
    // 0048FC80 (std::list _Incsize instantiation: length check, then size +=
    // n), __thiscall(list, n), RET 4.
    virtual void int_list_incsize_0048fc80(void* list, std::uint32_t count) = 0;

    // ---- free-camera bursts (segment 15) ----
    // 0084BAD0 BSP_Explosion_ApplyRadialDamage, __fastcall(ECX = centre,
    // EDX = const float* radius; const float* damage, int ignore_falloff,
    // source entity, shot), RET 10h (docs/HIT_NARROWPHASE.md).
    virtual void explosion_apply_radial_damage_0084bad0(const float* centre, const float* radius,
                                                        const float* damage, std::int32_t ignore_falloff,
                                                        void* source, void* shot) = 0;
    // 0053D9C0 BSP_PointEffect_SetWorldMatrix, __thiscall(effect, const
    // float* matrix), RET 4.
    virtual void point_effect_set_world_matrix_0053d9c0(void* effect, const float* matrix) = 0;
    // 009043A0, __thiscall(ECX = [game+19CCh], const float3* from, const
    // float3* to, hit record*, ignored entity, mask), RET 14h, AL = hit.
    virtual bool world_ray_test_009043a0(void* world, const float* from, const float* to, void* hit, void* ignore,
                                         std::uint32_t mask) = 0;
    // 0078D6C0, __thiscall(ECX = [game+19F0h], float3 from BY VALUE, float3
    // to BY VALUE, float3* out), RET 1Ch, AL; the two by-value vectors are
    // passed here as pointers to the caller's copies.
    virtual bool segment_test_0078d6c0(void* object, const float* from_copy, const float* to_copy, float* out) = 0;
    // 0051F5E0 BSP_Singleton00E19AAC_GetOrCreate, no arguments, EAX.
    virtual void* singleton_00e19aac_get_0051f5e0() = 0;
    // 006FB6A0 BSP_UnitDebugInspector_SetTarget, __thiscall(inspector, unit), RET 4.
    virtual void unit_debug_inspector_set_target_006fb6a0(void* inspector, void* unit) = 0;
    // 0053EB10 STL_List_Erase_0053eb10 (std::list erase instantiation),
    // __thiscall(list, iterator* result, container, node), RET 0Ch.
    virtual void effect_list_erase_0053eb10(void* list, void* result, void* container, void* node) = 0;
};

// 006191A0 BSP_HudPowerupManagerScreen_RebuildSecondSlots, __fastcall(ECX =
// power-up screen), RET; EBP frame aligned to 8, EH handler 00C77E30 (states
// noted, not modelled). Rotates the second list's three widget columns
// (+58h..+64h clones, +68h..+74h icons, +88h..+94h highlight icons) one way
// or the other by the direction byte +1F8h, clones a new slot from the
// template [[00E198C4]+BCh]+138h into the freed end, fills its icon states and
// positions it.
void cc13_sweepB7_powerup_screen_rebuild_second_slots_006191a0(void* screen, Cc13SweepB7Host& host);

// 0078A430 BSP_NetworkPacketStatistics_FrameUpdate, __fastcall(ECX = packet
// statistics block), RET; EBP frame aligned to 8, EH handler 00C8AAFB. When
// +762900h is set: every fifth call aggregates the per-type counters into
// 1001 rows of 50h bytes from +74F02Ch and every tenth row into rates; the
// CSV row is written when +7649A8h is set; unless +764934h is set, draws the
// scale lines with labels and the 1000-sample bar graph (00860030 lines).
void cc13_sweepB7_packet_stats_frame_update_0078a430(void* stats, Cc13SweepB7Host& host);

// 005A6AD0 BSP_HudMap_CreateUnitMarkerModel (constructor), __thiscall(ECX =
// marker block of 1A0h bytes, unit, scene root, const float* shadow-fade
// source), RET 0Ch; returns the marker. Stores the vtable 00CF00ACh and the
// fields, picks the unit's marker model and texture table from the in-game
// GUI's +54h block by unit kind, creates up to two "mapmodel" models with
// their cShadowFade (and alphablend cAlpha) material parameters, textures
// them by the recon relation, places them at the unit's pose scaled by
// 0.001f and raised 1500, then resets the +E8h vector to 11 nulls.
void* cc13_sweepB7_hud_map_unit_marker_construct_005a6ad0(void* marker, void* unit, void* scene,
                                                          void* shadow_fade_source, Cc13SweepB7Host& host);

// 005BE240 BSP_HudMinimapScreen_LoadLayout, __fastcall(ECX = minimap
// screen), RET; vtable +14h of HUD screen slot 35h (no direct caller). Binds
// the compass, the white group's five item icons (the group hidden), the
// item_ship_Icon colour of the white, red, blue, grey, yellow and silver
// groups (each group hidden), the direction icon and the marker group, all
// under the page root +1Ch.
void cc13_sweepB7_hud_minimap_load_layout_005be240(void* screen, Cc13SweepB7Host& host);

// 0065BF00 (support-manager map icon pass), __fastcall(ECX = support-manager
// screen), RET; caller 00675C40. Shows the selection marker +128h, then
// either fills the four icon slots +BCh..+C8h from the cyclic list +3E0h
// around the selected entry (+34Ch set) or from the selected support unit's
// slot entries (unit+790h vector) or air-ops group records (kinds 45h / 9).
void cc13_sweepB7_support_map_icon_pass_0065bf00(void* screen, Cc13SweepB7Host& host);

// 006B9450 BSP_AchievementTable_Load (constructor of the 20h-byte registry
// held in 00E19900), __fastcall(ECX = registry), RET; returns the registry.
// Clears the name vector +0h..+0Ch and the map +10h..+18h and +1Ch, then
// runs "Scripts\datatables\Achievements.lua" in a private Lua state and, for
// every string key of "Achievements", fills the 0050FC30 record from the
// row's Description, GUITexture, Name, Sound, ID, Multi, Score,
// XLastAchievementID, Params[1..21], Unlock.Type and Unlock.Index fields.
void* cc13_sweepB7_achievement_table_load_006b9450(void* registry, Cc13SweepB7Host& host);

// 0053F2D0 BSP_HudFreeCameraScreen_UpdateEffects, __thiscall(ECX = the
// free-camera burst block (screen+64h), float dt, camera block), RET 8;
// caller 00540390. Advances each burst of the list +14h by dt: expired ones
// spawn their impact effect and blast (0084BAD0) and are erased; live ones
// move under the type's gravity, steer their trail effect, and on a world
// hit (009043A0) or segment hit (0078D6C0) spawn the impact, blast and are
// erased. Ends with 0053EE00 on the block.
void cc13_sweepB7_free_camera_update_bursts_0053f2d0(void* block, float dt, const void* camera, Cc13SweepB7Host& host);

}  // namespace bsp
