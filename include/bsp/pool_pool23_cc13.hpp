#pragma once

// cc13 pool23: referenced game rows from triage T5 (segments 4, 6, 12, 26, 29, 33, 35, 39).
// Build only: no host path calls these. Descriptive names are hypotheses. Every body works on
// raw image memory (std::byte*) at the offsets the listing uses; library calls are bound to the
// host and carry no credit.

#include <cstddef>
#include <cstdint>

namespace bsp {

// The 8-byte NativeString {length +0, data +4}.
struct Pool23NativeString {
    std::uint32_t length;
    char* data;
};

// A checked std::map / std::vector iterator as the image passes it: {container, node or element}.
struct Pool23CheckedIterator {
    std::byte* container;
    std::byte* node;
};

// The checked vector<bool> iterator: {container, word pointer, bit offset}. The container holds the
// bit count at +0 and the word vector first/last at +8/+0Ch.
struct Pool23BitIterator {
    std::byte* container;
    std::uint32_t* word;
    std::uint32_t offset;
};

struct Pool23Host {
    virtual ~Pool23Host() = default;
    // 00BF6713 _invalid_parameter_noinfo (the _SECURE_SCL failure hook). The image continues
    // after it returns; the host decides whether it returns.
    virtual void invalid_parameter_00bf6713() = 0;

    // --- 00451B90 ---------------------------------------------------------------------------
    // 0044BA00 __thiscall(ECX = map, iterator* out, const NativeString* key), RET 8, EAX = out.
    virtual Pool23CheckedIterator* panel_map_find_0044ba00(std::byte* map, Pool23CheckedIterator* out,
                                                           const Pool23NativeString* key) = 0;
    // 00435C40 __thiscall(ECX = string, const NativeString* other), RET 4, AL: case-insensitive equal.
    virtual bool native_string_equals_insensitive_00435c40(const Pool23NativeString* string,
                                                           const std::byte* other) = 0;
    // 00451920 __thiscall(ECX = map, const NativeString* key), RET 4, EAX = the mapped record.
    virtual std::byte* panel_map_lookup_or_insert_00451920(std::byte* map, const Pool23NativeString* key) = 0;
    // 00451020 __thiscall(ECX = map, iterator* out, container, node), RET 0Ch: erase(where).
    virtual void panel_map_erase_00451020(std::byte* map, Pool23CheckedIterator* out, std::byte* container,
                                          std::byte* node) = 0;
    // 00419CC0 (no argument) EAX = the sized storage pool; 00BD1510 __thiscall(ECX = pool, data,
    // bytes, 1), RET 0Ch: return a block.
    virtual void* sized_storage_pool_00419cc0() = 0;
    virtual void sized_storage_return_block_00bd1510(void* pool, void* data, std::uint32_t bytes,
                                                     std::uint32_t one) = 0;
    // 0041DD20 __thiscall(ECX = string): release (the unwind funclet 00C60650).
    virtual void native_string_destroy_0041dd20(Pool23NativeString* string) = 0;

    // --- 0047BA40 ---------------------------------------------------------------------------
    // 00BF7420 _ftol: truncation of the x87 top (here a single-precision quotient, see below).
    virtual std::int32_t ftol_00bf7420(float value) = 0;

    // --- 004FAAC0 ---------------------------------------------------------------------------
    // 004F92E0 __thiscall(ECX = vector): size of the 6Ch-element vector (+4 first, +8 last).
    virtual std::uint32_t library_entry_vector_size_004f92e0(const std::byte* vector) = 0;

    // --- 006407A0 ---------------------------------------------------------------------------
    // 00BF681B operator new; 00BF65AC free (the unwind funclet 00C7A290).
    virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    // 0063C110 __thiscall(ECX = 84h block, owner), RET 4, EAX = block.
    virtual std::byte* hud_unit_marker_widgets_construct_0063c110(void* block, std::uint32_t owner) = 0;
    // 006401E0 __thiscall(ECX = vector, const value*), RET 4: push_back.
    virtual void marker_vector_push_back_006401e0(std::byte* vector, std::byte* const* value) = 0;

