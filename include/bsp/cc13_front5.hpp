#pragma once
// Packet cc13-front5: wiring-frontier callees, i.e. unreconstructed functions that
// block wiring of already-reconstructed callers (local/output/wire_readiness.json).
// docs/CC13_FRONT5.md lists every target with its outcome, ABI, the callers it
// unblocks and its uncertainty, plus the targets closed ledger-only or skipped.
//
// Objects are reached through cc13_sweepA::Raw (byte offsets into the Win32
// layout). The archive text emitters use the segment 95 types and host
// (cc13_seg95::Host) that their callers in src/cc13_seg95.cpp already use. Every
// call the image makes goes through a host method named for its target.
// Descriptive names are hypotheses, not recovered symbols. Nothing here is wired.

#include "bsp/cc13_seg95.hpp"
#include "bsp/cc13_sweepA.hpp"

#include <cstdint>

namespace bsp::cc13_front5 {

using cc13_seg95::ArchiveWriterGlobals;
using cc13_seg95::NativeStr;
using cc13_seg95::TaggedValue;
using cc13_sweepA::Raw;

// ---------------------------------------------------------------------------
// Segment 95: the archive text emitters (shared globals 0109CEE0 FILE,
// 0109CEE4 compact flag, 0109CECC storage singleton, 0109CEE5 empty string).
// ---------------------------------------------------------------------------
struct ArchiveEmitHost : cc13_seg95::Host {
    // 004261A0 BSP_NativeString_Concat, __thiscall(ECX = left; out, const right*) RET 8 -> out.
    virtual NativeStr* native_string_concat_004261a0(const NativeStr* left, NativeStr* out,
                                                     const NativeStr* right) = 0;
    // 004CACA0 BSP_NativeString_AppendInt, __thiscall(ECX = string; int) RET 4.
    virtual void native_string_append_int_004caca0(NativeStr* self, std::int32_t value) = 0;
    // 00781740 BSP_NativeString_AppendFloat, __thiscall(ECX = string; float) RET 4.
    virtual void native_string_append_float_00781740(NativeStr* self, float value) = 0;
    // 004CAD40 BSP_String_ReplaceSubstrings, __thiscall(ECX = string; find*, replacement*, count) RET 0Ch.
    virtual void string_replace_substrings_004cad40(NativeStr* self, const NativeStr* find,
                                                    const NativeStr* replacement, std::uint32_t count) = 0;
    // 006AF2B0 BSP_NativeString_ConcatFloat, __thiscall(ECX = left; out, float) RET 8 -> out.
    virtual NativeStr* native_string_concat_float_006af2b0(const NativeStr* left, NativeStr* out, float value) = 0;
    // 004263B0 BSP_NativeString_ConcatInt, __thiscall(ECX = left; out, int) RET 8 -> out.
    virtual NativeStr* native_string_concat_int_004263b0(const NativeStr* left, NativeStr* out,
                                                         std::int32_t value) = 0;
    // 00425E10 BSP_NativeString_Append, __thiscall(ECX = string; const source*) RET 4.
    virtual void native_string_append_00425e10(NativeStr* self, const NativeStr* source) = 0;
    // 00425F40 BSP_NativeString_AssignHeader, __thiscall(ECX = string; const source*) RET 4.
    virtual void native_string_assign_header_00425f40(NativeStr* self, const NativeStr* source) = 0;
};

// 00BD6100, __stdcall(NativeStr owned by value) RET 8. Writes each byte that is
// not space/CR/LF/TAB nibble-swapped (SAR 4 | SHL 4, byte width) to 0109CEE0,
// then releases the string.
void archive_emit_compact_encoded_00bd6100(NativeStr owned, const ArchiveWriterGlobals& g, ArchiveEmitHost& host);
// 00BD6990, __fastcall(ECX = depth) RET. `depth` TAB characters.
void archive_emit_indent_00bd6990(std::uint32_t depth, const ArchiveWriterGlobals& g, ArchiveEmitHost& host);
// 00BD6AC0, __fastcall(CL = root) RET. "\n" (00CE4390) at the root, else ",\n" (00CE438C).
void archive_emit_terminator_00bd6ac0(bool root, const ArchiveWriterGlobals& g, ArchiveEmitHost& host);
// 00BD6BD0, __stdcall(NativeStr owned by value) RET 8.
void archive_emit_string_00bd6bd0(NativeStr owned, const ArchiveWriterGlobals& g, ArchiveEmitHost& host);
// 00BD7B30, __fastcall(CL = root; key tag, key payload) RET 8. Root: "<name> = ";
// nested: "[" + ("\"name\"" | int | float) + "] = ".
void archive_emit_key_00bd7b30(bool root, TaggedValue key, const ArchiveWriterGlobals& g, ArchiveEmitHost& host);
// 00BD7F50, __stdcall(value tag, value payload) RET 8. Tag 0 quoted string with
// '\' then '"' escaped; 1 int; 2 float; 3 true/false; 4 [0109CEDC](object) id;
// 5 "{ x, y, z }"; 6 "{ x, y }"; 7 4x4 "{ { a, b, c, d, }, ... }"; 8 "{ a, b, c, d }";
// 10 "\"int\""; any other tag emits the empty string.
void archive_emit_value_00bd7f50(TaggedValue value, const ArchiveWriterGlobals& g, ArchiveEmitHost& host);

// ---------------------------------------------------------------------------
// The other targets.
// ---------------------------------------------------------------------------
struct Front5Host {
    virtual ~Front5Host() = default;

