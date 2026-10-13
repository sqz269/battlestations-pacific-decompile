#pragma once
// Partition segments 80-82 (00AD5590..00B1BD20): foliage, terrain data, bush and tree
// groups, texture atlases, the particle band and the scene-renderer remainder (packet
// cc13-terrain1). Evidence, original ABIs and uncertainty: docs/CC13_TERRAIN_1.md.
//
// These are new C++ interfaces over host data, not drop-in binary replacements. Objects are
// addressed in their image layout (byte offsets on void*, 32-bit pointers on Win32); every
// image routine a body calls is reached through TerrainHost, each virtual named by the
// address it stands for, so nothing is stubbed. Names are hypotheses, not recovered symbols.
// x87 arithmetic is evaluated in double and rounded to float wherever the native code stores
// a float. Status: reconstructed and build-tested; nothing here is fixture-tested,
// ABI-compatible or game-validated.
//
// RNG RULE. The particle helpers draw from 00BD2F10/00BD2FC0 stream 0, the generator the
// gameplay code shares on the main thread. No body here owns a generator: every draw is a
// TerrainHost callback (draw_u32_stream0_00bd2fc0 / draw_fraction_stream0) so a host can
// route presentation draws to a separate stream. Nothing is wired into a game host.

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

#include "bsp/scene_graph2_cc13.hpp"

namespace bsp::cc13_terrain {

using bsp::cc13_scene2::NativeArray;
using PtrArray = NativeArray<void*>;

// ---------------------------------------------------------------------------
// Image calls.
// ---------------------------------------------------------------------------
struct TerrainHost {
    virtual ~TerrainHost() = default;

    // Allocation. operator new 00BF55BE / 00BF681B (cdecl(size)); _free 00BF6989 / 00BF65AC.
    virtual void* operator_new(std::uint32_t size) = 0;
    virtual void free(void* block) = 0;
    // The pooled string release: 00419CC0(chars, size, 1) fetches the pool, then
    // 00BD1510(chars, size, 1). size is the string length + 1.
    virtual void return_string_block_00bd1510(void* chars, std::uint32_t size) = 0;
    // 0041DD40 BSP_NativeString_Resize(ECX = string, len, 1) followed by _memcpy of the
    // characters: the NativeString {len, chars} assignment the bodies inline.
    virtual void native_string_assign_0041dd40(void* string, const void* chars, std::uint32_t len) = 0;
    // 0041E870 BSP_NativeString_Assign(string, const char* literal).
    virtual void native_string_assign_literal_0041e870(void* string, const char* literal) = 0;
    // __stricmp 00BF7FBF.
    virtual int stricmp_00bf7fbf(const char* a, const char* b) = 0;

    // Virtual calls on image objects. slot is the vtable byte offset.
    virtual void call_virtual(void* object, std::uint32_t slot) = 0;
    virtual void call_virtual_u32(void* object, std::uint32_t slot, std::uint32_t argument) = 0;

    // Singleton lifetime manager (00415350): lock = EnterCriticalSection on manager+10h,
    // when present; the "depth" forms also bump the counter at cs+18h.
    virtual void lifetime_lock(bool bump_depth) = 0;
    virtual void lifetime_unlock(bool bump_depth) = 0;
    virtual void lifetime_register_00bd0c30(void* instance) = 0;
    virtual void lifetime_unregister_00bcfca0(void* instance) = 0;
    // Image globals (no global is modelled here; the host owns them).
    virtual void* read_global(std::uint32_t address) = 0;
    virtual void write_global(std::uint32_t address, void* value) = 0;

