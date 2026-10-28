#pragma once
// Packet cc13-front6: wiring-frontier callees, i.e. unreconstructed functions that
// block wiring of already-reconstructed callers (local/output/wire_readiness.json).
// docs/CC13_FRONT6.md lists every target with its ABI, evidence, the rows it
// unblocks and its uncertainty, plus the targets closed ledger-only or bound.
//
// Objects are reached through cc13_sweepA::Raw (byte offsets into the Win32
// layout). Every call the image makes goes through a Front6Host method named for
// its target; image globals are reached through global_address / global_pointer.
// Descriptive names are hypotheses, not recovered symbols. Nothing here is wired.

#include "bsp/cc13_seg95.hpp"
#include "bsp/cc13_sweepA.hpp"

#include <cstdint>

namespace bsp::cc13_front6 {

struct ObjectiveSlot04Calls {
    virtual ~ObjectiveSlot04Calls() = default;
    virtual void slot04(void* objective) = 0;
};
// 008DA170, ECX objective, tail JMP [vtable+4], no stack adaptation.
void objective_forward_slot04_008da170(void*, ObjectiveSlot04Calls&);
struct EntityReferenceArchiveCalls {
    virtual ~EntityReferenceArchiveCalls() = default;
    // key is {0,text}; payload is typed {4,entity} or {5,owner+4}.
    virtual void write_field_slot0c(void* archive, const char* key, std::uint32_t type, const void* value) = 0;
};
// 008DBA30 ECX reference owner, stack archive, RET4. The empty-key bytes
// at CF8FA4 remain an explicit borrowed argument until independently confirmed.
void entity_reference_write_archive_008dba30(void* owner, void* archive,
    const char* empty_key_cf8fa4, EntityReferenceArchiveCalls&);

using cc13_seg95::NativeStr;
using cc13_seg95::TaggedValue;
using cc13_sweepA::Raw;

// The VC9 checked std::map iterator the image passes around: {_Mycont, _Ptr}.
struct MapIterator {
    void* container;
    std::uint8_t* node;
};
static_assert(sizeof(MapIterator) == 8);

struct Front6Host {
    virtual ~Front6Host() = default;

    // The address of the image global at `address` (00D03E20 "_netEntity",
    // 00F8A740 the hint-weight map, 00E0E344 the current party index).
    virtual void* global_address(std::uint32_t address) = 0;
    // The pointer stored in the image global dword at `address` (00E188A8 the
    // game object, 00E198AC the front-end manager).
    virtual void* global_pointer(std::uint32_t address) = 0;

    // --- CRT / pools ---------------------------------------------------------
    // 00BF6713, __cdecl(): the checked-iterator failure (_invalid_parameter_noinfo).
    // Modelled as returning: every call site continues after it.
    virtual void invalid_parameter_00bf6713() = 0;
    // 00BF7FBF _stricmp, __cdecl(a, b) -> EAX.
    virtual std::int32_t stricmp_00bf7fbf(const char* a, const char* b) = 0;
    // 00419CC0, the sized-storage pool singleton (EAX). The call site pushes
    // (block, length + 1, 1) before it and leaves them for 00BD1510.
    virtual void* sized_pool_00419cc0() = 0;
    // 00BD1510, __thiscall(ECX = pool; block, bytes, count) RET 0Ch.
    virtual void sized_pool_return_00bd1510(void* pool, void* block, std::uint32_t bytes,
                                            std::uint32_t count) = 0;

    // --- strings and lists ---------------------------------------------------
    // 004260B0 BSP_NativeString_FromInt, __thiscall(ECX = out; int) RET 4 -> EAX = out.
    virtual NativeStr* native_string_from_int_004260b0(NativeStr* out, std::int32_t value) = 0;
    // 004BE940 BSP_LinkedList_NthNode, __thiscall(ECX = list; index) RET 4 -> EAX = node.
    virtual std::uint8_t* linked_list_nth_node_004be940(void* list, std::int32_t index) = 0;
    // 00A03B20 std::map<unsigned, float>::find, __thiscall(ECX = map; out, &key) RET 8 -> EAX = out.
    virtual MapIterator* map_find_00a03b20(void* map, MapIterator* out, const std::uint32_t* key) = 0;

