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
    // 0041DD40 alone: resize to len keeping the leading characters (new ones unspecified).
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t len) = 0;
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
    virtual std::uint32_t read_global_u32(std::uint32_t address) = 0;
    virtual std::uint8_t read_global_u8(std::uint32_t address) = 0;
    virtual void write_global_u8(std::uint32_t address, std::uint8_t value) = 0;

    // Named image routines.
    virtual void node_unlink_and_release_00b6d8f0(void* node) = 0;                   // BSP_Node_UnlinkAndRelease
    virtual void destroy_native_string_range_00432050(void* first, void* last) = 0;
    virtual void destroy_foliage_ref_object_00ad7880(void* object) = 0;              // when not modelled inline
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
    // Win32 LeaveCriticalSection / DeleteCriticalSection on an embedded section.
    virtual void cs_leave(void* cs) = 0;
    virtual void cs_delete(void* cs) = 0;
    // 00B73C10 BSP_MeshObject_ClearDrawSectionOwners (ECX = mesh); 00AE19B0
    // BSP_Shadow_ResizePointerVector(vector, n); the GeneratedModel type id and a node's
    // vtable +0Ch IsType(type) test.
    virtual void mesh_clear_draw_section_owners_00b73c10(void* mesh) = 0;
    virtual void shadow_resize_pointer_vector_00ae19b0(void* vector, std::int32_t n) = 0;
    virtual bool node_is_generated_model(void* node) = 0;
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
// 00AF88F0 (BSP_NativeParticleObject_ReserveModels), 00ADBDF0 (00ADB050), 00AEE010 (00735F90),
// 00AEE080 (BSP_PointerArray_ReserveCurrentStorage). The reserve is the caller's callback.
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

// ---------------------------------------------------------------------------
// Lazy singletons (double-checked under the lifetime lock with the depth counter):
//   00ADC7B0 [0109DBB4]: operator new(4), vtable 00D5D2D8, registers the object.
//   00AF0990 [00F8C278]: operator new(8), +04 = 00D5D7E8 then 00D5D7F0, +00 = 00D5D7F4;
//   registers object+4 (null when new failed). Both return the global.
// ---------------------------------------------------------------------------
void* lazy_singleton_get(std::uint32_t address, TerrainHost& host);

// ---------------------------------------------------------------------------
// Segment 82: the particle band.
// ---------------------------------------------------------------------------
struct ParticleHost {
    virtual ~ParticleHost() = default;
    // RNG RULE: both draws are image stream 0 (ECX = 0), shared with gameplay. A host that
    // wires these bodies in must route them to a presentation stream or keep them off.
    virtual std::uint32_t draw_u32_stream0_00bd2fc0() = 0;      // BSP_RandomThreads_NextU32
    virtual float draw_fraction_stream0_00bd2f40() = 0;         // DrawUnsignedFraction -> ST0
    // BSP_CriticalSection_Create (allocated, initialised); LeaveCriticalSection /
    // DeleteCriticalSection on the image section.
    virtual void* critical_section_create() = 0;
    virtual void critical_section_leave(void* cs) = 0;
    virtual void critical_section_delete(void* cs) = 0;
    // BSP_ParticleParameter_DisposePayload / _ReturnPoolSlot (ECX = parameter).
    virtual void particle_parameter_dispose_payload(void* parameter) = 0;
    virtual void particle_parameter_return_pool_slot(void* parameter) = 0;
    // 00AF3F50 BSP_ParticleDefinition_CopyBounds(ECX = definition, out) -> bounds;
    // 00B74390 BSP_GeneratedModel_SetLocalBounds(ECX = model, bounds).
    virtual void* particle_definition_copy_bounds_00af3f50(void* definition, void* out) = 0;
    virtual void generated_model_set_local_bounds_00b74390(void* model, void* bounds) = 0;
    // BSP_NativeParticleModel_BuildFrameGeometry (ECX = model).
    virtual void particle_model_build_frame_geometry(void* model) = 0;
    // The vertex stride of the model's geometry vertex stream 0: 00B74640 GetGeometry(0),
    // BSP_Geometry_GetVertexStream(.., 0), stream vtable +24h -> descriptor, descriptor +CCh.
    virtual std::int32_t particle_model_vertex_stride(void* model) = 0;
    // 00B74F90 BSP_TypeInfo_Initialize3dObject (ECX = the type-info global 01090034);
    // 006FAC20 BSP_TypeId_GetCounterSingleton (counter at +04).
    virtual void type_info_initialize_3d_object_00b74f90(std::uint32_t type_info_global) = 0;
    virtual void* type_id_counter_singleton() = 0;
};

// 00AF19B0 BSP_ParticleSystem_CreateLock (static initialiser, __fastcall, returns ECX):
// [00F8C284] = BSP_CriticalSection_Create().
void particle_system_create_lock_00af19b0(TerrainHost& host, ParticleHost& particles);
// 00AF2650 BSP_ParticleSystem_DestroyLock (atexit): when [00F8C284]: leave while the depth
// counter (cs+18h) is positive (decrementing first), DeleteCriticalSection, _free; then the
// global is cleared.
void particle_system_destroy_lock_00af2650(TerrainHost& host, ParticleHost& particles);
// 00AF55B0 BSP_ByteBuffer_Reset, __thiscall(buffer, size), RET 4 -> AL: +08 = size, +18h = 0,
// +00 = 0, frees +14h when set (not cleared), +14h = operator new(size); returns +14h != 0.
bool byte_buffer_reset_00af55b0(void* buffer, std::uint32_t size, TerrainHost& host);
// 00AF5A30 BSP_Random_VariationFactor, __thiscall(ECX ignored, float spread), RET 4 -> ST0:
// spread == 0 (UCOMISS; NaN computes) -> 1.0; else f((int32)draw_u32 * spread * 2^-31 + 1.0)
// (FILD of the draw, doubles 00D5DA30 / 00D7A210). One stream-0 draw.
float random_variation_factor_00af5a30(float spread, ParticleHost& particles);
// 00AF7F70 BSP_Random_FractionTimes, (float x) on the stack, RET 4 -> ST0: x == 0 -> 0.0;
// else f(fraction * x). One stream-0 draw.
float random_fraction_times_00af7f70(float x, ParticleHost& particles);
// 00AF7FB0 BSP_ParticleEmitter_RandomSign, __fastcall(emitter) -> ST0: byte +29h clear ->
// 1.0; else one stream-0 fraction: r >= 0.5 -> 1.0, otherwise (and unordered) -1.0.
float particle_emitter_random_sign_00af7fb0(const void* emitter, ParticleHost& particles);
// 00AFB790 / 00AFB7C0 BSP_ParticleRange_SetA/B, __thiscall(range, a, b), RET 8: +08/+0Ch or
// +10h/+14h.
void particle_range_set(std::uint32_t address, void* range, std::uint32_t a, std::uint32_t b);
// 00AF5B70 BSP_NativeParticleModel_RefreshBounds, __thiscall(model): bounds of the definition
// +18Ch (00AF3F50 into a 10h local) handed to 00B74390.
void particle_model_refresh_bounds_00af5b70(void* model, ParticleHost& particles);
// 00AF0520 BSP_FoliageGroupManager_RebuildParticleFrame (no arguments): BuildFrameGeometry on
// [[00F8C274] + 1Ch].
void foliage_group_manager_rebuild_particle_frame_00af0520(TerrainHost& host, ParticleHost& particles);
// 00AF0540, __thiscall(model, count), RET 4: when +24h and +28h are set, +28h += stride *
// count.
void particle_model_advance_vertex_cursor_00af0540(void* model, std::int32_t count, ParticleHost& particles);
// 00AF1B90 / 00AF5D80 pointer-array grow (capacity = 2 * capacity + 2, stored first; copy;
// free the old buffer when set): the bsp::cc13_scene2 "Grow" shape.
void pointer_array_grow(std::uint32_t address, PtrArray& a);
// 00AF6B90 BSP_TypeInfo_InitializeMwParticleSystem, __fastcall(type info): once (guard
// byte 00F8D2CC): +10h = "MwParticleSystem", 3d-object type init, +04..+0Ch = the dwords
// at 01090034..0109003C, +00 = the next type id (counter +04 post-increment). 00AF7490 calls
// it and returns ECX.
void type_info_initialize_mw_particle_system_00af6b90(void* type_info, TerrainHost& host, ParticleHost& particles);
void* type_info_construct_mw_particle_system_00af7490(void* type_info, TerrainHost& host, ParticleHost& particles);
// 00AF81C0, __thiscall(parameter, flags), RET 4 -> parameter: dispose payload; return the
// pool slot when bit 0 is set. 00AF8400, __fastcall(slot): when *slot: dispose, return the
// slot, *slot = 0.
void* particle_parameter_scalar_deleting_dtor_00af81c0(void* parameter, std::uint8_t flags, ParticleHost& particles);
void particle_parameter_ptr_delete_00af8400(void** slot, ParticleHost& particles);
// 00AF12A0 BSP_ParticleShaderSet_Destroy, __fastcall(set): vtable 00D5D844; +04 released
// without a null test and not cleared; then +18h, +08, +0Ch, +10h, +14h, +1Ch, +20h, +24h,
// +28h, +2Ch released and cleared; base 00AF11E0.
void particle_shader_set_destroy_00af12a0(void* set, TerrainHost& host);

