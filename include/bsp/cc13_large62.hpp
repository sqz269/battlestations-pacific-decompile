#pragma once
// Packet cc13-large62: the large segment 62/65 functions packet cc13-seg62 left
// (docs/CC13_SEG62.md "not reached"). docs/CC13_LARGE62.md lists each function
// with its ABI, evidence and uncertainty.
//
// The model is cc13-seg62's: objects are reached through byte offsets in their
// Win32 layout, and every call the image makes goes through a host method named
// for its target address, at the same point and with the same arguments.
// Large62Host extends Seg62Host, so a host that implements both can serve
// Seg62Host::resolve_entity_sound_keys_0097d650 and attach_flag_node_0095de00
// with the functions below. Names are hypotheses, not recovered symbols.
// Build-tested only; not wired into a game host, not ABI-compatible replacements.

#include "bsp/cc13_seg62.hpp"

#include <cstdint>

namespace bsp::cc13_large62 {

using cc13_seg62::Object;
using cc13_seg62::Seg62Host;

class Large62Host : public Seg62Host {
public:
    // 00BF9A50 _strcspn (cdecl) -> EAX.
    virtual std::uint32_t strcspn_00bf9a50(const char* text, const char* set) = 0;
    // 00469840 BSP_NativeString_Substring, __thiscall(source, out, start, count), RET 0Ch, EAX = out.
    virtual Object substring_00469840(Object source, Object out, std::uint32_t start, std::uint32_t count) = 0;
    // 0041DD20 BSP_NativeString_DestroyStorage, __thiscall(string).
    virtual void string_destroy_0041dd20(Object string) = 0;
    // vtable[28h]() on an entity -> EAX = const char* (a second name).
    virtual const char* vt28_name(Object entity) = 0;
    // 00922A90 BSP_ClassIdIsLandFamily, __fastcall(ECX entity) -> AL.
    virtual bool class_id_is_land_family_00922a90(Object entity) = 0;
    // 0097BD10 STL_inst_0097bd10: copy-constructs a {string, map<string,string>}
    // entry, __thiscall(dest, const source*), RET 4.
    virtual void key_entry_copy_0097bd10(Object dest, Object source) = 0;
    // 0097A190: destroys that entry in place, __thiscall(entry).
    virtual void key_entry_destroy_0097a190(Object entry) = 0;
    // 00427AC0 BSP_NativeStringVector_Size, __thiscall(vector) -> EAX.
    virtual std::uint32_t string_vector_size_00427ac0(Object vector) = 0;
    // 00485790 STL_TrafficStringTree_LowerBound, __thiscall(tree, const key*), RET 4 -> EAX = node.
    virtual Object tree_lower_bound_00485790(Object tree, Object key) = 0;
    // 0049C9C0 BSP_TrafficPairMap_LookupOrInsert, __thiscall(tree, const key*), RET 4
    // -> EAX = the mapped string (node+14h).
    virtual Object string_map_lookup_or_insert_0049c9c0(Object tree, Object key) = 0;
    // 0096EDD0, __thiscall(manager, out string, const suffix*, const mapped*), RET 0Ch.
    virtual void build_message_key_0096edd0(Object manager, Object out, Object suffix, Object mapped) = 0;
    // 00702300 BSP_MessageRecordMap_LowerBound, __thiscall(map, const key*), RET 4 -> EAX = node.
    virtual Object message_map_lower_bound_00702300(Object map, Object key) = 0;

    // Used by 00982540.
    // 004BEC00 BSP_InputManager_GetSingleton() -> EAX.
    virtual Object input_manager_004bec00() = 0;
    // 00697F40 LIBSTL_CheckedDwordVector_Append_00697F40, __thiscall(vector, const dword*), RET 4.
    virtual void dword_vector_append_00697f40(Object vector, const std::uint32_t* value) = 0;
    // 0045A120 STL_CheckedFloatVector_InsertOne_0045A120, __thiscall(vector; out
    // iterator*, where {container, pointer} by value, const float*), RET 10h.
    virtual void float_vector_insert_0045a120(Object vector, Object out_iterator, Object where_container,
                                              std::uint32_t where_pointer, const float* value) = 0;
    // Tree iterator increments, __thiscall(iterator {container, node}): 00966960
    // over the channel's subscriptions, 00969B50 over a subscription's input set.
    virtual void tree_iterator_next_00966960(Object iterator) = 0;
    virtual void tree_iterator_next_00969b50(Object iterator) = 0;