    // --- the archive writer (a polymorphic object; its vtable slots) ----------
    // vtable +4 enter key, __thiscall(archive; TaggedValue key by value) RET 8.
    virtual void archive_enter_04(void* archive, TaggedValue key) = 0;
    // vtable +8 leave, __thiscall(archive) RET.
    virtual void archive_leave_08(void* archive) = 0;
    // vtable +0Ch write, __thiscall(archive; TaggedValue key, TaggedValue value) RET 10h.
    virtual void archive_write_0c(void* archive, TaggedValue key, TaggedValue value) = 0;
    // 00928450 BSP_GameEntity_SaveThinkTimingRoles, __thiscall(ECX = entity; archive) RET 4.
    virtual void game_entity_write_archive_00928450(void* entity, void* archive) = 0;
    // 00805020 BSP_ReconDetectionSet_WriteArchive, __thiscall(ECX = set; archive) RET 4.
    virtual void recon_detection_set_write_archive_00805020(void* set, void* archive) = 0;
    // 0070E890 BSP_ObserverMember_WriteRef, __thiscall(ECX = member; archive) RET 4.
    virtual void observer_member_write_ref_0070e890(void* member, void* archive) = 0;

    // --- virtual dispatch ----------------------------------------------------
    // [object] vtable +8, __thiscall(ECX = object) -> EAX.
    virtual void* object_vslot_08(void* object) = 0;