// 2Ch-byte parameter keys {data, count, capacity} of the parameter builder.
// 00AFBE90 BSP_ParticleParameterBuilder_PushKey, __thiscall(keys, const key*), RET 4: when
// full, ReserveKeys(2 * capacity) (no minimum: a zero capacity asks for 0); append 11 dwords.
// 00AFC1D0 BSP_ParticleParameterBuilder_AssignKeys, __thiscall(keys, const keys*), RET 4 ->
// keys: _free(data), zero the header, ReserveKeys(source count), then the PushKey loop.
using KeyReserve = std::function<void(NativeArray<std::uint8_t>&, std::int32_t)>;  // data/count/cap over 2Ch keys
void particle_keys_push_00afbe90(void* keys, const void* key, const KeyReserve& reserve);
void* particle_keys_assign_00afc1d0(void* keys, const void* source, const KeyReserve& reserve, TerrainHost& host);
// MSVC vector deleting destructors, __thiscall(object, flags), RET 4: flags & 2 -> the
// element destructor over the count at object-4 (eh vector destructor iterator), _free of
// object-4 when flags & 1, returns object-4; otherwise (00AFCE80 only) the element destructor
// on the object, then _free when flags & 1. 00AFCE30: 1-byte elements, element destructor
// BSP_NativeParticleArrayByte_Destroy; its scalar path skips the destructor. 00AFCE80: 108h
// elements, BSP_ParticleTemporary_Destroy. 00B04AB0: 8-byte elements,
// BSP_NativeParticleEmitterRow_Destroy.
void* vector_deleting_destructor(std::uint32_t address, void* object, std::uint8_t flags,
    const std::function<void(void*)>& element_destructor, TerrainHost& host);
// 00AFD820 BSP_NativeParticleEmitterSet_UpdateTransforms, __thiscall(set, argument), RET 4:
// for i < +14h: 00AFDCF0(argument) with ECX = +0Ch + byte(+04)[i] * 108h.
void particle_emitter_set_update_transforms_00afd820(void* set, std::uint32_t argument,
    const std::function<void(void* emitter, std::uint32_t argument)>& update_00afdcf0);
// 00AFEBE0 pointer-array push back growing to capacity + 1 (minimum 1) through 00AFEB60,
// __thiscall(array, const T*), RET 4.
void pointer_array_push_back_plus_one_00afebe0(PtrArray& a, void* value);
// 00AFECE0, __thiscall(array, const array*), RET 4 -> array: resize 0 (00AFEC20), reserve
// the source count, then the 00AFEBE0 push loop.
PtrArray& pointer_array_assign_00afece0(PtrArray& a, const PtrArray& source);

// Named pointer lists, 14h bytes {NativeString name +00/+04, pointer array +08..+10}, and
// their arrays {data, count, capacity}; these live in image memory (TerrainHost).
// 00AFEE90 destroy (__fastcall): the value array resized to 0 twice (00AFEC20), its data
// freed, the name released.
void named_pointer_list_destroy_00afee90(void* list, TerrainHost& host);
// 00AFEF30, __thiscall(list, NativeString name by value, value), RET 0Ch: zero the five
// words, copy the name, append value (00AFEBE0 growth), release the by-value name.
void* named_pointer_list_construct_00afef30(void* list, void* name, void* value, TerrainHost& host);
// 00AFF010, __thiscall(list, const list*), RET 4: copy the name, zero the array, 00AFECE0.
void* named_pointer_list_copy_00aff010(void* list, const void* source, TerrainHost& host);
// 00AFF0B0 reserve (n clamped to 1; when capacity < n: operator new(n * 14h), copy-construct
// each element into the new block, destroy each old one, free, data = new, capacity = n);
// 00AFF190 push back (capacity + 1, minimum 1; copy-construct at the end); 00AFF210 resize
// (reserve, zero new elements, destroy from the top down to n, count = n); 00AFF420 /
// 00AFF450 destroy (resize 0 then free data). All __thiscall on the array.
void named_list_array_reserve_00aff0b0(void* array, std::int32_t n, TerrainHost& host);
void named_list_array_push_back_00aff190(void* array, const void* list, TerrainHost& host);
void named_list_array_resize_00aff210(void* array, std::int32_t n, TerrainHost& host);
void named_list_array_destroy(std::uint32_t address, void* array, TerrainHost& host);
// 00AFF290 BSP_NamedPointerListArray_AddUnderName, __thiscall(array, NativeString name by
// value, value), RET 0Ch: the first list whose name matches (equal length, __stricmp; empty
// equals empty) gets value appended (capacity + 1 growth); otherwise operator new(14h) is
// constructed by 00AFEF30(name copy, value) and pushed by copy (00AFF190). The 14h temporary
// is never destroyed or freed (00AFF30C..00AFF37B: no 00AFEE90 or _free follows): the image
// leaks it with its name and value array. The by-value name is released on both paths.
void named_list_array_add_under_name_00aff290(void* array, void* name, void* value, TerrainHost& host);
// 00AFF8C0 BSP_ParticleEmitter_Destruct, __fastcall(emitter): vtable 00D5DCB4; container +10h
// deleted through its vtable +04(1) and cleared; definition +0Ch released; refcounted base.
void particle_emitter_destruct_00aff8c0(void* emitter, TerrainHost& host, BlockHost& blocks);
// 00B04DF0, __thiscall(array, uint n), RET 4: frees data when set, data = operator new(n *
// 6Ch) (overflow -> FFFFFFFFh), count (+04) = n. The old contents are not kept.
void array_reallocate_6c_00b04df0(void* array, std::uint32_t n, TerrainHost& host);
// 00B0A710, __thiscall(array, const array*), RET 4: count = 0; grow to the source count as
// 00B0A440 (copying nothing since count is 0); count = source count; copy the dwords.
void dword_array_assign_00b0a710(NativeArray<std::uint32_t>& a, const NativeArray<std::uint32_t>& source,
    TerrainHost& host);

// ---------------------------------------------------------------------------
// Segment 82 tail: textures, lights, tracer resources, render batches and small records.
// ---------------------------------------------------------------------------
struct RenderMiscHost {
    virtual ~RenderMiscHost() = default;
    virtual std::int32_t texture_format_00b33e60(void* texture) = 0;        // D3DFORMAT or FOURCC
    virtual void texture_premultiply_alpha_00b4c6a0(void* texture) = 0;     // level 0
    virtual void texture_alpha_from_non_black_00b4c6d0(void* texture) = 0;  // level 0
    // 00B62D10 BSP_Vector4f_Transform(ECX = in[4], out[4], matrix) -> out.
    virtual const float* vector4_transform_00b62d10(const float in[4], float out[4], const void* matrix) = 0;
    // Batch lists: list vtable +0Ch(index), +08h(index, camera); 00B1BF40 camera of a command.
    virtual void batch_list_slot0c(void* list, std::int32_t index) = 0;
    virtual void batch_list_slot08(void* list, std::int32_t index, void* camera) = 0;
    virtual void* render_command_camera_00b1bf40(void* command) = 0;
    // 004C1130 BSP_FrameJobPool_GetSingleton; BSP_RenderBatchPreparationJob_GetSingleton;
    // pool+4 vtable +04h(job, list).
    virtual void* frame_job_pool_004c1130() = 0;
    virtual void* render_batch_preparation_job() = 0;
    virtual void job_queue_submit(void* queue, void* job, void* list) = 0;
    // 12Ch material records: 00B13070 copy-construct, 00B13180 assign, 00B10740 destroy.
    virtual void material_record_copy_00b13070(void* dst, const void* src) = 0;
    virtual void material_record_assign_00b13180(void* dst, const void* src) = 0;
    virtual void material_record_destroy_00b10740(void* record) = 0;
    // The stats filter vector at service+69Ch: BSP_SceneRenderer_FindStatsFilterName returns
    // a checked iterator whose element address equals [service+6A4h] (last) when absent;
    // 00589690 erase at that element, 00450540 append.
    virtual std::uint32_t stats_filter_find(void* service, const void* name) = 0;
    virtual void stats_filter_erase_00589690(void* service, std::uint32_t element) = 0;
    virtual void stats_filter_append_00450540(void* service, const void* name) = 0;
    // 00B17E10 BSP_Material_RegisterParameter(ECX = material, name, source, words, matrix).
    virtual void* material_register_parameter_00b17e10(void* material, const void* name, const void* source,
        std::int32_t words, std::int32_t matrix) = 0;
    // BSP_ReturnNative88BytePoolSlot(object).
    virtual void return_88_byte_pool_slot(void* object) = 0;
    // BSP_ResourceRegistryTree_IncrementIterator / BSP_ResourceRegistry_IteratorDecrement
    // (ECX = iterator).
    virtual void registry_iterator_increment(void* iterator) = 0;
    virtual void registry_iterator_decrement(void* iterator) = 0;
    // Lua objects are host handles: 00B67980 BSP_LuaStateOwner_GetGlobals(ECX = owner, out);
    // 00B68100 BSP_LuaObject_GetByNativeString(ECX = table, out, name); 00B661B0 IsTable;
    // 00B67C40 GetFloat4(out); 00B67700 destructor.
    virtual void* lua_get_globals_00b67980(void* owner) = 0;
    virtual void* lua_get_by_name_00b68100(void* table, const void* name) = 0;
    virtual bool lua_is_table_00b661b0(void* object) = 0;
    virtual void lua_get_float4_00b67c40(void* object, float out[4]) = 0;
    virtual void lua_object_destruct_00b67700(void* object) = 0;
};