    // The pointer stored in the image global dword at `address` (00E198C4 the
    // in-game interface, 00E188D8 the current unit block, 00F8A0C4 the warning manager).
    virtual void* global_pointer(std::uint32_t address) = 0;

    // --- 0064A2F0 ---
    // [widget] vtable +34h, __thiscall(widget; dword) RET 4 (the visibility setter).
    virtual void widget_vslot_34(void* widget, std::uint32_t value) = 0;
    // 00545360 BSP_HudWeaponGroupScreen_Suspend, __fastcall(ECX = screen).
    virtual void weapon_group_screen_suspend_00545360(void* screen) = 0;
    // 00983C10 BSP_WarningManager_DispatchRepairChannel, __thiscall(manager; entity, setting) RET 8.
    virtual void warning_dispatch_repair_00983c10(void* manager, void* entity, std::uint32_t setting) = 0;
    // 0068AB80 BSP_InGameInterface_CollapseOverlays, __thiscall(interface; byte, byte) RET 8.
    virtual void ingame_collapse_overlays_0068ab80(void* interface_, std::uint8_t clear_level2,
                                                   std::uint8_t run_extra_hook) = 0;
    // 0068AB30 BSP_InGameInterface_LeaveUnitRequestLevel, __fastcall(ECX = interface).
    virtual void ingame_leave_unit_request_level_0068ab30(void* interface_) = 0;

    // --- 0064B650 ---
    // 00519290 BSP_ShipCamera_ConstructBase, __fastcall(ECX = block) -> EAX = block.
    virtual void* ship_camera_construct_base_00519290(void* block) = 0;

    // --- 0047BB70 ---
    // [object] vtable +8h, __thiscall(object; dword) -> EAX.
    virtual void* object_vslot_08(void* object, std::uint32_t argument) = 0;

    // --- 008624C0 ---
    // [entity] vtable +5Ch, __thiscall(entity; kind) RET 4 -> AL (the class test).
    virtual bool entity_is_kind_vslot_5c(void* entity, std::uint32_t kind) = 0;
    // The four gunnery-AI group setters, __thiscall(ai; byte allowed) RET 4.
    virtual void gunnery_set_artillery_allowed_00861cd0(void* ai, std::uint8_t allowed) = 0;
    virtual void gunnery_set_aa_allowed_00861d20(void* ai, std::uint8_t allowed) = 0;
    virtual void gunnery_set_torpedo_allowed_00861d70(void* ai, std::uint8_t allowed) = 0;
    virtual void gunnery_set_depth_charge_allowed_00861dc0(void* ai, std::uint8_t allowed) = 0;