    // Used by 00980380 (the archive reader's other slots are in Seg62Host).
    // vtable[0Ch](key, value, fallback) RET 18h: reads into the value's pointer
    // or stores the fallback.
    virtual void reader_vtc_read_default(Object reader, cc13_seg62::TaggedValue key, cc13_seg62::TaggedValue value,
                                         cc13_seg62::TaggedValue fallback) = 0;
    // vtable[18h](key list*) RET 4: fills a ReaderKeyList with the current table's keys.
    virtual void reader_vt18_list_keys(Object reader, Object key_list) = 0;
    // 009766D0 BSP_Warning_CreateFromArchive, ECX = reader -> EAX = warning.
    virtual std::uint32_t warning_create_from_archive_009766d0(Object reader) = 0;
    // 0096B510 STL_List_BuyNode_0096b510, __thiscall(list, next, prev, const value*), RET 0Ch -> EAX node.
    virtual Object list_buy_node_0096b510(Object list, Object next, Object prev, const std::uint32_t* value) = 0;
    // 0096F960 STL_xlen_throw_0096f960, __thiscall(list, count), RET 4: the size
    // increment with its length check.
    virtual void list_increase_size_0096f960(Object list, std::uint32_t count) = 0;
    // 00978D60 STL_Map_Subscript_00978d60, __thiscall(map, const key string*), RET 4 -> EAX = mapped dword*.
    virtual std::uint32_t* subscription_slot_00978d60(Object map, Object key) = 0;
    // 0097E360 BSP_WarningManager_ParseEventBlock, __fastcall(ECX channel name*, EDX reader) -> EAX.
    virtual std::uint32_t parse_event_block_0097e360(Object channel_name, Object reader) = 0;
    // 005070C0 STL_CaseInsensitiveStringIntMap_Index, __thiscall(map, const key*), RET 4 -> EAX = int*.
    virtual Object int_map_slot_005070c0(Object map, Object key) = 0;
    // 00444BE0 BSP_WeatherFloatMap_LookupOrInsert, __thiscall(map, const key*), RET 4 -> EAX = float*.
    virtual Object float_map_slot_00444be0(Object map, Object key) = 0;
    // 00975D00, __thiscall(map +184h, const entity*), RET 4 -> EAX = the ship noise record.
    virtual Object ship_noise_slot_00975d00(Object map, const std::uint32_t* entity) = 0;
    // 0096C070 BSP_ShipNoiseInfo_StartAirRaidEffect, __thiscall(manager, record, entity), RET 8.
    virtual void ship_noise_start_air_raid_0096c070(Object manager, Object record, std::uint32_t entity) = 0;