// 00B00950 BSP_Texture_PremultiplyAlphaIfUncompressed, __thiscall(texture): format above
// 'DXT1' (31545844h, signed compare) or equal to it -> nothing; 15h (A8R8G8B8) -> 00B4C6A0;
// 16h (X8R8G8B8) -> 00B4C6D0 (alpha from non-black, not premultiply); others nothing.
void texture_premultiply_alpha_if_uncompressed_00b00950(void* texture, RenderMiscHost& render);
// 00B094E0 BSP_Light_SetPosition, __thiscall(light, const float xyz[3]), RET 4: +1ECh..+1F4h.
void light_set_position_00b094e0(void* light, const float xyz[3]);
// 00B0A420, __fastcall(object): [00F8D38C] = 0, vtable 00CE3818 (the tail of 00B0ACA0).
// 00B0ACA0 BSP_NativeParticleTracerResources_Destruct, __fastcall(object): vtable 00D5E074,
// releases +08 then +04, then that tail.
void tracer_resources_destruct_singleton_base_00b0a420(void* object, TerrainHost& host);
void tracer_resources_destruct_00b0aca0(void* object, TerrainHost& host);
// 00B0D720 BSP_Vector3f_TransformProjective, __fastcall(ECX = float v[3] in/out, EDX =
// matrix): (x, y, z, 1) through 00B62D10; inv = 1/w (x87, not rounded); v = f(x * inv) ...
void vector3_transform_projective_00b0d720(float v[3], const void* matrix, RenderMiscHost& render);
// 00B0E970 BSP_SceneRenderer_RunBatchList, __thiscall(command, index), RET 4: list =
// command[+0Ch + 4 * index]; list slot0C(index); list (re-read) slot08(index, camera).
void scene_renderer_run_batch_list_00b0e970(void* command, std::int32_t index, RenderMiscHost& render);
// 00B10700 BSP_RenderBatchJobs_Submit, __thiscall(owner, index), RET 4: list+08 = index;
// pool = 004C1130(); pool+4 vtable +04h(preparation job, list).
void render_batch_jobs_submit_00b10700(void* owner, std::int32_t index, RenderMiscHost& render);
// 00B13350 BSP_MaterialRecord_Swap, __fastcall(a, b): tmp = copy(a); a = b; b = tmp; destroy
// tmp (a 12Ch local).
void material_record_swap_00b13350(void* a, void* b, RenderMiscHost& render);
// 00B171C0 BSP_SceneRenderer_ToggleStatsFilterName, __thiscall(service, const NativeString*
// name), RET 4: erase the name when present, else append it.
void scene_renderer_toggle_stats_filter_name_00b171c0(void* service, const void* name, RenderMiscHost& render);
// 00B18AC0 BSP_Material_RegisterFloat4ArrayParameter, __thiscall(material, name, source,
// int count), RET 0Ch: 00B17E10(name, source, count * 4, 0).
void* material_register_float4_array_00b18ac0(void* material, const void* name, const void* source,
    std::int32_t count, RenderMiscHost& render);
// 00B191D0 BSP_NamedRecord88_ScalarDeletingDestructor, __thiscall(record, flags), RET 4 ->
// record: name string +00/+04 released; flags & 1 -> 88-byte pool slot returned.
void* named_record88_scalar_deleting_dtor_00b191d0(void* record, std::uint8_t flags, TerrainHost& host,
    RenderMiscHost& render);
// 00B199A0 BSP_SceneTypeInfo_IsKindOf_00f8d428, (type id) on the stack, RET 4 -> AL: id is
// one of the two dwords at 00F8D428 / 00F8D42C (the cShaderDataSource type ids).
bool scene_type_info_is_kind_of_00b199a0(std::uint32_t id, TerrainHost& host);
// Iterator thunks, __fastcall(iterator) -> EAX = iterator: 00B19AF0 / 00B19BF0 increment,
// 00B1ABE0 / 00B1ABF0 decrement.
void* registry_iterator_step(std::uint32_t address, void* iterator, RenderMiscHost& render);
// 00B1AC00 BSP_NamedValue_Construct, __thiscall(record, const NativeString* name, const
// uint* value), RET 8; 00B1AC50 copy, __thiscall(record, const record*), RET 4: name copy,
// +08 = value.
void* named_value_construct_00b1ac00(void* record, const void* name, const std::uint32_t* value, TerrainHost& host);
void* named_value_copy_00b1ac50(void* record, const void* source, TerrainHost& host);
// 00B1B980 (ledger: BSP_LuaGlobals_GetField), __thiscall(owner, float out[4], const
// NativeString* name), RET 8 -> out: globals[name] as a float4 when it is a table, else
// (1, 1, 1, 1) (00D7A24C).
float* lua_global_float4_00b1b980(void* owner, float out[4], const void* name, RenderMiscHost& render);

// ---------------------------------------------------------------------------
// Terrain quadtree (vtable 00D5D6A4) and its 14h node array {data +3Ch, count +40h,
// capacity +44h}; the walk itself is cc9's (node_walk_00ae9d80 / ray_walk_00aea2b0).
// ---------------------------------------------------------------------------
struct QuadNode14 { std::uint32_t words[5]; };
// 00AE9AC0 reserve (n clamped to 1, copy, free), 00AEA530 push back (2 * capacity, minimum
// 1), 00AEA590 resize (zero new nodes, count = n), 00AEA8A0 destroy (resize 0, free): the
// cc13_scene2 shapes over 14h nodes. __thiscall(array[, n | const node*]).
void quad_nodes_reserve_00ae9ac0(NativeArray<QuadNode14>& a, std::int32_t n);
void quad_nodes_push_back_00aea530(NativeArray<QuadNode14>& a, const QuadNode14& node);
void quad_nodes_resize_00aea590(NativeArray<QuadNode14>& a, std::int32_t n);
void quad_nodes_destroy_00aea8a0(NativeArray<QuadNode14>& a);
// 00AEA8C0 construct (__fastcall): vtable, +3Ch..+44h zeroed. 00AEA8E0 destroy: vtable,
// resize 0, free +3Ch.
void* terrain_quadtree_construct_00aea8c0(void* tree);
void terrain_quadtree_destroy_00aea8e0(void* tree);
// 00AEA900 BSP_TerrainQuadtree_SetBounds, __thiscall(tree, field, const float min[3], const
// float max[3], int depth), RET 10h: +48h = field, +04..+0Ch = min, +10h..+18h = max, +34h =
// depth, +38h = 1 << depth, +1Ch..+24h = f(max - min), +28h..+30h = f(extent / (float)(1 <<
// depth)), then the node array resized to 0.
void terrain_quadtree_set_bounds_00aea900(void* tree, void* field, const float min[3], const float max[3],
    std::int32_t depth);
// 00AE9B50 (ledger: BSP_TerrainTileGrid_GetTileOrigin; it returns the tile's height range),
// __thiscall(tree, float out[2], x, y), RET 0Ch: field = +48h; when x < columns, y < rows
// (signed; no lower bound) and the tile is present: out = tile +14h/+18h; else (-1000, 1000).
void terrain_quadtree_tile_range_00ae9b50(const void* tree, float out[2], std::int32_t x, std::int32_t y);
// 00AE9870 BSP_TerrainWeakLink_Construct, __thiscall(link, ref), RET 4: weak-owner base
// (BSP_WeakOwner_Construct), vtable 00D5D660, +0Ch = 0 then the retained assignment of ref,
// +10h = -1000, +14h = 1e10, +18h = 300.0 (tile size), +1Ch = 9.375 (cell size).
// 00AE9930 destruct: vtable, release +0Ch, BSP_WeakOwner_Destroy.
struct WeakOwnerHost {
    virtual ~WeakOwnerHost() = default;
    virtual void weak_owner_construct(void* object) = 0;
    virtual void weak_owner_destroy(void* object) = 0;
};
void* terrain_weak_link_construct_00ae9870(void* link, void* ref, TerrainHost& host, WeakOwnerHost& weak);
void terrain_weak_link_destruct_00ae9930(void* link, TerrainHost& host, WeakOwnerHost& weak);

// Bush quad nodes (38h): children at +1Ch..+28h. 00AE09D0 destroys each present child
// recursively, frees it and clears the slot (order +1Ch, +20h, +24h, +28h); 00AE08E0 deletes
// the owned root (*slot) the same way. 00AE0910: when *slot, BSP_Node_UnlinkAndRelease and
// clear.
void bush_quad_node_destroy_children_00ae09d0(void* node, TerrainHost& host);
void bush_quad_node_delete_owned_00ae08e0(void** slot, TerrainHost& host);
void node_ref_release_00ae0910(void** slot, TerrainHost& host);