    // --- 0086C120 ---
    // __thiscall(vector; count) RET 4 each.
    virtual void dword_vector_resize_0086a430(void* vector, std::uint32_t count) = 0;
    virtual void float_array_resize_00818030(void* array, std::int32_t count) = 0;
    virtual void qword_vector_reserve_0086a350(void* vector, std::uint32_t count) = 0;
    // 00BF6989 _free, __cdecl(block).
    virtual void free_00bf6989(void* block) = 0;

    // --- segment 40 gun bodies ---
    // 00728960 BSP_GunBase_Construct, __thiscall(gun; argument) RET 4.
    virtual void gun_base_construct_00728960(void* gun, std::uint32_t argument) = 0;
    // 00B6DB70 BSP_Transform_RefreshWorldMatrix, __fastcall(ECX = node).
    virtual void transform_refresh_world_00b6db70(void* node) = 0;
    // 007297B0 BSP_Gun_ApplyWreckVisibility / 00729870 BSP_Gun_ShowOnlyMeshHolder, __fastcall(ECX = gun).
    virtual void gun_apply_wreck_visibility_007297b0(void* gun) = 0;
    virtual void gun_show_only_mesh_holder_00729870(void* gun) = 0;
    // 0043FF40 (unnamed), __thiscall(gun; float dt) RET 4: the base update.
    virtual void entity_base_update_0043ff40(void* gun, float dt) = 0;
    // 00BF6713, the checked-iterator failure report; the call sites continue after it.
    virtual void invalid_parameter_00bf6713() = 0;
    // 0043FEA0 BSP_DeviceEntity_SaveArchive, __thiscall(device; archive) RET 4.
    virtual void device_entity_save_archive_0043fea0(void* device, void* archive) = 0;
    // The archive writer's slots: +4h open (key by value) RET 8, +8h close, +0Ch
    // write (key, value by value) RET 10h. A bool value's stack slot has only its
    // low byte written by the image (the upper three bytes are stale).
    virtual void archive_vslot_04_open(void* archive, TaggedValue key) = 0;
    virtual void archive_vslot_08_close(void* archive) = 0;
    virtual void archive_vslot_0c_write(void* archive, TaggedValue key, TaggedValue value) = 0;
    // [object] vtable +8h, __thiscall(object; archive) RET 4 (the gun's genParam saving itself).
    virtual void object_vslot_08_save(void* object, void* archive) = 0;