    // Named image routines.
    virtual void node_unlink_and_release_00b6d8f0(void* node) = 0;                   // BSP_Node_UnlinkAndRelease
    virtual void destroy_native_string_range_00432050(void* first, void* last) = 0;
    virtual void destroy_foliage_ref_object_00ad7880(void* object) = 0;              // when not modelled inline
    virtual void bush_group_destruct_00ae1c80(void* group) = 0;
    virtual void tree_group_destruct_00ae4c20(void* group) = 0;
    // __thiscall(ECX = group, list): detach a group from the system's per-kind list.
    virtual void bush_group_detach_00ae1e00(void* group, void* list_30) = 0;
    virtual void tree_group_detach_00ae4f30(void* group, void* list_24) = 0;
    virtual void texture_atlas_unload_00aefa30(void* name_string) = 0;
    virtual void* tree_group_instance_construct_00ae5070(void* memory, void* landscape, std::uint32_t type,
        std::uint32_t owner_3c, std::uint32_t extra) = 0;
    virtual void* bush_group_type_construct_00ae1e10(void* memory, void* landscape, void* name_a,
        void* name_b, std::uint32_t owner_3c) = 0;
    virtual void* terrain_render_block_construct_00adf6b0(void* memory, std::uint32_t a, std::uint32_t b,
        std::uint32_t c) = 0;
};

// ---------------------------------------------------------------------------
// Shared primitives.
// ---------------------------------------------------------------------------

// NativeString {+00 len, +04 chars}: released with return_string_block(chars, len + 1)
// when chars is non-null. The fields are left as they were (the destructors do not clear).
void release_native_string(void* string, TerrainHost& host);

// Ref-counted image objects keep the count at +04h; InterlockedDecrement to zero calls
// vtable slot 0 with ECX = object and no argument. release_ref_slot clears *slot.
void release_ref_slot(void** slot, TerrainHost& host);
// 00AD6680 BSP_RefPtr_Release_00ad6680, __fastcall(slot) -> EAX = slot: the shape above.
void** ref_ptr_release_00ad6680(void** slot, TerrainHost& host);
// 00B17D50 BSP_RefCountedPtr_AddRef, __fastcall(slot) -> EAX = slot: InterlockedIncrement
// (*slot + 4) when *slot is non-null.
void** ref_ptr_add_ref_00b17d50(void** slot);

// Single-field setters: MOV [ECX+off], arg; RET 4.
//   00AD56C0 +3Ch: ledger name BSP_TerrainRenderObject_SetField3C_Provisional, but both
//   callers pass ECX = the foliage system (00ADAC90 loads [00F8C210]; 00ADBF80 uses
//   height field +5Ch), so it is the foliage system's +3Ch (the value handed to new bush and
//   tree groups). 00AD56F0 +08h float and 00AD5710 +0Ch float (foliage system scalars),
//   00AD5730 +11h byte (bush normal allowed), 00AF0450 +2Ch float (foliage group shader
//   time). 00B172E0 / 00B17310 have the same shape and are reconstructed in
//   scene_renderer_cc13. 00AE0710 is NOT one of these (it also clears draw-section owners).
struct FieldSetter { std::uint32_t address; std::uint32_t offset; std::uint8_t width; };
extern const FieldSetter kFieldSetters[5];
// Stores `value` (low `width` bytes) at the setter's offset. False for an unknown address.
bool apply_field_setter(std::uint32_t address, void* object, std::uint32_t value);

// ---------------------------------------------------------------------------
// STL instantiations (MSVC 8 std::vector / algorithms over 4-byte elements).
// ---------------------------------------------------------------------------

// std::allocator<T>::deallocate, __thiscall(allocator, p, n), RET 8: _free(p).
//   00AD5AA0, 00AD5AC0, 00AD5AE0, 00AD5FD0.
void allocator_deallocate(void* p, TerrainHost& host);
// std::allocator<T>::allocate, __fastcall(ECX = count) -> EAX: 0 -> operator new(0);
// count * size overflowing 32 bits throws std::bad_alloc (modelled as std::bad_alloc).
//   00AD5CE0 (18h elements), 00AD5D40 and 00AD5DA0 (4-byte elements).
void* allocator_allocate(std::uint32_t count, std::uint32_t element_size, TerrainHost& host);
// std::fill(first, last, *value), cdecl: 00AD6ED0, 00AD6F20.
void fill_dwords(std::uint32_t* first, std::uint32_t* last, const std::uint32_t* value);
// std::fill_n(first, n, *value) -> first + n, cdecl: 00AD7990, 00AD79C0.
std::uint32_t* fill_n_dwords(std::uint32_t* first, std::uint32_t n, const std::uint32_t* value);
// std::copy(first, last, dest) -> dest + n through memmove_s, cdecl: 00AD7780, 00AD77B0.
std::uint32_t* copy_dwords(const std::uint32_t* first, const std::uint32_t* last, std::uint32_t* dest);
// std::copy_backward(first, last, dest_end) -> dest_end - n, cdecl: 00AD6EF0, 00AD6F40.
std::uint32_t* copy_backward_dwords(const std::uint32_t* first, const std::uint32_t* last,
    std::uint32_t* dest_end);

// MSVC 8 vector of 4-byte elements: {+00 allocator, +04 first, +08 last, +0C end}.
struct NativeVector4 {
    std::uint32_t allocator;
    std::uint32_t* first;
    std::uint32_t* last;
    std::uint32_t* end;
};
// std::vector<T*>::_Insert_n(where, count, value), __thiscall(vector, iterator-proxy, where,
// count, const T* value), RET 10h: 00AD8620 (bush groups, allocate 00AD5D40), 00AD87E0 (tree
// groups, allocate 00AD5DA0). Growth: capacity + capacity / 2 (0 on overflow), at least
// size + count; max size 3FFFFFFFh (length_error, modelled as std::length_error).
void vector4_insert_n(NativeVector4& v, std::uint32_t* where, std::uint32_t count, std::uint32_t value,
    TerrainHost& host);
// Frees first and zeroes first/last/end (the _Tidy without element destructors):
//   00AD6D40, 00AD6D80, 00AD7520, 00AD7590 (vector at +00 with the allocator word), and
//   00AF3E60 (TMPL_VectorFreeAndClear: the same three words at +04..+0Ch).
void vector4_tidy(NativeVector4& v, TerrainHost& host);

// ---------------------------------------------------------------------------
// Pointer arrays {+00 data, +04 count, +08 capacity} (signed). Shapes of
// bsp::cc13_scene2; the per-address tables live in terrain_particles_cc13.cpp.
// ---------------------------------------------------------------------------
// Pointer arrays use the process heap (::operator new / delete) exactly as the cc13_scene2
// templates do; every other block here goes through TerrainHost.
// Reserve (n clamped to 1; grows only when capacity < n; copies, frees the old buffer):
//   00AD62A0, 00AD65C0, 00AD6620, 00AFEB60, 00ADB050.
// Resize (reserve when needed, zero new cells, count = n):
//   00AD64D0 / 00AD6DB0 / 00AD6E00 (over 00AD62A0 / 00AD65C0 / 00AD6620), 00AFEC20,
//   00ADBE60 (over 00ADB050).
// Destroy (resize 0 through the named resize, then free data):
//   00AD7500, 00AD76E0, 00AD7700, 00AFEE50, 00ADD130.
// Push back (count == capacity -> reserve(2 * capacity, or 1 below 2), append) through the
// named reserve: 00AD6A90 (00AD62A0), 00AF6820 (BSP_NativeParticleEmitterPointers_Reserve),
// 00AF88F0 (BSP_NativeParticleObject_ReserveModels), 00ADBDF0 (00ADB050). The reserve is the caller's callback.
using PtrReserve = std::function<void(PtrArray&, std::int32_t)>;
bool is_pointer_array_routine(std::uint32_t address) noexcept;
void pointer_array_reserve(std::uint32_t address, PtrArray& a, std::int32_t n);
void pointer_array_resize(std::uint32_t address, PtrArray& a, std::int32_t n);
void pointer_array_destroy(std::uint32_t address, PtrArray& a);
void pointer_array_push_back(std::uint32_t address, PtrArray& a, void* value, const PtrReserve& reserve);

// 00B0A440 TMPL_DwordArray_Reserve, __thiscall(array, uint n), RET 4: when capacity < n
// (unsigned): capacity = n FIRST, operator new(n * 4) (std::bad_alloc on overflow), copy
// count dwords and free the old buffer when there was one, data = new.
void dword_array_reserve_00b0a440(NativeArray<std::uint32_t>& a, std::uint32_t n, TerrainHost& host);

// ---------------------------------------------------------------------------
// Singleton bases. The constructor stores the base vtable, takes the lifetime lock,
// publishes `this` in the global, registers the (re-read) global and unlocks. The
// destructor stores the base vtable, locks, unregisters the global, clears it, unlocks and
// stores the SingletonBase vtable 00CE3818.
// ---------------------------------------------------------------------------
struct SingletonBase {
    std::uint32_t construct_address;
    std::uint32_t destroy_address;   // 0 when there is no separate body in the range
    std::uint32_t instance_global;
    std::uint32_t base_vtable;
    bool bump_depth;                  // the lock also bumps the counter at cs+18h
};
inline constexpr std::uint32_t kSingletonBaseVTable = 0x00CE3818u;
// 00AD6810/00AD68B0 foliage type table, 00AD6950/00AD69F0 foliage system,
// 00AD9D00/00AD9DA0 (and the twin 00AD9E90) terrain render object, 00AF1140/00AF11E0
// particle shader set (no depth counter).
extern const SingletonBase kSingletonBases[4];
const SingletonBase* find_singleton_base(std::uint32_t address) noexcept;
void construct_singleton_base(void* object, const SingletonBase& base, TerrainHost& host);
void destroy_singleton_base(void* object, const SingletonBase& base, TerrainHost& host);

// ---------------------------------------------------------------------------
// Segment 80: foliage system ([00F8C210], A8h bytes) and the foliage type table
// ([00F8C264]).
//
// Foliage system layout (from 00AD9570, 00AD7B90, 00AD8C60, 00AD8DD0):
//   +00 vtable 00D5D270          +04 dirty byte (bush textures)   +08/+0C scalars
//   +11 bush normal allowed      +14..+20 vector<NativeString>    +24 pointer array (trees)
//   +30 pointer array (bushes)   +3C value passed to new groups   +40 node
//   +48 pointer array of 48h ref objects     +54..+60 vector<BushGroup*>
//   +64..+70 vector<TreeGroup*>  +78 diffuse factor               +7C/+84/+8C/+94 strings
//   +9C/+A0 atlas-loaded flags   +A4 ref-counted bush effect
// ---------------------------------------------------------------------------

// 00AD77E0 BSP_FoliageTypeRecord_Destroy, __fastcall(record 30h): releases the strings at
// +1Ch, +14h, +0Ch in that order.
void foliage_type_record_destroy_00ad77e0(void* record, TerrainHost& host);
// 00AD8C30, (first, last) on the stack, RET 8: 00AD77E0 on each 30h record.
void foliage_type_record_destroy_range_00ad8c30(std::uint8_t* first, std::uint8_t* last, TerrainHost& host);
// 00AD9520, __fastcall(vector): vector<FoliageTypeRecord> tidy over {+04 first, +08 last,
// +0C end}: destroy each record, free first, zero the three words.
void foliage_type_record_vector_tidy_00ad9520(void* vector, TerrainHost& host);
// 00AD97D0, __fastcall(group 18h): the FoliageTypeGroup destructor: 00AD9520 on the record
// vector at +08, then the name string at +00.
void foliage_type_group_destroy_00ad97d0(void* group, TerrainHost& host);
// 00AD9880, ECX = first, EDX = last plus two unused stack words, RET 8: group range destroy
// (18h stride); each group as 00AD97D0 with the record vector tidy inlined. 00AD9950
// (RET 8) forwards to it.
void foliage_type_group_destroy_range_00ad9880(std::uint8_t* first, std::uint8_t* last, TerrainHost& host);
// 00AD9970 / 00AD99B0, __fastcall(vector): vector<FoliageTypeGroup> tidy over {+04 first,
// +08 last, +0C end}.
void foliage_type_group_vector_tidy_00ad9970(void* vector, TerrainHost& host);
// 00AD99F0 BSP_FoliageTypeTable_Construct, __fastcall(table) -> table: singleton base
// 00AD6810, vtable 00D5D274, the group vector words +08..+10 zeroed.
void* foliage_type_table_construct_00ad99f0(void* table, TerrainHost& host);
// 00AD9A40 BSP_FoliageTypeTable_Destroy, __fastcall(table): group vector tidy at +04, then
// the base destructor 00AD68B0.
void foliage_type_table_destroy_00ad9a40(void* table, TerrainHost& host);

// 00AD7880 BSP_FoliageRefObject_Destroy_Provisional, __fastcall(object 48h): releases the
// ref-counted slots +04, +00, +40h, then the strings +38h, +30h, +28h.
void foliage_ref_object_destroy_00ad7880(void* object, TerrainHost& host);
// 00AD7B50, __fastcall(slot): when *slot: 00AD7880, _free, *slot = 0.
void foliage_ref_object_delete_00ad7b50(void** slot, TerrainHost& host);

// 00AD7B90 BSP_FoliageSystem_RemoveLandscape, __thiscall(system, landscape), RET 4.
void foliage_system_remove_landscape_00ad7b90(void* system, void* landscape, TerrainHost& host);
// 00AD8C60 BSP_FoliageSystem_FindOrCreateTreeGroup, __thiscall(system, landscape, type,
// extra), RET 0Ch -> group: the first +64h entry with +00 == landscape and +04 == type, else
// operator new(3Ch), 00AE5070(landscape, type, system+3Ch, extra) and append (null stays
// null and is appended).
void* foliage_system_find_or_create_tree_group_00ad8c60(void* system, void* landscape, std::uint32_t type,
    std::uint32_t extra, TerrainHost& host);
// 00AD8DD0 BSP_FoliageSystem_FindOrCreateBushGroup, __thiscall(system, landscape,
// NativeString a (by value), NativeString b (by value)), RET 14h -> group. Both by-value
// strings are released before returning.
void* foliage_system_find_or_create_bush_group_00ad8dd0(void* system, void* landscape, void* string_a,
    void* string_b, TerrainHost& host);
// 00AD9570 BSP_FoliageSystem_Destroy, __fastcall(system).
void foliage_system_destroy_00ad9570(void* system, TerrainHost& host);

// 00AD9C40 BSP_TerrainRenderObject_CreateBlock_Provisional, __thiscall(ECX unused, a, b, c),
// RET 0Ch: operator new(38h), 00ADF6B0(a, b, c), vtable 00D5D278; null when new fails.
void* terrain_render_object_create_block_00ad9c40(std::uint32_t a, std::uint32_t b, std::uint32_t c,
    TerrainHost& host);
// 00AD9E60, __fastcall(object): base 00AD9D00, vtable 00D5D288.
// 00AD9F90, __fastcall(object): base 00AD9D00, +04..+10 zeroed, vtable 00D5D2A0.
void* terrain_render_object_construct(std::uint32_t address, void* object, TerrainHost& host);
// 00AD9EC0 BSP_TerrainRenderObject_Destroy, __fastcall(object): vtable 00D5D2A0, releases
// the ref slots +04, +10h, +08, +0Ch, then base 00AD9DA0.
void terrain_render_object_destroy_00ad9ec0(void* object, TerrainHost& host);
// 00AD9FD0 BSP_Terrain_GetShaderName, __thiscall(ECX unused, NativeString* out), RET 4:
// out = "Terrain.mshd"; returns out.
void* terrain_get_shader_name_00ad9fd0(void* out, TerrainHost& host);

// Image calls of the segment-80 loaders.
struct FoliageLoadHost {
    virtual ~FoliageLoadHost() = default;
    // [0109CEEC] vtable +04h (path, mode 2) -> ref-counted stream (count at +04h).
    virtual void* vfs_open_0109ceec(const void* path, std::uint32_t mode) = 0;
    // stream vtable +38h (0) -> int; vtable +48h (NativeString* out, 0).
    virtual std::int32_t stream_read_int(void* stream) = 0;
    virtual void stream_read_string(void* stream, void* out_string) = 0;
    // 00AD7030 (system, const NativeString* model), RET 4 -> tree group type index
    // (reconstructed as bsp::foliage_setup_cc13::find_tree_group_type_00ad7030).
    virtual std::uint32_t find_tree_group_type_00ad7030(void* system, const void* model) = 0;
    // 00AE5870 (group, stream, v), RET 8; 00AE2450 (group, stream, count, NativeString a,
    // NativeString b by value), RET 18h.
    virtual void tree_group_initialise_00ae5870(void* group, void* stream, std::int32_t v) = 0;
    virtual void bush_group_initialise_00ae2450(void* group, void* stream, std::int32_t count, void* string_a,
        void* string_b) = 0;
    // 00AE84E0 (ECX = [00F8C264] type table): the Effects\foliage\FoliageTypes.lua parse.
    virtual void parse_foliage_types_00ae84e0(void* table) = 0;
    // 00AD42C0: allocates a red-black tree head node.
    virtual void* allocate_tree_head_00ad42c0() = 0;
};

// 00AD9050 BSP_FoliageSystem_LoadBushes, __thiscall(system, landscape, const NativeString*
// path), RET 8: count = read_int; when non-zero: a = read_string, b = read_string, group =
// 00AD8DD0(landscape, copy a, copy b), 00AE2450(group, stream, count, copy a, copy b); the
// locals b then a are released, then the stream reference.
void foliage_system_load_bushes_00ad9050(void* system, void* landscape, const void* path, TerrainHost& host,
    FoliageLoadHost& load);
// 00AD91C0 BSP_FoliageSystem_LoadTrees, __thiscall(system, landscape, path), RET 8:
// v = read_int; while v: x = read_int, model = read_string, type = 00AD7030(model), group =
// 00AD8C60(landscape, type, x), 00AE5870(group, stream, v), release model, v = read_int.
void foliage_system_load_trees_00ad91c0(void* system, void* landscape, const void* path, TerrainHost& host,
    FoliageLoadHost& load);
// 00AD9AC0 (ledger name BSP_FoliageSystem_LoadTypes; it is the A8h foliage system
// constructor), __fastcall(system) -> system: base 00AD6950, vtable 00D5D270, words
// +18h..+40h, +48h..+50h, +58h..+60h, +68h..+70h, +7Ch..+98h zeroed, bytes +05 = 0 and
// +06 = 1, a 14h type table built inline (00AD6810, vtable 00D5D274, +08..+10 zeroed), then
// 00AE84E0 on [00F8C264], then +9Ch..+A4h and byte +04 zeroed. Words +14h, +44h, +54h,
// +64h and +74h/+78h are not written here (allocator words and fields set by callers).
void* foliage_system_construct_00ad9ac0(void* system, TerrainHost& host, FoliageLoadHost& load);
// 00AD5590 STL_Tree_Construct, __fastcall(tree) -> tree: head = 00AD42C0(), head+15h
// (nil) = 1, head left/parent/right = head, size (+08) = 0.
void* tree_construct_00ad5590(void* tree, FoliageLoadHost& load);
// 00AD55C0 BSP_FontDescriptor_Construct_Provisional, __thiscall(this, const NativeString*
// name, a, b, byte c), RET 10h: tree at +04 as 00AD5590, NativeString +0Ch = *name, +18h = a,
// word +14h = 0, +1Ch = b, +20h..+44h zeroed, byte +48h = c. Font-registry code despite the
// segment (only caller 00AC3910).
void* font_descriptor_construct_00ad55c0(void* object, const void* name, std::uint32_t a, std::uint32_t b,
    std::uint8_t c, TerrainHost& host, FoliageLoadHost& load);

// ---------------------------------------------------------------------------
// Segment 81: terrain height field (vtable 00D5D350) and its grids.
//
// Height-field layout used here (docs/SCENE_CONTENTS_HOSTS.md section 6 has the rest):
//   +10h/+14h height range (floats)  +18h tile size (300.0)  +1Ch cell size (9.375)
//   +2Ch scene node (+5Ch bit 1 = world matrix fresh, +F0h world matrix, +120h/+128h world
//   x/z)  +30h/+34h/+60h/+64h ref-counted members  +38h columns  +3Ch rows
//   +40h tile pointers [row * columns + column]  +5Ch foliage system  +80h/+84h origin
// cc9 reconstructed slots 20h/24h/28h/30h/34h/38h/3Ch/44h/48h and the .tdt load in
// src/game_hosts_scene_contents.cpp (SceneTerrainHeightField); the records below backfill
// those and these functions add the remaining slots.
// ---------------------------------------------------------------------------
struct HeightFieldHost {
    virtual ~HeightFieldHost() = default;
    // 00B6DB70 BSP_Transform_RefreshWorldMatrix (ECX = node), called when node+5Ch bit 1 is
    // clear.
    virtual void refresh_world_matrix_00b6db70(void* node) = 0;
    // 004142E0 BSP_Vector3f_TransformAffinePoint(in, out, matrix) -> out.
    virtual void transform_affine_point_004142e0(const float in[3], float out[3], const void* matrix) = 0;
    // Height-field virtual slots (ECX = field).
    virtual std::int32_t slot10_samples_wide(void* field) = 0;
    virtual std::int32_t slot14_samples_deep(void* field) = 0;
    virtual void slot44_split_index(void* field, std::int32_t* tx, std::int32_t* tz, std::int32_t* lx,
        std::int32_t* lz, std::int32_t i, std::int32_t j) = 0;
    virtual float slot48_grid_height(void* field, float u, float v) = 0;
    virtual float slot4c_sample(void* field, std::int32_t i, std::int32_t j) = 0;
    virtual float slot50_sample_bilinear(void* field, float u, float v) = 0;
    // [00F8C218] (the terrain render object) vtable +04h (field, x, y) -> new tile.
    virtual void* create_tile(void* field, std::int32_t x, std::int32_t y) = 0;
    // 00BE99F0 BSP_StructuredNode_ReadDwordSlot38 (ECX = node).
    virtual std::int32_t node_read_dword_00be99f0(void* node) = 0;
    // 00ADFEB0 BSP_TerrainTile_ReadChunks (ECX = tile, node), reconstructed by cc9.
    virtual void tile_read_chunks_00adfeb0(void* tile, void* node) = 0;
    // tile+24h vtable +50h (argument, 1).
    virtual void tile_member_slot50(void* member, std::uint32_t argument) = 0;
};

inline constexpr float kNoDataHeight_00D7A240 = -1000.0f;
inline constexpr std::int32_t kSamplesPerTile_00D5D658 = 32;

// 00ADA0D0 BSP_Landscape_LocalGridToWorldXZ, slot (absolute dword 00D5D36C),
// __thiscall(field, float out[2], float u, float v), RET 0Ch: p = (f(cell*u + +80h), 0,
// f(cell*v + +84h)); node world refreshed when stale; out = (world*p).x, z.
void height_field_local_grid_to_world_xz_00ada0d0(void* field, float out[2], float u, float v,
    HeightFieldHost& host);
// 00ADA720 BSP_TerrainHeightField_GetWorldBounds, slot 0Ch, __thiscall(field, float
// out[6]), RET 4: {+80h + node x, -1000, +84h + node z, node x + tile*columns, +14h,
// node z + tile*rows}. The maximum corner does not add the +80h/+84h origin.
void height_field_world_bounds_00ada720(void* field, float out[6], HeightFieldHost& host);
// 00ADA810 BSP_TerrainHeightField_WorldToCell, slot 18h, __thiscall(field, int out[2],
// float x, float z), RET 0Ch -> out: out[k] = _ftol2(((p - node) - origin) * f(1/cell)).
std::int32_t* height_field_world_to_cell_00ada810(void* field, std::int32_t out[2], float x, float z,
    HeightFieldHost& host);
// 00ADA990 BSP_TerrainHeightField_HeightAtWorld, slot 2Ch, __thiscall(field, x, z), RET 8
// -> ST0: u, v as in 00ADA810 but kept as floats; returns f(slot48(u, v) + slot50(u, v)).
float height_field_height_at_world_00ada990(void* field, float x, float z, HeightFieldHost& host);
// 00ADAC50 / 00ADAC70, slots 10h/14h, __thiscall(field) -> EAX: +38h (+3Ch) * 32 + 1, or 0
// for an empty grid. (00ADAC50 is also reconstructed by cc9.)
std::int32_t height_field_samples_00adac50(const void* field, bool deep);
// 00ADB280 / 00ADBD00 height-range scans, __fastcall(field): +10h/+14h reset to the
// sentinels (100000, -100000 for 00ADB280 from 00CE4BC8/00D5D2D4; 1e10, -1000 for 00ADBD00
// from 00CE4970/00D7A240), then min/max over every present tile's +14h/+18h:
// min = lo > min ? min : lo, max = max > hi ? max : hi (x87 FCOMI, unordered takes the tile).
void height_field_compute_height_range(std::uint32_t address, void* field);
// 00ADB550 BSP_TerrainHeightField_SampleAt, slot 4Ch, __thiscall(field, i, j), RET 8 -> ST0:
// 0 outside [0, slot10) x [0, slot14); slot44 splits (i, j) into tile and local indices;
// a missing tile gives 0; else the tile's byte layer (tile+1Ch, 00ADEA70) sampled clamped
// by 00ADB1D0 at (lx, lz): f(byte / 255.0 * 60.0).
float height_field_sample_at_00adb550(void* field, std::int32_t i, std::int32_t j, HeightFieldHost& host);
// 00ADB620 BSP_TerrainHeightField_HeightAtGridBilinear, slot 50h, __thiscall(field, u, v),
// RET 8 -> ST0: i = (int)u, j = (int)v (CVTTSS2SI); fu = f(u - i), fv = f(v - j);
// h11, h01, h10, h00 from slot 4Ch in that call order; a = f(h00 + (h10 - h00) * fu),
// b = f(h01 + (h11 - h01) * fu), result f(a + (b - a) * fv).
float height_field_sample_bilinear_00adb620(void* field, float u, float v, HeightFieldHost& host);
// 00ADBC90 BSP_TerrainTileGrid_LoadTileFromNode, __thiscall(field, node, two unused stack
// words), RET 0Ch: x, y = two node dwords; the node world matrix is refreshed twice (the
// results are not used); tile = create_tile(field, x, y) stored at +40h[y * columns + x]
// (unchecked); 00ADFEB0(tile, node).
void tile_grid_load_tile_from_node_00adbc90(void* field, void* node, HeightFieldHost& host);
// 00ADBF80 BSP_TerrainLandscape_ReleaseTiles, __fastcall(field): releases every tile slot
// (rows x columns), then on the foliage system +5Ch: 00AD7B90(field) and 00AD56C0(0)
// (its +3Ch); then the ref slots +30h, +34h, +64h, +60h.
void landscape_release_tiles_00adbf80(void* field, TerrainHost& host);
// 00ADB230 BSP_TerrainTileGrid_ForEachTile, __thiscall(field, argument), RET 4: for each
// row and column calls 00ADEAD0(argument) with ECX = the tile slot's tile. 00ADEAD0
// (__thiscall(tile, argument), RET 4) forwards to tile+24h vtable +50h(argument, 1).
void tile_grid_for_each_tile_00adb230(void* field, std::uint32_t argument, HeightFieldHost& host);
void tile_forward_slot50_00adead0(void* tile, std::uint32_t argument, HeightFieldHost& host);
// 00ADEA70, __thiscall(tile) -> EAX = tile+1Ch (the tile's block).
void* tile_block_00adea70(const void* tile);

// Byte layers {.. +20h width, +24h height, +28h bytes}.
// 00ADB160 (ledger: BSP_TerrainByteLayer_SampleClamped; it writes), __thiscall(layer, x, y,
// byte value), RET 0Ch: x, y clamped to [0, w-1] x [0, h-1]; cell = max(cell, value).
void byte_layer_stamp_max_00adb160(void* layer, std::int32_t x, std::int32_t y, std::uint8_t value);
// 00ADB1D0, __thiscall(layer, x, y), RET 8 -> AL: x, y clamped to [0, 21h] (a 33x33 block,
// width still from +20h).
std::uint8_t block_byte_layer_sample_00adb1d0(const void* layer, std::int32_t x, std::int32_t y);

// Grids {+00 width, +04 height, +08 cells}. Resize: operator new(w * h * size) (the size is
// OR-ed to FFFFFFFFh on overflow), copy the overlapping min(w, old w) x min(h, old h) block
// row by row; the "fill" forms write *fill into every other cell, the plain forms leave them
// unset (here: as allocated). The old cells are freed and +08 cleared first; then w, h, cells.
//   00ADACF0 (4 bytes), 00ADAF50 (2), 00ADF5E0 (1), 00AED2D0 (4, float), 00AED480 (0Ch):
//   __thiscall(grid, w, h), RET 8.  00ADADC0 (4) and 00AED210 (1): (grid, w, h, const T*
//   fill), RET 0Ch.
struct GridShape { std::uint32_t address; std::uint32_t element_size; bool fill; };
extern const GridShape kGridResizes[7];
void grid_resize(std::uint32_t address, void* grid, std::int32_t w, std::int32_t h, const void* fill,
    TerrainHost& host);
// Clear: free +08 when set (and clear it), zero +04 and +00. __fastcall(grid):
//   00ADB020, 00ADB0F0, 00ADBDC0, 00ADBE30, 00ADF680, 00ADF710, 00AED690, 00AED6C0, 00AED940,
//   00AED970.
void grid_clear(void* grid, TerrainHost& host);

// ---------------------------------------------------------------------------
// Terrain blocks (33x33 sample patches) and tiles.
//
// Block base (00AED9A0): +00 vtable, +04 refcount 1, +08 0, +0Ch -1, +10h width, +14h
// height, +18h 1e10 (00CE4970), +1Ch -1e10 (00CE4ADC), +20h..+28h byte grid (zero-filled
// by 00AED210). Derived: float block (vtable 00D5D724, float grid +2Ch..+34h, +0Ch = 0),
// U16 block (00D5D2DC, u16 grid +38h..+40h), U8 block (00D5D424, byte grid +38h..+40h; the
// 44h form of 00ADFE10 then installs 00D5D45C and +0Ch = 1). The second base 00AEDBC0
// (vtable 00D5D75C) keeps a 0Ch-element grid at +18h..+20h instead.
// Tile (vtable 00D5D418, 38h bytes, 00ADF6B0): +08..+10 the three constructor arguments,
// +14h/+18h sample range, +1Ch block, +20h second block, +24h node, +28h..+34h four
// ref-counted slots.
// ---------------------------------------------------------------------------
struct BlockHost {
    virtual ~BlockHost() = default;
    // Block vtable slots (ECX = block): +08h sample (float* out, i, j), RET 0Ch; +0Ch range
    // (float* lo, float* hi), RET 8; +10h read (chunk), RET 4.
    virtual void block_slot08_sample(void* block, float* out, std::int32_t i, std::int32_t j) = 0;
    virtual void block_slot0c_range(void* block, float* lo, float* hi) = 0;
    virtual void block_slot10_read(void* block, void* chunk) = 0;
    // 00BCFAE0-style base destructor BSP_RefCounted_DestroyBase (ECX = object).
    virtual void ref_counted_destroy_base(void* object) = 0;
};

// 00AED9A0 / 00AEDBC0 base constructors, __thiscall(block, w, h), RET 8 -> block.
void* terrain_block_construct_base_00aed9a0(void* block, std::int32_t w, std::int32_t h, TerrainHost& host);
void* terrain_block_construct_base_00aedbc0(void* block, std::int32_t w, std::int32_t h, TerrainHost& host);
// 00AEDAC0 float, 00ADC420 U16, 00ADF9F0 U8 constructors, __thiscall(block, w, h), RET 8.
void* terrain_block_construct(std::uint32_t address, void* block, std::int32_t w, std::int32_t h,
    TerrainHost& host);
// Destructors, __fastcall(block). 00AEDA80 base level (vtable 00D5D6EC, frees +28h, zeroes
// +20h/+24h, vtable 00D5D6B0, refcounted base); 00AEDC40 the 00D5D75C base (grid +18h);
// 00AEDB60 float (frees +34h, then the 00AEDA80 level inline); 00ADC680 / 00ADC8A0 U16 and
// 00ADFC50 / 00ADFD10 U8 (vtable, free +40h, zero +38h/+3Ch, then 00AEDA80).
void terrain_block_destruct(std::uint32_t address, void* block, TerrainHost& host, BlockHost& blocks);
// 00AEDB40, slot of the float block, __thiscall(block) -> ST0: -1e10 (00CE4ADC).
float terrain_float_block_default_height_00aedb40();
// 00ADFB80, U8 block sample (slot +14h of 00D5D418 per the ledger), __thiscall(block,
// float* out, i, j), RET 0Ch: byte at +40h[j * (+38h) + i]; FFh -> -1000, else
// f(byte * (+34h) + (+2Ch)).
void terrain_u8_block_sample_00adfb80(const void* block, float* out, std::int32_t i, std::int32_t j);
// 00AED020 (block slot 0Ch, the 33x33 min/max) is reconstructed by cc9 as
// tile_range_00aed020 in src/game_hosts_scene_contents.cpp.
// Float layer of the float block: 00AED710 get (out, i, j), RET 0Ch: +34h[j * (+2Ch) + i];
// 00AED890 fill (value), RET 4: every cell of the +30h x +2Ch grid.
void terrain_float_layer_get_00aed710(const void* block, float* out, std::int32_t i, std::int32_t j);
void terrain_float_layer_fill_00aed890(void* block, float value);

// 00ADF6B0 tile constructor (ledger: BSP_TerrainRenderBlock_Construct), __thiscall(tile,
// a, b, c), RET 0Ch: +08..+10 = a, b, c; refcounted base (00CEB130, count 1); vtable
// 00D5D418; +1Ch..+24h and +28h..+34h zeroed.
void* terrain_tile_construct_00adf6b0(void* tile, std::uint32_t a, std::uint32_t b, std::uint32_t c);
// 00ADF7E0 tile destructor (ledger: BSP_TerrainBlockU8_Destroy), __fastcall(tile): vtable
// 00D5D418; node +24h unlinked when set; slots +28h..+34h, +1Ch, +20h released; refcounted
// base.
void terrain_tile_destruct_00adf7e0(void* tile, TerrainHost& host, BlockHost& blocks);
// 00ADF8B0 BSP_TerrainTile_SetBlock, __thiscall(tile, block), RET 4: when +1Ch differs:
// store, AddRef the new, release the old, then the new block's slot0C(&lo, &hi) into
// +14h/+18h.
void terrain_tile_set_block_00adf8b0(void* tile, void* block, TerrainHost& host, BlockHost& blocks);
// 00ADF920, __thiscall(tile, block), RET 4: the same retained assignment on +20h (no range).
void terrain_tile_set_second_block_00adf920(void* tile, void* block, TerrainHost& host);
// 00ADF960 / 00ADFE10 BSP_TerrainTile_Create{Float,U8}Block, __thiscall(tile, chunk), RET 4:
// new block (float 38h through 00AEDAC0(21h, 21h); U8 44h through 00ADF9F0(21h, 21h), vtable
// 00D5D45C, +0Ch = 1), block slot10(chunk), SetBlock, then the creation reference released.
void terrain_tile_create_block(std::uint32_t address, void* tile, void* chunk, TerrainHost& host,
    BlockHost& blocks);
// 00ADEB00 BSP_TerrainTile_RefreshHeightRange, __thiscall(tile): +1Ch slot0C(&lo, &hi) into
// +14h/+18h.
void terrain_tile_refresh_height_range_00adeb00(void* tile, BlockHost& blocks);
// 00ADC8E0, __fastcall(grid): when byte +50h is set: for each tile in the pointer array
// +44h: 00ADEB00 then tile vtable +08h; 00ADBE60 resize 0; +50h = 0.
void tile_list_release_00adc8e0(void* owner, TerrainHost& host, BlockHost& blocks);
// 00ADD1B0 BSP_TerrainHeightField_Destroy, __fastcall(field): vtable 00D5D350, 00ADBF80,
// quadtree +88h destroyed (00AEA8E0), string +54h, pointer array +44h resized to 0 and
// freed, tile grid +38h..+40h cleared, string +20h, then the weak-link base 00AE9930.
struct HeightFieldDestroyHost {
    virtual ~HeightFieldDestroyHost() = default;
    virtual void quadtree_destroy_00aea8e0(void* quadtree) = 0;
    virtual void weak_link_destruct_00ae9930(void* object) = 0;
};
void height_field_destroy_00add1b0(void* field, TerrainHost& host, HeightFieldDestroyHost& more);

} // namespace bsp::cc13_terrain