// Tree quad nodes (58h): when +24h > 0, children +2Ch..+38h destroyed recursively, freed and
// cleared; then +54h released; the array +14h freed and +14h..+1Ch zeroed (00AE4F70).
// 00AE2EC0 deletes *slot the same way (unreferenced).
void tree_quad_node_destroy_children_00ae4f70(void* node, TerrainHost& host);
void tree_quad_node_delete_00ae2ec0(void** slot, TerrainHost& host);
// 00AE4DD0, __thiscall({data, count}, const T* value), RET 4 -> AL: finds the first equal
// cell, moves the last cell into it (unless it is the last), count--; false when absent.
bool pointer_array_erase_value_swap_last_00ae4dd0(PtrArray& a, const void* value);
// 00AE0710 (ledger: BSP_TerrainRenderObject_SetFlag40), __thiscall(bush group, byte),
// RET 4: +40h = byte; mesh +2Ch (when set) 00B73C10 BSP_MeshObject_ClearDrawSectionOwners.
// 00AE2C80 (ledger: ..._SetFlag38): tree group +38h, mesh +28h, unconditional. Both are the
// per-group "enabled" setters 00AD71C0 runs over the system's +54h / +64h vectors.
void bush_group_set_enabled_00ae0710(void* group, std::uint8_t enabled, TerrainHost& host);
void tree_group_set_enabled_00ae2c80(void* group, std::uint8_t enabled, TerrainHost& host);
// 00AE2CB0 BSP_GeneratedModelFinder_Visit, __thiscall(finder, node), RET 4 -> AL: the node
// itself when its vtable +0Ch(GeneratedModel type) is true; else the first child (list +34h,
// siblings +3Ch) that is; else the first child whose recursive visit succeeds; finder+8
// receives the match (0 when none).
bool generated_model_finder_visit_00ae2cb0(void* finder, void* node, TerrainHost& host);
// 00AE4C20 BSP_TreeGroup_Destruct, __fastcall(group): quad root +10h deleted, node +18h
// unlinked, [+04]+44h (the type's instance count) decremented, ref slots +1Ch, +20h, +24h,
// +28h, +34h released.
void tree_group_destruct_00ae4c20(void* group, TerrainHost& host);
// 00AE1C80 BSP_BushGroup_Destruct, __fastcall(group): ref slots +50h, +28h, +2Ch, +3Ch,
// +38h; node +20h; quad root +18h; the critical section +54h left while its depth +6Ch is
// positive, then deleted; the shadow vector +44h resized to 0 (00AE19B0) and freed; strings
// +0Ch and +04.
void bush_group_destruct_00ae1c80(void* group, TerrainHost& host);

// Foliage type record (30h): +00 byte, +04, +08, NativeStrings +0Ch / +14h / +1Ch, dwords
// +24h..+2Ch. 00AE6590 construct, __thiscall(record, byte, a, NativeString s1, b, NativeString
// s2, NativeString s3, c, d, e) with the strings by value, RET 30h: the three strings zeroed,
// fields stored in argument order, the strings copied, then the by-value strings released.
// 00AE6720 copy-construct and 00AE6840 assign, __thiscall(record, const record*), RET 4.
struct FoliageTypeRecordArgs {
    std::uint8_t flag; std::uint32_t a; void* s1; std::uint32_t b; void* s2; void* s3;
    std::uint32_t c, d, e;
};
void* foliage_type_record_construct_00ae6590(void* record, const FoliageTypeRecordArgs& args, TerrainHost& host);
void* foliage_type_record_copy_00ae6720(void* record, const void* source, TerrainHost& host);
void* foliage_type_record_assign_00ae6840(void* record, const void* source, TerrainHost& host);
// 00AE7470 BSP_FoliageTypeGroup_CopyConstruct, __thiscall(group, const group*), RET 4: name
// copy, then 00AE7030 (vector<record> copy) on +08. 00AE76C0, __fastcall(group): +00, +04,
// +0Ch..+14h zeroed (+08, the allocator word, untouched).
void* foliage_type_group_copy_00ae7470(void* group, const void* source, TerrainHost& host,
    const std::function<void(void* dst_vector, const void* src_vector)>& vector_copy_00ae7030);
void foliage_type_group_construct_empty_00ae76c0(void* group);

// Float-layer quantizers of the float block (__thiscall(block, out, i, j), RET 0Ch):
// 00AED730: the owner +08 is retained around the body (InterlockedIncrement /
// InterlockedDecrement, slot 0 at zero); c = clamp(cell, owner+10h, owner+14h) (lo test
// first; NaN passes through); out u16 = trunc((lo + c) / f(hi - lo) * 65535.0) under a
// chop-mode FISTP. The numerator is lo + c, not c - lo, as the bytes show (00AED7BA FADD).
// 00AED820: c = 0 when 0 > cell, 1 when cell > 1, else cell (NaN passes); out byte =
// trunc(c * 255.0) (chop mode; the ledger's "round" is a truncation).
void terrain_float_layer_quantize16_00aed730(void* block, std::uint16_t* out, std::int32_t i, std::int32_t j,
    TerrainHost& host);
void terrain_float_layer_quantize8_00aed820(const void* block, std::uint8_t* out, std::int32_t i, std::int32_t j);

// Chunked stream I/O. Writers bracket their payload with BeginChunk / EndChunk on a
// pooled NativeString tag built from the literal ("U16" 00D5D6A8, "U8", "F32" 00D5D49C,
// "NODE" 00D5D3B8); readers pull from a structured node.
struct ChunkHost {
    virtual ~ChunkHost() = default;
    virtual void begin_chunk(void* writer, const char* tag) = 0;   // BSP_StreamWriter_BeginChunk
    virtual void end_chunk(void* writer, const char* tag) = 0;     // BSP_StreamWriter_EndChunk
    virtual void write_float(void* writer, float value) = 0;       // 00BF0360 ..WriteFloatAndDebit
    virtual void write_dword(void* writer, std::uint32_t value) = 0;
    virtual void write_bytes(void* writer, const void* bytes, std::uint32_t n) = 0;
    virtual float node_read_float_00be99d0(void* node) = 0;        // BSP_StructuredNode_ReadFloat
    virtual void node_read_raw_bytes(void* node, void* out, std::uint32_t n) = 0;
    virtual void block_slot14_write(void* block, void* writer) = 0; // block vtable +14h(writer)
};
// 00ADE670 U16 block write (__thiscall(block, writer), RET 4): "U16"; floats +2Ch, +30h; the
// +3Ch x +38h u16 samples of +40h row-major, two bytes each. 00AE0390: the same for the U8
// block, one byte each, tag "U8". 00AEDD60 F32 block: "F32"; for row < +30h, column < +2Ch
// the block's slot 08 sample (column, row) written as a float.
void terrain_block_write_chunk(std::uint32_t address, void* block, void* writer, ChunkHost& chunks,
    BlockHost& blocks);
// 00AE0250 (ledger: BSP_TerrainHeightField_WriteNodeChunk; ECX is the tile), __thiscall(tile,
// writer), RET 4: "NODE"; dwords +0Ch, +10h; the block +1Ch writes itself (vtable +14h).
void terrain_tile_write_node_chunk_00ae0250(void* tile, void* writer, ChunkHost& chunks);
// 00ADC6C0 U16 / 00ADFBD0 U8 block read (block slot 10h, __thiscall(block, node), RET 4):
// offset -> +2Ch, range -> +30h, +34h = f(1 / range); then +3Ch x +38h samples read raw
// (two bytes / one byte each) into +40h row-major. (00ADC6C0's load path is also modelled in
// cc9's loader.)
void terrain_block_read_chunk(std::uint32_t address, void* block, void* node, ChunkHost& chunks);

// ---------------------------------------------------------------------------
// Atlases, models and small leftovers.
// ---------------------------------------------------------------------------
struct AtlasHost {
    virtual ~AtlasHost() = default;
    // 00B33E40 BSP_LogicalTextureBase_GetNameAddress (ECX = texture) -> NativeString*.
    virtual const void* texture_name_00b33e40(void* texture) = 0;
    // _strstr 00BF9440.
    virtual const char* strstr_00bf9440(const char* haystack, const char* needle) = 0;
    // 00467CF0 BSP_NativeString_ReverseFindHeader(ECX = string, set, start) -> index or -1.
    virtual std::uint32_t reverse_find_00467cf0(const void* string, const void* set, std::uint32_t start) = 0;
    // BSP_Model_Destroy (ECX = model); BSP_BushGroupModelPool_Free (00AE13D0, ECX = pool).
    virtual void model_destroy(void* model) = 0;
    virtual void bush_model_pool_free_00ae13d0(void* object) = 0;
    // 00AD92C0 BSP_FoliageSystem_LoadLandscapeFoliage (ECX = system, landscape)
    // (reconstructed as bsp::foliage_setup_cc13::load_landscape_foliage_00ad92c0).
    virtual void foliage_load_landscape_00ad92c0(void* system, void* landscape) = 0;
};
// 00AEE150 BSP_TextureAtlas_FindTextureBySubstring, __thiscall(atlas, const NativeString*
// needle), RET 4 -> EAX: the first texture of the list +10h (count +14h) whose name contains
// the needle (_strstr, both strings non-null, offset > -1). DEVIATION: on a miss the image
// returns EAX as the loop left it (0 after a failed _strstr, the name record when a string
// was null, undefined for an empty list); this model returns null.
void* texture_atlas_find_by_substring_00aee150(void* atlas, const void* needle, AtlasHost& atlases);
// 00AEE220 BSP_TextureAtlasItem_Destruct, __fastcall(item): strings +0Ch then +00.
void texture_atlas_item_destruct_00aee220(void* item, TerrainHost& host);
// 00AEE4D0 BSP_TextureAtlasItem_Ctor, __thiscall(item, const NativeString* name, texture,
// const NativeString* descriptor), RET 0Ch -> item: +00 = descriptor, +0Ch = name, then the
// name is cut at the last "." (a pooled one-character string from 00CE3A70; 00467CF0 from
// 7FFFFFFFh); a found index resizes +0Ch to it (a growth would blank-fill with spaces); +08 =
// texture on every path.
void* texture_atlas_item_construct_00aee4d0(void* item, const void* name, void* texture, const void* descriptor,
    TerrainHost& host, AtlasHost& atlases);