    // --- 006F3270 (tail jumps, ECX = building) ------------------------------------------------
    virtual void unit_react_to_health_change_00958a30(std::byte* unit) = 0;
    virtual void command_building_send_neutralize_006f2940(std::byte* building) = 0;

    // --- 007030D0 ---------------------------------------------------------------------------
    // 00701FC0 __thiscall(ECX = iterator*): ++ on a checked map iterator.
    virtual void message_map_iterator_increment_00701fc0(Pool23CheckedIterator* iterator) = 0;

    // --- 00723840 ---------------------------------------------------------------------------
    // 00722CA0 __thiscall(ECX = 2Ch segment record, arg, index mod 7), RET 8, EAX.
    virtual std::uint32_t unit_part_segment_walk_00722ca0(std::byte* segment, std::uint32_t arg,
                                                          std::uint32_t phase) = 0;

    // --- 006AC490 ---------------------------------------------------------------------------
    // 006952A0 (ECX = first endpoint, EDX = callback owner) and 00694A60 (same registers).
    virtual void observer_unregister_pair_006952a0(std::byte* endpoint, std::byte* owner) = 0;
    virtual void observer_register_pair_00694a60(std::byte* endpoint, std::byte* owner) = 0;

    // --- 00626C50 ---------------------------------------------------------------------------
    // The image global at `address` (the option tables 00E196D0.., the vector<bool>s 00E196FC,
    // 00E19710, 00E19724 and the record owner 00E188A8).
    virtual std::byte* global_address(std::uint32_t address) = 0;
    // 004B4750 __thiscall(ECX = [00E188A8]), EAX = the scoring commit record.
    virtual std::byte* mission_scoring_get_commit_record_004b4750(std::byte* owner) = 0;
    // 00625F50 __thiscall(ECX = map, const NativeString* key), RET 4: map<NativeString, {NativeString
    // value, int flag +8}>::operator[]; EAX = the mapped value. Library instantiation.
    virtual std::byte* option_map_subscript_00625f50(std::byte* map, const std::byte* key) = 0;
    // 0041DD40 __thiscall(ECX = string, length, 1), RET 8: NativeString resize.
    virtual void native_string_resize_0041dd40(std::byte* string, std::uint32_t length, std::uint32_t one) = 0;
    // 00BF7680 memcpy (cdecl).
    virtual void memcpy_00bf7680(void* destination, const void* source, std::uint32_t bytes) = 0;
    // 0048F930 __thiscall(ECX = vector<bool>, iterator* out, index), RET 8, EAX = out (begin + index);
    // 0048F230 __thiscall(ECX = iterator*), AL = *iterator. Library instantiations.
    virtual Pool23BitIterator* bit_vector_iterator_at_0048f930(std::byte* bits, Pool23BitIterator* out,
                                                               std::uint32_t index) = 0;
    virtual bool bit_iterator_deref_0048f230(Pool23BitIterator* iterator) = 0;
    // 0048D3B0 __thiscall(ECX = iterator*, offset), RET 4: checked iterator += offset. Library.
    virtual void bit_iterator_advance_0048d3b0(Pool23BitIterator* iterator, std::uint32_t offset) = 0;

