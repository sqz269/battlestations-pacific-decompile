#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

// Partition segments 84-86 (00B25DC0..00B65AC0): the renderer object helpers, texture
// and surface wrappers, shader builder, mshd tree lifetime, slab pools, distortion,
// luminance, bloom and old-film post effects (packet cc13-render1). Evidence, original
// ABIs and uncertainty: docs/CC13_RENDER_1.md. Names are hypotheses, not recovered
// symbols.
//
// These are new C++ interfaces over host data, not drop-in binary replacements. Image
// routines that these bodies call are reached through the abstract service interfaces
// below, each virtual named by the address it stands for; nothing is stubbed. The
// native containers allocate with operator new (00BF55BE) and release with _free
// (00BF6989); the models pair ::operator new with ::operator delete and copy elements
// bitwise, as the image does.
//
// RNG: nothing in this file draws from the shared gameplay generator 00BD2F10. The
// old-film effect (00B529A0) DOES draw from stream 0 in the image; its model takes the
// draws from a caller-supplied source and is marked below. Nothing here is wired into a
// host.

namespace bsp::cc13_render1 {

// ---------------------------------------------------------------------------
// Engine arrays: {+00 data, +04 count, +08 capacity}, signed count and capacity. The
// element stride is a parameter because one routine shape is instantiated for many
// element types; each table row records the stride of its instantiation.
struct RawArray {
    std::uint8_t* data = nullptr;
    std::int32_t count = 0;
    std::int32_t capacity = 0;
};

using ArrayResizeFn = std::function<void(RawArray&, std::int32_t)>;
using ElementFn = std::function<void(std::uint8_t* element)>;
using ElementAssignFn = std::function<void(std::uint8_t* destination, const std::uint8_t* source)>;

// Requested-capacity rules of the push-back shapes (count == capacity triggers them).
enum class CapacityRule : std::uint8_t {
    kDoubleMinOne,     // c*2, 1 when below 2 (most push-backs)
    kPlusFiveMinTen,   // c+5, 10 when below 11 (00B34970, 00B564D0; 00B34740 is the rule alone)
    kPlusFourMinSix,   // c+4, 6 when below 7 (00B41050; 00B40D60 is the rule alone)
    kPlusEightMin16,   // c+8, 16 when below 17 (00B34750 is the rule alone; no user in range)
    kDoubleMin256,     // c*2, 256 when below 257 (00B51C20)
};
std::int32_t apply_capacity_rule(CapacityRule rule, std::int32_t capacity) noexcept;

// "ReallocateExact" (00B3DA70 and eight identical bodies): capacity = n is stored FIRST,
// a buffer of n elements is allocated, the count live elements are copied (unsigned
// loop), the old buffer is freed when non-null and the new one published.
void array_reallocate_exact(RawArray& a, std::int32_t n, std::size_t stride);
// "Grow" (00B3DAC0 and eight identical bodies): reallocate_exact(capacity*2 + 2).
void array_grow_double_plus_two(RawArray& a, std::size_t stride);
// "ReserveAtLeast" (00B3E880 and eight identical bodies): reallocate_exact(n) only when
// the capacity, compared UNSIGNED, is below n.
void array_reserve_at_least(RawArray& a, std::uint32_t n, std::size_t stride);
// "PushBackGrow" (00B3E800, 00B3E8E0 and seven identical bodies): grow inline when
// count == capacity, then copy the element to data[count] and increment count.
void array_push_back_grow(RawArray& a, const void* value, std::size_t stride);
// "PushBack through a named reserve": when count == capacity call the image reserve with
// the rule's capacity, then copy and increment.
void array_push_back_via_reserve(RawArray& a, const void* value, std::size_t stride,
    CapacityRule rule, const ArrayResizeFn& reserve);
// "ReserveMinOne" (00B56200 and three identical bodies): n clamped to at least 1; grows
// when capacity < n (signed); the old buffer is freed unconditionally and capacity is
// stored LAST.
void array_reserve_min_one(RawArray& a, std::int32_t n, std::size_t stride);
// "Resize": reserve(n) when capacity < n, fill the new elements, count = n. The fill is
// zero for the pointer arrays (00B34760 and five identical bodies), FFh bytes for the
// interpolator words (00B34A50) and absent for the state arrays (00B40D70 and three
// identical bodies, misnamed PushBack in the ledger).
enum class ResizeFill : std::uint8_t { kNone, kZero, kOnes };
void array_resize(RawArray& a, std::int32_t n, std::size_t stride, ResizeFill fill,
    const ArrayResizeFn& reserve);
// "Clear by count-down" (00B28110 and seven identical bodies; 00B29230 on renderer
// +1CF4h): reserve(0) when the capacity is NEGATIVE (never in practice), count stepped
// down to 0 (POD elements, nothing destroyed).
void array_clear_count_down(RawArray& a, const ArrayResizeFn& reserve);
// 00B41EF0 / 00B41F40: the count-down clear followed by _free(data).
void array_destroy_count_down(RawArray& a, const ArrayResizeFn& reserve);
// "Clear" (00B25E50 and sixteen identical bodies): the element-typed resize with 0.
void array_clear_via_resize(RawArray& a, const ArrayResizeFn& resize);
// "Destroy" (00B28350 and thirteen identical bodies): resize(0), then _free(data); the
// data, count and capacity words are left as they were.
void array_destroy_via_resize(RawArray& a, const ArrayResizeFn& resize);
// Element address: data + i*stride (no bounds check). "End": data + count*stride.
std::uint8_t* array_at(const RawArray& a, std::int32_t index, std::size_t stride) noexcept;
std::uint8_t* array_end(const RawArray& a, std::size_t stride) noexcept;
// 00B2C8A0: resize(index + 1) when count <= index, then the element address (stride 0Ch).
std::uint8_t* array_at_growing(RawArray& a, std::int32_t index, std::size_t stride,
    const ArrayResizeFn& resize);
// "Construct" (00B345A0 and nineteen identical bodies): the three words zeroed.
void array_construct_empty(RawArray& a) noexcept;
// "PopBack" (00B30210, 00B30230, 00B30250; stride 2Ch): when count != 0 destroy the
// last element through the named routine and decrement count.
void array_pop_back_destroy(RawArray& a, std::size_t stride, const ElementFn& destroy);
// 00B315E0: erase `where` by assigning the last element over it (skipped when `where` is
// the last), destroying the last element's storage and decrementing count.
void array_erase_swap_last(RawArray& a, std::uint8_t* where, std::size_t stride,
    const ElementAssignFn& assign, const ElementFn& destroy_storage);
// 00B4A970: pointer array; index -1 is a no-op; otherwise the last cell moves into the
// slot (unless it is the last) and count decrements.
void array_remove_at_swap_last(RawArray& a, std::int32_t index) noexcept;
// "FreeData": _free(data) when non-null. 00B47820 and four identical bodies leave the
// word; 00B4D180 and 00B61DA0 clear it.
void array_free_data(RawArray& a, bool clear_pointer) noexcept;

// Inline fixed arrays (element storage first, count word after it): 00B40880 pushes a
// dword with the count at +80h, 00B40920 with the count at +10h; 00B408A0 returns
// base + count*4 (count at +80h); 00B40900 / 00B40910 / 00B40940 / 00B47940 return
// base + i*stride.
void inline_array_push_dword(std::uint8_t* base, std::size_t count_offset, std::uint32_t value) noexcept;
std::uint8_t* inline_array_end_dword(std::uint8_t* base, std::size_t count_offset) noexcept;

// ---------------------------------------------------------------------------
// 32-slot slabs. Every slab pool in segment 85 lays one slab out as 32 slots of S bytes,
// the slab index in each slot's last dword (+S-4), a u16 free-index stack at 32*S and
// its u16 count at 32*S + 40h. Slot sizes: D3D9 surface 38h, D3D9 2D texture 54h, cube
// texture 34h, material pass 8Ch, vertex declaration D4h, hardware layout 44h, physical
// buffer 30h, logical vertex stream 78h, logical index stream 28h.
enum class SlabKind : std::uint8_t {
    kSurface, kTexture2D, kCubeTexture, kMaterialPass, kVertexDeclaration,
    kHardwareLayout, kPhysicalBuffer, kVertexStream, kIndexStream,
};
std::size_t slab_slot_size(SlabKind kind) noexcept;
// PopFreeSlot: --count; return slab + stack[count]*S.
std::uint8_t* slab_pop_free_slot(std::uint8_t* slab, std::size_t slot_size) noexcept;
// PushFreeSlot: stack[count] = (slot - slab)/S (signed division), ++count.
void slab_push_free_slot(std::uint8_t* slab, std::size_t slot_size, const std::uint8_t* slot) noexcept;
bool slab_has_no_free_slot(const std::uint8_t* slab, std::size_t slot_size) noexcept;   // count == 0
bool slab_all_slots_free(const std::uint8_t* slab, std::size_t slot_size) noexcept;     // count == 32
// StampSlabIndex: every slot's last dword = index.
void slab_stamp_index(std::uint8_t* slab, std::size_t slot_size, std::uint32_t index) noexcept;

// ---------------------------------------------------------------------------
// Reference counting. Engine ref-counted objects keep the count at +04h; reaching zero
// calls vtable slot 0 with ECX = object and no stack argument.
struct RefCounted {
    virtual void destroy_slot0() = 0;   // vtable +00h
    std::int32_t refcount = 0;          // +04h (InterlockedIncrement/Decrement)
protected:
    ~RefCounted() = default;
};
// "RefPtr assign" (00B3C580 and seven identical bodies): no-op when unchanged; store,
// retain the new one, release the old one. Returns the slot.
RefCounted** ref_slot_assign(RefCounted** slot, RefCounted* const* source);
// 00B483A0: the same on word 0 of a 0Ch record, then words 1 and 2 copied.
void ref_record_assign_00b483a0(std::uint32_t* destination, const std::uint32_t* source);
// "RefPtr release" (00B3EAA0 and seven identical bodies; 00B627C0 on the word at +04h):
// release *slot when non-null and clear it. Returns the slot.
RefCounted** ref_slot_release(RefCounted** slot);
// 00B31DA0 (cdecl): release one reference of `object` (never null at the call sites).
void ref_release_cdecl_00b31da0(RefCounted* object);

// COM objects (IUnknown order: +00h QueryInterface, +04h AddRef, +08h Release).
struct ComObject {
    virtual long __stdcall query_interface(const void* iid, void** out) = 0;
    virtual unsigned long __stdcall add_ref() = 0;
    virtual unsigned long __stdcall release() = 0;
protected:
    ~ComObject() = default;
};
// "ComPtr assign" (00B3D470, 00B490F0, 00B49120): no-op when unchanged; store, AddRef the
// new one, Release the old one.
ComObject** com_slot_assign(ComObject** slot, ComObject* const* source);
// "ComPtr release" (00B3D450 and six identical bodies): Release when non-null, clear.
ComObject** com_slot_release(ComObject** slot);
// "RefCount probe" (00B3D420, 00B5E650, 00B5E6D0, 00B60070): 0 for null, otherwise
// AddRef() - 1 after a balancing Release (the AddRef return value is the count).
std::int32_t com_reference_count(ComObject* object);

// Owned objects deleted through vtable slot 0 with flags 1 (00B34530 and five identical
// bodies): call and clear when non-null.
struct OwnedVirtual {
    virtual void* deleting_destructor_slot0(std::uint32_t flags) = 0;  // vtable +00h
protected:
    ~OwnedVirtual() = default;
};
void owned_slot_delete(OwnedVirtual** slot);

// Pooled string storage {+00 length, +04 chars}: the chars block (length + 1 bytes) goes
// back to the sized storage pool through 00419CC0 (returns the pool for the size in
// ECX) and 00BD1510 (thiscall return).
class StoragePoolHost {
public:
    virtual ~StoragePoolHost() = default;
    // Stack (chars, size, 1) to 00419CC0, then 00BD1510 with ECX = the returned pool.
    virtual void return_block_00419cc0_00bd1510(void* chars, std::uint32_t size) = 0;
};
struct PooledString {
    std::uint32_t length = 0;
    char* chars = nullptr;
};
// 00B2ABB0 and three identical bodies on the string at +00h, 00B56AF0 at +04h, 00B34CC0
// at +14h: return the block when chars is non-null; the words are not cleared.
void pooled_string_release(PooledString& s, StoragePoolHost& pool);
// 00B35270: the same guarded by length != 0, then both words cleared.
void pooled_string_reset_00b35270(PooledString& s, StoragePoolHost& pool);

// Red-black tree header {+00 allocator byte, +04 head, +08 size}; the head sentinel is
// allocated by the instantiation's AllocateSentinel and gets isnil (+25h) = 1 and
// left/parent/right (+00/+04/+08) = itself.
struct TreeHeader {
    std::uint32_t allocator = 0;
    std::uint8_t* head = nullptr;
    std::uint32_t size = 0;
};
using SentinelAllocateFn = std::function<std::uint8_t*()>;
// 00B28310 (void) and 00B29CA0 / 00B2F8E0 (returns this).
TreeHeader* tree_initialize_empty(TreeHeader& tree, const SentinelAllocateFn& allocate_sentinel);
// 00B2F630 and two identical bodies: erase [head->left, head) through the instantiation's
// EraseRange, free the head, zero the head and size words.
using TreeEraseRangeFn = std::function<void(TreeHeader&, std::uint8_t* first, std::uint8_t* last)>;
void tree_destruct(TreeHeader& tree, const TreeEraseRangeFn& erase_range);

// ---------------------------------------------------------------------------
// Per-address table. One row per instantiation covered by the shared shapes above; the
// source file lists every row with its stride or offset.
enum class Shape : std::uint8_t {
    kClearViaResize, kDestroyViaResize, kClearCountDown, kDestroyCountDown,
    kReallocateExact, kGrow, kReserveAtLeast, kPushBackGrow, kPushBackViaReserve,
    kReserveMinOne, kResize, kAt, kAtGrowing, kEnd, kConstructEmpty,
    kPopBackDestroy, kFreeData, kFreeDataAndClear, kRemoveAtSwapLast,
    kInlinePush, kInlineEnd, kInlineAt, kCapacityRule,
    kSlabPop, kSlabPush, kSlabHasNoFree, kSlabAllFree, kSlabStamp,
    kRefAssign, kRefRelease, kRefFieldAssign, kRefFieldCopy, kComAssign, kComRelease,
    kComRefCount, kOwnedDelete, kPooledStringRelease, kTreeInitEmpty, kTreeDestruct,
    kGetFloat, kPointerDelete,
};
struct RoutineRow {
    std::uint32_t address;
    Shape shape;
    std::uint16_t stride_or_offset;  // element stride, field offset or slab slot size
    std::uint16_t extra;             // capacity rule, fill, lock offset or count offset
    const char* note;                // image element type or callee
};
// Returns nullptr when `address` is not a table row.
const RoutineRow* find_routine_row(std::uint32_t address) noexcept;
std::size_t routine_row_count() noexcept;

// ---------------------------------------------------------------------------
// Packet cc13-rendershapes: the leaf accessors that were table rows, one function per
// address. The object is raw storage at `self` (the image's ECX). Each comment gives the
// original ABI read from the listing; the C++ signature is the new interface.

// Record addressing, __thiscall(ECX self, index), RET 4, EAX = address. Index arithmetic
// is 32-bit and wraps as the SHL/LEA forms do.
std::uint8_t* float4_block_at_00b405d0(std::uint8_t* self, std::uint32_t index);       // self+4+16i
std::uint8_t* shader_effect_pass_record_00b40050(std::uint8_t* self, std::uint32_t index);  // self+144h+40h*i
std::uint8_t* vertex_declaration_element_at_00b47c80(const std::uint8_t* self,
                                                     std::uint32_t index);  // [self+0Ch]+14h*i

// __thiscall(ECX self), RET, EAX = self: dwords +00h and +04h zeroed.
void* pooled_string_construct_empty_00b34d60(void* self);
void* pod_pair_zero_init_00b4ec30(void* self);

// __thiscall(ECX self), RET, EAX = the dword at the offset.
std::uint32_t d3d9_surface_sample_count_00b3cd30(const std::uint8_t* self);      // +24h
std::uint32_t cube_texture_reported_width_00b3cf70(const std::uint8_t* self);    // +24h
std::uint32_t cube_texture_reported_height_00b3cf80(const std::uint8_t* self);   // +28h
std::uint32_t logical_vertex_stream_flags_00b48cc0(const std::uint8_t* self);    // +60h
std::uint32_t logical_index_stream_flags_00b48d80(const std::uint8_t* self);     // +10h
std::uint32_t logical_index_stream_count_00b48d90(const std::uint8_t* self);     // +14h
std::uint32_t logical_index_stream_format_00b48da0(const std::uint8_t* self);    // +18h
// __thiscall(ECX self, value), RET 4: +24h = value.
void d3d9_surface_set_sample_count_00b3ce20(std::uint8_t* self, std::uint32_t value);

// __thiscall(ECX self), RET. 00B3FF70 is MOVZX (EAX zero-extended); 00B400B0 is MOV AL,
// leaving EAX bits 8-31 as they were on entry. Both return the byte in AL.
std::uint8_t shader_reflection_owner_byte74_00b3ff70(const std::uint8_t* self);
std::uint8_t material_effect_byte13c_00b400b0(const std::uint8_t* self);

// __thiscall(ECX self), RET. Lock pointer at +48h (dwords) or +4Ch (bytes); listing order
// in each body below.
std::uint32_t locked_object_field40_00b5df90(const std::uint8_t* self);
std::uint32_t locked_object_field3c_00b5dfc0(const std::uint8_t* self);
std::uint8_t locked_object_byte44_00b5dff0(const std::uint8_t* self);
std::uint8_t locked_object_byte45_00b5e020(const std::uint8_t* self);

// A bare RET: vtable slots with no stack arguments.
void physical_index_buffer_noop_slot10_00b4b480(void* self);
void physical_vertex_buffer_noop_slot10_00b4b520(void* self);
void post_effect_owner_noop_slot0c_00b4ec40(void* self);
void bloom_owner_noop_slot0c_00b54cc0(void* self);

// __thiscall(ECX self), RET, EAX = a constant.
std::uint32_t d3d9_texture2d_kind_00b3ce40(const void* self);   // 0
std::uint32_t cube_texture_kind_00b3cf60(const void* self);     // 1
std::uint32_t volume_texture_kind_00b3d080(const void* self);   // 2

// Forwards to a pool routine on an image global (passed in, as the file passes every
// image global). __thiscall(ECX self), RET: PUSH ECX, MOV ECX pool, CALL the callee,
// which pops its stack argument (RET 4).
void vertex_declaration_pool_return_raw_slot_unwind_00b47c00(void* pool_0108fd38, void* self);
void logical_vertex_pool_return_slot_unwind_00b49960(void* pool_0108fe18, void* self);
void logical_index_stream_pool_return_slot_unwind_00b49970(void* pool_0108fe50, void* self);
}  // namespace bsp::cc13_render1
namespace bsp::cc13_render_sprites { class SlabPoolHost; }
namespace bsp::cc13_render1 {
void hardware_layout_return_raw_slot_00b47c10(std::uint8_t* pool_0108fd70, void* self,
                                              cc13_render_sprites::SlabPoolHost& host);
// 00B488D0: MOV ECX 0108FD70, JMP 00B48780; the caller's ECX is not read.
void* hardware_layout_pool_allocate_raw_slot_stub_00b488d0(
    std::uint8_t* pool_0108fd70, cc13_render_sprites::SlabPoolHost& host);

// ===========================================================================
// Segment 84/85: renderer object helpers and resource wrappers. Native objects are
// addressed as raw storage (`std::uint8_t*` plus the byte offsets the image uses).

// 00B26100, __fastcall(ECX guard), RET: vtable 00D5E60C, the owned critical section at
// +04h destroyed (0041CC80, ECX = guard+4), global [0108D5A0] = 0, vtable 00CE3818
// (singleton base). The image global is passed by reference.
class RecordGuardHost {
public:
    virtual ~RecordGuardHost() = default;
    virtual void destroy_owned_critical_section_0041cc80(std::uint8_t* section_holder) = 0;
};
void record_guard_destruct_00b26100(std::uint8_t* guard, std::uint32_t& guard_singleton_0108d5a0,
    RecordGuardHost& host);

// 00B264A0, __thiscall(ECX renderer, name), RET 4: declaration = renderer vt+38h(name)
// (returns a retained reference); a 14h stack record {+00 declaration, +10h 1} (words
// +04..+0C are NOT written: the image passes stack garbage there) goes to [00F8D394]
// vt+40h, whose result is returned; the declaration reference is then released.
class LayoutLookupHost {
public:
    virtual ~LayoutLookupHost() = default;
    virtual RefCounted* renderer_declaration_for_name_vt38(void* renderer, const void* name) = 0;
    // `record` points at the 14h record; words 1..3 carry whatever the caller had there.
    virtual void* singleton_layout_for_record_vt40(const std::uint32_t record[5]) = 0;
};
void* hardware_layout_for_declaration_name_00b264a0(void* renderer, const void* name,
    const std::uint32_t stale_words[3], LayoutLookupHost& host);

// 00B27A40, __fastcall: +A8h = 0, then 00B24160 (ECX = this), returns this.
// 00B27A60: sampler record {+00, +04, +08 dwords, +0C word} zeroed.
// 00B27F20: twenty records of stride 48h each reset as 00B27A60 (loop counter 13h..0).
std::uint8_t* binding_state_construct_00b27a40(std::uint8_t* state, const std::function<void(std::uint8_t*)>& init_00b24160);
void sampler_record_reset_00b27a60(std::uint8_t* record) noexcept;
void sampler_cache_reset_00b27f20(std::uint8_t* records) noexcept;

// 00B28290, __thiscall(ECX iterator, out, unused), RET 8: post-increment of a two-word
// tree iterator: *out = *this, then 00B20DC0 advances THIS; returns out.
struct TreeIterator { std::uint8_t* node; void* tree; };
TreeIterator* tree_iterator_post_increment_00b28290(TreeIterator& it, TreeIterator* out,
    const std::function<void(TreeIterator&)>& advance_00b20dc0);

// 00B283D0, __thiscall(ECX this, src), RET 4: the three words zeroed, then the image
// copy-assign 00B25E60 with src; returns this.
// 00B29D10, __thiscall(ECX this, flags), RET 4: resize(0), _free(data), _free(this)
// when flags & 1; returns this. (The array object itself is heap-allocated there.)
RawArray* records_copy_construct_00b283d0(RawArray& a, const RawArray& source,
    const std::function<void(RawArray&, const RawArray&)>& copy_assign);
void records_scalar_delete_00b29d10(RawArray* a, std::uint32_t flags, const ArrayResizeFn& resize);

// 00B2B270, __thiscall(ECX renderer, camera), RET 4: debug line records (16 bytes each:
// x, y, z, colour) at renderer +1CF4h {data, +1CF8h count, +1CFCh capacity}. When the
// count is non-zero: lazily create the generated model +19E0h through 00B4C700 with the
// names "debugshader.mshd", "pf43cc.mvfm", "DebugLines" and (kind 2, 0, 0, 0), publish it
// BEFORE the three temporary names are returned, and give it an identity local
// transform (model vt+38h with a 4x4 identity built from [00D7A24C] = 1.0f). Then: lock
// geometry(0) vertex stream 0 for `count` vertices (stream vt+10h(count, 0, 0)), copy
// count*16 bytes, unlock (vt+14h); element(0) +18h = count/2 (lines, signed division),
// +10h = count; 00B46A70 (ECX 0, EDX camera); a render entry from the shared pool
// [0108FE88] initialised by 00B51A20 (0.0, element, geometry, model, camera, 1.0, 0.0, 0)
// is submitted to element+20h ->+7Ch ->+9Ch vt+08h; finally the count-down clear.
class DebugLinesHost {
public:
    virtual ~DebugLinesHost() = default;
    virtual void* create_generated_model_00b4c700(const char* shader, const char* vertex_format,
        const char* name, std::uint32_t kind, std::uint32_t a, std::uint32_t b, std::uint32_t c) = 0;
    virtual void model_set_local_transform_vt38(void* model, const float identity[16]) = 0;
    virtual void* model_geometry_00b74640(void* model, std::uint32_t index) = 0;
    virtual void* geometry_vertex_stream_00b73260(void* geometry, std::uint32_t index) = 0;
    virtual void* geometry_element_00b732c0(void* geometry, std::uint32_t index) = 0;
    virtual void* stream_lock_vt10(void* stream, std::uint32_t count, std::uint32_t a, std::uint32_t b) = 0;
    virtual void stream_unlock_vt14(void* stream) = 0;
    virtual void gather_system_constants_00b46a70(void* zero_ecx, void* camera) = 0;
    // InterlockedIncrement([0108FE88]+8) = n, entry = [[0108FE88]+4] + (n-1)*28h.
    virtual void* allocate_render_entry_0108fe88() = 0;
    virtual void render_entry_initialize_00b51a20(void* entry, float sort_bias, void* element,
        void* geometry, void* model, void* camera, float visibility, float depth_override,
        std::uint32_t flags) = 0;
    virtual void submit_render_entry(void* element, void* entry) = 0;  // element+20h->+7Ch->+9Ch vt+08h
};
void draw_debug_line_records_00b2b270(std::uint8_t* renderer, void* camera, DebugLinesHost& host,
    const ArrayResizeFn& reserve_00b228b0);

// 00B2FA90 / 00B2FAF0 / 00B2FB50, __fastcall, returns this: owner {+00 0, +04 0, +08
// allocator (untouched), +0Ch list head from the instantiation's AllocateSentinel,
// +10h list size 0, +14h..+24h zeroed}.
std::uint8_t* pooled_string_list_owner_construct(std::uint8_t* owner, const SentinelAllocateFn& allocate_sentinel);

// 00B31AC0, __stdcall(const NativeString* name, word), RET 8 (renderer vtable slot
// +68h; the renderer pointer is not used). With [0108D6DC] set the optional guard of
// [00F8D394] is entered (00B33AD0) and left afterwards (00B33B00); the name
// is copied, lower-cased (004BCC00) and looked up through 00B30B40 (cache load)
// (name, word, 0, 0): the last argument 0 is "do not load" (the +64h twin passes 1).
class TextureCacheHost {
public:
    virtual ~TextureCacheHost() = default;
    virtual bool guard_enabled_0108d6dc() = 0;
    virtual std::uint8_t enter_optional_guard() = 0;
    virtual void leave_optional_guard(std::uint8_t token) = 0;
    virtual void* texture_cache_load(const char* lowered_name, std::uint32_t length,
        std::uint32_t word, std::uint32_t a, std::uint32_t allow_load) = 0;
};
void* find_cached_texture_00b31ac0(const char* name, std::uint32_t length, std::uint32_t word,
    TextureCacheHost& host);

// Registry base constructors, __fastcall, return nothing: vtable + four zero words.
// 00B31D00 vtable 00D5F024, 00B31D60 00D5F038, 00B31FD0 00D5F04C, 00B321B0 00D5F060,
// 00B321E0 00D5F074. 00B32340 (texture resource cache, vtable 00D5F088) also zeroes +14h,
// +18h, byte +1Ch and +20h.
void registry_base_construct(std::uint8_t* object, std::uint32_t vtable) noexcept;
void texture_resource_cache_construct_00b32340(std::uint8_t* object) noexcept;
// 00B321D0, __fastcall: vtable 00D5F024 (the base), 00B32030 (registry destroy, ECX =
// this), records +04h resized to 0 (stride 2Ch), _free(+04h data).
void declaration_registry_destruct_00b321d0(std::uint8_t* registry,
    const std::function<void(std::uint8_t*)>& flush, const ArrayResizeFn& resize_records);

// 00B33AC0, __cdecl(byte): [0108D6DD] = byte. 00B33BC0, __thiscall(worker, a, b, c),
// RET 0Ch: +1Ch = a, +18h = c, +20h = b, byte +04h = 1, then +0Ch vt+04h() (the worker
// thread's event/start slot).
void set_observed_synchronization_mode_00b33ac0(std::uint8_t& mode_0108d6dd, std::uint8_t value) noexcept;
void render_worker_start_00b33bc0(std::uint8_t* worker, std::uint32_t a, std::uint32_t b, std::uint32_t c,
    const std::function<void(void*)>& start_vt04);

// Logical texture base (vtable 00D5F1F4, ref-counted). 00B33E90: (flags +1Ch >> 25) & 1.
// 00B33EC0, __thiscall(this, a, flags), RET 8: refcount 1, +08h/+0Ch name {0, null},
// +14h 0, +10h = a, +1Ch = flags, +20h = id from [0108D6E8] (post-incremented).
// 00B341D0, __thiscall(this, const NativeString*), RET 4: name +08h assigned (resize +
// memcpy) unless it is the same object.
bool logical_texture_flag_bit25_00b33e90(const std::uint8_t* texture) noexcept;
void logical_texture_construct_00b33ec0(std::uint8_t* texture, std::uint32_t a, std::uint32_t flags,
    std::uint32_t& next_id_0108d6e8) noexcept;

// Render-state byte pair: 00B34380 sets both bytes to FFh, 00B34390 stores (a, b).
void render_state_pair_reset_00b34380(std::uint8_t pair[2]) noexcept;
void render_state_pair_set_00b34390(std::uint8_t pair[2], std::uint8_t a, std::uint8_t b) noexcept;
// Shader reflection owner byte maps (+08h and +3Eh, 36h bytes each): 00B343C0 reads
// map8[i], 00B3FF80 reads map3E[i], 00B34570 writes both. 00B3B190 constructs the owner
// (vtable 00D61810, refcount 1, byte +74h 0, +78h..+84h 0, both maps memset FFh).
std::uint8_t reflection_map8_at_00b343c0(const std::uint8_t* owner, std::int32_t i) noexcept;
std::uint8_t reflection_map3e_at_00b3ff80(const std::uint8_t* owner, std::int32_t i) noexcept;
void reflection_maps_set_00b34570(std::uint8_t* owner, std::int32_t i, std::uint8_t a, std::uint8_t b) noexcept;
void reflection_owner_construct_00b3b190(std::uint8_t* owner) noexcept;

// 00B34400, __thiscall(cache, key, size, data), RET 0Ch: unless [0108D6F1] is set,
// writes to the stream at +08h: vt+64h(key, 0), vt+54h(size, 0), vt+28h(data, size, 0),
// then +04h (record count) += 1.
class BinaryStreamHost {
public:
    virtual ~BinaryStreamHost() = default;
    virtual void write_key_vt64(void* stream, std::uint32_t key, std::uint32_t zero) = 0;
    virtual void write_size_vt54(void* stream, std::uint32_t size, std::uint32_t zero) = 0;
    virtual void write_bytes_vt28(void* stream, const void* data, std::uint32_t size, std::uint32_t zero) = 0;
};
void shader_binary_cache_append_00b34400(std::uint8_t* cache, bool suppressed_0108d6f1, std::uint32_t key,
    std::uint32_t size, const void* data, BinaryStreamHost& host);

// 00B347D0: the chars of a NativeString, or the empty string at 0108D6F2.
const char* native_string_c_str_00b347d0(const PooledString& s) noexcept;
// 00B34860: count of the sampler pointers (+C4h data, +C8h count) whose record byte +0Ch
// is ZERO (the ledger name "FindFirstFlaggedSampler" was wrong).
std::int32_t count_unflagged_samplers_00b34860(const std::uint8_t* descriptor) noexcept;

// Shader binary cache records (10h: {+00 name length, +04 chars, +08 data, +0Ch}).
// 00B34CE0, __thiscall(record, flags), RET 4: the MSVC vector deleting destructor:
// flags & 2 runs the destructor over the count stored at record-4 and frees the block
// at record-4 when flags & 1; otherwise _free(+08h) (cleared), the name block returned,
// _free(record) when flags & 1. 00B34DD0 deletes the array held in a slot the same way.
void shader_binary_cache_record_destroy(std::uint8_t* record, StoragePoolHost& pool);

// Shader builder helper text (00B38210 / 00B38220 / 00B38290, ECX builder): appended to
// the source at builder+4Ch by 00B34F20 (AppendCStringLine). The texts equal the
// existing literals vertex_shadow_helper (.rdata 00D5F770), vertex_ambient_fog_helpers
// (00D5F870), pixel_ambient_fog_helpers (00D60448) and pixel_srgb_helpers (00D60258) in
// src/shader_*_literals.inc (byte-compared against the PE).
class ShaderTextHost {
public:
    virtual ~ShaderTextHost() = default;
    virtual void append_cstring_line_00b34f20(std::uint8_t* source, const char* text) = 0;
    virtual void append_system_constant_header_00b38ff0(std::uint8_t* builder, std::uint32_t kind) = 0;
    virtual void append_formatted_line_00b35110(std::uint8_t* source, const char* format,
        const char* a, const char* b) = 0;
};
void append_null_shadow_fade_out_00b38210(std::uint8_t* builder, ShaderTextHost& host);
void append_vertex_ambient_helpers_00b38220(std::uint8_t* builder, ShaderTextHost& host);
void append_pixel_ambient_and_srgb_helpers_00b38290(std::uint8_t* builder, ShaderTextHost& host);
// 00B39070 / 00B390C0 (byte-identical bodies): header kind 1, then "\n%s\n%s" with the
// texts of the +70h and +74h children (string at child+E8h+04h, or "" at 0108D6F2).
void append_system_constant_pair(std::uint8_t* builder, ShaderTextHost& host);

// Singleton lifetime client (00B61B10 construct / 00B61BB0 destruct, vtable 00D62B4C)
// and its two leaves: 00B61C70 (vtable 00D62B50) and 00B3C4C0 (vtable 00D61844, the
// object published in [0108FED8]); 00B3C4E0 destructs the latter (vtable 00D61844, then
// 00B61C90 the adjustor thunk to the base destructor). Registration happens under the
// manager's optional critical section (manager+10h; depth word at cs+18h).
class SingletonLifetimeHost {
public:
    virtual ~SingletonLifetimeHost() = default;
    virtual void enter_manager_lock() = 0;   // no-op when the manager has no section
    virtual void leave_manager_lock() = 0;
    virtual void register_singleton(void* object) = 0;
    virtual void unregister_singleton(void* object) = 0;
};
void singleton_client_construct_00b61b10(std::uint8_t* object, void*& published_0108fed8, SingletonLifetimeHost& host);
void singleton_client_destruct_00b61bb0(std::uint8_t* object, void*& published_0108fed8, SingletonLifetimeHost& host);

// Cockpit helper (+08h node, +0Ch camera). 00B3C550: node non-null and node byte +44h
// == 0. 00B3C570: when the camera is set, 00B71490 (ECX camera) - the
// matrix argument is the caller's. 00B3C650, __thiscall(helper, node), RET 4: the old
// node's visibility factor is set to 0.0 (00B6DA70(0.0, 0)), the node slot assigned with
// retain/release semantics, and the NEW node's factor set to 0.0 as well.
class CockpitNodeHost {
public:
    virtual ~CockpitNodeHost() = default;
    virtual void node_set_visibility_factor_00b6da70(void* node, float factor, std::uint32_t flags) = 0;
};
bool cockpit_node_visible_00b3c550(const std::uint8_t* helper) noexcept;
void cockpit_set_attached_node_00b3c650(std::uint8_t* helper, RefCounted* node, CockpitNodeHost& host);

// D3D9 resource wrappers. Surface wrapper: +24h sample count, +2Ch IDirect3DSurface9;
// texture wrappers: +10h IDirect3D{Texture,CubeTexture,VolumeTexture}9, +14h level count,
// +18h format, +24h/+28h(/+2Ch) width/height(/depth). The COM interface is passed as an
// abstract object whose virtuals carry the vtable offsets the image calls.
class D3DResourceHost {
public:
    virtual ~D3DResourceHost() = default;
    virtual std::int32_t com_call(void* com, std::uint32_t vtable_offset, const std::uint32_t* args, int argc) = 0;
    virtual std::int32_t d3dx_save_surface_to_file(const char* file, std::uint32_t format, void* surface) = 0;
    virtual std::int32_t d3dx_save_texture_to_file(const char* file, std::uint32_t format, void* texture) = 0;
    // D3DXLoadSurfaceFromSurface(dst, NULL, dst_rect, src, NULL, src_rect, D3DX_FILTER_NONE, 0).
    virtual std::int32_t d3dx_load_surface_from_surface(void* destination, std::uint32_t destination_rect,
        void* source, std::uint32_t source_rect) = 0;
};
// 00B3CD40 surface LockRect (+2Ch vt+34h(a, b, c), returns 1); 00B3CD60 UnlockRect
// (vt+38h); 00B3D030 / 00B3D590 volume LockBox / 2D LockRect (+10h vt+4Ch(level, out,
// rect, flags), returns 1); 00B3D060 / 00B3D5C0 unlock (vt+50h(level)); 00B3D6D0 cube
// LockRect (+10h vt+4Ch(face = arg2, level = arg1... the image passes (arg2, arg3,
// arg1, arg4, arg5)); 00B3D700 cube UnlockRect (vt+50h(a, b)).
std::int32_t surface_lock_rect_00b3cd40(std::uint8_t* surface, std::uint32_t a, std::uint32_t b, std::uint32_t c, D3DResourceHost& host);
void surface_unlock_rect_00b3cd60(std::uint8_t* surface, D3DResourceHost& host);
std::int32_t texture_lock_level(std::uint8_t* texture, std::uint32_t level, std::uint32_t out,
    std::uint32_t rect, std::uint32_t flags, D3DResourceHost& host);
void texture_unlock_level(std::uint8_t* texture, std::uint32_t level, D3DResourceHost& host);
std::int32_t cube_lock_face_00b3d6d0(std::uint8_t* cube, std::uint32_t a1, std::uint32_t a2, std::uint32_t a3,
    std::uint32_t a4, std::uint32_t a5, D3DResourceHost& host);
void cube_unlock_face_00b3d700(std::uint8_t* cube, std::uint32_t a, std::uint32_t b, D3DResourceHost& host);
// 00B3CD70, __thiscall(destination, source, source_rect, destination_rect), RET 0Ch:
// D3DXLoadSurfaceFromSurface(dst+2Ch, NULL, destination_rect, src+2Ch, NULL, source_rect,
// D3DX_FILTER_NONE, 0) == D3D_OK.
bool surface_load_from_surface_00b3cd70(std::uint8_t* destination, std::uint8_t* source,
    std::uint32_t source_rect, std::uint32_t destination_rect, D3DResourceHost& host);
// 00B3CDA0, __thiscall(destination, source), RET 4: device from dst+2Ch GetDevice (vt+0Ch);
// rect {0, 0, dst vt+1Ch width, dst vt+20h height}; device StretchRect (vt+88h)(src+2Ch,
// NULL, dst+2Ch, &rect, D3DTEXF_LINEAR); device Release.
class StretchHost {
public:
    virtual ~StretchHost() = default;
    virtual void* surface_device_vt0c(void* com_surface) = 0;
    virtual std::uint32_t wrapper_width_vt1c(std::uint8_t* wrapper) = 0;
    virtual std::uint32_t wrapper_height_vt20(std::uint8_t* wrapper) = 0;
    virtual void device_stretch_rect_vt88(void* device, void* source, const void* source_rect,
        void* destination, const std::int32_t destination_rect[4], std::uint32_t filter) = 0;
    virtual void device_release(void* device) = 0;
};
void surface_stretch_from_00b3cda0(std::uint8_t* destination, std::uint8_t* source, StretchHost& host);
// 00B3DD00 / 00B3DF10 / 00B3E050, __thiscall(wrapper, const NativeString* file, format),
// RET 8: D3DXSaveSurfaceToFileA / D3DXSaveTextureToFileA (file chars or "" at 0108DBE0).
void surface_save_to_file_00b3dd00(std::uint8_t* surface, const PooledString& file, std::uint32_t format, D3DResourceHost& host);
void texture_save_to_file(std::uint8_t* texture, const PooledString& file, std::uint32_t format, D3DResourceHost& host);

// 00B3DF40 (cube) / 00B3E080 (volume), __fastcall(ECX texture), RET: reopen the file named
// at +08h through [0109CEEC] vt+04h(name, 2); a stream that does not open (vt+18h false)
// is deleted (vt+04h(1)) and nothing else happens. Otherwise the stream is converted to a
// memory stream, released, the renderer is told (vt+6Ch with the name address), the old
// COM texture +10h is released and cleared, and D3DXCreate{Cube,Volume}TextureFromFile
// InMemory(device, data, size, &+10h) recreates it; on success level 0's description
// gives cube +24h = width, volume +24h/+28h/+2Ch = width/height/depth, then +14h = level
// count (vt+34h) and +18h = format; the memory stream is released.
class TextureReloadHost {
public:
    virtual ~TextureReloadHost() = default;
    virtual void* open_file_stream(const PooledString& name) = 0;        // [0109CEEC] vt+04h(name, 2)
    virtual bool stream_is_open_vt18(void* stream) = 0;
    virtual void stream_delete_vt04(void* stream) = 0;                   // vt+04h(1)
    virtual void* stream_to_memory(void* stream) = 0;                    // BSP_Stream_ConvertToMemory
    virtual void release(void* ref_counted) = 0;                         // InterlockedDecrement +04h
    virtual void renderer_notify_reload_vt6c(const void* name_address) = 0;
    virtual void com_release(void* com) = 0;
    virtual void* device_00b1fef0() = 0;
    virtual std::uint32_t memory_size_vt30(void* memory) = 0;
    virtual const void* memory_data_00bef610(void* memory) = 0;
    virtual void* d3dx_create_from_memory(bool volume, void* device, const void* data, std::uint32_t size) = 0;
    // Level-0 description: {format, width, height, depth (volume only)}.
    virtual void level_desc_vt44(void* com, bool volume, std::uint32_t out[4]) = 0;
    virtual std::uint32_t level_count_vt34(void* com) = 0;
};
void texture_reload_from_file(std::uint8_t* texture, bool volume, TextureReloadHost& host);

// 00B3F450, __fastcall: D3D9 surface wrapper default construction (vtable 00D619A0,
// refcount 1, +08h/+0Ch = [00D7A24C], +10h..+28h zero, +2Ch 0, byte +30h 0).
// 00B3DC50: ref-counted object (vtable 00D618F8) with +08h/+0Ch = [00D7A24C].
// 00B3D0D0: float4 (x, y, z, w) constructor.
void d3d9_surface_construct_default_00b3f450(std::uint8_t* surface, std::uint32_t word_00d7a24c) noexcept;
void ref_counted_d618f8_construct_00b3dc50(std::uint8_t* object, std::uint32_t word_00d7a24c) noexcept;

// 00B3FE90, __thiscall(cube, level, face), RET 8: +10h GetCubeMapSurface(face, level,
// &surface) (vt+48h); a slot from the surface pool [0108DB00] (00B3ED40); when it is
// non-null the D3D9 surface wrapper 00B3F630(surface, cube+1Ch, 0) is built there; the
// surface reference is released (vt+08h); returns the wrapper or null.
class FaceSurfaceHost {
public:
    virtual ~FaceSurfaceHost() = default;
    virtual void* cube_map_surface_vt48(void* com_cube, std::uint32_t face, std::uint32_t level) = 0;
    virtual void* surface_pool_allocate_00b3ed40() = 0;
    virtual void* surface_construct_00b3f630(void* slot, void* com_surface, std::uint32_t word_1c, std::uint32_t zero) = 0;
    virtual void com_release(void* com) = 0;
};
void* cube_face_surface_00b3fe90(std::uint8_t* cube, std::uint32_t level, std::uint32_t face, FaceSurfaceHost& host);

// Shader effect / material effect accessors: 00B40050 pass record = this + 144h + i*40h;
// 00B40FF0 parameter record data = [+184h] + i*60h + 20h; 00B41010 its key = [[+184h] +
// i*60h]; 00B40840 float at +140h + i*4.
float material_effect_indexed_float_00b40840(const std::uint8_t* effect, std::int32_t i) noexcept;
std::uint8_t* shader_effect_parameter_data_00b40ff0(const std::uint8_t* effect, std::int32_t i) noexcept;
std::uint32_t shader_effect_parameter_key_00b41010(const std::uint8_t* effect, std::int32_t i) noexcept;
// 00B40320: dynamic cast: object non-null and object vt+0Ch(type id of the optimized node
// animator, 00B75E50) true -> object, else null.
void* optimized_animator_dynamic_cast_00b40320(void* object, const std::function<bool(void*)>& is_kind_vt0c);

// Constant blocks: float4 slots at block+04h + i*10h. 00B406B0 / 00B406E0 copy four
// words, 00B40710 stores (value, 0, 0, 0). 00B40400 / 00B40440 write the first two /
// three columns of a 4x4 as rows (transposed 2x4 / 3x4). 00B405E0, __thiscall(block,
// slot, matrix, rows), RET 0Ch: rows 2 -> the 2x4 transpose copied into slots slot and
// slot+1; rows 3 -> 00B40440 (ECX = the slot) ; rows 4 -> 00B404A0 (WriteTransposedMatrix,
// ECX = the slot); any other count does nothing.
void float4_block_set_dwords(std::uint8_t* block, std::int32_t slot, const std::uint32_t value[4]) noexcept;
void float4_block_set_scalar_00b40710(std::uint8_t* block, std::int32_t slot, std::uint32_t value) noexcept;
void float2x4_from_matrix_transposed_00b40400(std::uint32_t out[8], const std::uint32_t m[16]) noexcept;
void float3x4_from_matrix_transposed_00b40440(std::uint32_t out[12], const std::uint32_t m[16]) noexcept;
void float4_block_set_matrix_rows_00b405e0(std::uint8_t* block, std::int32_t slot, const std::uint32_t m[16],
    std::int32_t rows, const std::function<void(std::uint8_t*, const std::uint32_t*)>& write_transposed_4x4);

// Ref-counted constructors (base vtable 00CEB130 then the class vtable, refcount 1):
// 00B41FD0 / 00B42020 / 00B42070 material state lists (vtables 00D61A2C, ...; +08h..+10h
// an empty array); 00B49430 (00D61D24) and 00B49B10 (00D61D30) vtable only; 00B40520
// instance generator binding (00D619F8; +0Ch 0, +08h id from [0108FD30]).
void ref_counted_construct(std::uint8_t* object, std::uint32_t vtable) noexcept;
void instance_generator_binding_construct_00b40520(std::uint8_t* object, std::uint32_t& next_id_0108fd30) noexcept;
// 00B42000 / 00B42050 / 00B420A0, __thiscall(this, src), RET 4: +04h (the refcount word!)
// copied from src, then the rows array +08h copy-assigned from src+08h; returns this.
std::uint8_t* material_state_list_assign(std::uint8_t* object, const std::uint8_t* source,
    const std::function<void(RawArray&, const RawArray&)>& copy_rows);

// 00B45330, __fastcall(effect): when byte +B4h is set, 00B44C00 for each non-null of the
// 14 section pointers at +C8h.
void material_effect_apply_generator_modes_00b45330(std::uint8_t* effect, const std::function<void(void*)>& apply_00b44c00);
// 00B45090 / 00B45190: generic / building instance generator destructors: vtable
// 00D61BFC / 00D61C1C, then 00B55BE0 (shared generator teardown).

// Vertex elements (14h: {+00 stream, +04 type, +08 method, +0Ch usage, +10h index}).
// 00B478B0: {-1, 11h (D3DDECLTYPE_UNUSED), 0, 0Eh, -1}. 00B478E0: type size from the
// dword table 00D61CC0[type].
void vertex_element_construct_unused_00b478b0(std::uint32_t element[5]) noexcept;
// 00B478E0: the type size from the .rdata dword table 00D61CC0 (18 entries, read from the
// PE: FLOAT1..4 = 4/8/12/16, D3DCOLOR/UBYTE4/SHORT2 = 4, SHORT4 8, UBYTE4N/SHORT2N 4,
// SHORT4N 8, USHORT2N 4, USHORT4N 8, UDEC3/DEC3N/FLOAT16_2 4, FLOAT16_4 8, UNUSED 4).
std::uint32_t vertex_element_type_size_00b478e0(const std::uint32_t element[5]) noexcept;
// 00B49170: (format & F000h) == 1000h.
bool d3d_format_is_kind_1000_00b49170(std::uint32_t format) noexcept;

// Hardware layout record quads (4 x 0Ch records + count at +30h). 00B48430 constructs
// them through eh_vector_constructor_iterator (0Ch x 4, constructor 00B47640, unwind
// destructor 00B483F0) and sets count 0; 00B48460 assigns the next record (count incremented first) from src
// through 00B483A0. 00B489E0: word 0 = 0 then 00B483A0. 00B48C80, __thiscall(layout,
// flags), RET 4: 00B48960 (base destructor), then the slot returned to pool [0108FD70]
// through 00B479C0 when flags & 1.
void layout_record_quad_push_assign_00b48460(std::uint8_t* quad, const std::uint32_t* source);

// Logical streams: 00B493E0, __thiscall(stream, base), RET 4: the CPU copy +6Ch
// (count +64h * stride (vt+24h()->+CCh)) is memcpy'd to base + [+5Ch], freed, +6Ch = 0,
// +5Ch = 0. 00B49490: index stream, format +18h (65h -> 2 bytes, 66h -> 4, else 0),
// CPU copy +1Ch (count +14h) to base + first(+20h)*size; freed, +1Ch = 0.
void logical_vertex_stream_commit_00b493e0(std::uint8_t* stream, std::uint8_t* base, std::uint32_t vertex_stride);
void logical_index_stream_commit_00b49490(std::uint8_t* stream, std::uint8_t* base);
// 00B4A9B0, __thiscall(memory, s20, s24, physical, byte offset), RET 10h: logical vertex
// stream view (vtable 00D61D6C) over a shared physical buffer: base constructor
// 00B61E20 (s24), +58h..+70h zero, +64h = s20, +60h = 1,
// +68h = s24 retained (when non-null), +58h = physical (retain/release assign),
// +5Ch = byte offset, then 00B28A40(this).

// 00B61F90, __thiscall(cache, index, const float element[8]), RET 8: the 20h-byte element
// array +50h is allocated on first use with (element count of vt+24h()'s declaration,
// 00B47900) * 20h bytes (operator new of -1 when the product overflows); element[index]
// = the eight floats (FLD/FSTP float pairs: bit-exact except that a signalling NaN would
// be quieted).
void vertex_declaration_cache_set_element_00b61f90(std::uint8_t* cache, std::int32_t index,
    const float element[8], const std::function<std::uint32_t()>& element_count);

// 00B64CE0, __fastcall(ECX out 4x4, EDX translation*, quaternion*, scale*), RET 8:
// out = diag(scale.x, scale.y, scale.z, 1) * R(quaternion) (0047C2A0 builds R from the
// quaternion; 00413920 multiplies with ECX = the scale matrix, stack (out, R)), then the
// translation row out[12..14] = translation; returns out.
float* matrix_from_scale_rotation_translation_00b64ce0(float out[16], const float translation[3],
    const void* quaternion, const float scale[3],
    const std::function<void(float rotation[16], const void* quaternion)>& rotation_0047c2a0,
    const std::function<void(const float a[16], float out[16], const float b[16])>& multiply_00413920);

// 00B65660, __thiscall(planes[6], matrix), RET 4: each of the six planes (10h apart)
// replaced by 00B65BA0(plane, matrix) (the transformed plane); returns planes.
float* plane_set_transform_six_00b65660(float planes[24], const void* matrix,
    const std::function<void(float out[4], const float plane[4], const void* matrix)>& transform_00b65ba0);

// ===========================================================================
// Segment 86: post-effect passes. Vocabulary shared by the bodies below (all thiscall
// on the object named first): holder 00B4CB10 texture, holder 00B4CB20 primary surface,
// pass 00B4CB70 SetColor0Surface(surface), pass 00B4CB90 frame targets, pass 00B4CBA0
// material, pass 00B4CBB0 camera, material 00B189F0 SetTextureSlot(index, texture),
// pass 00B4DCC0 draw full-screen (unused word 0), camera 00B6FE10 SetClearFlags(flag),
// camera 00B6FE50 SetClearColor(argb), targets 00B1F6D0 colour surface(index), texture
// vt+3Ch width / vt+40h height, surface vt+1Ch width / vt+20h height.
class PostEffectHost {
public:
    virtual ~PostEffectHost() = default;
    virtual void* holder_texture_00b4cb10(void* holder) = 0;
    virtual void* holder_primary_surface_00b4cb20(void* holder) = 0;
    virtual void pass_set_color0_surface_00b4cb70(void* pass, void* surface) = 0;
    virtual void* pass_frame_targets_00b4cb90(void* pass, std::uint32_t zero) = 0;
    virtual void* pass_material_00b4cba0(void* pass) = 0;
    virtual void* pass_camera_00b4cbb0(void* pass) = 0;
    virtual void material_set_texture_slot_00b189f0(void* material, std::uint32_t index, void* texture) = 0;
    virtual void pass_draw_fullscreen_00b4dcc0(void* pass, std::uint32_t unused) = 0;
    virtual void camera_set_clear_flags_00b6fe10(void* camera, std::uint32_t flags) = 0;
    virtual void camera_set_clear_color_00b6fe50(void* camera, std::uint32_t argb) = 0;
    virtual void* targets_color_surface_00b1f6d0(void* targets, std::uint32_t index) = 0;
    virtual std::uint32_t texture_width_vt3c(void* texture) = 0;
    virtual std::uint32_t texture_height_vt40(void* texture) = 0;
    virtual std::uint32_t surface_width_vt1c(void* surface) = 0;
    virtual std::uint32_t surface_height_vt20(void* surface) = 0;
    virtual void renderer_set_targets_vt98(void* targets) = 0;   // [00F8D394] vt+98h
};

// 00B4C6A0 / 00B4C6D0, __fastcall(ECX texture): level 0 surface = vt+30h(0, 0) (a retained
// reference), 00B4C490 (premultiply) / 00B4C5D0 (alpha from non-black) on it
// (cc13fx::premultiply_alpha_bgra / alpha_from_non_black_bgra), then released.
void texture_fix_alpha_level0(void* texture, const std::function<RefCounted*(void*)>& level0_surface_vt30,
    const std::function<void(RefCounted*)>& fix);

// Capture holder {+08h targets object, +0Ch source surface, +10h destination surface,
// +14h enabled byte}. 00B4CAF0: when enabled, 00B3CDA0(destination = +10h, source =
// +0Ch). 00B4D270, __thiscall(holder, samples), RET 4: when enabled and the destination
// +0Ch's vt+24h() (sample count) differs: 00B3CE20(samples) on it and +08h vt+54h with a
// zeroed 10h record (recreate).
void capture_copy_if_enabled_00b4caf0(std::uint8_t* holder, StretchHost& host);
void capture_set_sample_count_00b4d270(std::uint8_t* holder, std::uint32_t samples,
    const std::function<std::uint32_t(void*)>& sample_count_vt24,
    const std::function<void(void*, const std::uint32_t zero_record[4])>& recreate_vt54);

// 00B4DEF0, __fastcall(ECX pass {+08h targets, +0Ch camera, +14h material, +1Ch render
// entry}): the camera viewport (00B6FDE0) is resized (00B1F940) to {colour surface(0)
// vt+1Ch width, colour surface(0) vt+20h height}; [00F8D394] vt+A0h(camera), vt+A4h(the
// camera's viewport), vt+98h(targets); the entry is submitted to material+7Ch->+9Ch
// vt+08h.
class PassSubmitHost {
public:
    virtual ~PassSubmitHost() = default;
    virtual void* targets_color_surface_00b1f6d0(void* targets, std::uint32_t index) = 0;
    virtual std::uint32_t surface_width_vt1c(void* surface) = 0;
    virtual std::uint32_t surface_height_vt20(void* surface) = 0;
    virtual void* camera_viewport_00b6fde0(void* camera) = 0;
    virtual void viewport_set_dimensions_00b1f940(void* viewport, const std::uint32_t size[2]) = 0;
    virtual void renderer_set_camera_vta0(void* camera) = 0;
    virtual void renderer_set_viewport_vta4(void* viewport) = 0;
    virtual void renderer_set_targets_vt98(void* targets) = 0;
    virtual void material_queue_submit(void* material, void* entry) = 0;  // +7Ch->+9Ch vt+08h
};
void post_effect_submit_current_entry_00b4def0(std::uint8_t* pass, PassSubmitHost& host);
// 00B4DFB0, __fastcall(ECX pass): 00B4DCC0(0) on the pass (a draw wrapper).
// 00B4ED30, __thiscall(pass, out[2]), RET 4: out = {+24h, +28h}.
// 00B548C0, __thiscall(pass, holder, a, b, c, d, e), RET 18h: +210h..+220h = a..e (five
// floats), the pass +08h material's slot 1 = holder texture, then +08h draws.
void tonemap_set_params_and_draw_00b548c0(std::uint8_t* pass, void* holder, const float params[5], PostEffectHost& host);

// Distortion owner (layout by use): +10h/+14h ping-pong texture
// holders, +1Ch/+20h passes, +34h scene camera, +38h targets, +3Ch scene root, +40h
// GaussBlur5x5 offsets (13 float4), +140h weights, +244h float, +250h clear-scene byte,
// +251h clear-targets-once byte, +254h {data, +258h count, +25Ch capacity} of 8-byte
// {node clone, object clone} records, +260h distortion objects enabled byte.
class DistortionHost : public PostEffectHost {
public:
    virtual void* node_clone_vt10(void* node, std::uint32_t flags, void* parent) = 0;
    virtual void node_set_visibility_factor_00b6da70(void* node, float factor, std::uint32_t flags) = 0;
    virtual void node_attach_00b6d890(void* node, void* parent) = 0;
    virtual void node_unlink_and_release_00b6dfa0(void* node) = 0;
    virtual void release(void* ref_counted) = 0;
    virtual void targets_set_color_surface_00b1fab0(void* targets, std::uint32_t index, void* surface) = 0;
    virtual void camera_execute_frame_00b71360(void* camera) = 0;
    // Scene walk: first child of root (+0Ch), next sibling at node+3Ch.
    virtual bool node_is_generated_model_vt0c(void* node) = 0;   // vt+0Ch(00B74310 type)
    virtual void* model_geometry_00b74640(void* model, std::uint32_t index) = 0;
    virtual void* geometry_element_00b732c0(void* geometry, std::uint32_t index) = 0;
    virtual void node_prepare_vt58(void* node, void* camera, float weight) = 0;
    virtual void* allocate_render_entry_0108fe88() = 0;
    virtual void render_entry_initialize_00b51a20(void* entry, float sort_bias, void* element,
        void* geometry, void* model, void* camera, float visibility, float depth_override,
        std::uint32_t flags) = 0;
    virtual void submit_render_entry(void* element, void* entry) = 0;  // element+20h->+7Ch->+9Ch vt+08h
};
// 00B4EF90, __thiscall(owner, node, object), RET 8: object null -> 00B6D890(node, +3Ch)
// (attach the node to the scene root). Otherwise both are cloned (vt+10h(0, 0)), the node
// clone's visibility factor set to 0.0 and {node clone, object clone} appended to +254h
// (reserve 00B4ED70, capacity rule double/min 1).
void distortion_add_object_00b4ef90(std::uint8_t* owner, void* node, void* object, DistortionHost& host,
    const ArrayResizeFn& reserve_00b4ed70);
// 00B4F030, __fastcall: for each record: 00B6DFA0 on the node clone (when non-null), the
// object clone released; then the records resized to 0 (00B4EE20).
void distortion_release_objects_00b4f030(std::uint8_t* owner, DistortionHost& host, const ArrayResizeFn& resize_00b4ee20);
// 00B4F1C0, __thiscall(owner, camera), RET 4 (the ledger ABI said fastcall). See the doc
// for the full order: optional one-time clear of both holders through pass +1Ch (with
// +244h temporarily [00D217E8]), scene pass of the generated models under +3Ch into
// +10h, GaussBlur5x5 offsets for the +10h texture (00B4D980, multiplier 1.0), blur pass
// +1Ch into +14h sampling +10h, composite pass +20h sampling +14h, the distortion objects
// drawn with `camera`, holders swapped, objects released.
void distortion_render_00b4f1c0(std::uint8_t* owner, void* camera, DistortionHost& host,
    const ArrayResizeFn& resize_00b4ee20);

// Luminance owner (layout by use): +10h sample offsets (16 float4), +210h clear pass,
// +214h average-luminance pass, +218h/+21Ch/+220h downscale passes, +224h adaptation pass,
// +228h measured-luminance holder, +22Ch/+230h/+234h downscale source holders, +238h/
// +23Ch adaptation ping-pong holders, +240h the word argument, +248h readback surface,
// +24Ch clear-adaptation-once byte.
// 00B50050, __thiscall(owner, word), RET 4: +240h = word; pass 1: offsets = AvgLum3x3 of
// the +214h material's first texture (material word +34h >= 1, else the image
// dereferences null), pass 2-4: DownScale4x4 of the +234h/+230h/+22Ch textures; before
// each draw every offset gets the destination half texel added: x += f(0.5/w),
// y += f(0.5/h), z += 0.0f, w += 0.0f (w, h = the pass frame targets' colour surface 0;
// the 0.0f adds turn -0.0 into +0.0). Then the optional clear of both adaptation holders
// (camera clear flags 1, colour FFFFFFFFh, pass +210h drawn into +23Ch then +238h), the
// +238h/+23Ch swap and the adaptation pass +224h into the new +23Ch sampling +238h (slot
// 0) and +228h (slot 1).
void luminance_measure_and_adapt_00b50050(std::uint8_t* owner, std::uint32_t word, PostEffectHost& host);
// 00B51000, __fastcall: [00F8D394] 00B1FF70(primary surface of +238h, +248h) (render
// target data into the readback surface); +248h vt+2Ch(&rect, 0, D3DLOCK_READONLY) and
// vt+30h (unlock); 004C14C0 (diagnostic sink) is called and its result dropped: nothing
// is read from the locked rectangle.

// 00B54CE0, __fastcall(ECX bloom owner): GaussBlur5x5 (+10h, +14h, +228h, +328h, 1.0); pass
// +24h into the +18h holder; then Bloom15 for the +18h texture (width vt+3Ch, height
// vt+40h, offsets +28h, weights +128h, deviation +438h, multiplier 1.0) with every offset
// y zeroed (horizontal), pass +08h sampling +18h into +1Ch; Bloom15 again (width of +1Ch,
// height of +18h) with every x zeroed (vertical), [00F8D394] vt+98h(0), pass +08h
// sampling +1Ch into +20h.
void bloom_render_00b54ce0(std::uint8_t* owner, PostEffectHost& host);

// ===========================================================================
// Segment 86: mshd tree lifetime (layouts in bsp/post_effects_cc13.hpp). Owned children
// are deleted through vtable slot 0 with flags 1.
class MshdLifetimeHost : public StoragePoolHost {
public:
    // The pointer-array resize instantiation named by its address (00B34760, 00B56380,
    // 00B563D0, 00B56420, 00B56470: zero-fill resize of 4-byte cells).
    virtual void resize_pointer_array(std::uint32_t resize_address, RawArray& a, std::int32_t n) = 0;
    virtual void resize_constant_rows_00b580c0(RawArray& rows, std::int32_t n) = 0;
    virtual void destroy_state_list_00b56de0(std::uint8_t* list) = 0;          // layer +0Ch
    virtual void destroy_caps_00b2f690(std::uint8_t* caps) = 0;                // version +0Ch
    virtual void ref_counted_destroy_base_00bd30f0(std::uint8_t* object) = 0;  // shader root
};
// Owned pointer vector teardown shared by the destructors: every non-null cell deleted
// (slot 0, flags 1) and cleared, the vector resized to 0 through `resize_address`, then
// _free(data).
void mshd_delete_owned_vector(RawArray& cells, std::uint32_t resize_address, MshdLifetimeHost& host);
// 00B56A30 LOD phase (vtable 00D621F0): versions +0Ch (resize 00B56420).
// 00B56C90 render mode (vtable 00D621FC): layers +10h (resize 00B56380), name +04h.
// 00B572C0 shader root (vtable 00D62200): LOD phases +10h (resize 00B56470), name +08h,
//   then 00BD30F0 (ref-counted base).
// 00B584C0 version (vtable 00D62264): render modes +68h (resize 00B563D0), caps +0Ch
//   destroyed (00B2F690), name +04h.
// 00B5B650 layer (vtable 00D621F8): state list +0Ch (00B56DE0), samplers +24h owned
//   (cells deleted), VS/PS constant lists +64h/+68h (rows resized to 0 by 00B580C0, rows
//   data freed, list freed, slot cleared), names returned in the order +70h, +48h, +40h,
//   +38h, +30h, the sampler vector resized to 0 (00B34760) and freed, name +04h.
void mshd_lod_phase_destruct_00b56a30(std::uint8_t* phase, MshdLifetimeHost& host);
void mshd_render_mode_destruct_00b56c90(std::uint8_t* mode, MshdLifetimeHost& host);
void mshd_shader_destruct_00b572c0(std::uint8_t* shader, MshdLifetimeHost& host);
void mshd_version_destruct_00b584c0(std::uint8_t* version, MshdLifetimeHost& host);
void mshd_layer_destruct_00b5b650(std::uint8_t* layer, MshdLifetimeHost& host);
// 00B5B620: the constant-list slot delete used by the layer (rows resized to 0, rows data
// and the list freed, slot cleared).
void shader_constant_list_slot_delete_00b5b620(std::uint8_t** slot, MshdLifetimeHost& host);

// 00B56B90, __fastcall: layer construction (raw): vtable 00D621F8, +04h..+10h 0, bytes
// +18h..+1Ah 0, +1Ch/+20h 0, +14h = 1 (FogType FT_NORMAL), +24h..+4Ch 0, byte +58h 0,
// +5Ch..+68h 0, +6Ch = 1 (PS_Code), +70h/+74h 0. +50h, +54h and byte +59h..+5Bh are not
// written. These are the defaults of cc13fx::MshdLayer.
void mshd_layer_construct_00b56b90(std::uint8_t* layer) noexcept;
// 00B571E0, __thiscall(shader, const NativeString* name), RET 4: ref-counted (vtable
// 00D62200, refcount 1), +08h..+18h zero, name +08h assigned.
// 00B56E30, __thiscall(row, name*, d8, dC, count, definition, reg), RET 18h: row {+00
// definition, +04 name copy, +0Ch d8, +10h dC, +14h count, +18h register}.
// 00B57680: the row copy constructor (same fields, name deep-copied).
// 00B573A0, __thiscall(pair, name*, a, b), RET 0Ch: {name copy, a, b}.
struct ConstantRowRaw {
    std::uint32_t definition;
    PooledString name;
    std::int32_t dimension_8, dimension_c, array_count, register_index;
};
// 00B58160, __thiscall(list, name*, d8, dC, count, definition), RET 14h: a temporary row
// (00B56E30 with register = list+0Ch) is appended (00B57FD0: copy construct, capacity rule
// double/min 1, reserve 00B576F0), the temporary's name returned, then list+0Ch +=
// d8 * count.
void shader_constant_list_append_00b58160(RawArray& rows, std::int32_t& next_register_0c,
    const ConstantRowRaw& row_without_register, const std::function<void(RawArray&, const ConstantRowRaw&)>& push_back_00b57fd0);
// 00B58400, __fastcall: list {rows 0, 0, 0, +0Ch 0}, then the rows resized to 0
// (00B580C0); returns the list.
// 00B576F0 reserve (min 1, rows copy-constructed then the old names returned, capacity
// LAST), 00B57FD0 push-back, 00B580C0 resize (new rows get an empty name only; rows above
// n have their names returned).
// `copy_row_00b57680` is the image row copy constructor (00B57680: definition and scalars
// copied, the name deep-copied through 0041DD40 + memcpy).
using RowCopyFn = std::function<void(std::uint8_t* destination, const std::uint8_t* source)>;
void shader_constant_rows_reserve_00b576f0(RawArray& rows, std::int32_t n, const RowCopyFn& copy_row_00b57680,
    StoragePoolHost& pool);
void shader_constant_rows_resize_00b580c0(RawArray& rows, std::int32_t n, const RowCopyFn& copy_row_00b57680,
    StoragePoolHost& pool);

// 00B60DC0 / 00B60E90, __fastcall(ECX const NativeString* file, EDX shader** out): read the
// whole file ([0109CEEC] vt+04h(name, 2), size vt+30h, buffer new(size + 4) zeroed, read
// vt+24h(buffer, size, 0)) and create a vertex (device vt+16Ch) / pixel (vt+1A8h) shader
// from it; E_FAIL (80004005h) when the buffer allocation fails; returns the HRESULT.
// 00B611C0, __fastcall(ECX unused, EDX source text, shader** out), RET 4:
// D3DXAssembleShader(text, strlen, NULL, NULL, 0, &code, &errors), device CreatePixelShader
// (vt+1A8h)(code->GetBufferPointer(), out); both buffers released; returns the HRESULT.
class ShaderFileHost {
public:
    virtual ~ShaderFileHost() = default;
    virtual void* device_00b1fef0() = 0;
    virtual void* open_file_stream(const void* name) = 0;
    virtual std::uint32_t stream_size_vt30(void* stream) = 0;
    virtual void stream_read_vt24(void* stream, void* buffer, std::uint32_t size, std::uint32_t zero) = 0;
    virtual void release(void* ref_counted) = 0;
    virtual std::int32_t device_create_shader(void* device, std::uint32_t vtable_offset, const void* code, void** out) = 0;
    virtual std::int32_t d3dx_assemble_shader(const char* text, std::uint32_t length, void** code, void** errors) = 0;
    virtual const void* buffer_pointer_vt0c(void* buffer) = 0;
    virtual void com_release(void* com) = 0;
};
std::int32_t shader_file_create(const void* file, void** out, bool pixel, ShaderFileHost& host);
std::int32_t assemble_pixel_shader_00b611c0(const char* text, void** out, ShaderFileHost& host);

// ===========================================================================
// Small constructors, accessors and wrappers not covered above.

// Native string assignment: 0041DD40 (resize to the source length) then memcpy, skipped
// when source and destination are the same object.
class StringHost {
public:
    virtual ~StringHost() = default;
    virtual void assign_0041dd40(PooledString& destination, const PooledString& source) = 0;
};
// 00B341D0, __thiscall(texture, const NativeString*), RET 4: name +08h assigned.
void logical_texture_set_name_00b341d0(std::uint8_t* texture, const PooledString& name, StringHost& strings);
// 00B3A6E0, __fastcall(PooledString** slot): when *slot is set, its block returned, the
// 8-byte string object freed, the slot cleared.
void pooled_string_pointer_delete_00b3a6e0(PooledString** slot, StoragePoolHost& pool);
// 00B3A720 / 00B44FA0 (table kPointerDelete): *slot destroyed through the named routine
// (ShaderBinaryCache_Destroy / MaterialPassBinding_Destroy, ECX = object), freed,
// cleared.
void pointer_delete_with(void** slot, const std::function<void(void*)>& destroy);
// Singleton leaves: 00B61C70 = 00B61B10 then vtable 00D62B50; 00B3C4C0 = 00B61C70 then
// vtable 00D61844 (returns the object); 00B3C4E0 = vtable 00D61844 then 00B61C90 (the
// adjustor thunk into the base destructor 00B61BB0).
std::uint8_t* render_singleton_construct_00b3c4c0(std::uint8_t* object, void*& published_0108fed8, SingletonLifetimeHost& host);
void render_singleton_destruct_00b3c4e0(std::uint8_t* object, void*& published_0108fed8, SingletonLifetimeHost& host);
// 00B3C570, __fastcall(ECX helper, matrix on the stack untouched): 00B71490 (ECX = the
// +0Ch camera) when the camera is set.
void cockpit_set_camera_view_matrix_00b3c570(std::uint8_t* helper, const std::function<void(void*)>& set_view_00b71490);
// 00B3D0D0, __thiscall(v, x, y, z, w), RET 10h.
void float4_construct_00b3d0d0(std::uint32_t v[4], std::uint32_t x, std::uint32_t y, std::uint32_t z, std::uint32_t w) noexcept;
// 00B41FD0 / 00B42020 / 00B42070, __fastcall: ref-counted (vtables 00D61A2C / 00D61A34 /
// 00D61A3C) with the rows array +08h..+10h empty.
void material_state_list_construct(std::uint8_t* object, std::uint32_t vtable) noexcept;
// 00B45090 / 00B45190, __fastcall: vtable 00D61BFC / 00D61C1C, then 00B55BE0 (tail jump).
void instance_generator_destruct(std::uint8_t* generator, std::uint32_t vtable,
    const std::function<void(std::uint8_t*)>& teardown_00b55be0);
// 00B48430, __fastcall: four 0Ch layout records constructed by 00B47640 (eh vector
// iterator, unwind 00B483F0), count +30h = 0. 00B489E0, __thiscall(record, src), RET 4:
// word 0 = 0, then 00B483A0. 00B48C80, __thiscall(layout, flags), RET 4: 00B48960, the
// slot returned to [0108FD70] (00B479C0) when flags & 1; returns the layout.
std::uint8_t* layout_record_quad_construct_00b48430(std::uint8_t* quad, const std::function<void(std::uint8_t*)>& construct_00b47640);
std::uint32_t* layout_record_copy_construct_00b489e0(std::uint32_t* record, const std::uint32_t* source);
std::uint8_t* hardware_layout_deleting_destructor_00b48c80(std::uint8_t* layout, std::uint32_t flags,
    const std::function<void(std::uint8_t*)>& destroy_base_00b48960, const std::function<void(std::uint8_t*)>& return_slot_00b479c0);
// 00B4A9B0 logical vertex stream view construction (see the comment above 00B61F90).
std::uint8_t* logical_vertex_stream_construct_00b4a9b0(std::uint8_t* memory, std::uint32_t s20, RefCounted* s24,
    RefCounted* physical, std::uint32_t byte_offset, const std::function<void(std::uint8_t*, RefCounted*)>& base_00b61e20,
    const std::function<void(std::uint8_t*)>& register_00b28a40);
// 00B4DFB0, __fastcall(pass): 00B4DCC0(0). 00B4ED30: extent {+24h, +28h}.
void post_effect_draw_fullscreen_pass_00b4dfb0(void* pass, PostEffectHost& host);
void post_effect_pass_extent_00b4ed30(const std::uint8_t* pass, std::uint32_t out[2]) noexcept;
// 00B51000 (see the luminance comment).
class ReadbackHost {
public:
    virtual ~ReadbackHost() = default;
    virtual void* holder_primary_surface_00b4cb20(void* holder) = 0;
    virtual void renderer_get_render_target_data_00b1ff70(void* source, void* readback) = 0;
    virtual void readback_lock_vt2c(void* readback, std::uint32_t out_rect[2], std::uint32_t rect, std::uint32_t flags) = 0;
    virtual void readback_unlock_vt30(void* readback) = 0;
    virtual void* diagnostic_sink_004c14c0() = 0;
};
void luminance_lock_readback_00b51000(std::uint8_t* owner, ReadbackHost& host);
// 00B56E30 (row construct), 00B57680 (row copy), 00B573A0 (named pair), 00B571E0
// (shader root), 00B58400 (constant list) and 00B57FD0 (row push-back).
void shader_constant_row_construct_00b56e30(std::uint8_t* row, const PooledString& name, std::int32_t d8, std::int32_t dc,
    std::int32_t count, std::uint32_t definition, std::int32_t register_index, StringHost& strings);
void shader_constant_row_copy_00b57680(std::uint8_t* row, const std::uint8_t* source, StringHost& strings);
void named_pair_construct_00b573a0(std::uint32_t* pair, const PooledString& name, std::uint32_t a, std::uint32_t b, StringHost& strings);
void mshd_shader_construct_00b571e0(std::uint8_t* shader, const PooledString& name, StringHost& strings);
std::uint8_t* shader_constant_list_construct_00b58400(std::uint8_t* list, const ArrayResizeFn& resize_00b580c0);
void shader_constant_rows_push_back_00b57fd0(RawArray& rows, const std::uint8_t* row, const RowCopyFn& copy_row_00b57680,
    const ArrayResizeFn& reserve_00b576f0);
// ===========================================================================
// 00B529A0 old film (packet cc13-render2b). __thiscall(state, targets, const float* params,
// char extended, float a, float b), RET 14h; caller 00B156C0. Full evidence and layout in
// docs/CC13_RENDER_2.md.
//
// RNG: the image draws every uniform from 00BD2F10 (BSP_Random_UniformFloatRange(NULL, lo,
// hi), result in ST0) and every integer from 00BD2FC0 (BSP_RandomThreads_NextU32), both on
// the SHARED GAMEPLAY STREAM 0. This model takes them from the host callbacks below in the
// image's draw order; a host must route them to a presentation stream (or keep the effect
// OFF) so that same-binary gameplay pairs stay coupled only where the image couples them.
// Nothing wires this model into a host.
class OldFilmHost {
public:
    virtual ~OldFilmHost() = default;
    virtual void* targets_color_surface_00b1f6d0(void* targets, std::uint32_t index) = 0;
    virtual std::uint32_t surface_width_vt1c(void* surface) = 0;
    virtual std::uint32_t surface_height_vt20(void* surface) = 0;
    // operator new(24h) + 00B4E840(this, "oldfilm_dust.mshd", quad_count, 0); null when the
    // allocation fails (the name temporary is still returned to the pool).
    virtual void* create_child_00b4e840(std::uint32_t quad_count) = 0;
    virtual void release(void* ref_counted) = 0;                              // +04h, slot 0
    virtual void child_set_texture_slot0(void* child, void* texture) = 0;     // 00B4CBF0 + 00B189F0
    virtual std::uint32_t child_quad_count_00b4cc00(void* child) = 0;
    virtual void child_populate_00b4d4c0(void* child, const void* records) = 0;
    virtual void child_set_frame_target_00b4e3d0(void* child, void* targets) = 0;
    virtual void child_submit_00b4def0(void* child) = 0;
    virtual float clock_now_01090ab0() = 0;                                   // [[01090AB0]+04h]
    virtual double uniform_00bd2f10(float lo, float hi) = 0;                  // image: stream 0
    virtual std::uint32_t next_u32_00bd2fc0() = 0;                            // image: stream 0
    // Image reserves of the 24h record lists (00B51FC0) and the 0Ch line states (00B52040).
    virtual void reserve_records_00b51fc0(RawArray& records, std::int32_t n) = 0;
    virtual void reserve_line_states_00b52040(RawArray& states, std::int32_t n) = 0;
};
void old_film_render_00b529a0(std::uint8_t* state, void* targets, const float* params, bool extended,
    float a, float b, OldFilmHost& host);
} // namespace bsp::cc13_render1