// 00AE16B0, __thiscall(vector {data, count, capacity}, const T*), RET 4: when full,
// BSP_Shadow_ReservePointerVector(2 * capacity, at least 4); append.
void pointer_vector_push_back_min4_00ae16b0(PtrArray& a, void* value, const PtrReserve& reserve);
// 00AE1680 BSP_BushGroupModel_ScalarDeletingDtor, __thiscall(model, flags), RET 4 -> model:
// vtable 00D5D4A0, BSP_Model_Destroy, the pool Free when bit 0 is set.
void* bush_group_model_scalar_deleting_dtor_00ae1680(void* model, std::uint8_t flags, AtlasHost& atlases);
// 00B05400, slot 0 of the vtable 00D5DF24: a bare RET 4 (00B05400..00B05402); no effect.
void particle_container_unused_slot0_00b05400(std::uint32_t ignored);
// 00ADAC90 (ledger: BSP_TerrainRenderObject_LoadFoliageDefault), __fastcall(field) -> EAX = 0:
// field+5Ch = [00F8C210]; on that foliage system 00AD56C0(field+0Ch) (its +3Ch) and
// 00AD92C0(field).
std::uint32_t height_field_attach_foliage_00adac90(void* field, TerrainHost& host, AtlasHost& atlases);

struct GroupModelHost {
    virtual ~GroupModelHost() = default;
    // 00B18B20 BSP_Material_RegisterFloatParameter(ECX = material, const NativeString* name,
    // const float* source).
    virtual void material_register_float_00b18b20(void* material, const char* name, const float* source) = 0;
    // 00AD5760 BSP_FoliageSystem_GetBushShadowAllowed (ECX = [00F8C210]).
    virtual bool foliage_bush_shadow_allowed_00ad5760(void* system) = 0;
    // 00AE1FB0 BSP_BushGroupModel_BuildBushObject (ECX = model+184h object, view) -> object;
    // object vtable +20h(ctx, a, b, c).
    virtual void* bush_build_object_00ae1fb0(void* owner, void* view) = 0;
    virtual void render_slot20(void* object, void* ctx, std::uint32_t a, std::uint32_t b, std::uint32_t c) = 0;
    // 00B74640 BSP_GeneratedModel_GetGeometry (ECX = model, 0); 00B73260 vertex stream of a
    // geometry; 00B72270 BSP_Scene_FindNodeByName (ECX = root, name, 0).
    virtual void* generated_model_geometry_00b74640(void* model) = 0;
    virtual void* geometry_vertex_stream_00b73260(void* geometry) = 0;
    virtual void* scene_find_node_by_name_00b72270(void* root, const char* name) = 0;
};
// 00AE22D0 (ledger: BSP_BushGroupModel_RegisterDiffuseFactor; ECX is a bush group),
// __thiscall(group, float factor), RET 4: +24h = factor; "cDiffuseFactor" registered with
// source &+24h on the material +3Ch, then on +38h (a pooled name built and released each
// time).
void bush_group_set_diffuse_factor_00ae22d0(void* group, float factor, GroupModelHost& models);
// 00AE23F0 BSP_BushGroupModel_RenderFilter, slot +20h of 00D5D4A0, __thiscall(model, ctx, a,
// b, c), RET 10h: view = ctx+08; view type (+198h) 2 with bush shadows disallowed -> skip;
// when +184h is set, 00AE1FB0(view) and the built object's slot +20h(ctx, a, b, c).
void bush_group_model_render_filter_00ae23f0(void* model, void* ctx, std::uint32_t a, std::uint32_t b,
    std::uint32_t c, TerrainHost& host, GroupModelHost& models);
// 00AE5070 BSP_TreeGroupInstance_Construct, __thiscall(instance 3Ch, landscape, type,
// root, extra), RET 10h -> instance: +00 landscape, +04 type, +20h = the vertex stream of
// the type's model (+08) geometry, retained; type+44h++ (instances); +14h root, +08 = 0,
// +0Ch extra, +18h = root's node "TreeGroup", +10h = 0.
void* tree_group_instance_construct_00ae5070(void* instance, void* landscape, void* type, void* root,
    std::uint32_t extra, GroupModelHost& models);

// ---------------------------------------------------------------------------
// cc13-terrain2: ballistic particle height extrema (x87 traced by hand from the listing).
// The curve is y(t) = ((a t + b) t + c) t + d about x0: +00h x0, +04h y0, +1Ch a, +20h b,
// +24h c, +28h d; the other point gives +00h x and +04h y.
// 00AFB7F0 BSP_ParticleBallistic_MaxHeight and 00AFBA40 BSP_ParticleBallistic_MinHeight,
// __thiscall(ECX curve, const float* other), RET 4, ST0 result (a float re-loaded):
//   disc = f(4.0*b*b - 12.0*a*c) (doubles 00D7A328 / 00CE42D0); when 0 > disc or a == 0
//   (UCOMISS: unordered continues) the fallback applies. Otherwise s = f(sqrt(disc))
//   (00BF7030), t1 = f((s - 2b)/(6a) + x0), t2 = f((-2b - s)/(6a) + x0) (00CE6628 = 6,
//   00D7A308 = 2, 00D19660 = -2), each kept when x0 < t < other.x; for u = f(t - x0) the
//   extremum test is 6a*u + 2b < 0 (Max) / > 0 (Min), and the value
//   f(((a u + b) u + c) u + d) is returned (t1 first, then t2).
//   Fallback: Max returns curve.y0 when curve.y0 > other.y else other.y; Min returns
//   curve.y0 when other.y > curve.y0 else other.y (NaN picks other.y in both).
float ballistic_max_height_00afb7f0(const float* curve, const float* other) noexcept;
float ballistic_min_height_00afba40(const float* curve, const float* other) noexcept;

// ===========================================================================
// cc13-left1 (docs/CC13_LEFT_1.md): bodies left open by Pass 2. Read from the live decompile
// with the listing checked for register arguments and x87.
// ===========================================================================