    // --- 006AC5D0 ---------------------------------------------------------------------------
    // 006AC260 __thiscall(ECX = pad, float out[3]), RET 4, EAX = out: the planar facing.
    virtual float* entity_forward_xz_normalized_006ac260(std::byte* pad, float* out) = 0;
    // 00414DB0 __thiscall(ECX = entity): refresh the world pose.
    virtual void entity_pose_refresh_world_00414db0(std::byte* entity) = 0;
    // 004218E0 (no argument) EAX = the avoid-zone manager; 004120D0 __thiscall(ECX = manager,
    // zone group id), RET 4, EAX = the group or 0.
    virtual std::byte* avoid_zone_manager_get_singleton_004218e0() = 0;
    virtual std::byte* avoid_zone_manager_group_for_layer_004120d0(std::byte* manager, std::uint32_t group_id) = 0;
    // 004178F0 __thiscall(ECX = group, const float xz[2]), RET 4, EAX = the zone holding the point.
    virtual std::byte* avoid_zone_group_zone_containing_004178f0(std::byte* group, const float* xz) = 0;
    // 00416DD0 __thiscall(ECX = zone, from[2], to[2], hit[2], extra[2]), RET 10h, AL = hit.
    virtual bool avoid_zone_segment_hit_00416dd0(std::byte* zone, const float* from, const float* to, float* hit,
                                                 float* extra) = 0;
    // 0041B4E0 __thiscall(ECX = group, from[2], to[2], hit[2]), RET 0Ch, AL = hit.
    virtual bool avoid_zone_group_segment_hit_0041b4e0(std::byte* group, const float* from, const float* to,
                                                       float* hit) = 0;
    // 00414C60 __thiscall(ECX = const float xz[2]), ST0 = the planar length.
    virtual float vector2_length_00414c60(const float* xz) = 0;
    // 00419010 __stdcall(float x5), RET 14h, ST0: clamped interpolation.
    virtual float math_interpolate_clamped_00419010(float a, float b, float c, float d, float t) = 0;
};

// 00451B90 __thiscall(ECX = panel sequence owner, NativeString name by value), RET 8.
void panel_sequence_map_break_dialog_00451b90(std::byte* owner, Pool23NativeString name, Pool23Host& host);

// 0047BA40 __thiscall(ECX = quad tree, const float box[6], int* x0, int* x1, int* z0, int* z1),
// RET 14h.
void quad_tree_cell_range_for_bounds_0047ba40(const std::byte* tree, const float* box, std::int32_t* x0,
                                              std::int32_t* x1, std::int32_t* z0, std::int32_t* z1,
                                              Pool23Host& host);

// 004FAAC0 __stdcall(const vector*, key), RET 8, EAX = index or -1. ECX is not read.
std::int32_t tactical_library_entries_find_index_004faac0(const std::byte* vector, std::int32_t key,
                                                          Pool23Host& host);

// 006407A0 __thiscall(ECX = HUD markers, byte flag), RET 4, EAX = the acquired widget set.
std::byte* hud_markers_acquire_widgets_006407a0(std::byte* markers, std::uint8_t flag, Pool23Host& host);

// 006F3270, CommandBuilding vtable 00CFB028 slot, __fastcall(ECX = building), plain RET.
void command_building_on_health_changed_006f3270(std::byte* building, Pool23Host& host);

// 007030D0 __thiscall(ECX = owner, const void* value), RET 4, EAX = the key address.
std::byte* message_ref_map_find_key_by_value_007030d0(std::byte* owner, const void* value, Pool23Host& host);

// 00723840 __thiscall(ECX = part, arg), RET 4, EAX = the last 00722CA0 result (see the body).
std::uint32_t unit_part_collision_walk_all_segments_00723840(std::byte* part, std::uint32_t arg,
                                                             Pool23Host& host);

// 006AC490 __thiscall(ECX = pad, unit), RET 4.
void building_pad_set_occupant_006ac490(std::byte* pad, std::byte* unit, Pool23Host& host);

// 00626C50 void(void), plain RET.
void lua_option_tables_apply_default_strings_00626c50(Pool23Host& host);

// 006AC5D0 __thiscall(ECX = pad, float out[3], const float from[3], zone group id, float half_width),
// RET 10h.
void ship_ai_land_pad_approach_point_whole_006ac5d0(std::byte* pad, float* out, const float* from,
                                                    std::uint32_t zone_group, float half_width,
                                                    Pool23Host& host);

}  // namespace bsp