    // Used by 0095DE00.
    // 0070BBA0 BSP_FlagParams_InitDefaults, __thiscall(params).
    virtual void flag_params_init_0070bba0(Object params) = 0;
    // 00424C40 BSP_GameSettings_GetSingleton() -> EAX.
    virtual Object game_settings_00424c40() = 0;
    // 00408720 BSP_CharacterString_AssignCounted, __thiscall(std string, text, count), RET 8.
    virtual void std_string_assign_00408720(Object string, const char* text, std::uint32_t count) = 0;
    // 00718870 BSP_GameResource_LookupNamedPointGroup, __thiscall(resource, const name*, index), RET 8 -> EAX.
    virtual Object point_group_lookup_00718870(Object resource, Object name, std::uint32_t index) = 0;
    // 009589D0 STL_inst_009589d0: copy-constructs flag params, __thiscall(dest, const source*), RET 4.
    virtual void flag_params_copy_009589d0(Object dest, Object source) = 0;
    // 004F9B30 BSP_Vector3f_Cross, ECX = out, EDX = a, stack b, RET 4 -> EAX = out.
    virtual float* vector3_cross_004f9b30(float* out, const float* a, const float* b) = 0;
    // 0085DC80 BSP_Matrix_OrthonormalizeBasisRows, ECX = float[16] in place.
    virtual void orthonormalize_0085dc80(float* matrix) = 0;
    // 00413920 BSP_Matrix_Multiply4x4, __thiscall(left; dest, right), RET 8 -> EAX = dest.
    virtual Object matrix_multiply_00413920(Object left, Object dest, Object right) = 0;
    // 0070B210: MOV ECX,00E19B14 / JMP 0070B070 BSP_FlagPool_Allocate -> EAX (the
    // caller's ECX = 19Ch is overwritten).
    virtual Object flag_pool_allocate_0070b210() = 0;
    // 0070BFC0 BSP_FlagModel_Construct, __thiscall(model, const name*, const params*), RET 8 -> EAX.
    virtual Object flag_model_construct_0070bfc0(Object model, Object name, Object params) = 0;
    // 0080E490 BSP_UnitInstance_GetPartsObject, __thiscall(unit) -> EAX (unit+1018h).
    virtual Object unit_parts_0080e490(Object unit) = 0;
    // 0092BEA0 (MOV EAX,[ECX+370h]) and 0092BE90 (MOV EAX,[ECX+374h]), __thiscall(parts).
    virtual std::uint32_t parts_node_0092bea0(Object parts) = 0;
    virtual std::uint32_t parts_node_0092be90(Object parts) = 0;
    // 0095D210 and 0095D160 STL_Vector_PushBack, __thiscall(vector, const dword*), RET 4.
    virtual void vector_push_0095d210(Object vector, const std::uint32_t* value) = 0;
    virtual void vector_push_0095d160(Object vector, const std::uint32_t* value) = 0;
    // 00B6E680 BSP_Node_SetParent, __thiscall(node, parent), RET 4.
    virtual void node_set_parent_00b6e680(Object node, std::uint32_t parent) = 0;
    // vtable[38h](const float matrix[16]) on the flag model, RET 4.
    virtual void vt38_set_matrix(Object model, const float* matrix) = 0;
};

// The key list the reader's vtable[18h] fills: 250 tagged keys, then the count
// (the image's two stack blocks at +30h and +808h of 00980380's frame).
struct ReaderKeyList {
    cc13_seg62::TaggedValue keys[250];
    std::int32_t count;
};
static_assert(sizeof(ReaderKeyList) == 0x7d4, "ReaderKeyList is the image's 7D4h-byte stack block");

// 0097B5E0 __thiscall(ECX manager; out string*, entry by value (14h bytes),
// suffix string by value (8 bytes), const vector<string>* names), RET 24h.
// entry and suffix point at the callee-owned argument copies; this function
// destroys both, as the image does.
void resolve_message_key_0097b5e0(Large62Host& host, Object manager, Object out, Object entry, Object suffix,
                                  Object names);

// 0097D650 BSP_Warning_ResolveEntitySoundKeys, __thiscall(ECX manager; text string*, entity), RET 8.
void resolve_entity_sound_keys_0097d650(Large62Host& host, Object manager, Object text, Object entity);

// 0095DE00 BSP_Unit_AttachFlagNode, __fastcall(ECX unit), RET.
void attach_flag_node_0095de00(Large62Host& host, Object unit);

// 00982540 BSP_WarningManager_PumpInputChannel, __thiscall(ECX manager), RET.
void pump_input_channel_00982540(Large62Host& host, Object manager);

// 00980380 BSP_WarningManager_LoadArchive, __thiscall(ECX manager; reader), RET 4.
void warning_manager_load_archive_00980380(Large62Host& host, Object manager, Object reader);

}  // namespace bsp::cc13_large62