// Image routines the bodies below call (each virtual named by its address).
struct TerrainLeftHost {
    virtual ~TerrainLeftHost() = default;
    // 00AEF3C0 BSP_TextureAtlas_SplitNameMatches: ECX const string* requested, EDX atlas item -> AL.
    // (atlas_split_name_matches_00aef3c0 in world_effects_startup models it over std::string.)
    virtual bool atlas_split_name_matches_00aef3c0(const void* requested, void* item) = 0;
    // 00AEE220 is texture_atlas_item_destruct_00aee220 in this file.
    // 00AF5600 BSP_NativeTextBuffer_Construct (ECX 1Ch block) -> EAX; 00AF5620 destroy (ECX);
    // 00AF5850 load file (ECX buffer, const char* path) RET 4.
    virtual void* text_buffer_construct_00af5600(void* block) = 0;
    virtual void text_buffer_destroy_00af5620(void* buffer) = 0;
    virtual void text_buffer_load_file_00af5850(void* buffer, const char* path) = 0;
    // 00BDF4C0 BSP_VFS_ResolveExistingName (ECX vfs, string*) RET 4 -> AL.
    virtual bool vfs_resolve_existing_name_00bdf4c0(void* vfs, void* name) = 0;
    // 00AEEAF0 BSP_TextureAtlas_ParseBuffer (ECX atlas manager, buffer) RET 4.
    virtual void texture_atlas_parse_buffer_00aeeaf0(void* manager, void* buffer) = 0;
    // 004254B0 (cdecl format, ...): the log call; its body in this build is a bare RET.
    virtual void log_004254b0(const char* format, const char* argument) = 0;
    // 00886280 BSP_Vfs_EnumerateResourceList (ECX vfs, list*, directory, const string* ext, 1) RET 10h;
    // 00557A90 BSP_PooledStringList_PopFront (ECX list, string* out) RET 4 -> EAX out;
    // 004D05E0 BSP_PooledStringList_ClearStorage (ECX list).
    virtual void vfs_enumerate_resource_list_00886280(void* vfs, void* list, void* directory, const void* ext,
                                                      std::uint32_t flags) = 0;
    virtual void* pooled_string_list_pop_front_00557a90(void* list, void* out) = 0;
    virtual void pooled_string_list_clear_storage_004d05e0(void* list) = 0;
    // 007375E0 BSP_SingletonHolder_Destruct_00f8c26c (ECX holder).
    virtual void singleton_holder_destruct_007375e0(void* holder) = 0;
    // 00B4C700 BSP_D3D9Renderer_CreateGeneratedModel: ECX name*, EDX geometry*, stack (shader*, a, b, c, d).
    virtual void* create_generated_model_00b4c700(const void* name, const void* geometry, const void* shader,
        std::uint32_t a, std::uint32_t b, std::uint32_t c, std::uint32_t d) = 0;
    // 00B189F0 BSP_Material_SetTextureSlot (ECX material, slot, texture) RET 8.
    virtual void material_set_texture_slot_00b189f0(void* material, std::uint32_t slot, void* texture) = 0;
    // 00AF10A0 BSP_NativeParticleResources_GetIndexStream (ECX resources) -> EAX.
    virtual void* particle_index_stream_00af10a0(void* resources) = 0;
    // 00B73B70 BSP_Geometry_SetIndexStream (ECX geometry, stream) RET 4.
    virtual void geometry_set_index_stream_00b73b70(void* geometry, void* stream) = 0;
    // 00B72270 BSP_Scene_FindNodeByName (ECX scene, const string* name, flags) RET 8 -> EAX.
    virtual void* scene_find_node_by_name_00b72270(void* scene, const void* name, std::uint32_t flags) = 0;
    // Win32 InitializeCriticalSection (import 00CE220C) on an embedded 18h-byte section.
    virtual void initialize_critical_section(void* section) = 0;
    // Draw sections and meshes.
    virtual void* draw_section_create_00533fa0() = 0;
    virtual void draw_section_set_material_00b864c0(void* section, void* material) = 0;       // RET 4
    virtual void draw_section_append_vertex_stream_00b85b80(void* section, void* stream) = 0;  // RET 4
    virtual void draw_section_set_vertex_layout_00b86650(void* section, void* layout) = 0;     // RET 4
    virtual void mesh_append_draw_section_00b73c60(void* mesh, void* section) = 0;             // RET 4
    // 00419440 BSP_Vector3f_Length (ECX const float[3]) -> ST0; 00AD5720 BSP_FoliageSystem_GetScalar0C
    // (ECX system) -> ST0. Both returned as the x87 value before any store.
    virtual double vector3_length_00419440(const float* v) = 0;
    virtual double foliage_system_scalar_0c_00ad5720(void* system) = 0;
    // 00AE0A50 BSP_FoliageEntryNode_DrawBox (ECX cell, DWORD argb, float) RET 8.
    virtual void foliage_cell_draw_box_00ae0a50(void* cell, std::uint32_t argb, float width) = 0;
    // Model assembly (ECX carries the byte size into the raw-slot allocators).
    virtual void* model_allocate_raw_slot_00b74eb0(std::uint32_t size) = 0;
    virtual void* generated_model_construct_00b75030(void* slot, const void* name) = 0;           // RET 4
    virtual void* mesh_allocate_raw_slot_00b73b60(std::uint32_t size) = 0;
    virtual void* mesh_construct_00b73d70(void* slot) = 0;
    virtual void mesh_set_vertex_stream_00b73bb0(void* mesh, std::uint32_t index, void* stream) = 0;  // RET 8
    virtual void generated_model_set_geometry_00b75170(void* model, std::uint32_t a, void* mesh, float x, float y) = 0;
    // Cameras: 00AD5700 BSP_Foliage_GetScalar08 (ECX system) -> ST0; 00B6FC10 BSP_Camera_SetFarPlane (ECX
    // camera, float) RET 4; 00B6FDD0 plane count and 00B70710 plane set (ECX camera); 00B656F0
    // BSP_PlaneSet_GetPlane (ECX set, index) RET 4 -> const float[4]; 00B6DB70 refresh world matrix (ECX node).
    virtual double foliage_scalar_08_00ad5700(void* system) = 0;
    virtual void camera_set_far_plane_00b6fc10(void* camera, float distance) = 0;
    virtual std::int32_t camera_plane_count_00b6fdd0(void* camera) = 0;
    virtual void* camera_plane_set_00b70710(void* camera) = 0;
    virtual const float* plane_set_get_plane_00b656f0(void* set, std::int32_t index) = 0;
    virtual void node_refresh_world_matrix_00b6db70(void* node) = 0;
    // 00AE1440 BSP_Shadow_ReservePointerVector (ECX vector, n) RET 4; Win32 EnterCriticalSection (00CE2218)
    // and LeaveCriticalSection (00CE2210).
    virtual void shadow_reserve_pointer_vector_00ae1440(void* vector, std::int32_t n) = 0;
    virtual void enter_critical_section(void* section) = 0;
    virtual void leave_critical_section(void* section) = 0;
    // RAM images: 00BEFBB0 BSP_RamImage_Construct (ECX 1Ch block, width, height, a, b) RET 10h -> EAX;
    // 00BEFB60 BSP_RamImage_GetData (ECX image) -> EAX bytes; 00BF0050 BSP_RamImage_SaveTgaFile (ECX
    // const string* file, EDX image).
    virtual void* ram_image_construct_00befbb0(void* block, std::int32_t width, std::int32_t height,
                                               std::uint32_t a, std::uint32_t b) = 0;
    virtual std::uint8_t* ram_image_data_00befb60(void* image) = 0;
    virtual void ram_image_save_tga_00bf0050(const void* file, void* image) = 0;
    // 00ADBD00 BSP_TerrainHeightField_ComputeHeightRange (ECX field): refreshes +10h min / +14h max.
    virtual void height_field_compute_range_00adbd00(void* field) = 0;
    // 00B74390 BSP_GeneratedModel_SetLocalBounds (ECX model, const float sphere[4]) RET 4.
    virtual void generated_model_set_local_bounds_00b74390(void* model, const float* bounds) = 0;
};
// One-load getters the bodies below call, inlined: 00B74640 model geometry [+180h] (RET 4, argument
// unused); 00B732C0 element i [[+54h] + 4i]; 00AF1130 particle resources sub-object [+24h].

// Texture atlas registry (the manager published at 00F8C26C): +4h item pointer vector {data, count,
// capacity}, +10h texture pointer vector.
// 00AEFA30 BSP_TextureAtlas_Unload, __thiscall(ECX registry, const string* name) RET 4: walk the
// items with a pointer; for each item 00AEF3C0 matches: release (no clear) and remove every texture
// equal to item +8h, shifting the rest down; destroy (00AEE220) and operator delete (00BF65AC) the
// item when non-null; move the last item into the slot unless it is the last; count--. The pointer
// advances only on a miss; the end is reread every pass.
void texture_atlas_unload_00aefa30(void* registry, const void* name, TerrainHost& host, TerrainLeftHost& left);
// 00AEF280 BSP_TextureAtlas_RegisterDescriptor, __stdcall(const string* name) RET 4: text buffer
// (operator new(1Ch) + 00AF5600, null stays null); a copy of the name; when the VFS (0109CEEC)
// resolves the copy, load the file named by the ORIGINAL name's data (00F8C270, an empty string,
// when null) and parse it into the manager 00F8C26C; otherwise log "Atlas file not found: %s"
// with that path; destroy and free the buffer when non-null; release the copy.
void texture_atlas_register_descriptor_00aef280(const void* name, void* vfs_0109ceec, void* manager_00f8c26c,
    const char* empty_00f8c270, TerrainHost& host, TerrainLeftHost& left);
// 00AEFF40 BSP_TextureAtlas_LoadDirectory, __thiscall(ECX unused, directory) RET 4: enumerate the "ats"
// files of the directory into a 0Ch pooled string list (+4h storage, +8h count); while the count is
// nonzero pop the front into a temporary, register it (00AEF280) and release it; clear the list
// storage (004D05E0) and free +4h (00BF65AC).
void texture_atlas_load_directory_00aeff40(void* directory, void* vfs_0109ceec, void* manager_00f8c26c,
    const char* empty_00f8c270, TerrainHost& host, TerrainLeftHost& left);
// 00AEF090 BSP_TextureAtlasRegistry_Destruct, __fastcall(ECX registry): vtable 00CFEAE8; destroy and
// free every item; release every texture (no null test); then the inline 4-byte vector teardown
// (reserve to 1 when capacity < 0, zero any negative-count tail, count down, free) of +10h, then +4h;
// BSP_SingletonHolder_Destruct_00f8c26c 007375E0.
void texture_atlas_registry_destruct_00aef090(void* registry, TerrainHost& host, TerrainLeftHost& left);
// 00AF0F20 BSP_FoliageGroupManager_CreateFloatingParticles, __fastcall(ECX manager): +24h = 0, +2Ch = 1.0f;
// +24h = 00B4C700("FloatingParticles", "ParticleFloating.mvfm", "ParticleFloating.mshd", 4, 0, 0, 0)
// (temporaries from 0041E870, released name, geometry, shader); the texture of the particle resources
// (00F8C280 -> [+24h]: [+10h] when WORD [+34h] > 0, else null) into slot 0 of element 0's material;
// the resources' index stream (00AF10A0) on the model geometry (00B73B70).
void foliage_group_manager_create_floating_particles_00af0f20(void* manager, void* particle_resources_00f8c280,
    TerrainHost& host, TerrainLeftHost& left);
// 00AE1E10 BSP_BushGroupType_Construct, __thiscall(ECX, landscape, NativeString name_a, NativeString
// name_b, owner) RET 18h -> EAX; the two strings arrive BY VALUE (length, data on the stack) and the
// callee releases them at the end. +4h/+0Ch empty strings, +44h..+4Ch = 0, InitializeCriticalSection
// +54h, +6Ch = 0, +0h = landscape, +1Ch = owner; copy name_a to +4h and name_b to +0Ch; +14h = 0;
// +20h = 00B72270(ECX owner, "BushGroup", 0); +2Ch = +18h = +50h = 0.
void* bush_group_type_construct_00ae1e10(void* type, void* landscape, std::uint32_t name_a_length, void* name_a_data,
    std::uint32_t name_b_length, void* name_b_data, void* owner, TerrainHost& host, TerrainLeftHost& left);