    // --- engine arrays {data +0, count +4, capacity +8} ----------------------
    // 00BF55BE operator new(size), __cdecl.
    virtual void* operator_new_00bf55be(std::uint32_t size) = 0;
    // 00BF6989 _free(block), __cdecl.
    virtual void free_00bf6989(void* block) = 0;
    // 0086CDB0 BSP_EffectNamedRecord14_CopyConstruct, __thiscall(ECX = slot; source) RET 4.
    virtual void effect_named_record14_copy_construct_0086cdb0(void* slot, const void* source) = 0;
    // 00BEE140 BSP_Vector4_ScaleByScalarRef, ECX = out, EDX = const float* scalar,
    // stack const float* vec4, RET 4 -> EAX = out.
    virtual float* vector4_scale_00bee140(float* out, const float* scalar, const float* vec) = 0;
};

// ---------------------------------------------------------------------------
// One-instruction bodies (each verified from the bytes; docs/CC13_FRONT6.md).
// ---------------------------------------------------------------------------
// 0054D3C0, __fastcall(ECX = screen) RET: CVTTSS2SI EAX, [ECX+54h].
std::int32_t hints_screen_seconds_left_0054d3c0(Raw screen) noexcept;
// 004B47E0, __cdecl variadic: a bare RET (the debug print is compiled out).
void debug_print_004b47e0(const void* first, ...) noexcept;
// 00ADEAF0 / 00AF1130, __fastcall(ECX = object) RET -> EAX = [ECX+24h].
void* terrain_tile_model_00adeaf0(Raw tile) noexcept;
void* particle_resources_sub_object_00af1130(Raw object) noexcept;
// 00ADAC30 / 00ADAC40, __fastcall(ECX = height field) RET -> EAX = [ECX+38h] / [ECX+3Ch].
std::int32_t height_field_cells_x_00adac30(Raw height_field) noexcept;
std::int32_t height_field_cells_z_00adac40(Raw height_field) noexcept;
// 00BBDDC0 / 00B4CBB0, __fastcall(ECX = object) RET -> EAX = [ECX+0Ch].
void* atmosphere_source_00bbddc0(Raw manager_child) noexcept;
void* render_pass_camera_00b4cbb0(Raw pass) noexcept;
// 00B0CF20, ECX ignored, RET: XOR AL,AL (only AL is defined on return).
bool post_effect_enabled_00b0cf20(Raw object) noexcept;
// 00A4C2C0 / 006F7B50 / 00778800, ECX ignored, bare RET.
void empty_body_00a4c2c0(Raw object) noexcept;
void empty_body_006f7b50(Raw object) noexcept;
void empty_body_00778800(Raw object) noexcept;
// 00786CA0, __thiscall(ECX ignored; three dwords) RET 0Ch, no other instruction.
void empty_body_00786ca0(Raw object, std::uint32_t a, std::uint32_t b, std::uint32_t c) noexcept;
// 00B916F0, RET -> EAX = [010903E0] (the quadtree node type token).
std::uint32_t quadtree_type_00b916f0(Front6Host& host);
// 008D45C0, ECX ignored, RET -> EAX = [00F88978].
std::int32_t global_f88978_get_008d45c0(Front6Host& host);
// 0078E470, __fastcall(ECX = view) RET -> EAX = [ECX+38Ch] (the view's unit).
void* camera_view_unit_0078e470(Raw view) noexcept;
// 005662F0, __fastcall(ECX = mode) RET: [00E086C0] = ECX.
void mode_selector_set_005662f0(std::int32_t mode, Front6Host& host);
// 008DA180, __fastcall(ECX = owner) RET -> EAX = ECX + 24h.
std::uint8_t* objective_manager_008da180(Raw owner) noexcept;
// 007F7D90, __fastcall(ECX = object) RET: [ECX] = 00CE4974 (a vtable address).
void session_message_root_store_vtable_007f7d90(Raw object) noexcept;
// 007B9000, __fastcall(ECX = object) RET: byte [ECX+0C01h] = 2.
void run_surface_mark_released_007b9000(Raw object) noexcept;
// 005CE750, __fastcall(ECX = panel) RET: byte [ECX+8Dh] = 1.
void option_panel_set_flag_8d_005ce750(Raw panel) noexcept;
// 00472520 / 00479BC0, __fastcall(ECX = object) RET -> EAX = [ECX+28h].
void* field_28_get_00472520(Raw object) noexcept;
void* vehicle_slot_class_00479bc0(Raw slot) noexcept;
// 0047BBA0, __thiscall(ECX ignored; object) RET 4 -> EAX = object->vtable[8]().
void* forward_object_vslot_08_0047bba0(Raw ignored, void* object, Front6Host& host);

// ---------------------------------------------------------------------------
// Larger bodies.
// ---------------------------------------------------------------------------
// 0077C160 BSP_NetEntity_WriteReplicationState, __thiscall(ECX = entity; archive) RET 4.
void net_entity_write_replication_state_0077c160(Raw entity, void* archive, Front6Host& host);
// 005D7070, __fastcall(ECX = entry index) RET -> EAX: the index of the front-end menu
// record whose key string equals the entry's kind (0x29 -> 0x26, 0x2A -> 0x27) as a
// decimal string; 0 when the entry index is out of range or no record matches.
std::uint32_t find_menu_record_for_entry_kind_005d7070(std::int32_t entry_index, Front6Host& host);
// 00A04240 BSP_AiHintWeight_EntityMultiplier, __fastcall(ECX = entity) RET -> ST0:
// 1.0 times the entity's weight in map 00F8A740 times its weight in the current
// party's map 00F8A750 + 0Ch * [00E0E344] (each factor only when present).
float ai_hint_weight_entity_multiplier_00a04240(void* entity, Front6Host& host);
// 0077A2F0 BSP_Unit_HealthColour, __thiscall(ECX = out float[4]; float health) RET 4
// -> EAX = out: a two-segment blend of three static RGBA colours (00F8715C,
// 00F8716C, 00F8717C, initialised once under the guard bits of 00F8718C).
float* unit_health_colour_0077a2f0(float* out, float health, Front6Host& host);

// ---------------------------------------------------------------------------
// Engine array templates over {data +0, count +4, capacity +8} (signed counts).
// ---------------------------------------------------------------------------
// 0073E9A0 BSP_DecalRecordVector_Reserve, __thiscall(ECX = array; int requested) RET 4:
// dword elements.
void dword_array_reserve_0073e9a0(Raw array, std::int32_t requested, Front6Host& host);
// 0086A350 BSP_NativeQwordVector_Reserve, __thiscall(ECX = array; int requested) RET 4:
// 8-byte elements copied as two floats through x87 FLD/FSTP.
void float_pair_array_reserve_0086a350(Raw array, std::int32_t requested, Front6Host& host);
// 0086CE10 BSP_EffectNamedRecord14Array_Reserve, __thiscall(ECX = array; int requested) RET 4:
// 14h-byte records with a NativeString at +8.
void named_record14_array_reserve_0086ce10(Raw array, std::int32_t requested, Front6Host& host);
// 0086E6F0 BSP_EffectNamedRecord14Array_Resize, __thiscall(ECX = array; int count) RET 4.
void named_record14_array_resize_0086e6f0(Raw array, std::int32_t count, Front6Host& host);

}  // namespace bsp::cc13_front6