    // --- 0072F060: the Lua save-data reader (a 14h-byte frame object) ---
    // 00440E10, __fastcall(ECX = entity): the base class's saved-state load.
    virtual void entity_load_saved_state_00440e10(void* entity) = 0;
    // 009238A0 SEntity_GetSavedEntityLuaData, __thiscall(entity; LuaObject* out) RET 4.
    virtual void saved_entity_lua_data_009238a0(void* entity, void* out_lua_object) = 0;
    // 004425C0 BSP_LuaReader_Construct, __thiscall(reader; LuaObject by value) RET 14h;
    // `lua_object_by_value` is that argument storage, owned (destroyed) by the callee.
    virtual void lua_reader_construct_004425c0(void* reader, void* lua_object_by_value) = 0;
    // 00BD8E20 EnterKey (reader; key) RET 8; 00BD7A20 LeaveKeyThunk (reader);
    // 00BD5EB0 HasKey (reader; key) RET 8 -> AL; 00BD6830 ReadField (reader; key,
    // {type, out*}) RET 10h; reader vtable +10h the same read, dispatched.
    virtual void lua_reader_enter_key_00bd8e20(void* reader, TaggedValue key) = 0;
    virtual void lua_reader_leave_key_00bd7a20(void* reader) = 0;
    virtual bool lua_reader_has_key_00bd5eb0(void* reader, TaggedValue key) = 0;
    virtual void lua_reader_read_field_00bd6830(void* reader, TaggedValue key, TaggedValue typed_out) = 0;
    virtual void lua_reader_vslot_10_read(void* reader, TaggedValue key, TaggedValue typed_out) = 0;
    // 00731AE0 BSP_GunShotCommand_ReadAndCreate, __fastcall(ECX = reader) -> EAX.
    virtual void* gun_shot_command_read_and_create_00731ae0(void* reader) = 0;
    // 0072D5B0 BSP_Gun_SelectFireMode, __thiscall(gun; mode, flag) RET 8.
    virtual void gun_select_fire_mode_0072d5b0(void* gun, std::uint32_t mode, std::uint32_t flag) = 0;
    // [object] vtable +0, __thiscall(object; flags) RET 4: the deleting destructor.
    virtual void object_vslot_00_delete(void* object, std::uint32_t flags) = 0;
    // 00B67700 BSP_LuaObject_Destruct, __fastcall(ECX = object).
    virtual void lua_object_destruct_00b67700(void* object) = 0;
    // 00BF65AC operator delete, __cdecl(block).
    virtual void operator_delete_00bf65ac(void* block) = 0;
};

// 0064A2F0 BSP_HudShipScreen_CloseOrderMenu, __fastcall(ECX = screen) RET.
void hud_ship_screen_close_order_menu_0064a2f0(Raw screen, Front5Host& host);
// 0064B650 BSP_ShipCaptainCamera_Construct, __fastcall(ECX = block) -> EAX = block.
void* ship_captain_camera_construct_0064b650(Raw block, Front5Host& host);
// 0064A520 / 0064A530, __fastcall(ECX = object) RET: +20h = 0 / 1.
void quad_fade_target_clear_0064a520(Raw object) noexcept;
void quad_fade_target_set_0064a530(Raw object) noexcept;
// 00652A30, __fastcall(ECX = object) RET -> AL = byte +8h.
std::uint8_t byte_08_get_00652a30(Raw object) noexcept;
// 008356B0 / 008356F0, __fastcall(ECX = ship) RET -> ST0 = float +244h / +248h.
float ship_float_244_get_008356b0(Raw ship) noexcept;
float ship_float_248_get_008356f0(Raw ship) noexcept;
// 008356C0, __fastcall(ECX = ship) RET -> AL = byte +243h.
std::uint8_t ship_byte_243_get_008356c0(Raw ship) noexcept;
// 0047BB70, __thiscall with an unused ECX; stack (object, argument) RET 8 ->
// EAX = object->vtable[8h](argument).
void* object_forward_vslot_08_0047bb70(void* unused_ecx, void* object, std::uint32_t argument, Front5Host& host);
// 008624C0 BSP_UnitGunneryDirectorBridge_Apply, __thiscall(ECX = bridge; byte force) RET 4.
// Bridge {+0 ai, +4 director, +8 countdown, +0Ch..+10h last pushed bytes}.
void director_bridge_apply_008624c0(Raw bridge, std::uint8_t force, Front5Host& host);
// 0086C120 BSP_EffectWaterTracer_DestroyCurves, __fastcall(ECX = tracer) RET.
void water_tracer_destroy_curves_0086c120(Raw tracer, Front5Host& host);

// 0072E510 BSP_Gun_Construct, __thiscall(ECX = block; argument) RET 4 -> EAX = block.
void* gun_construct_0072e510(Raw block, std::uint32_t argument, Front5Host& host);
// 0072B2D0 BSP_Gun_UpdateBarrelRecoil, __thiscall(ECX = gun; float dt) RET 4 (vtable +0DCh).
void gun_update_barrel_recoil_0072b2d0(Raw gun, float dt, Front5Host& host);
// 0072ADC0 BSP_Gun_DebugDumpFields, __thiscall(ECX = gun; archive) RET 4.
void gun_save_fields_0072adc0(Raw gun, void* archive, Front5Host& host);
// 0072F060 BSP_Gun_LoadSavedState, __fastcall(ECX = gun) RET (vtable +0A4h).
void gun_load_saved_state_0072f060(Raw gun, Front5Host& host);

}  // namespace bsp::cc13_front5