// 00AE1A10 BSP_BushCell_CullAndEmitLod, __thiscall(ECX cell, group, int running) RET 8 -> EAX. The cell:
// +0h centre[3], +0Ch half extent[3], +1Ch..+28h four children, +2Ch first and +30h last vertex, +34h
// "has children". The group: +0h plane count, +4h planes (10h apart), +104h vertex stream, +108h mesh,
// +10Ch far / +110h near material, +114h BYTE debug boxes, +118h eye[3], +124h vertex layout.
// Classify a copy of the box against the planes (00AE05E0, terrain_box_classify_planes_00ae05e0):
// Outside returns running; Intersecting with children recurses into each non-null child, threading
// the sum (the first child gets running and its result is added to running; each later child gets
// the running sum and adds its result); otherwise emit: n = last - first + 1; a draw section with
// the far material when the float distance |centre - eye| (each difference stored as a float,
// length 00419440 stored as a float) exceeds 00AD5720 of the foliage system 00F8C210 as a double
// (unordered picks near), {+8h 4, +0Ch first, +10h n, +14h first, +18h n / 3 unsigned, +34h distance},
// vertex stream, layout, appended to the mesh and released; the debug box (FFFFFFFFh, 0.0f) when
// enabled; returns running + n.
std::int32_t bush_cell_cull_and_emit_lod_00ae1a10(void* cell, void* group, std::int32_t running,
                                                 void* foliage_system_00f8c210, TerrainLeftHost& left);
// 00AE1FB0 BSP_BushGroupModel_BuildBushObject, __thiscall(ECX bush type, camera) RET 4 -> EAX model:
// model "BushOjjekt" (184h slot; null stays null); unless the camera view type +198h is 2 or 7, save the
// camera far plane +1D8h and set 00AD5700 of the foliage system (otherwise the saved value is 0.0f);
// a mesh (0BCh) as the model geometry (0, -1.0f, -1.0f) with vertex stream 0 = type +28h and cleared
// section owners; a 128h-byte cull record on the stack (count, the camera's planes copied by FLD/FSTP
// with d -= 50.0 (double 00CE3938), +104h stream, +108h mesh, +10Ch type +3Ch, +110h type +38h, +114h
// BYTE 0, +118h camera +120h..+128h after a world-matrix refresh unless camera +5Ch bit 1, +124h type
// +50h); +34h = 00AE1A10(type +18h root cell, record, 0); clear the owners again unless BYTE +40h;
// release the mesh; refresh the BushGroup node +20h likewise and pass its +0F0h to the model's vtable
// +34h; under the section +54h (depth +6Ch) append the model to the pointer vector +44h (growth
// max(2 * capacity, 4) through 00AE1440); restore the saved far plane (same view-type test).
void* bush_group_model_build_bush_object_00ae1fb0(void* type, void* camera, void* foliage_system_00f8c210,
    TerrainHost& host, TerrainLeftHost& left);
// 00ADC1C0 BSP_TerrainHeightField_DumpHeightTga, __thiscall(ECX field, const string* file) RET 4: an 8-bit
// RAM image (operator new(1Ch) + 00BEFBB0(32 * cols + 1, 32 * rows + 1, 1, 32h); cols +38h, rows +3Ch,
// k = 32 from .rdata 00D5D658); the field's range (00ADBD00); lo = +10h when > 0 else 0 (COMISS; NaN
// gives 0); range = f(+14h - lo). For each tile of the grid +40h (row-major, cols wide): a null tile
// zeroes its 33 x 33 block; otherwise its block +1Ch is sampled through vtable +8h(&out, u, v) for
// v, u in 0..32, each sample clamped to >= 0 (NaN gives 0), lo re-read and clamped, and the byte is
// CVTTSS2SI(f((sample - lo) / range * 255.0)) (double 00CE4B48) at ((32 row + v) * stride + u + 32 col),
// stride = 32 cols + 1. Then 00BF0050(ECX file, EDX image) and release the image (no null test).
void height_field_dump_height_tga_00adc1c0(void* field, const void* file, TerrainHost& host, TerrainLeftHost& left);
// 00ADB6F0 BSP_TerrainHeightField_StampByteAtWorld, __thiscall(ECX field, float x, float z, float h) RET
// 0Ch; no caller or data xref. inv = f(1 / +1Ch); i = _ftol2(f(x * inv)), j = _ftol2(f(z * inv))
// (00BF7420, truncation); i, j raised to 0 and lowered to vtable +10h / +14h (the count itself, not
// count - 1); split through vtable +44h(&tx, &tz, &lx, &lz, i, j); when the tile +40h[+38h * tz + tx]
// is set: h raised to 0 (NaN gives 0), hf = f(h / 60.0 * 255.0) (doubles 00CE3D68 / 00CE4B48), value
// = CVTTSS2SI(hf when 255.0 > hf, else 255.0f from 00CEDD44; unordered takes 255) as a byte, stamped
// with 00ADB160 (max) on the tile's layer (+1Ch) at (lx, lz), (lx + 1, lz), (lx + 1, lz + 1), (lx, lz + 1).
void height_field_stamp_byte_at_world_00adb6f0(void* field, float x, float z, float h, HeightFieldHost& fields);
// 00ADB980 BSP_TerrainTileGrid_GetOrCreateTile, __thiscall(ECX grid, x, y) RET 8 -> EAX tile: an existing
// grid +40h[+38h * y + x] is returned. Otherwise tile = [00F8C218] vtable +4h(grid, x, y) (00AD9C40,
// RET 0Ch); two 33 x 33 float blocks (operator new(38h) + 00AEDAC0, null stays null) set through
// 00ADF8B0 and 00ADF920 and released (no null test); every sample of both is set to 0.0f through
// vtable +1Ch(0.0f, i, j), with the inlined min/max of those zeros (m from 1.0e10f: m = 0 unless
// 0 > m; M = m when m > 0 else 0); 00ADEB00; tile vtable +8h; model = tile +24h; scales sx, sy, sz =
// model +0F0h, +104h, +118h (world matrix refreshed before each read unless model +5Ch bit 1); box
// min {0/sx, m/sy, 0/sz}, max {g/sx, M/sy, g/sz} with g = grid +18h (x87, each quotient stored as a
// float); centre from 00ADA600; radius = f(g / max(sx, sy, sz)) (sx > sy ? sx : sy, then > sz ? : sz;
// unordered picks the later one); 00B74390(model, {centre, radius}); model vtable +48h; store and
// return the tile.
void* terrain_tile_grid_get_or_create_tile_00adb980(void* grid, std::int32_t x, std::int32_t y,
    HeightFieldHost& fields, TerrainHost& host, BlockHost& blocks, TerrainLeftHost& left);
// ===========================================================================
// cc13-left2: bodies left open by cc13-left1. docs/CC13_LEFT_2.md. Read from the live decompile and
// the full listing (stack slots tracked with local/l2_esp.py; x87 forms checked against bytes).
// ===========================================================================
struct TerrainLeft2Host : TerrainLeftHost {
    // 00AFDA80 BSP_NativeParticleRecord_GetMatrix (ECX emitter) -> EAX const float[16].
    virtual const float* particle_record_matrix_00afda80(void* emitter) = 0;
    // 00B6E680 BSP_Node_SetParent (ECX node, parent) RET 4; 00B6D890 BSP_Node_PropagateRootRegistration
    // (ECX node, root) RET 4.
    virtual void node_set_parent_00b6e680(void* node, void* parent) = 0;
    virtual void node_propagate_root_00b6d890(void* node, void* root) = 0;
    // 00535320 BSP_Material_CreateForEffectName (ECX const string*) -> EAX.
    virtual void* material_create_for_effect_name_00535320(const void* name) = 0;
    // 004C1400 BSP_ResourceManager_GetSingleton -> EAX; 00B80D70 BSP_NativeResourceManager_LoadDefaultFactory
    // (ECX manager, const string* name) RET 4 -> EAX.
    virtual void* resource_manager_singleton_004c1400() = 0;
    virtual void* resource_manager_load_00b80d70(void* manager, const void* name) = 0;
    // 00AE2CB0 BSP_GeneratedModelFinder_Visit (ECX tree type, node) RET 4; 00B732D0
    // BSP_TreeGroupType_ComputeBounds_Provisional (ECX geometry).
    virtual void generated_model_finder_visit_00ae2cb0(void* type, void* node) = 0;
    virtual void geometry_compute_bounds_00b732d0(void* geometry) = 0;
    // 00AE5550 (ECX 58h block, stream, const float rect[4]) RET 8 -> EAX: the tree group's visibility
    // object built from the stream (not reconstructed here).
    virtual void* tree_visibility_construct_00ae5550(void* block, void* stream, const float* rect) = 0;
};

// 00AFDCF0 BSP_NativeParticleEmitter_UpdateTransform (the name is the snapshot's; the body draws debug
// axes), __thiscall(ECX emitter, unused DWORD) RET 4. Only when BYTE [[emitter +0A0h] +7Ch] is set. With
// node = emitter +0A4h: origin = 004142E0(emitter +0h, node world +0F0h) when BYTE node +1B0h is set (the
// world refreshed by 00B6DB70 unless node +5Ch bit 1), else emitter +0h..+8h. M = 00413920(00AFDA80(emitter),
// node world) under the same test, else 00AFDA80(emitter), copied (004134F0). Each unit axis X, Y, Z is
// transformed by M (004142E0) less M's translation (+30h..+38h); device (00F8D394) vtable +0B4h(from,
// to, colour, colour) RET 10h then draws origin -> origin + axis for X in FFFF0000h, Z in FF0000FFh and Y in
// FF00FF00h, in that order. The x87 differences and sums are each stored as a float (listing 00AFDE16..00AFDFE5).
void particle_emitter_draw_debug_axes_00afdcf0(void* emitter, TerrainHost& host, TerrainLeft2Host& left);

// 00AE5160 BSP_TreeGroupType_Construct, __thiscall(ECX type, const string* name) RET 4 -> EAX. Caller
// 00AD7030. +0h = +4h = 0; +18h = -1.0e10f (00CE4ADC), +1Ch = 1.0e10f; strings +28h, +30h zeroed, +38h =
// copy of name; +0h = 00B80D70(004C1400(), name); +4h = (+0h) vtable +8h(0, 1.0f); 00AE2CB0(type, +4h
// +0Ch) (sets the model +8h). With geometry = [model +180h] (00B74640, re-read every use): when
// [geometry +50h] is set, +20h / +24h = the first slot's (geometry +10h) +8h / +0Ch, else 0 and
// [geometry +58h] - 1. +30h = the name (+8h) of element(+20h) +20h's first texture ([+10h] when WORD +34h > 0,
// else a null texture, whose name address is 8); +28h = the effect name (+0B8h) of that element's
// material +7Ch; +0Ch / +10h sum elements +20h..+24h's +10h / +18h; 00B732D0(geometry); +14h = float of
// bounds +10h - bounds +4h (geometry +98h); +40h = 00535320(+28h); texture = device +64h(+30h text or ""
// (00F8C260, loader zero-fill), 0) in slot 0 of +40h; +44h = 0; texture released when non-null.
void* tree_group_type_construct_00ae5160(void* type, const void* name, TerrainHost& host, TerrainLeft2Host& left);

// 00AE5870 BSP_TreeGroup_Initialise, __thiscall(ECX group, stream, count) RET 8. BYTE +38h = 1; +8h =
// count; declaration "ccuf44uf43.mvfm" (device +38h); +1Ch = device +5Ch(count, 1, decl), locked (+10h
// with count, 0, 0), filled by stream vtable +24h(data, count * 20h, 0) and unlocked; d = (+20h) vtable
// +24h(). When +18h is null: model "TreeGroup" at +18h (+138h &= ~3), parent = (+0h) vtable +40h(),
// root = that node's +0A4h; a mesh at +28h with geometry (0, mesh, -1.0f, -1.0f); +34h = material
// "instanced.mshd" / "coral.mshd" / "seaweed.mshd" for kind (+0Ch) 0 / 1 / 2 (none otherwise); +24h =
// device +40h({d, decl, count 2}); stream flags +20h +54h = 40000000h, +1Ch +54h = 80000000h; mesh
// streams 0 = +20h, 1 = +1Ch; index stream = [type +8h]'s geometry +60h; slot 0 of +34h = the type
// material's (+4h +40h) first texture when WORD +34h > 0, else null. Then the owner's box (vtable +0Ch)
// less the owner node's world translation (vtable +40h, refreshed unless +5Ch bit 1, read four times:
// +128h, +120h, +128h, +120h) gives {min.x, min.z, max.x, max.z}; +10h = 00AE5550(new 58h, stream, rect)
// (null when the allocation fails); the declaration released when non-null.
void tree_group_initialise_00ae5870(void* group, void* stream, std::uint32_t count, TerrainHost& host,
                                    TerrainLeft2Host& left);
// 00ADE0E0 BSP_TerrainVisibilityGroup_BuildFlat, __thiscall(ECX group, const string* name) RET 4. No caller
// or data reference. +28h = 0; name copied into +54h unless the same; +2Ch = node "Terrain Visibility Group"
// (00B6ED70 slot of 174h, 00B6F5A0), its root +0Ch (00B6D890); grid +38h resized to 4 x 4 (00ADACF0); then
// for y, x in 0..3 the 00ADB980 tile body without the existing-tile test, except that only the first
// block's samples are set (one vtable +1Ch(0.0f, i, j) per sample; listing 00ADE2E2..00ADE323): tile =
// [00F8C218] vtable +4h(group, x, y), stored at +40h[+38h * y + x] after its bounds. The box quotients are
// the same x87 sequence (DE F2 / DE FA / DE FB / DE F3 / DE F2 checked against bytes).
struct TerrainLeft2NodeCalls {
    void* (*node_allocate_raw_slot_00b6ed70)(std::uint32_t size);           // ECX size
    void* (*node_construct_00b6f5a0)(void* slot, const void* name);          // RET 4
};
void terrain_visibility_group_build_flat_00ade0e0(void* group, const void* name, HeightFieldHost& fields,
    TerrainHost& host, BlockHost& blocks, TerrainLeft2Host& left, const TerrainLeft2NodeCalls& nodes);

// 00AE2450 BSP_BushGroup_Initialise, __thiscall(ECX group, stream, count, NativeString a, NativeString b)
// RET 18h; the two strings arrive by value and the callee releases them at the end. BYTE +40h = 1; +14h =
// count; a and b are assigned into +4h and +0Ch (resize keeping, copy; no self test needed); the normal
// names a[0 .. len-4) + "_normal.tga" and b[...] + "_normal.tga" (00469840, 004261A0) are kept as
// temporaries; declaration "pf44nss2uf42uf44cc.mvfm"; +28h = device +5Ch(6 * count, 1, decl), locked
// (+10h with 6 * count, 0, 0), filled by stream vtable +24h(data, count * 120h, 0) and unlocked; +50h =
// device +40h({decl, count 1}); the declaration released when non-null. When +20h is null: +20h = model
// "BushGroup" from the pool at 00F8C228 (00AE1870, then 00B75030 and vtable 00D5D4A0), +138h &= ~3, parent
// and root from the owner node (vtable +40h), model +184h = group; +3Ch = "bush_lores.mshd", +38h =
// "bush.mshd" (00535320); atlas 00AEFB20 lookups (results unused) of b, a, b's normal and a's normal, each
// "" (00F8C224, zero-fill) when null; +3Ch slots 0 / 1 / 2 = 00AD57D0 / 00AD57E0 of [00F8C210] and
// 00B0D140 of [00F8D39C]; +38h slots 0 / 1 the first two. Then, on every path, the owner's box (vtable
// +0Ch) gives centre = float(float(min + max) * 0.5) and half = float(float(max - min) * 0.5) per axis
// (x87 checked: DC C9, DE E6 / E1 / E2); +18h = 00AE0F10(new 38h, stream, &centre, &half, owner node +124h)
// (null when the allocation fails). The temporaries are released, normal b then normal a, then a, b.
} // namespace bsp::cc13_terrain

namespace bsp::cc13_terrain {
// oct18b-nC additions; native Win32 layouts. 00AFD860 / 00B05210:
// ECX=container, stack named-list-array destination, RET 4. Copy each native
// name, attach float(age/lifetime*100) as its raw dword, and transfer the name
// to 00AFF290 (which consumes it). Reload live counts/pointers each iteration.
void emitter_set_collect_percentages_00afd860(void* set, void* destination, TerrainHost& host);
void particle_group_collect_percentages_00b05210(void* group, void* destination, TerrainHost& host);
// ECX=array; no stack args, RET. 00AFF210(0), retaining allocation.
void named_list_array_clear_00aff410(void* array, TerrainHost& host);
// ECX=emitter; stack destination; RET 4. Tail-dispatch to group at +10h if set.
void emitter_collect_percentages_00aff580(void* emitter, void* destination, TerrainHost& host);
// ECX=renderer; RET; sum each +28h resource's virtual +0Ch result over the
// captured 2Ch-stride range at renderer+1A78h/+1A7Ch. Interpretation provisional.
using ResourceWord = std::uint32_t (*)(void* resource);
std::uint32_t renderer_sum_resource_words_00b23d40(void* renderer, ResourceWord slot0c);
// Native tail thunks: ECX=texture, original stack unchanged; JMP vtable+3Ch/+40h.
// Stack arguments/return type belong to the destination and remain unresolved.
// These naked entry points must be called only with that original register ABI.
void texture_tail_slot3c_00b33ea0();
void texture_tail_slot40_00b33eb0();
} // namespace bsp::cc13_terrain
namespace bsp {
class NativeStringStorage;
}
namespace bsp::cc13_terrain {
struct TerrainLeft2BushCalls {
    void* (*bush_model_pool_allocate_00ae1870)(void* pool_00f8c228);
    void* (*atlas_find_item_00aefb20)(void* registry, const char* name);       // ECX [00F8C26C]
    void* (*bush_texture_00ad57d0)(void* foliage);                             // ECX [00F8C210]
    void* (*bush_normal_texture_00ad57e0)(void* foliage);
    void* (*shadow_map_texture_00b0d140)(void* shadow_owner);                  // ECX [00F8D39C]
    void* (*bush_quad_node_read_00ae0f10)(void* node, void* stream, const float* centre, const float* half, float y);
};
void bush_group_initialise_00ae2450(void* group, void* stream, std::uint32_t count, void* name_a,
    void* name_b, NativeStringStorage& strings, TerrainHost& host, TerrainLeft2Host& left,
    const TerrainLeft2BushCalls& bush);
} // namespace bsp::cc13_terrain
