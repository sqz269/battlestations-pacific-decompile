#include "bsp/render_fx_cc13.hpp"

#include "bsp/post_effects_cc13.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/native_logical_index_owner.hpp"
#include "bsp/native_logical_vertex_pool_return.hpp"
#include "bsp/native_vertex_declaration_owner.hpp"
#include "bsp/render_sprites_cc13.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include <cmath>
#include <cstring>
#include <new>
#include <string>

// Segments 84-86, packet cc13-render1. See docs/CC13_RENDER_1.md for the evidence of
// each routine; addresses below are the image functions each model stands for.

namespace bsp::cc13_render1 {

// ---------------------------------------------------------------------------
// Engine arrays.

std::int32_t apply_capacity_rule(CapacityRule rule, std::int32_t c) noexcept {
    switch (rule) {
    case CapacityRule::kDoubleMinOne: { const std::int32_t n = c * 2; return n < 2 ? 1 : n; }
    case CapacityRule::kPlusFiveMinTen: { const std::int32_t n = c + 5; return n < 11 ? 10 : n; }
    case CapacityRule::kPlusFourMinSix: { const std::int32_t n = c + 4; return n < 7 ? 6 : n; }
    case CapacityRule::kPlusEightMin16: { const std::int32_t n = c + 8; return n < 17 ? 16 : n; }
    case CapacityRule::kDoubleMin256: { const std::int32_t n = c * 2; return n < 257 ? 256 : n; }
    }
    return c;
}

void array_reallocate_exact(RawArray& a, std::int32_t n, std::size_t stride) {
    a.capacity = n;
    auto* fresh = static_cast<std::uint8_t*>(::operator new(static_cast<std::size_t>(n) * stride));
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(a.count); ++i)
        std::memcpy(fresh + i * stride, a.data + i * stride, stride);
    if (a.data) ::operator delete(a.data);
    a.data = fresh;
}

void array_grow_double_plus_two(RawArray& a, std::size_t stride) {
    array_reallocate_exact(a, a.capacity * 2 + 2, stride);
}

void array_reserve_at_least(RawArray& a, std::uint32_t n, std::size_t stride) {
    if (static_cast<std::uint32_t>(a.capacity) < n) array_reallocate_exact(a, static_cast<std::int32_t>(n), stride);
}

void array_push_back_grow(RawArray& a, const void* value, std::size_t stride) {
    if (a.count == a.capacity) array_grow_double_plus_two(a, stride);
    std::memcpy(a.data + static_cast<std::size_t>(a.count) * stride, value, stride);
    ++a.count;
}

void array_push_back_via_reserve(RawArray& a, const void* value, std::size_t stride,
    CapacityRule rule, const ArrayResizeFn& reserve) {
    if (a.count == a.capacity) reserve(a, apply_capacity_rule(rule, a.capacity));
    std::memcpy(a.data + static_cast<std::size_t>(a.count) * stride, value, stride);
    ++a.count;
}

void array_reserve_min_one(RawArray& a, std::int32_t n, std::size_t stride) {
    if (n < 1) n = 1;
    if (a.capacity >= n) return;
    auto* fresh = static_cast<std::uint8_t*>(::operator new(static_cast<std::size_t>(n) * stride));
    for (std::int32_t i = 0; i < a.count; ++i)
        std::memcpy(fresh + static_cast<std::size_t>(i) * stride, a.data + static_cast<std::size_t>(i) * stride, stride);
    ::operator delete(a.data);
    a.data = fresh;
    a.capacity = n;
}

void array_resize(RawArray& a, std::int32_t n, std::size_t stride, ResizeFill fill,
    const ArrayResizeFn& reserve) {
    if (a.capacity < n) reserve(a, n);
    if (fill != ResizeFill::kNone) {
        const int byte = fill == ResizeFill::kZero ? 0 : 0xFF;
        for (std::int32_t i = a.count; i < n; ++i)
            std::memset(a.data + static_cast<std::size_t>(i) * stride, byte, stride);
    }
    a.count = n;  // the native count-down loop above n ends at the same value
}

void array_clear_count_down(RawArray& a, const ArrayResizeFn& reserve) {
    if (a.capacity < 0) reserve(a, 0);
    a.count = 0;
}

void array_destroy_count_down(RawArray& a, const ArrayResizeFn& reserve) {
    array_clear_count_down(a, reserve);
    ::operator delete(a.data);
}

void array_clear_via_resize(RawArray& a, const ArrayResizeFn& resize) { resize(a, 0); }

void array_destroy_via_resize(RawArray& a, const ArrayResizeFn& resize) {
    resize(a, 0);
    ::operator delete(a.data);
}

std::uint8_t* array_at(const RawArray& a, std::int32_t index, std::size_t stride) noexcept {
    return a.data + static_cast<std::ptrdiff_t>(index) * static_cast<std::ptrdiff_t>(stride);
}

std::uint8_t* array_end(const RawArray& a, std::size_t stride) noexcept {
    return array_at(a, a.count, stride);
}

std::uint8_t* array_at_growing(RawArray& a, std::int32_t index, std::size_t stride,
    const ArrayResizeFn& resize) {
    if (a.count <= index) resize(a, index + 1);
    return array_at(a, index, stride);
}

void array_construct_empty(RawArray& a) noexcept { a = RawArray{}; }

void array_pop_back_destroy(RawArray& a, std::size_t stride, const ElementFn& destroy) {
    if (a.count == 0) return;
    destroy(array_at(a, a.count - 1, stride));
    --a.count;
}

void array_erase_swap_last(RawArray& a, std::uint8_t* where, std::size_t stride,
    const ElementAssignFn& assign, const ElementFn& destroy_storage) {
    std::uint8_t* last = array_at(a, a.count - 1, stride);
    if (where != last) assign(where, last);
    destroy_storage(last);
    --a.count;
}

void array_remove_at_swap_last(RawArray& a, std::int32_t index) noexcept {
    if (index == -1) return;
    auto* cells = reinterpret_cast<std::uint32_t*>(a.data);
    if (index != a.count - 1) cells[index] = cells[a.count - 1];
    --a.count;
}

void array_free_data(RawArray& a, bool clear_pointer) noexcept {
    if (!a.data) return;
    ::operator delete(a.data);
    if (clear_pointer) a.data = nullptr;
}

void inline_array_push_dword(std::uint8_t* base, std::size_t count_offset, std::uint32_t value) noexcept {
    std::int32_t count;
    std::memcpy(&count, base + count_offset, 4);
    std::memcpy(base + static_cast<std::size_t>(count) * 4, &value, 4);
    ++count;
    std::memcpy(base + count_offset, &count, 4);
}

std::uint8_t* inline_array_end_dword(std::uint8_t* base, std::size_t count_offset) noexcept {
    std::int32_t count;
    std::memcpy(&count, base + count_offset, 4);
    return base + static_cast<std::ptrdiff_t>(count) * 4;
}

// ---------------------------------------------------------------------------
// Slabs.

std::size_t slab_slot_size(SlabKind kind) noexcept {
    switch (kind) {
    case SlabKind::kSurface: return 0x38;
    case SlabKind::kTexture2D: return 0x54;
    case SlabKind::kCubeTexture: return 0x34;
    case SlabKind::kMaterialPass: return 0x8C;
    case SlabKind::kVertexDeclaration: return 0xD4;
    case SlabKind::kHardwareLayout: return 0x44;
    case SlabKind::kPhysicalBuffer: return 0x30;
    case SlabKind::kVertexStream: return 0x78;
    case SlabKind::kIndexStream: return 0x28;
    }
    return 0;
}

namespace {
std::uint16_t* slab_stack(std::uint8_t* slab, std::size_t s) noexcept {
    return reinterpret_cast<std::uint16_t*>(slab + 32 * s);
}
std::uint16_t& slab_count(std::uint8_t* slab, std::size_t s) noexcept {
    return *reinterpret_cast<std::uint16_t*>(slab + 32 * s + 0x40);
}
std::uint16_t slab_count(const std::uint8_t* slab, std::size_t s) noexcept {
    std::uint16_t c;
    std::memcpy(&c, slab + 32 * s + 0x40, 2);
    return c;
}
} // namespace

std::uint8_t* slab_pop_free_slot(std::uint8_t* slab, std::size_t s) noexcept {
    std::uint16_t& count = slab_count(slab, s);
    --count;
    return slab + static_cast<std::size_t>(slab_stack(slab, s)[count]) * s;
}

void slab_push_free_slot(std::uint8_t* slab, std::size_t s, const std::uint8_t* slot) noexcept {
    std::uint16_t& count = slab_count(slab, s);
    const std::int32_t delta = static_cast<std::int32_t>(slot - slab);
    slab_stack(slab, s)[count] = static_cast<std::uint16_t>(delta / static_cast<std::int32_t>(s));
    ++count;
}

bool slab_has_no_free_slot(const std::uint8_t* slab, std::size_t s) noexcept { return slab_count(slab, s) == 0; }
bool slab_all_slots_free(const std::uint8_t* slab, std::size_t s) noexcept { return slab_count(slab, s) == 32; }

void slab_stamp_index(std::uint8_t* slab, std::size_t s, std::uint32_t index) noexcept {
    for (std::size_t i = 0; i < 32; ++i) std::memcpy(slab + i * s + s - 4, &index, 4);
}

// ---------------------------------------------------------------------------
// Reference counting.

namespace {
void release_one(RefCounted* p) {
    if (--p->refcount == 0) p->destroy_slot0();
}
} // namespace

RefCounted** ref_slot_assign(RefCounted** slot, RefCounted* const* source) {
    RefCounted* value = *source;
    RefCounted* old = *slot;
    if (old != value) {
        *slot = value;
        if (value) ++value->refcount;
        if (old) release_one(old);
    }
    return slot;
}

void ref_record_assign_00b483a0(std::uint32_t* destination, const std::uint32_t* source) {
    ref_slot_assign(reinterpret_cast<RefCounted**>(destination), reinterpret_cast<RefCounted* const*>(source));
    destination[1] = source[1];
    destination[2] = source[2];
}

RefCounted** ref_slot_release(RefCounted** slot) {
    if (RefCounted* p = *slot) {
        release_one(p);
        *slot = nullptr;
    }
    return slot;
}

void ref_release_cdecl_00b31da0(RefCounted* object) { release_one(object); }

ComObject** com_slot_assign(ComObject** slot, ComObject* const* source) {
    ComObject* value = *source;
    ComObject* old = *slot;
    if (old != value) {
        *slot = value;
        if (value) value->add_ref();
        if (old) old->release();
    }
    return slot;
}

ComObject** com_slot_release(ComObject** slot) {
    if (ComObject* p = *slot) {
        p->release();
        *slot = nullptr;
    }
    return slot;
}

std::int32_t com_reference_count(ComObject* object) {
    if (!object) return 0;
    const auto count = static_cast<std::int32_t>(object->add_ref());
    object->release();
    return count - 1;
}

void owned_slot_delete(OwnedVirtual** slot) {
    if (OwnedVirtual* p = *slot) {
        p->deleting_destructor_slot0(1);
        *slot = nullptr;
    }
}

void pooled_string_release(PooledString& s, StoragePoolHost& pool) {
    if (s.chars) pool.return_block_00419cc0_00bd1510(s.chars, s.length + 1);
}

void pooled_string_reset_00b35270(PooledString& s, StoragePoolHost& pool) {
    if (s.length == 0) return;
    pooled_string_release(s, pool);
    s.chars = nullptr;
    s.length = 0;
}

TreeHeader* tree_initialize_empty(TreeHeader& tree, const SentinelAllocateFn& allocate_sentinel) {
    std::uint8_t* head = allocate_sentinel();
    tree.head = head;
    head[0x25] = 1;
    std::memcpy(head + 4, &head, 4);   // parent
    std::memcpy(head + 0, &head, 4);   // left
    std::memcpy(head + 8, &head, 4);   // right
    tree.size = 0;
    return &tree;
}

void tree_destruct(TreeHeader& tree, const TreeEraseRangeFn& erase_range) {
    std::uint8_t* first;
    std::memcpy(&first, tree.head, 4);
    erase_range(tree, first, tree.head);
    ::operator delete(tree.head);
    tree.head = nullptr;
    tree.size = 0;
}

// ---------------------------------------------------------------------------
// Per-address table. stride_or_offset / extra per shape:
//   arrays: element stride (informational for the count-down and resize-only shapes);
//     kClearCountDown extra = array offset in the owner (00B29230: renderer +1CF4h);
//     kPushBackViaReserve / kCapacityRule extra = CapacityRule; kResize extra = ResizeFill;
//     kInlinePush / kInlineEnd extra = count offset.
//   slabs: slot size.  refs: field offset.
//   The former accessor rows are functions after routine_row_count (cc13-rendershapes).
namespace {
constexpr RoutineRow kRows[] = {
// BEGIN ROWS
    {0x00B25E50U, Shape::kClearViaResize, 0x0, 0x0, "RendererCapabilityRecords_Clear_00b25e50"},
    {0x00B27FB0U, Shape::kClearViaResize, 0x0, 0x0, "RendererVertexPointerArray_Clear_00b27fb0"},
    {0x00B28000U, Shape::kClearViaResize, 0x0, 0x0, "RendererIndexPointerArray_Clear_00b28000"},
    {0x00B282F0U, Shape::kClearViaResize, 0x0, 0x0, "RendererVertexShaderPointerArray_Clear_00b282f0"},
    {0x00B28300U, Shape::kClearViaResize, 0x0, 0x0, "RendererPixelShaderPointerArray_Clear_00b28300"},
    {0x00B28560U, Shape::kClearViaResize, 0x0, 0x0, "RendererCapabilityDwords_Clear_00b28560"},
    {0x00B2C8D0U, Shape::kClearViaResize, 0x0, 0x0, "RendererCapabilityHeaders_Clear_00b2c8d0"},
    {0x00B305C0U, Shape::kClearViaResize, 0x0, 0x0, "NativeDeclarationRecordArray_Clear_00b305c0"},
    {0x00B305D0U, Shape::kClearViaResize, 0x0, 0x0, "NativeRenderResourceRecordArray_Clear_00b305d0"},
    {0x00B305E0U, Shape::kClearViaResize, 0x0, 0x0, "NativeEffectRecordArray_Clear_00b305e0"},
    {0x00B349F0U, Shape::kClearViaResize, 0x0, 0x0, "PointerArrayMinSix_Clear_00b349f0"},
    {0x00B34C10U, Shape::kClearViaResize, 0x0, 0x0, "PointerArrayMinTen_Clear_00b34c10"},
    {0x00B34C20U, Shape::kClearViaResize, 0x0, 0x0, "ShaderInterpolatorWordArray_Clear_00b34c20"},
    {0x00B3A6D0U, Shape::kClearViaResize, 0x0, 0x0, "NativeShaderConstantArray_Clear_Unreferenced"},
    {0x00B3E7F0U, Shape::kClearViaResize, 0x0, 0x0, "TextureSurfaceCache_Clear_00b3e7f0"},
    {0x00B41530U, Shape::kClearViaResize, 0x0, 0x0, "MaterialPassPairs_Clear_00b41530"},
    {0x00B48420U, Shape::kClearViaResize, 0x0, 0x0, "VertexElementArray_Clear_00b48420"},
    {0x00B4A9A0U, Shape::kClearViaResize, 0x0, 0x0, "NativePhysicalBufferPointerArray_Clear_00b4a9a0"},
    {0x00B28350U, Shape::kDestroyViaResize, 0x0, 0x0, "RendererCapabilityRecords_Destroy_Unreferenced"},
    {0x00B2EEE0U, Shape::kDestroyViaResize, 0x0, 0x0, "RendererCapabilityHeaders_Destroy_Unreferenced_00b2eee0"},
    {0x00B2F480U, Shape::kDestroyViaResize, 0x0, 0x0, "RendererCapabilityHeaders_Destroy_Unreferenced_00b2f480"},
    {0x00B34BF0U, Shape::kDestroyViaResize, 0x0, 0x0, "MshdLayerArrays_Destroy"},
    {0x00B34C30U, Shape::kDestroyViaResize, 0x0, 0x0, "PointerArrayMinTen_Destroy"},
    {0x00B34C50U, Shape::kDestroyViaResize, 0x0, 0x0, "ShaderInterpolatorWordArray_Destroy"},
    {0x00B3B170U, Shape::kDestroyViaResize, 0x0, 0x0, "NativeShaderConstantArray_Destroy_Unreferenced"},
    {0x00B4B2C0U, Shape::kDestroyViaResize, 0x0, 0x0, "NativePhysicalBufferPointerArray_Destroy_Unreferenced"},
    {0x00B56950U, Shape::kDestroyViaResize, 0x0, 0x0, "MshdLayerPointers_Destroy"},
    {0x00B56970U, Shape::kDestroyViaResize, 0x0, 0x0, "MshdRenderModePointers_Destroy"},
    {0x00B56990U, Shape::kDestroyViaResize, 0x0, 0x0, "MshdVersionPointers_Destroy"},
    {0x00B569B0U, Shape::kDestroyViaResize, 0x0, 0x0, "MshdLodPhasePointers_Destroy"},
    {0x00B582D0U, Shape::kDestroyViaResize, 0x0, 0x0, "ShaderStateDefinitionArray_Tidy"},
    {0x00B583C0U, Shape::kDestroyViaResize, 0x0, 0x0, "ShaderConstantList_Destroy"},
    {0x00B28110U, Shape::kClearCountDown, 0x10, 0x0, "RendererRecords16_Clear_Unreferenced"},
    {0x00B28150U, Shape::kClearCountDown, 0x14, 0x0, "RendererRecords20_Clear_Unreferenced"},
    {0x00B28190U, Shape::kClearCountDown, 0x18, 0x0, "RendererRecords24_Clear_Unreferenced"},
    {0x00B281D0U, Shape::kClearCountDown, 0x28, 0x0, "RendererRecords40_Clear_Unreferenced"},
    {0x00B29230U, Shape::kClearCountDown, 0x10, 0x1CF4, "D3D9Renderer_ClearDebugLineRecords"},
    {0x00B41390U, Shape::kClearCountDown, 0x8, 0x0, "MaterialRenderStates_Clear_Unreferenced"},
    {0x00B41420U, Shape::kClearCountDown, 0xC, 0x0, "MaterialThirdStates_Clear_Unreferenced"},
    {0x00B414B0U, Shape::kClearCountDown, 0xC, 0x0, "MaterialSamplerStates_Clear_Unreferenced"},
    {0x00B41540U, Shape::kClearCountDown, 0x8, 0x0, "ShaderStatePairs_Clear_Unreferenced"},
    {0x00B41EF0U, Shape::kDestroyCountDown, 0x8, 0x0, "ShaderStatePairs_Destroy_Unreferenced"},
    {0x00B41F40U, Shape::kDestroyCountDown, 0x8, 0x0, "ShaderStatePairs_Destroy"},
    {0x00B3DA70U, Shape::kReallocateExact, 0x4, 0x0, "PointerArray_ReallocateExact_Unreferenced_00b3da70"},
    {0x00B3DB10U, Shape::kReallocateExact, 0x4, 0x0, "PointerArray_ReallocateExact_Unreferenced_00b3db10"},
    {0x00B3DBB0U, Shape::kReallocateExact, 0x4, 0x0, "PointerArray_ReallocateExact_Unreferenced_00b3dbb0"},
    {0x00B40350U, Shape::kReallocateExact, 0x4, 0x0, "PointerArray_ReallocateExact_Unreferenced_00b40350"},
    {0x00B47AC0U, Shape::kReallocateExact, 0x4, 0x0, "PointerArray_ReallocateExact_Unreferenced_00b47ac0"},
    {0x00B47B60U, Shape::kReallocateExact, 0x4, 0x0, "PointerArray_ReallocateExact_Unreferenced_00b47b60"},
    {0x00B49760U, Shape::kReallocateExact, 0x4, 0x0, "PointerArray_ReallocateExact_Unreferenced_00b49760"},
    {0x00B49800U, Shape::kReallocateExact, 0x4, 0x0, "PointerArray_ReallocateExact_Unreferenced_00b49800"},
    {0x00B498A0U, Shape::kReallocateExact, 0x4, 0x0, "PointerArray_ReallocateExact_Unreferenced_00b498a0"},
    {0x00B3DAC0U, Shape::kGrow, 0x4, 0x0, "PointerArray_Grow_Unreferenced_00b3dac0"},
    {0x00B3DB60U, Shape::kGrow, 0x4, 0x0, "PointerArray_Grow_Unreferenced_00b3db60"},
    {0x00B3DC00U, Shape::kGrow, 0x4, 0x0, "PointerArray_Grow_Unreferenced_00b3dc00"},
    {0x00B403A0U, Shape::kGrow, 0x4, 0x0, "PointerArray_Grow_Unreferenced_00b403a0"},
    {0x00B47B10U, Shape::kGrow, 0x4, 0x0, "PointerArray_Grow_Unreferenced_00b47b10"},
    {0x00B47BB0U, Shape::kGrow, 0x4, 0x0, "PointerArray_Grow_Unreferenced_00b47bb0"},
    {0x00B497B0U, Shape::kGrow, 0x4, 0x0, "PointerArray_Grow_Unreferenced_00b497b0"},
    {0x00B49850U, Shape::kGrow, 0x4, 0x0, "PointerArray_Grow_Unreferenced_00b49850"},
    {0x00B498F0U, Shape::kGrow, 0x4, 0x0, "PointerArray_Grow_Unreferenced_00b498f0"},
    {0x00B3E880U, Shape::kReserveAtLeast, 0x4, 0x0, "PointerArray_ReserveAtLeast_Unreferenced_00b3e880"},
    {0x00B3E960U, Shape::kReserveAtLeast, 0x4, 0x0, "PointerArray_ReserveAtLeast_Unreferenced_00b3e960"},
    {0x00B3EA40U, Shape::kReserveAtLeast, 0x4, 0x0, "PointerArray_ReserveAtLeast_Unreferenced_00b3ea40"},
    {0x00B40F20U, Shape::kReserveAtLeast, 0x4, 0x0, "PointerArray_ReserveAtLeast_Unreferenced_00b40f20"},
    {0x00B481F0U, Shape::kReserveAtLeast, 0x4, 0x0, "PointerArray_ReserveAtLeast_Unreferenced_00b481f0"},
    {0x00B482D0U, Shape::kReserveAtLeast, 0x4, 0x0, "PointerArray_ReserveAtLeast_Unreferenced_00b482d0"},
    {0x00B4A690U, Shape::kReserveAtLeast, 0x4, 0x0, "PointerArray_ReserveAtLeast_Unreferenced_00b4a690"},
    {0x00B4A770U, Shape::kReserveAtLeast, 0x4, 0x0, "PointerArray_ReserveAtLeast_Unreferenced_00b4a770"},
    {0x00B4A850U, Shape::kReserveAtLeast, 0x4, 0x0, "PointerArray_ReserveAtLeast_Unreferenced_00b4a850"},
    {0x00B3E800U, Shape::kPushBackGrow, 0x4, 0x0, "PointerArray_PushBackGrow_00b3e800"},
    {0x00B3E8E0U, Shape::kPushBackGrow, 0x4, 0x0, "PointerArray_PushBackGrow_Unreferenced_00b3e8e0"},
    {0x00B3E9C0U, Shape::kPushBackGrow, 0x4, 0x0, "PointerArray_PushBackGrow_Unreferenced_00b3e9c0"},
    {0x00B40E90U, Shape::kPushBackGrow, 0x4, 0x0, "PointerArray_PushBackGrow_Unreferenced_00b40e90"},
    {0x00B48170U, Shape::kPushBackGrow, 0x4, 0x0, "PointerArray_PushBackGrow_Unreferenced_00b48170"},
    {0x00B48250U, Shape::kPushBackGrow, 0x4, 0x0, "PointerArray_PushBackGrow_Unreferenced_00b48250"},
    {0x00B4A610U, Shape::kPushBackGrow, 0x4, 0x0, "PointerArray_PushBackGrow_Unreferenced_00b4a610"},
    {0x00B4A6F0U, Shape::kPushBackGrow, 0x4, 0x0, "PointerArray_PushBackGrow_Unreferenced_00b4a6f0"},
    {0x00B4A7D0U, Shape::kPushBackGrow, 0x4, 0x0, "PointerArray_PushBackGrow_Unreferenced_00b4a7d0"},
    {0x00B27F70U, Shape::kPushBackViaReserve, 0x4, 0x0, "PointerArray_PushBack_Unreferenced"},
    {0x00B27FC0U, Shape::kPushBackViaReserve, 0x4, 0x0, "RendererIndexPointerArray_PushBack_Unreferenced"},
    {0x00B28010U, Shape::kPushBackViaReserve, 0x4, 0x0, "NativeVertexShaderRegistry_PushBack_Unreferenced"},
    {0x00B28050U, Shape::kPushBackViaReserve, 0x4, 0x0, "NativePixelShaderRegistry_PushBack_Unreferenced"},
    {0x00B4A930U, Shape::kPushBackViaReserve, 0x4, 0x0, "NativePhysicalBufferPointerArray_PushBack_Unreferenced"},
    {0x00B555A0U, Shape::kPushBackViaReserve, 0x4, 0x0, "RenderBatchPoolSlots_PushBack"},
    {0x00B56510U, Shape::kPushBackViaReserve, 0x4, 0x0, "MshdLayerPointers_PushBack"},
    {0x00B56550U, Shape::kPushBackViaReserve, 0x4, 0x0, "MshdRenderModePointers_PushBack"},
    {0x00B56590U, Shape::kPushBackViaReserve, 0x4, 0x0, "MshdVersionPointers_PushBack"},
    {0x00B565D0U, Shape::kPushBackViaReserve, 0x4, 0x0, "MshdLodPhasePointers_PushBack"},
    {0x00B3E270U, Shape::kPushBackViaReserve, 0x8, 0x0, "TextureSurfaceCache_PushBack_Unreferenced"},
    {0x00B41350U, Shape::kPushBackViaReserve, 0x8, 0x0, "MaterialRenderStates_PushBack_Unreferenced"},
    {0x00B414F0U, Shape::kPushBackViaReserve, 0x8, 0x0, "MaterialPassPairs_PushBack_Unreferenced"},
    {0x00B4EDE0U, Shape::kPushBackViaReserve, 0x8, 0x0, "DistortionRecords_PushBack8"},
    {0x00B413D0U, Shape::kPushBackViaReserve, 0xC, 0x0, "MaterialThirdStates_PushBack_Unreferenced"},
    {0x00B41460U, Shape::kPushBackViaReserve, 0xC, 0x0, "MaterialSamplerStates_PushBack_Unreferenced"},
    {0x00B52120U, Shape::kPushBackViaReserve, 0xC, 0x0, "TextureVector12_PushBack"},
    {0x00B52350U, Shape::kPushBackViaReserve, 0xC, 0x0, "TextureVector12_PushBackB"},
    {0x00B520D0U, Shape::kPushBackViaReserve, 0x24, 0x0, "TextureVector24_PushBack"},
    {0x00B349B0U, Shape::kPushBackViaReserve, 0x2, 0x0, "ShaderInterpolatorWordArray_PushBack_Unreferenced"},
    {0x00B34970U, Shape::kPushBackViaReserve, 0x4, 0x1, "PointerArrayMinTen_PushBack_Unreferenced"},
    {0x00B564D0U, Shape::kPushBackViaReserve, 0x8, 0x1, "ShaderStatePairs_PushBack_00b564d0"},
    {0x00B41050U, Shape::kPushBackViaReserve, 0x4, 0x2, "PointerArrayMinSix_PushBack_Unreferenced"},
    {0x00B51C20U, Shape::kPushBackViaReserve, 0x4, 0x4, "RenderBatchEntryPointers_PushBack"},
    {0x00B34740U, Shape::kCapacityRule, 0x0, 0x1, "TMPL_PodArray_GrowCapacityMin10_Unreferenced_00b34740"},
    {0x00B40D60U, Shape::kCapacityRule, 0x0, 0x2, "TMPL_PodArray_GrowCapacityMin6_Unreferenced_00b40d60"},
    {0x00B34750U, Shape::kCapacityRule, 0x0, 0x3, "TMPL_PodArray_GrowCapacityMin16_Unreferenced_00b34750"},
    {0x00B56200U, Shape::kReserveMinOne, 0x4, 0x0, "MshdLayerPointers_Reserve"},
    {0x00B56260U, Shape::kReserveMinOne, 0x4, 0x0, "MshdRenderModePointers_Reserve"},
    {0x00B562C0U, Shape::kReserveMinOne, 0x4, 0x0, "MshdVersionPointers_Reserve"},
    {0x00B56320U, Shape::kReserveMinOne, 0x4, 0x0, "MshdLodPhasePointers_Reserve"},
    {0x00B34760U, Shape::kResize, 0x4, 0x1, "PointerArrayMinSix_Resize"},
    {0x00B34A00U, Shape::kResize, 0x4, 0x1, "PointerArrayMinTen_Resize"},
    {0x00B56380U, Shape::kResize, 0x4, 0x1, "MshdLayerPointers_Resize"},
    {0x00B563D0U, Shape::kResize, 0x4, 0x1, "MshdRenderModePointers_Resize"},
    {0x00B56420U, Shape::kResize, 0x4, 0x1, "MshdVersionPointers_Resize"},
    {0x00B56470U, Shape::kResize, 0x4, 0x1, "MshdLodPhasePointers_Resize"},
    {0x00B34A50U, Shape::kResize, 0x2, 0x2, "ShaderInterpolatorWordArray_Resize"},
    {0x00B40D70U, Shape::kResize, 0x8, 0x0, "MaterialRenderStates_Resize_00b40d70"},
    {0x00B40DA0U, Shape::kResize, 0xC, 0x0, "MaterialThirdStates_Resize_00b40da0"},
    {0x00B40DD0U, Shape::kResize, 0xC, 0x0, "MaterialSamplerStates_Resize_00b40dd0"},
    {0x00B40E50U, Shape::kResize, 0x8, 0x0, "ShaderStatePairs_Resize_00b40e50"},
    {0x00B34590U, Shape::kAt, 0x8, 0x0, "TMPL_PodArray_At8_Unreferenced_00b34590"},
    {0x00B347B0U, Shape::kAt, 0x8, 0x0, "TMPL_PodArray_At8_Unreferenced_00b347b0"},
    {0x00B3D850U, Shape::kAt, 0x8, 0x0, "TMPL_PodArray_At8_Unreferenced_00b3d850"},
    {0x00B408E0U, Shape::kAt, 0x8, 0x0, "TMPL_PodArray_At8_Unreferenced_00b408e0"},
    {0x00B40AB0U, Shape::kAt, 0x8, 0x0, "TMPL_PodArray_At8_Unreferenced_00b40ab0"},
    {0x00B40C70U, Shape::kAt, 0x8, 0x0, "TMPL_PodArray_At8_Unreferenced_00b40c70"},
    {0x00B345B0U, Shape::kAt, 0x4, 0x0, "TMPL_PodArray_At4_Unreferenced_00b345b0"},
    {0x00B345D0U, Shape::kAt, 0x4, 0x0, "TMPL_PodArray_At4_Unreferenced_00b345d0"},
    {0x00B345E0U, Shape::kAt, 0x4, 0x0, "TMPL_PodArray_At4_Unreferenced_00b345e0"},
    {0x00B40950U, Shape::kAt, 0x4, 0x0, "TMPL_PodArray_At4_Unreferenced_00b40950"},
    {0x00B34600U, Shape::kAt, 0x2, 0x0, "TMPL_PodArray_At2_Unreferenced_00b34600"},
    {0x00B40B30U, Shape::kAt, 0xC, 0x0, "TMPL_PodArray_At12_Unreferenced_00b40b30"},
    {0x00B40BD0U, Shape::kAt, 0xC, 0x0, "TMPL_PodArray_At12_Unreferenced_00b40bd0"},
    {0x00B40960U, Shape::kAt, 0x60, 0x0, "TMPL_PodArray_At96_Unreferenced_00b40960"},
    {0x00B47920U, Shape::kAt, 0x14, 0x0, "VertexElementArray_At_Unreferenced_00b47920"},
    {0x00B47930U, Shape::kAt, 0x14, 0x0, "VertexElementArray_At_Unreferenced_00b47930"},
    {0x00B2C8A0U, Shape::kAtGrowing, 0xC, 0x0, "RendererCapabilityHeaders_At"},
    {0x00B344B0U, Shape::kEnd, 0x20, 0x0, "TMPL_PodArray32_End_Unreferenced_00b344b0"},
    {0x00B40110U, Shape::kEnd, 0x20, 0x0, "TMPL_PodArray32_End_Unreferenced_00b40110"},
    {0x00B40880U, Shape::kInlinePush, 0x4, 0x80, "TMPL_InlineArray32_PushBack_Unreferenced_00b40880"},
    {0x00B40920U, Shape::kInlinePush, 0x4, 0x10, "TMPL_InlineArray4_PushBack_Unreferenced_00b40920"},
    {0x00B408A0U, Shape::kInlineEnd, 0x4, 0x80, "TMPL_InlineArray32_End_Unreferenced_00b408a0"},
    {0x00B40900U, Shape::kInlineAt, 0x8, 0x0, "TMPL_InlineArray_At8_Unreferenced_00b40900"},
    {0x00B40910U, Shape::kInlineAt, 0x4, 0x0, "TMPL_InlineArray_At4_Unreferenced_00b40910"},
    {0x00B40940U, Shape::kInlineAt, 0x4, 0x0, "TMPL_InlineArray_At4_Unreferenced_00b40940"},
    {0x00B47940U, Shape::kInlineAt, 0xC, 0x0, "TMPL_InlineArray_At12_Unreferenced_00b47940"},
    {0x00B345A0U, Shape::kConstructEmpty, 0x0, 0x0, "TMPL_PodArray_Construct_Unreferenced_00b345a0"},
    {0x00B345C0U, Shape::kConstructEmpty, 0x0, 0x0, "TMPL_PodArray_Construct_Unreferenced_00b345c0"},
    {0x00B345F0U, Shape::kConstructEmpty, 0x0, 0x0, "TMPL_PodArray_Construct_Unreferenced_00b345f0"},
    {0x00B34610U, Shape::kConstructEmpty, 0x0, 0x0, "TMPL_PodArray_Construct_Unreferenced_00b34610"},
    {0x00B3D360U, Shape::kConstructEmpty, 0x0, 0x0, "TMPL_PodArray_Construct_Unreferenced_00b3d360"},
    {0x00B3D3A0U, Shape::kConstructEmpty, 0x0, 0x0, "TMPL_PodArray_Construct_Unreferenced_00b3d3a0"},
    {0x00B3D3E0U, Shape::kConstructEmpty, 0x0, 0x0, "TMPL_PodArray_Construct_Unreferenced_00b3d3e0"},
    {0x00B3D840U, Shape::kConstructEmpty, 0x0, 0x0, "TMPL_PodArray_Construct_Unreferenced_00b3d840"},
    {0x00B402A0U, Shape::kConstructEmpty, 0x0, 0x0, "TMPL_PodArray_Construct_Unreferenced_00b402a0"},
    {0x00B408B0U, Shape::kConstructEmpty, 0x0, 0x0, "TMPL_PodArray_Construct_Unreferenced_00b408b0"},
    {0x00B408C0U, Shape::kConstructEmpty, 0x0, 0x0, "TMPL_PodArray_Construct_Unreferenced_00b408c0"},
    {0x00B408D0U, Shape::kConstructEmpty, 0x0, 0x0, "TMPL_PodArray_Construct_Unreferenced_00b408d0"},
    {0x00B408F0U, Shape::kConstructEmpty, 0x0, 0x0, "TMPL_PodArray_Construct_Unreferenced_00b408f0"},
    {0x00B41F30U, Shape::kConstructEmpty, 0x0, 0x0, "TMPL_PodArray_Construct_Unreferenced_00b41f30"},
    {0x00B47810U, Shape::kConstructEmpty, 0x0, 0x0, "TMPL_PodArray_Construct_Unreferenced_00b47810"},
    {0x00B47850U, Shape::kConstructEmpty, 0x0, 0x0, "TMPL_PodArray_Construct_Unreferenced_00b47850"},
    {0x00B49030U, Shape::kConstructEmpty, 0x0, 0x0, "TMPL_PodArray_Construct_Unreferenced_00b49030"},
    {0x00B49070U, Shape::kConstructEmpty, 0x0, 0x0, "TMPL_PodArray_Construct_Unreferenced_00b49070"},
    {0x00B490B0U, Shape::kConstructEmpty, 0x0, 0x0, "TMPL_PodArray_Construct_Unreferenced_00b490b0"},
    {0x00B49650U, Shape::kConstructEmpty, 0x0, 0x0, "TMPL_PodArray_Construct_Unreferenced_00b49650"},
    {0x00B30210U, Shape::kPopBackDestroy, 0x2C, 0x0, "NativeDeclarationRecordArray_PopBack_Unreferenced"},
    {0x00B30230U, Shape::kPopBackDestroy, 0x2C, 0x0, "RenderResourceRecordArray_PopBack_Unreferenced"},
    {0x00B30250U, Shape::kPopBackDestroy, 0x2C, 0x0, "MaterialEffectCacheEntryArray_PopBack_Unreferenced"},
    {0x00B47820U, Shape::kFreeData, 0x0, 0x0, "PodVector_FreeData_00b47820"},
    {0x00B47860U, Shape::kFreeData, 0x0, 0x0, "PodVector_FreeData_00b47860"},
    {0x00B49040U, Shape::kFreeData, 0x0, 0x0, "PodVector_FreeData_00b49040"},
    {0x00B49080U, Shape::kFreeData, 0x0, 0x0, "PodVector_FreeData_00b49080"},
    {0x00B490C0U, Shape::kFreeData, 0x0, 0x0, "PodVector_FreeData_00b490c0"},
    {0x00B4D180U, Shape::kFreeDataAndClear, 0x0, 0x0, "PodVector_FreeData"},
    {0x00B61DA0U, Shape::kFreeDataAndClear, 0x0, 0x0, "PodVector_FreeDataB"},
    {0x00B4A970U, Shape::kRemoveAtSwapLast, 0x4, 0x0, "PointerArray_RemoveAtSwapLast_Unreferenced"},
    {0x00B3D160U, Shape::kSlabPop, 0x38, 0x0, "D3D9SurfaceSlab_PopFreeSlot_Unreferenced"},
    {0x00B3D220U, Shape::kSlabPop, 0x54, 0x0, "D3D9Texture2DSlab_PopFreeSlot_Unreferenced"},
    {0x00B3D2E0U, Shape::kSlabPop, 0x34, 0x0, "CubeTextureSlab_PopFreeSlot_Unreferenced"},
    {0x00B40220U, Shape::kSlabPop, 0x8C, 0x0, "MaterialPassSlab_PopFreeSlot_Unreferenced"},
    {0x00B476D0U, Shape::kSlabPop, 0xD4, 0x0, "NativeVertexDeclarationSlab_PopFreeSlot_Unreferenced"},
    {0x00B47790U, Shape::kSlabPop, 0x44, 0x0, "HardwareLayoutSlab_PopFreeSlot_Unreferenced"},
    {0x00B48E30U, Shape::kSlabPop, 0x30, 0x0, "NativePhysicalBufferSlab_PopFreeSlot_Unreferenced"},
    {0x00B48EF0U, Shape::kSlabPop, 0x78, 0x0, "VertexStreamSlab_PopFreeSlot_Unreferenced"},
    {0x00B48FB0U, Shape::kSlabPop, 0x28, 0x0, "LogicalIndexSlab_PopFreeSlot_Unreferenced"},
    {0x00B3D190U, Shape::kSlabPush, 0x38, 0x0, "D3D9SurfaceSlab_PushFreeSlot_Unreferenced"},
    {0x00B3D250U, Shape::kSlabPush, 0x54, 0x0, "D3D9Texture2DSlab_PushFreeSlot_Unreferenced"},
    {0x00B3D310U, Shape::kSlabPush, 0x34, 0x0, "CubeTextureSlab_PushFreeSlot_Unreferenced"},
    {0x00B40250U, Shape::kSlabPush, 0x8C, 0x0, "MaterialPassSlab_PushFreeSlot_Unreferenced"},
    {0x00B47700U, Shape::kSlabPush, 0xD4, 0x0, "NativeVertexDeclarationSlab_PushFreeSlot_Unreferenced"},
    {0x00B477C0U, Shape::kSlabPush, 0x44, 0x0, "HardwareLayoutSlab_PushFreeSlot_Unreferenced"},
    {0x00B48E60U, Shape::kSlabPush, 0x30, 0x0, "NativePhysicalBufferSlab_PushFreeSlot_Unreferenced"},
    {0x00B48F20U, Shape::kSlabPush, 0x78, 0x0, "VertexStreamSlab_PushFreeSlot_Unreferenced"},
    {0x00B48FE0U, Shape::kSlabPush, 0x28, 0x0, "LogicalIndexSlab_PushFreeSlot_Unreferenced"},
    {0x00B3D1D0U, Shape::kSlabHasNoFree, 0x38, 0x0, "D3D9SurfaceSlab_HasNoFreeSlot_Unreferenced"},
    {0x00B3D290U, Shape::kSlabHasNoFree, 0x54, 0x0, "D3D9Texture2DSlab_HasNoFreeSlot_Unreferenced"},
    {0x00B3D350U, Shape::kSlabHasNoFree, 0x34, 0x0, "CubeTextureSlab_HasNoFreeSlot_Unreferenced"},
    {0x00B40290U, Shape::kSlabHasNoFree, 0x8C, 0x0, "MaterialPassSlab_HasNoFreeSlot_Unreferenced"},
    {0x00B47740U, Shape::kSlabHasNoFree, 0xD4, 0x0, "NativeVertexDeclarationSlab_HasNoFreeSlot_Unreferenced"},
    {0x00B47800U, Shape::kSlabHasNoFree, 0x44, 0x0, "HardwareLayoutSlab_HasNoFreeSlot_Unreferenced"},
    {0x00B48EA0U, Shape::kSlabHasNoFree, 0x30, 0x0, "NativePhysicalBufferSlab_HasNoFreeSlot_Unreferenced"},
    {0x00B48F60U, Shape::kSlabHasNoFree, 0x78, 0x0, "VertexStreamSlab_HasNoFreeSlot_Unreferenced"},
    {0x00B49020U, Shape::kSlabHasNoFree, 0x28, 0x0, "LogicalIndexSlab_HasNoFreeSlot_Unreferenced"},
    {0x00B3E360U, Shape::kSlabAllFree, 0x38, 0x0, "D3D9SurfaceSlab_AllSlotsFree_Unreferenced"},
    {0x00B3E4E0U, Shape::kSlabAllFree, 0x54, 0x0, "D3D9Texture2DSlab_AllSlotsFree_Unreferenced"},
    {0x00B3E660U, Shape::kSlabAllFree, 0x34, 0x0, "CubeTextureSlab_AllSlotsFree_Unreferenced"},
    {0x00B40A30U, Shape::kSlabAllFree, 0x8C, 0x0, "MaterialPassSlab_AllSlotsFree_Unreferenced"},
    {0x00B47EA0U, Shape::kSlabAllFree, 0xD4, 0x0, "NativeVertexDeclarationSlab_AllSlotsFree_Unreferenced"},
    {0x00B48020U, Shape::kSlabAllFree, 0x44, 0x0, "HardwareLayoutSlab_AllSlotsFree_Unreferenced"},
    {0x00B4A240U, Shape::kSlabAllFree, 0x30, 0x0, "NativePhysicalBufferSlab_AllSlotsFree_Unreferenced"},
    {0x00B4A3C0U, Shape::kSlabAllFree, 0x78, 0x0, "VertexStreamSlab_AllSlotsFree_Unreferenced"},
    {0x00B4A540U, Shape::kSlabAllFree, 0x28, 0x0, "LogicalIndexSlab_AllSlotsFree_Unreferenced"},
    {0x00B3E340U, Shape::kSlabStamp, 0x38, 0x0, "D3D9SurfaceSlab_StampSlabIndex_Unreferenced"},
    {0x00B3E4C0U, Shape::kSlabStamp, 0x54, 0x0, "D3D9Texture2DSlab_StampSlabIndex_Unreferenced"},
    {0x00B3E640U, Shape::kSlabStamp, 0x34, 0x0, "CubeTextureSlab_StampSlabIndex_Unreferenced"},
    {0x00B40A10U, Shape::kSlabStamp, 0x8C, 0x0, "MaterialPassSlab_StampSlabIndex_Unreferenced"},
    {0x00B47E80U, Shape::kSlabStamp, 0xD4, 0x0, "NativeVertexDeclarationSlab_StampSlabIndex_Unreferenced"},
    {0x00B48000U, Shape::kSlabStamp, 0x44, 0x0, "HardwareLayoutSlab_StampSlabIndex_Unreferenced"},
    {0x00B4A220U, Shape::kSlabStamp, 0x30, 0x0, "NativePhysicalBufferSlab_StampSlabIndex_Unreferenced"},
    {0x00B4A3A0U, Shape::kSlabStamp, 0x78, 0x0, "VertexStreamSlab_StampSlabIndex_Unreferenced"},
    {0x00B4A520U, Shape::kSlabStamp, 0x28, 0x0, "LogicalIndexSlab_StampSlabIndex_Unreferenced"},
    {0x00B3C580U, Shape::kRefAssign, 0x0, 0x0, "RefPtr_Assign_Unreferenced_00b3c580"},
    {0x00B40FB0U, Shape::kRefAssign, 0x0, 0x0, "RefPtr_Assign_Unreferenced_00b40fb0"},
    {0x00B41580U, Shape::kRefAssign, 0x0, 0x0, "RefPtr_Assign_Unreferenced_00b41580"},
    {0x00B41650U, Shape::kRefAssign, 0x0, 0x0, "RefPtr_Assign_Unreferenced_00b41650"},
    {0x00B41690U, Shape::kRefAssign, 0x0, 0x0, "RefPtr_Assign_Unreferenced_00b41690"},
    {0x00B416D0U, Shape::kRefAssign, 0x0, 0x0, "RefPtr_Assign_Unreferenced_00b416d0"},
    {0x00B4A8B0U, Shape::kRefAssign, 0x0, 0x0, "RefPtr_Assign_Unreferenced_00b4a8b0"},
    {0x00B4A8F0U, Shape::kRefAssign, 0x0, 0x0, "RefPtr_Assign_Unreferenced_00b4a8f0"},
    {0x00B3EAA0U, Shape::kRefRelease, 0x0, 0x0, "RefPtr_Release_Unreferenced_00b3eaa0"},
    {0x00B40F80U, Shape::kRefRelease, 0x0, 0x0, "RefPtr_Release_Unreferenced_00b40f80"},
    {0x00B415C0U, Shape::kRefRelease, 0x0, 0x0, "RefPtr_Release_Unreferenced_00b415c0"},
    {0x00B415F0U, Shape::kRefRelease, 0x0, 0x0, "RefPtr_Release_Unreferenced_00b415f0"},
    {0x00B41620U, Shape::kRefRelease, 0x0, 0x0, "RefPtr_Release_Unreferenced_00b41620"},
    {0x00B52240U, Shape::kRefRelease, 0x0, 0x0, "RefCountedPtr_ResetB"},
    {0x00B5EBE0U, Shape::kRefRelease, 0x0, 0x0, "RefCountedPtr_ResetC"},
    {0x00B5EC10U, Shape::kRefRelease, 0x0, 0x0, "RefCountedPtr_ResetD"},
    {0x00B627C0U, Shape::kRefRelease, 0x4, 0x0, "RefCountedMember4_Reset"},
    {0x00B4AAC0U, Shape::kRefFieldAssign, 0x58, 0x0, "RefPtrField58_Assign_Unreferenced_00b4aac0"},
    {0x00B4AB40U, Shape::kRefFieldAssign, 0x8, 0x0, "RefPtrField8_Assign_Unreferenced_00b4ab40"},
    {0x00B4AB00U, Shape::kRefFieldCopy, 0x58, 0x0, "RefPtrField58_CopyFrom_Unreferenced_00b4ab00"},
    {0x00B3D470U, Shape::kComAssign, 0x0, 0x0, "ComPtr_Assign_Unreferenced_00b3d470"},
    {0x00B490F0U, Shape::kComAssign, 0x0, 0x0, "ComPtr_Assign_Unreferenced_00b490f0"},
    {0x00B49120U, Shape::kComAssign, 0x0, 0x0, "ComPtr_Assign_Unreferenced_00b49120"},
    {0x00B3D450U, Shape::kComRelease, 0x0, 0x0, "ComPtr_Release_Unreferenced_00b3d450"},
    {0x00B3D4A0U, Shape::kComRelease, 0x0, 0x0, "ComPtr_Release_Unreferenced_00b3d4a0"},
    {0x00B5E630U, Shape::kComRelease, 0x0, 0x0, "ComPtr_ReleaseA"},
    {0x00B5E6B0U, Shape::kComRelease, 0x0, 0x0, "ComPtr_ReleaseB"},
    {0x00B5FCF0U, Shape::kComRelease, 0x0, 0x0, "ComPtr_ReleaseC"},
    {0x00B60050U, Shape::kComRelease, 0x0, 0x0, "ComPtr_ReleaseD"},
    {0x00B60D10U, Shape::kComRelease, 0x0, 0x0, "ComPtr_ReleaseE"},
    {0x00B3D420U, Shape::kComRefCount, 0x0, 0x0, "ComObject_GetRefCount_Unreferenced"},
    {0x00B5E650U, Shape::kComRefCount, 0x0, 0x0, "ComObject_RefCountProbeA"},
    {0x00B5E6D0U, Shape::kComRefCount, 0x0, 0x0, "ComObject_RefCountProbeB"},
    {0x00B60070U, Shape::kComRefCount, 0x0, 0x0, "ComObject_RefCountProbeC"},
    {0x00B34530U, Shape::kOwnedDelete, 0x0, 0x0, "OwnedObject_Delete_Unreferenced_00b34530"},
    {0x00B402E0U, Shape::kOwnedDelete, 0x0, 0x0, "OwnedObject_Delete_Unreferenced_00b402e0"},
    {0x00B56A00U, Shape::kOwnedDelete, 0x0, 0x0, "MshdOwnedPtr_DeleteA"},
    {0x00B56C60U, Shape::kOwnedDelete, 0x0, 0x0, "MshdOwnedPtr_DeleteB"},
    {0x00B57290U, Shape::kOwnedDelete, 0x0, 0x0, "MshdOwnedPtr_DeleteC"},
    {0x00B58490U, Shape::kOwnedDelete, 0x0, 0x0, "MshdOwnedPtr_DeleteD"},
    {0x00B3A720U, Shape::kPointerDelete, 0x0, 0x0, "ShaderBinaryCachePointer_Delete_Unreferenced"},
    {0x00B44FA0U, Shape::kPointerDelete, 0x0, 0x0, "MaterialPassBindingPointer_Delete_Unreferenced"},
    {0x00B2ABB0U, Shape::kPooledStringRelease, 0x0, 0x0, "PooledString_ReleaseStorage"},
    {0x00B34D70U, Shape::kPooledStringRelease, 0x0, 0x0, "PooledString_ReleaseStorage_00b34d70"},
    {0x00B34E00U, Shape::kPooledStringRelease, 0x0, 0x0, "PooledString_ReleaseStorage_Unreferenced_00b34e00"},
    {0x00B56C10U, Shape::kPooledStringRelease, 0x0, 0x0, "NativeString_ReturnStorage"},
    {0x00B56AF0U, Shape::kPooledStringRelease, 0x4, 0x0, "MshdNode_ReturnNameStorage"},
    {0x00B34CC0U, Shape::kPooledStringRelease, 0x14, 0x0, "PooledStringAt14_ReleaseStorage"},
    {0x00B28310U, Shape::kTreeInitEmpty, 0x0, 0x0, "HardwareLayoutTree_InitEmpty_Unreferenced"},
    {0x00B29CA0U, Shape::kTreeInitEmpty, 0x0, 0x0, "HardwareLayoutTree_Construct_Unreferenced_00b29ca0"},
    {0x00B2F8E0U, Shape::kTreeInitEmpty, 0x0, 0x0, "HardwareLayoutTree_Construct_Unreferenced_00b2f8e0"},
    {0x00B2F630U, Shape::kTreeDestruct, 0x0, 0x0, "HardwareLayoutTree_Destruct_Unreferenced_00b2f630"},
    {0x00B2F800U, Shape::kTreeDestruct, 0x0, 0x0, "HardwareLayoutTree_Destruct_Unreferenced_00b2f800"},
    {0x00B2F8A0U, Shape::kTreeDestruct, 0x0, 0x0, "HardwareLayoutTree_Destruct_Unreferenced_00b2f8a0"},
// END ROWS
};
} // namespace

const RoutineRow* find_routine_row(std::uint32_t address) noexcept {
    for (const RoutineRow& row : kRows)
        if (row.address == address) return &row;
    return nullptr;
}

std::size_t routine_row_count() noexcept { return sizeof(kRows) / sizeof(kRows[0]); }

// ===========================================================================
// Packet cc13-rendershapes: the former accessor rows of kRows (getters, setter, record
// addressing, pair construction, locked getters, no-ops, constants and forwards), one
// function per address. Each body was re-read from the disk bytes (Capstone).

namespace {
std::uint32_t dword_at(const std::uint8_t* self, std::uint32_t offset) {
    std::uint32_t v;
    std::memcpy(&v, self + offset, 4);
    return v;
}
std::uint8_t* pointer_at(const std::uint8_t* self, std::uint32_t offset) {
    std::uint8_t* p;
    std::memcpy(&p, self + offset, 4);
    return p;
}
// The lock object: a CRITICAL_SECTION (18h bytes) followed by the depth dword at +18h.
CRITICAL_SECTION* locked_enter(const std::uint8_t* self, std::uint32_t lock_offset) {
    auto* const lock = pointer_at(self, lock_offset);
    EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(lock));
    std::uint32_t depth;
    std::memcpy(&depth, lock + 0x18, 4);
    ++depth;
    std::memcpy(lock + 0x18, &depth, 4);
    return reinterpret_cast<CRITICAL_SECTION*>(lock);
}
void locked_leave(const std::uint8_t* self, std::uint32_t lock_offset) {
    // The image reloads the lock pointer from the owner after the read.
    auto* const lock = pointer_at(self, lock_offset);
    std::uint32_t depth;
    std::memcpy(&depth, lock + 0x18, 4);
    --depth;
    std::memcpy(lock + 0x18, &depth, 4);
    LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(lock));
}
} // namespace

// 00B405D0: MOV EAX,[ESP+4]; SHL EAX,4; LEA EAX,[EAX+ECX+4]; RET 4.
std::uint8_t* float4_block_at_00b405d0(std::uint8_t* self, std::uint32_t index) {
    return self + static_cast<std::uint32_t>((index << 4) + 4u);
}
// 00B40050: MOV EAX,[ESP+4]; SHL EAX,6; LEA EAX,[EAX+ECX+144h]; RET 4.
std::uint8_t* shader_effect_pass_record_00b40050(std::uint8_t* self, std::uint32_t index) {
    return self + static_cast<std::uint32_t>((index << 6) + 0x144u);
}
// 00B47C80: MOV EAX,[ESP+4]; MOV ECX,[ECX+0Ch]; LEA EAX,[EAX+EAX*4]; LEA EAX,[ECX+EAX*4];
// RET 4. The element array's base pointer is at +0Ch; elements are 14h bytes.
std::uint8_t* vertex_declaration_element_at_00b47c80(const std::uint8_t* self,
                                                     std::uint32_t index) {
    return pointer_at(self, 0x0c) + static_cast<std::uint32_t>(index * 0x14u);
}

// 00B34D60 and 00B4EC30 (identical): MOV EAX,ECX; MOV [EAX],0; MOV [EAX+4],0; RET.
void* pooled_string_construct_empty_00b34d60(void* self) {
    std::memset(self, 0, 8);
    return self;
}
void* pod_pair_zero_init_00b4ec30(void* self) {
    std::memset(self, 0, 8);
    return self;
}

// MOV EAX,[ECX+offset]; RET.
std::uint32_t d3d9_surface_sample_count_00b3cd30(const std::uint8_t* self) { return dword_at(self, 0x24); }
std::uint32_t cube_texture_reported_width_00b3cf70(const std::uint8_t* self) { return dword_at(self, 0x24); }
std::uint32_t cube_texture_reported_height_00b3cf80(const std::uint8_t* self) { return dword_at(self, 0x28); }
std::uint32_t logical_vertex_stream_flags_00b48cc0(const std::uint8_t* self) { return dword_at(self, 0x60); }
std::uint32_t logical_index_stream_flags_00b48d80(const std::uint8_t* self) { return dword_at(self, 0x10); }
std::uint32_t logical_index_stream_count_00b48d90(const std::uint8_t* self) { return dword_at(self, 0x14); }
std::uint32_t logical_index_stream_format_00b48da0(const std::uint8_t* self) { return dword_at(self, 0x18); }
// 00B3CE20: MOV EAX,[ESP+4]; MOV [ECX+24h],EAX; RET 4.
void d3d9_surface_set_sample_count_00b3ce20(std::uint8_t* self, std::uint32_t value) {
    std::memcpy(self + 0x24, &value, 4);
}

// 00B3FF70: MOVZX EAX,BYTE [ECX+74h]; RET.
std::uint8_t shader_reflection_owner_byte74_00b3ff70(const std::uint8_t* self) { return self[0x74]; }
// 00B400B0: MOV AL,[ECX+13Ch]; RET.
std::uint8_t material_effect_byte13c_00b400b0(const std::uint8_t* self) { return self[0x13c]; }

// 00B5DF90 / 00B5DFC0: EDI = [ESI+48h]; CALL [00CE2218] EnterCriticalSection(EDI);
// ADD [EDI+18h],1; EDI = [ESI+40h] / [ESI+3Ch]; ESI = [ESI+48h]; ADD [ESI+18h],-1;
// CALL [00CE2210] LeaveCriticalSection(ESI); EAX = EDI; RET.
std::uint32_t locked_object_field40_00b5df90(const std::uint8_t* self) {
    locked_enter(self, 0x48);
    const std::uint32_t value = dword_at(self, 0x40);
    locked_leave(self, 0x48);
    return value;
}
std::uint32_t locked_object_field3c_00b5dfc0(const std::uint8_t* self) {
    locked_enter(self, 0x48);
    const std::uint32_t value = dword_at(self, 0x3c);
    locked_leave(self, 0x48);
    return value;
}
// 00B5DFF0 / 00B5E020: the same over the lock at +4Ch, BL = [ESI+44h] / [ESI+45h],
// AL = BL after the leave.
std::uint8_t locked_object_byte44_00b5dff0(const std::uint8_t* self) {
    locked_enter(self, 0x4c);
    const std::uint8_t value = self[0x44];
    locked_leave(self, 0x4c);
    return value;
}
std::uint8_t locked_object_byte45_00b5e020(const std::uint8_t* self) {
    locked_enter(self, 0x4c);
    const std::uint8_t value = self[0x45];
    locked_leave(self, 0x4c);
    return value;
}

// C3: RET.
void physical_index_buffer_noop_slot10_00b4b480(void*) {}
void physical_vertex_buffer_noop_slot10_00b4b520(void*) {}
void post_effect_owner_noop_slot0c_00b4ec40(void*) {}
void bloom_owner_noop_slot0c_00b54cc0(void*) {}

// 00B3CE40 XOR EAX,EAX; 00B3CF60 MOV EAX,1; 00B3D080 MOV EAX,2; each RET.
std::uint32_t d3d9_texture2d_kind_00b3ce40(const void*) { return 0; }
std::uint32_t cube_texture_kind_00b3cf60(const void*) { return 1; }
std::uint32_t volume_texture_kind_00b3d080(const void*) { return 2; }

// 00B47C00: PUSH ECX; MOV ECX,0108FD38; CALL 00B47950; RET.
void vertex_declaration_pool_return_raw_slot_unwind_00b47c00(void* pool_0108fd38, void* self) {
    return_native_vertex_declaration_slot_00b47950(pool_0108fd38, self);
}
// 00B49960: PUSH ECX; MOV ECX,0108FE18; CALL 00B49570; RET.
void logical_vertex_pool_return_slot_unwind_00b49960(void* pool_0108fe18, void* self) {
    return_native_logical_vertex_pool_slot_00b49570(pool_0108fe18, self);
}
// 00B49970: PUSH ECX; MOV ECX,0108FE50; CALL 00B495E0; RET.
void logical_index_stream_pool_return_slot_unwind_00b49970(void* pool_0108fe50, void* self) {
    return_native_logical_index_slot_00b495e0(pool_0108fe50, self);
}
// 00B47C10: PUSH ECX; MOV ECX,0108FD70; CALL 00B479C0; RET.
void hardware_layout_return_raw_slot_00b47c10(std::uint8_t* pool_0108fd70, void* self,
                                              cc13_render_sprites::SlabPoolHost& host) {
    cc13_render_sprites::return_layout_slot_00b479c0(pool_0108fd70, self, host);
}
// 00B488D0: MOV ECX,0108FD70; JMP 00B48780.
void* hardware_layout_pool_allocate_raw_slot_stub_00b488d0(
    std::uint8_t* pool_0108fd70, cc13_render_sprites::SlabPoolHost& host) {
    return cc13_render_sprites::allocate_layout_slot_00b48780(pool_0108fd70, host);
}

// ===========================================================================
// Segment 84/85 bodies.

namespace {
std::uint32_t rd32(const std::uint8_t* base, std::size_t offset) noexcept {
    std::uint32_t v;
    std::memcpy(&v, base + offset, 4);
    return v;
}
void wr32(std::uint8_t* base, std::size_t offset, std::uint32_t v) noexcept { std::memcpy(base + offset, &v, 4); }
void* rdptr(const std::uint8_t* base, std::size_t offset) noexcept {
    void* p;
    std::memcpy(&p, base + offset, sizeof p);
    return p;
}
void wrptr(std::uint8_t* base, std::size_t offset, const void* p) noexcept { std::memcpy(base + offset, &p, sizeof p); }
std::uint32_t ptr_word(const void* p) noexcept { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
RawArray& array_at_offset(std::uint8_t* owner, std::size_t offset) noexcept {
    return *reinterpret_cast<RawArray*>(owner + offset);
}
constexpr char kEmptyString[] = "";  // the shared empty strings at 0108D6F2 / 0108DBE0
} // namespace

void record_guard_destruct_00b26100(std::uint8_t* guard, std::uint32_t& guard_singleton_0108d5a0,
    RecordGuardHost& host) {
    wr32(guard, 0, 0x00D5E60Cu);
    host.destroy_owned_critical_section_0041cc80(guard + 4);
    guard_singleton_0108d5a0 = 0;
    wr32(guard, 0, 0x00CE3818u);
}

void* hardware_layout_for_declaration_name_00b264a0(void* renderer, const void* name,
    const std::uint32_t stale_words[3], LayoutLookupHost& host) {
    RefCounted* declaration = host.renderer_declaration_for_name_vt38(renderer, name);
    const std::uint32_t record[5] = {ptr_word(declaration), stale_words[0], stale_words[1], stale_words[2], 1};
    void* layout = host.singleton_layout_for_record_vt40(record);
    ref_release_cdecl_00b31da0(declaration);
    return layout;
}

std::uint8_t* binding_state_construct_00b27a40(std::uint8_t* state, const std::function<void(std::uint8_t*)>& init_00b24160) {
    wr32(state, 0xA8, 0);
    init_00b24160(state);
    return state;
}

void sampler_record_reset_00b27a60(std::uint8_t* record) noexcept { std::memset(record, 0, 0x0E); }

void sampler_cache_reset_00b27f20(std::uint8_t* records) noexcept {
    for (int i = 0; i < 20; ++i) sampler_record_reset_00b27a60(records + i * 0x48);
}

TreeIterator* tree_iterator_post_increment_00b28290(TreeIterator& it, TreeIterator* out,
    const std::function<void(TreeIterator&)>& advance_00b20dc0) {
    *out = it;
    advance_00b20dc0(it);
    return out;
}

RawArray* records_copy_construct_00b283d0(RawArray& a, const RawArray& source,
    const std::function<void(RawArray&, const RawArray&)>& copy_assign) {
    a = RawArray{};
    copy_assign(a, source);
    return &a;
}

void records_scalar_delete_00b29d10(RawArray* a, std::uint32_t flags, const ArrayResizeFn& resize) {
    resize(*a, 0);
    ::operator delete(a->data);
    if (flags & 1) ::operator delete(a);
}

void draw_debug_line_records_00b2b270(std::uint8_t* renderer, void* camera, DebugLinesHost& host,
    const ArrayResizeFn& reserve_00b228b0) {
    RawArray& records = array_at_offset(renderer, 0x1CF4);
    if (records.count == 0) return;
    if (!rdptr(renderer, 0x19E0)) {
        void* model = host.create_generated_model_00b4c700("debugshader.mshd", "pf43cc.mvfm", "DebugLines", 2, 0, 0, 0);
        wrptr(renderer, 0x19E0, model);  // published before the temporary names are returned
        static const float kIdentity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        host.model_set_local_transform_vt38(model, kIdentity);
    }
    void* model = rdptr(renderer, 0x19E0);
    void* stream = host.geometry_vertex_stream_00b73260(host.model_geometry_00b74640(model, 0), 0);
    void* vertices = host.stream_lock_vt10(stream, static_cast<std::uint32_t>(records.count), 0, 0);
    std::memcpy(vertices, records.data, static_cast<std::size_t>(records.count) << 4);
    host.stream_unlock_vt14(host.geometry_vertex_stream_00b73260(host.model_geometry_00b74640(model, 0), 0));
    auto* element = static_cast<std::uint8_t*>(host.geometry_element_00b732c0(host.model_geometry_00b74640(model, 0), 0));
    wr32(element, 0x18, static_cast<std::uint32_t>(records.count / 2));
    element = static_cast<std::uint8_t*>(host.geometry_element_00b732c0(host.model_geometry_00b74640(model, 0), 0));
    wr32(element, 0x10, static_cast<std::uint32_t>(records.count));
    host.gather_system_constants_00b46a70(nullptr, camera);
    void* entry = host.allocate_render_entry_0108fe88();
    void* geometry = host.model_geometry_00b74640(model, 0);
    host.render_entry_initialize_00b51a20(entry, 0.0f, host.geometry_element_00b732c0(host.model_geometry_00b74640(model, 0), 0),
        geometry, model, camera, 1.0f, 0.0f, 0);
    host.submit_render_entry(host.geometry_element_00b732c0(host.model_geometry_00b74640(model, 0), 0), entry);
    array_clear_count_down(records, reserve_00b228b0);
}

std::uint8_t* pooled_string_list_owner_construct(std::uint8_t* owner, const SentinelAllocateFn& allocate_sentinel) {
    wr32(owner, 0x00, 0);
    wr32(owner, 0x04, 0);
    wrptr(owner, 0x0C, allocate_sentinel());
    wr32(owner, 0x10, 0);
    for (std::size_t off = 0x14; off <= 0x24; off += 4) wr32(owner, off, 0);
    return owner;
}

void* find_cached_texture_00b31ac0(const char* name, std::uint32_t length, std::uint32_t word,
    TextureCacheHost& host) {
    const bool guarded = host.guard_enabled_0108d6dc();
    std::uint8_t token = 0;
    if (guarded) token = host.enter_optional_guard();
    std::string lowered(name, length);
    for (char& c : lowered)
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + ('a' - 'A'));
    void* result = host.texture_cache_load(lowered.c_str(), length, word, 0, 0);
    if (host.guard_enabled_0108d6dc()) host.leave_optional_guard(token);  // the flag is re-read
    return result;
}

void registry_base_construct(std::uint8_t* object, std::uint32_t vtable) noexcept {
    wr32(object, 0, vtable);
    for (std::size_t off = 4; off <= 0x10; off += 4) wr32(object, off, 0);
}

void texture_resource_cache_construct_00b32340(std::uint8_t* object) noexcept {
    registry_base_construct(object, 0x00D5F088u);
    wr32(object, 0x14, 0);
    wr32(object, 0x18, 0);
    object[0x1C] = 0;
    wr32(object, 0x20, 0);
}

void declaration_registry_destruct_00b321d0(std::uint8_t* registry,
    const std::function<void(std::uint8_t*)>& flush, const ArrayResizeFn& resize_records) {
    wr32(registry, 0, 0x00D5F024u);
    flush(registry);
    RawArray& records = array_at_offset(registry, 4);
    resize_records(records, 0);
    ::operator delete(records.data);
}

void set_observed_synchronization_mode_00b33ac0(std::uint8_t& mode_0108d6dd, std::uint8_t value) noexcept {
    mode_0108d6dd = value;
}

void render_worker_start_00b33bc0(std::uint8_t* worker, std::uint32_t a, std::uint32_t b, std::uint32_t c,
    const std::function<void(void*)>& start_vt04) {
    wr32(worker, 0x1C, a);
    wr32(worker, 0x18, c);
    wr32(worker, 0x20, b);
    worker[4] = 1;
    start_vt04(rdptr(worker, 0x0C));
}

bool logical_texture_flag_bit25_00b33e90(const std::uint8_t* texture) noexcept {
    return ((rd32(texture, 0x1C) >> 25) & 1) != 0;
}

void logical_texture_construct_00b33ec0(std::uint8_t* texture, std::uint32_t a, std::uint32_t flags,
    std::uint32_t& next_id_0108d6e8) noexcept {
    ref_counted_construct(texture, 0x00D5F1F4u);
    wr32(texture, 0x08, 0);
    wr32(texture, 0x0C, 0);
    wr32(texture, 0x14, 0);
    wr32(texture, 0x10, a);
    wr32(texture, 0x1C, flags);
    wr32(texture, 0x20, next_id_0108d6e8++);
}

void render_state_pair_reset_00b34380(std::uint8_t pair[2]) noexcept { pair[0] = pair[1] = 0xFF; }
void render_state_pair_set_00b34390(std::uint8_t pair[2], std::uint8_t a, std::uint8_t b) noexcept {
    pair[0] = a;
    pair[1] = b;
}

std::uint8_t reflection_map8_at_00b343c0(const std::uint8_t* owner, std::int32_t i) noexcept { return owner[8 + i]; }
std::uint8_t reflection_map3e_at_00b3ff80(const std::uint8_t* owner, std::int32_t i) noexcept { return owner[0x3E + i]; }
void reflection_maps_set_00b34570(std::uint8_t* owner, std::int32_t i, std::uint8_t a, std::uint8_t b) noexcept {
    owner[8 + i] = a;
    owner[0x3E + i] = b;
}

void reflection_owner_construct_00b3b190(std::uint8_t* owner) noexcept {
    wr32(owner, 0, 0x00CEB130u);
    owner[0x74] = 0;
    wr32(owner, 4, 1);
    wr32(owner, 0, 0x00D61810u);
    for (std::size_t off = 0x78; off <= 0x84; off += 4) wr32(owner, off, 0);
    std::memset(owner + 8, 0xFF, 0x36);
}

void shader_binary_cache_append_00b34400(std::uint8_t* cache, bool suppressed_0108d6f1, std::uint32_t key,
    std::uint32_t size, const void* data, BinaryStreamHost& host) {
    if (suppressed_0108d6f1) return;
    void* stream = rdptr(cache, 8);
    host.write_key_vt64(stream, key, 0);
    host.write_size_vt54(stream, size, 0);
    host.write_bytes_vt28(stream, data, size, 0);
    wr32(cache, 4, rd32(cache, 4) + 1);
}

const char* native_string_c_str_00b347d0(const PooledString& s) noexcept { return s.chars ? s.chars : kEmptyString; }

std::int32_t count_unflagged_samplers_00b34860(const std::uint8_t* descriptor) noexcept {
    const auto* cells = static_cast<const std::uint8_t* const*>(rdptr(descriptor, 0xC4));
    std::int32_t n = 0;
    for (std::uint32_t i = 0, count = rd32(descriptor, 0xC8); i < count; ++i)
        if (cells[i][0x0C] == 0) ++n;
    return n;
}

void shader_binary_cache_record_destroy(std::uint8_t* record, StoragePoolHost& pool) {
    if (void* data = rdptr(record, 8)) {
        ::operator delete(data);
        wrptr(record, 8, nullptr);
    }
    pooled_string_release(*reinterpret_cast<PooledString*>(record), pool);
}

#include "shader_vertex_literals.inc"
#include "shader_pixel_literals.inc"

void append_null_shadow_fade_out_00b38210(std::uint8_t* builder, ShaderTextHost& host) {
    host.append_cstring_line_00b34f20(builder + 0x4C, vertex_shadow_helper);
}

void append_vertex_ambient_helpers_00b38220(std::uint8_t* builder, ShaderTextHost& host) {
    host.append_cstring_line_00b34f20(builder + 0x4C, vertex_ambient_fog_helpers);
}

void append_pixel_ambient_and_srgb_helpers_00b38290(std::uint8_t* builder, ShaderTextHost& host) {
    host.append_cstring_line_00b34f20(builder + 0x4C, pixel_ambient_fog_helpers);
    host.append_cstring_line_00b34f20(builder + 0x4C, pixel_srgb_helpers);
}

void append_system_constant_pair(std::uint8_t* builder, ShaderTextHost& host) {
    host.append_system_constant_header_00b38ff0(builder, 1);
    auto text_of = [&](std::size_t child_offset) {
        const auto* child = static_cast<const std::uint8_t*>(rdptr(builder, child_offset));
        const auto* chars = static_cast<const char*>(rdptr(child, 0xE8 + 4));
        return chars ? chars : kEmptyString;
    };
    const char* second = text_of(0x74);
    const char* first = text_of(0x70);
    host.append_formatted_line_00b35110(builder + 0x4C, "\n%s\n%s", first, second);
}

void singleton_client_construct_00b61b10(std::uint8_t* object, void*& published_0108fed8, SingletonLifetimeHost& host) {
    wr32(object, 0, 0x00D62B4Cu);
    host.enter_manager_lock();
    published_0108fed8 = object;
    host.register_singleton(published_0108fed8);
    host.leave_manager_lock();
}

void singleton_client_destruct_00b61bb0(std::uint8_t* object, void*& published_0108fed8, SingletonLifetimeHost& host) {
    wr32(object, 0, 0x00D62B4Cu);
    host.enter_manager_lock();
    host.unregister_singleton(published_0108fed8);
    published_0108fed8 = nullptr;
    host.leave_manager_lock();
    wr32(object, 0, 0x00CE3818u);
}

bool cockpit_node_visible_00b3c550(const std::uint8_t* helper) noexcept {
    const auto* node = static_cast<const std::uint8_t*>(rdptr(helper, 8));
    return node && node[0x44] == 0;
}

void cockpit_set_attached_node_00b3c650(std::uint8_t* helper, RefCounted* node, CockpitNodeHost& host) {
    if (void* old = rdptr(helper, 8)) host.node_set_visibility_factor_00b6da70(old, 0.0f, 0);
    ref_slot_assign(reinterpret_cast<RefCounted**>(helper + 8), &node);
    if (void* now = rdptr(helper, 8)) host.node_set_visibility_factor_00b6da70(now, 0.0f, 0);
}

std::int32_t surface_lock_rect_00b3cd40(std::uint8_t* surface, std::uint32_t a, std::uint32_t b, std::uint32_t c, D3DResourceHost& host) {
    const std::uint32_t args[3] = {a, b, c};
    host.com_call(rdptr(surface, 0x2C), 0x34, args, 3);
    return 1;
}

void surface_unlock_rect_00b3cd60(std::uint8_t* surface, D3DResourceHost& host) {
    host.com_call(rdptr(surface, 0x2C), 0x38, nullptr, 0);
}

std::int32_t texture_lock_level(std::uint8_t* texture, std::uint32_t level, std::uint32_t out,
    std::uint32_t rect, std::uint32_t flags, D3DResourceHost& host) {
    const std::uint32_t args[4] = {level, out, rect, flags};
    host.com_call(rdptr(texture, 0x10), 0x4C, args, 4);
    return 1;
}

void texture_unlock_level(std::uint8_t* texture, std::uint32_t level, D3DResourceHost& host) {
    host.com_call(rdptr(texture, 0x10), 0x50, &level, 1);
}

std::int32_t cube_lock_face_00b3d6d0(std::uint8_t* cube, std::uint32_t a1, std::uint32_t a2, std::uint32_t a3,
    std::uint32_t a4, std::uint32_t a5, D3DResourceHost& host) {
    const std::uint32_t args[5] = {a2, a3, a1, a4, a5};
    host.com_call(rdptr(cube, 0x10), 0x4C, args, 5);
    return 1;
}

void cube_unlock_face_00b3d700(std::uint8_t* cube, std::uint32_t a, std::uint32_t b, D3DResourceHost& host) {
    const std::uint32_t args[2] = {a, b};
    host.com_call(rdptr(cube, 0x10), 0x50, args, 2);
}

bool surface_load_from_surface_00b3cd70(std::uint8_t* destination, std::uint8_t* source,
    std::uint32_t source_rect, std::uint32_t destination_rect, D3DResourceHost& host) {
    return host.d3dx_load_surface_from_surface(rdptr(destination, 0x2C), destination_rect, rdptr(source, 0x2C), source_rect) == 0;
}

void surface_stretch_from_00b3cda0(std::uint8_t* destination, std::uint8_t* source, StretchHost& host) {
    void* device = host.surface_device_vt0c(rdptr(destination, 0x2C));
    const std::int32_t rect[4] = {0, 0, static_cast<std::int32_t>(host.wrapper_width_vt1c(destination)),
        static_cast<std::int32_t>(host.wrapper_height_vt20(destination))};
    host.device_stretch_rect_vt88(device, rdptr(source, 0x2C), nullptr, rdptr(destination, 0x2C), rect, 2);
    host.device_release(device);
}

void surface_save_to_file_00b3dd00(std::uint8_t* surface, const PooledString& file, std::uint32_t format, D3DResourceHost& host) {
    host.d3dx_save_surface_to_file(file.chars ? file.chars : kEmptyString, format, rdptr(surface, 0x2C));
}

void texture_save_to_file(std::uint8_t* texture, const PooledString& file, std::uint32_t format, D3DResourceHost& host) {
    host.d3dx_save_texture_to_file(file.chars ? file.chars : kEmptyString, format, rdptr(texture, 0x10));
}

void texture_reload_from_file(std::uint8_t* texture, bool volume, TextureReloadHost& host) {
    void* stream = host.open_file_stream(*reinterpret_cast<const PooledString*>(texture + 8));
    if (!host.stream_is_open_vt18(stream)) {
        host.stream_delete_vt04(stream);
        return;
    }
    void* memory = host.stream_to_memory(stream);
    host.release(stream);
    host.renderer_notify_reload_vt6c(texture + 8);
    if (void* old = rdptr(texture, 0x10)) {
        host.com_release(old);
        wrptr(texture, 0x10, nullptr);
    }
    void* device = host.device_00b1fef0();
    const std::uint32_t size = host.memory_size_vt30(memory);
    wrptr(texture, 0x10, host.d3dx_create_from_memory(volume, device, host.memory_data_00bef610(memory), size));
    if (void* com = rdptr(texture, 0x10)) {
        std::uint32_t desc[4] = {};
        host.level_desc_vt44(com, volume, desc);
        wr32(texture, 0x24, desc[1]);
        if (volume) {
            wr32(texture, 0x28, desc[2]);
            wr32(texture, 0x2C, desc[3]);
        }
        wr32(texture, 0x14, host.level_count_vt34(com));
        wr32(texture, 0x18, desc[0]);
    }
    host.release(memory);
}

void d3d9_surface_construct_default_00b3f450(std::uint8_t* surface, std::uint32_t word_00d7a24c) noexcept {
    wr32(surface, 0, 0x00CEB130u);
    wr32(surface, 4, 1);
    wr32(surface, 8, word_00d7a24c);
    wr32(surface, 0x0C, word_00d7a24c);
    wr32(surface, 0, 0x00D619A0u);
    wr32(surface, 0x10, 0);
    wr32(surface, 0x14, 0);
    surface[0x30] = 0;
    wr32(surface, 0x2C, 0);
    for (std::size_t off = 0x18; off <= 0x28; off += 4) wr32(surface, off, 0);
}

void ref_counted_d618f8_construct_00b3dc50(std::uint8_t* object, std::uint32_t word_00d7a24c) noexcept {
    ref_counted_construct(object, 0x00D618F8u);
    wr32(object, 8, word_00d7a24c);
    wr32(object, 0x0C, word_00d7a24c);
}

void* cube_face_surface_00b3fe90(std::uint8_t* cube, std::uint32_t level, std::uint32_t face, FaceSurfaceHost& host) {
    void* surface = host.cube_map_surface_vt48(rdptr(cube, 0x10), face, level);
    void* slot = host.surface_pool_allocate_00b3ed40();
    void* wrapper = slot ? host.surface_construct_00b3f630(slot, surface, rd32(cube, 0x1C), 0) : nullptr;
    host.com_release(surface);
    return wrapper;
}

float material_effect_indexed_float_00b40840(const std::uint8_t* effect, std::int32_t i) noexcept {
    float v;
    std::memcpy(&v, effect + 0x140 + i * 4, 4);
    return v;
}

std::uint8_t* shader_effect_parameter_data_00b40ff0(const std::uint8_t* effect, std::int32_t i) noexcept {
    return static_cast<std::uint8_t*>(rdptr(effect, 0x184)) + 0x20 + i * 0x60;
}

std::uint32_t shader_effect_parameter_key_00b41010(const std::uint8_t* effect, std::int32_t i) noexcept {
    return rd32(static_cast<const std::uint8_t*>(rdptr(effect, 0x184)), static_cast<std::size_t>(i) * 0x60);
}

void* optimized_animator_dynamic_cast_00b40320(void* object, const std::function<bool(void*)>& is_kind_vt0c) {
    return object && is_kind_vt0c(object) ? object : nullptr;
}

void float4_block_set_dwords(std::uint8_t* block, std::int32_t slot, const std::uint32_t value[4]) noexcept {
    std::memcpy(block + 4 + slot * 0x10, value, 16);
}

void float4_block_set_scalar_00b40710(std::uint8_t* block, std::int32_t slot, std::uint32_t value) noexcept {
    const std::uint32_t v[4] = {value, 0, 0, 0};
    float4_block_set_dwords(block, slot, v);
}

// Each element moves by FLD / FSTP DWORD, which quiets a signalling NaN (cc13-fixture9).
void float2x4_from_matrix_transposed_00b40400(std::uint32_t out[8], const std::uint32_t m[16]) noexcept {
    for (int r = 0; r < 2; ++r)
        for (int c = 0; c < 4; ++c) out[r * 4 + c] = cc13x87::x87_fld_fstp_bits(m[c * 4 + r]);
}

// Each element moves by FLD / FSTP DWORD, which quiets a signalling NaN (cc13-fixture6).
void float3x4_from_matrix_transposed_00b40440(std::uint32_t out[12], const std::uint32_t m[16]) noexcept {
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 4; ++c) out[r * 4 + c] = cc13x87::x87_fld_fstp_bits(m[c * 4 + r]);
}

void float4_block_set_matrix_rows_00b405e0(std::uint8_t* block, std::int32_t slot, const std::uint32_t m[16],
    std::int32_t rows, const std::function<void(std::uint8_t*, const std::uint32_t*)>& write_transposed_4x4) {
    std::uint8_t* target = block + 4 + slot * 0x10;
    if (rows == 2) {
        std::uint32_t t[8];
        float2x4_from_matrix_transposed_00b40400(t, m);
        std::memcpy(target, t, sizeof t);
    } else if (rows == 3) {
        std::uint32_t t[12];
        float3x4_from_matrix_transposed_00b40440(t, m);
        std::memcpy(target, t, sizeof t);
    } else if (rows == 4) {
        write_transposed_4x4(target, m);
    }
}

void ref_counted_construct(std::uint8_t* object, std::uint32_t vtable) noexcept {
    wr32(object, 0, 0x00CEB130u);
    wr32(object, 4, 1);
    wr32(object, 0, vtable);
}

void instance_generator_binding_construct_00b40520(std::uint8_t* object, std::uint32_t& next_id_0108fd30) noexcept {
    ref_counted_construct(object, 0x00D619F8u);
    wr32(object, 0x0C, 0);
    wr32(object, 0x08, next_id_0108fd30++);
}

std::uint8_t* material_state_list_assign(std::uint8_t* object, const std::uint8_t* source,
    const std::function<void(RawArray&, const RawArray&)>& copy_rows) {
    wr32(object, 4, rd32(source, 4));
    copy_rows(array_at_offset(object, 8), *reinterpret_cast<const RawArray*>(source + 8));
    return object;
}

void material_effect_apply_generator_modes_00b45330(std::uint8_t* effect, const std::function<void(void*)>& apply_00b44c00) {
    if (effect[0xB4] == 0) return;
    for (int i = 0; i < 14; ++i)
        if (void* section = rdptr(effect, 0xC8 + i * 4)) apply_00b44c00(section);
}

void vertex_element_construct_unused_00b478b0(std::uint32_t element[5]) noexcept {
    element[1] = 0x11;
    element[2] = 0;
    element[3] = 0x0E;
    element[0] = 0xFFFFFFFFu;
    element[4] = 0xFFFFFFFFu;
}

std::uint32_t vertex_element_type_size_00b478e0(const std::uint32_t element[5]) noexcept {
    static constexpr std::uint32_t kTypeSize_00d61cc0[18] = {4, 8, 12, 16, 4, 4, 4, 8, 4, 4, 8, 4, 8, 4, 4, 4, 8, 4};
    return kTypeSize_00d61cc0[element[1]];  // unchecked index, as in the image
}

bool d3d_format_is_kind_1000_00b49170(std::uint32_t format) noexcept { return (format & 0xF000u) == 0x1000u; }

void layout_record_quad_push_assign_00b48460(std::uint8_t* quad, const std::uint32_t* source) {
    const std::uint32_t index = rd32(quad, 0x30);
    wr32(quad, 0x30, index + 1);
    ref_record_assign_00b483a0(reinterpret_cast<std::uint32_t*>(quad + index * 0x0C), source);
}

void logical_vertex_stream_commit_00b493e0(std::uint8_t* stream, std::uint8_t* base, std::uint32_t vertex_stride) {
    void* copy = rdptr(stream, 0x6C);
    std::memcpy(base + rd32(stream, 0x5C), copy, static_cast<std::size_t>(vertex_stride) * rd32(stream, 0x64));
    ::operator delete(copy);
    wr32(stream, 0x6C, 0);
    wr32(stream, 0x5C, 0);
}

void logical_index_stream_commit_00b49490(std::uint8_t* stream, std::uint8_t* base) {
    const std::uint32_t format = rd32(stream, 0x18);
    const std::uint32_t size = format == 0x65 ? 2 : format == 0x66 ? 4 : 0;
    void* copy = rdptr(stream, 0x1C);
    std::memcpy(base + static_cast<std::size_t>(rd32(stream, 0x20)) * size, copy, static_cast<std::size_t>(rd32(stream, 0x14)) * size);
    ::operator delete(copy);
    wr32(stream, 0x1C, 0);
}

void vertex_declaration_cache_set_element_00b61f90(std::uint8_t* cache, std::int32_t index,
    const float element[8], const std::function<std::uint32_t()>& element_count) {
    if (!rdptr(cache, 0x50)) {
        const std::uint64_t bytes = static_cast<std::uint64_t>(element_count()) * 0x20u;
        const std::size_t request = (bytes >> 32) ? static_cast<std::size_t>(-1) : static_cast<std::size_t>(bytes);
        wrptr(cache, 0x50, ::operator new(request));
    }
    std::memcpy(static_cast<std::uint8_t*>(rdptr(cache, 0x50)) + index * 0x20, element, 0x20);
}

float* matrix_from_scale_rotation_translation_00b64ce0(float out[16], const float translation[3],
    const void* quaternion, const float scale[3],
    const std::function<void(float rotation[16], const void* quaternion)>& rotation_0047c2a0,
    const std::function<void(const float a[16], float out[16], const float b[16])>& multiply_00413920) {
    const float s[16] = {scale[0], 0, 0, 0, 0, scale[1], 0, 0, 0, 0, scale[2], 0, 0, 0, 0, 1.0f};
    float r[16];
    rotation_0047c2a0(r, quaternion);
    multiply_00413920(s, out, r);
    out[12] = translation[0];
    out[13] = translation[1];
    out[14] = translation[2];
    return out;
}

float* plane_set_transform_six_00b65660(float planes[24], const void* matrix,
    const std::function<void(float out[4], const float plane[4], const void* matrix)>& transform_00b65ba0) {
    for (int i = 0; i < 6; ++i) {
        float t[4];
        transform_00b65ba0(t, planes + i * 4, matrix);
        std::memcpy(planes + i * 4, t, sizeof t);
    }
    return planes;
}

// ===========================================================================
// Segment 86 bodies.

void texture_fix_alpha_level0(void* texture, const std::function<RefCounted*(void*)>& level0_surface_vt30,
    const std::function<void(RefCounted*)>& fix) {
    RefCounted* surface = level0_surface_vt30(texture);
    fix(surface);
    release_one(surface);
}

void capture_copy_if_enabled_00b4caf0(std::uint8_t* holder, StretchHost& host) {
    if (holder[0x14] == 0) return;
    surface_stretch_from_00b3cda0(static_cast<std::uint8_t*>(rdptr(holder, 0x10)),
        static_cast<std::uint8_t*>(rdptr(holder, 0x0C)), host);
}

void capture_set_sample_count_00b4d270(std::uint8_t* holder, std::uint32_t samples,
    const std::function<std::uint32_t(void*)>& sample_count_vt24,
    const std::function<void(void*, const std::uint32_t zero_record[4])>& recreate_vt54) {
    if (holder[0x14] == 0) return;
    auto* destination = static_cast<std::uint8_t*>(rdptr(holder, 0x0C));
    if (sample_count_vt24(destination) == samples) return;
    wr32(destination, 0x24, samples);  // 00B3CE20
    const std::uint32_t zero[4] = {};
    recreate_vt54(rdptr(holder, 8), zero);
}

void post_effect_submit_current_entry_00b4def0(std::uint8_t* pass, PassSubmitHost& host) {
    void* targets = rdptr(pass, 8);
    void* first = host.targets_color_surface_00b1f6d0(targets, 0);
    void* second = host.targets_color_surface_00b1f6d0(targets, 0);
    const std::uint32_t height = host.surface_height_vt20(first);
    const std::uint32_t width = host.surface_width_vt1c(second);
    const std::uint32_t size[2] = {width, height};
    void* camera = rdptr(pass, 0x0C);
    host.viewport_set_dimensions_00b1f940(host.camera_viewport_00b6fde0(camera), size);
    host.renderer_set_camera_vta0(camera);
    host.renderer_set_viewport_vta4(host.camera_viewport_00b6fde0(camera));
    host.renderer_set_targets_vt98(targets);
    host.material_queue_submit(rdptr(pass, 0x14), rdptr(pass, 0x1C));
}

void tonemap_set_params_and_draw_00b548c0(std::uint8_t* pass, void* holder, const float params[5], PostEffectHost& host) {
    std::memcpy(pass + 0x210, params, 5 * sizeof(float));
    void* child = rdptr(pass, 8);
    host.material_set_texture_slot_00b189f0(host.pass_material_00b4cba0(child), 1, host.holder_texture_00b4cb10(holder));
    host.pass_draw_fullscreen_00b4dcc0(child, 0);
}

void distortion_add_object_00b4ef90(std::uint8_t* owner, void* node, void* object, DistortionHost& host,
    const ArrayResizeFn& reserve_00b4ed70) {
    if (!object) {
        host.node_attach_00b6d890(node, rdptr(owner, 0x3C));
        return;
    }
    void* node_clone = host.node_clone_vt10(node, 0, nullptr);
    host.node_set_visibility_factor_00b6da70(node_clone, 0.0f, 0);
    void* object_clone = host.node_clone_vt10(object, 0, nullptr);
    void* record[2] = {node_clone, object_clone};
    array_push_back_via_reserve(array_at_offset(owner, 0x254), record, 8, CapacityRule::kDoubleMinOne, reserve_00b4ed70);
}

void distortion_release_objects_00b4f030(std::uint8_t* owner, DistortionHost& host, const ArrayResizeFn& resize_00b4ee20) {
    RawArray& records = array_at_offset(owner, 0x254);
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(records.count); ++i) {
        void* node_clone = rdptr(records.data, i * 8);
        void* object_clone = rdptr(records.data, i * 8 + 4);
        if (node_clone) host.node_unlink_and_release_00b6dfa0(node_clone);
        if (object_clone) host.release(object_clone);
    }
    resize_00b4ee20(records, 0);
}

namespace {
// One full-screen pass: optional camera clear flags 1, optional colour-0 target, slot 0
// texture, draw. Steps are skipped where the image skips them.
void fullscreen_pass(PostEffectHost& host, void* pass, bool clear, void* target_holder, void* texture_holder) {
    if (clear) host.camera_set_clear_flags_00b6fe10(host.pass_camera_00b4cbb0(pass), 1);
    if (target_holder) host.pass_set_color0_surface_00b4cb70(pass, host.holder_primary_surface_00b4cb20(target_holder));
    if (texture_holder)
        host.material_set_texture_slot_00b189f0(host.pass_material_00b4cba0(pass), 0, host.holder_texture_00b4cb10(texture_holder));
    host.pass_draw_fullscreen_00b4dcc0(pass, 0);
}
} // namespace

void distortion_render_00b4f1c0(std::uint8_t* owner, void* camera, DistortionHost& host,
    const ArrayResizeFn& resize_00b4ee20) {
    void* blur = rdptr(owner, 0x1C);
    if (owner[0x251] != 0) {
        const std::uint32_t saved = rd32(owner, 0x244);
        owner[0x251] = 0;
        const float far_value = 1.0e9f;  // MOVSS from .rdata 00D217E8
        std::memcpy(owner + 0x244, &far_value, 4);
        fullscreen_pass(host, blur, true, rdptr(owner, 0x10), rdptr(owner, 0x14));
        fullscreen_pass(host, blur, true, rdptr(owner, 0x14), rdptr(owner, 0x10));
        wr32(owner, 0x244, saved);
    }
    void* scene_camera = rdptr(owner, 0x34);
    host.targets_set_color_surface_00b1fab0(rdptr(owner, 0x38), 0, host.holder_primary_surface_00b4cb20(rdptr(owner, 0x10)));
    host.renderer_set_targets_vt98(rdptr(owner, 0x38));
    host.camera_set_clear_flags_00b6fe10(scene_camera, owner[0x250] != 0 ? 1u : 0u);
    host.camera_execute_frame_00b71360(scene_camera);
    owner[0x250] = 0;
    for (auto* node = static_cast<std::uint8_t*>(rdptr(static_cast<std::uint8_t*>(rdptr(owner, 0x3C)), 0x0C)); node;
         node = static_cast<std::uint8_t*>(rdptr(node, 0x3C))) {
        if (!host.node_is_generated_model_vt0c(node)) continue;
        void* geometry = host.model_geometry_00b74640(node, 0);
        void* element = host.geometry_element_00b732c0(geometry, 0);
        host.node_prepare_vt58(node, scene_camera, 1.0f);
        void* entry = host.allocate_render_entry_0108fe88();
        host.render_entry_initialize_00b51a20(entry, 1.0f, element, geometry, node, scene_camera, 1.0f, 0.0f, 0);
        host.submit_render_entry(element, entry);
    }
    void* texture_a = host.holder_texture_00b4cb10(rdptr(owner, 0x10));
    void* texture_b = host.holder_texture_00b4cb10(rdptr(owner, 0x10));
    const std::uint32_t height = host.texture_height_vt40(texture_a);
    const std::uint32_t width = host.texture_width_vt3c(texture_b);
    cc13fx::sample_offsets_gauss_blur5x5(width, height, reinterpret_cast<cc13fx::Float4*>(owner + 0x40),
        reinterpret_cast<cc13fx::Float4*>(owner + 0x140), 1.0f);
    fullscreen_pass(host, blur, true, rdptr(owner, 0x14), rdptr(owner, 0x10));
    fullscreen_pass(host, rdptr(owner, 0x20), true, nullptr, rdptr(owner, 0x14));
    RawArray& records = array_at_offset(owner, 0x254);
    if (owner[0x260] != 0) {
        for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(records.count); ++i) {
            void* node_clone = rdptr(records.data, i * 8);
            host.node_set_visibility_factor_00b6da70(node_clone, 1.0f, 0);
            void* object_clone = rdptr(records.data, i * 8 + 4);
            void* element = host.geometry_element_00b732c0(object_clone, 0);
            void* entry = host.allocate_render_entry_0108fe88();
            host.render_entry_initialize_00b51a20(entry, 1.0f, element, object_clone, node_clone, camera, 1.0f, 0.0f, 0);
            host.submit_render_entry(element, entry);
        }
    }
    void* swap = rdptr(owner, 0x10);
    wrptr(owner, 0x10, rdptr(owner, 0x14));
    wrptr(owner, 0x14, swap);
    distortion_release_objects_00b4f030(owner, host, resize_00b4ee20);
}

namespace {
// The destination half-texel added to the 16 sample offsets at owner+10h (00B50050).
void add_half_texel(std::uint8_t* owner, void* pass, PostEffectHost& host) {
    void* surface = host.targets_color_surface_00b1f6d0(host.pass_frame_targets_00b4cb90(pass, 0), 0);
    const float w = static_cast<float>(host.surface_width_vt1c(surface));   // FILD + 2^32 when negative
    surface = host.targets_color_surface_00b1f6d0(host.pass_frame_targets_00b4cb90(pass, 0), 0);
    const float h = static_cast<float>(host.surface_height_vt20(surface));
    const float dx = static_cast<float>(0.5 / w);  // double D7A280 = 0.5, FSTP float
    const float dy = static_cast<float>(0.5 / h);
    auto* f = reinterpret_cast<float*>(owner + 0x10);
    for (int i = 0; i < 16; ++i) {
        f[i * 4 + 0] = f[i * 4 + 0] + dx;
        f[i * 4 + 1] = f[i * 4 + 1] + dy;
        f[i * 4 + 2] = f[i * 4 + 2] + 0.0f;
        f[i * 4 + 3] = f[i * 4 + 3] + 0.0f;
    }
}
} // namespace

void luminance_measure_and_adapt_00b50050(std::uint8_t* owner, std::uint32_t word, PostEffectHost& host) {
    wr32(owner, 0x240, word);
    auto* offsets = reinterpret_cast<cc13fx::Float4*>(owner + 0x10);
    void* average = rdptr(owner, 0x214);
    auto* material = static_cast<std::uint8_t*>(host.pass_material_00b4cba0(average));
    std::int16_t texture_count;
    std::memcpy(&texture_count, material + 0x34, 2);
    void* source = texture_count < 1 ? nullptr : rdptr(material, 0x10);  // null is dereferenced next in the image
    const std::uint32_t source_height = host.texture_height_vt40(source);
    cc13fx::sample_offsets_avg_lum3x3(host.texture_width_vt3c(source), source_height, offsets);
    add_half_texel(owner, average, host);
    host.pass_draw_fullscreen_00b4dcc0(average, 0);
    static constexpr std::size_t kSources[3] = {0x234, 0x230, 0x22C};
    static constexpr std::size_t kPasses[3] = {0x218, 0x21C, 0x220};
    for (int i = 0; i < 3; ++i) {
        void* a = host.holder_texture_00b4cb10(rdptr(owner, kSources[i]));
        void* b = host.holder_texture_00b4cb10(rdptr(owner, kSources[i]));
        const std::uint32_t height = host.texture_height_vt40(a);
        cc13fx::sample_offsets_downscale4x4(host.texture_width_vt3c(b), height, offsets);
        add_half_texel(owner, rdptr(owner, kPasses[i]), host);
        host.pass_draw_fullscreen_00b4dcc0(rdptr(owner, kPasses[i]), 0);
    }
    void* clear = rdptr(owner, 0x210);
    if (owner[0x24C] != 0) {
        host.camera_set_clear_flags_00b6fe10(host.pass_camera_00b4cbb0(clear), 1);
        host.camera_set_clear_color_00b6fe50(host.pass_camera_00b4cbb0(clear), 0xFFFFFFFFu);
        fullscreen_pass(host, clear, false, rdptr(owner, 0x23C), nullptr);
        fullscreen_pass(host, clear, false, rdptr(owner, 0x238), nullptr);
        owner[0x24C] = 0;
    }
    void* previous = rdptr(owner, 0x238);
    wrptr(owner, 0x238, rdptr(owner, 0x23C));
    wrptr(owner, 0x23C, previous);
    void* adapt = rdptr(owner, 0x224);
    host.pass_set_color0_surface_00b4cb70(adapt, host.holder_primary_surface_00b4cb20(previous));
    host.material_set_texture_slot_00b189f0(host.pass_material_00b4cba0(adapt), 0, host.holder_texture_00b4cb10(rdptr(owner, 0x238)));
    host.material_set_texture_slot_00b189f0(host.pass_material_00b4cba0(adapt), 1, host.holder_texture_00b4cb10(rdptr(owner, 0x228)));
    host.pass_draw_fullscreen_00b4dcc0(adapt, 0);
}

void bloom_render_00b54ce0(std::uint8_t* owner, PostEffectHost& host) {
    cc13fx::sample_offsets_gauss_blur5x5(rd32(owner, 0x10), rd32(owner, 0x14), reinterpret_cast<cc13fx::Float4*>(owner + 0x228),
        reinterpret_cast<cc13fx::Float4*>(owner + 0x328), 1.0f);
    host.pass_set_color0_surface_00b4cb70(rdptr(owner, 0x24), host.holder_primary_surface_00b4cb20(rdptr(owner, 0x18)));
    host.pass_draw_fullscreen_00b4dcc0(rdptr(owner, 0x24), 0);
    auto* offsets = reinterpret_cast<cc13fx::Float4*>(owner + 0x28);
    auto* weights = reinterpret_cast<cc13fx::Float4*>(owner + 0x128);
    float deviation;
    std::memcpy(&deviation, owner + 0x438, 4);
    void* blur = rdptr(owner, 8);
    for (int round = 0; round < 2; ++round) {
        // Round 0: both textures from +18h, y cleared (horizontal), source +18h, target +1Ch.
        // Round 1: height from +18h, width from +1Ch, x cleared (vertical), renderer
        // targets reset (vt+98h(0)), source +1Ch, target +20h.
        void* a = host.holder_texture_00b4cb10(rdptr(owner, 0x18));
        void* b = host.holder_texture_00b4cb10(rdptr(owner, round == 0 ? 0x18 : 0x1C));
        const std::uint32_t height = host.texture_height_vt40(a);
        cc13fx::sample_offsets_bloom15(host.texture_width_vt3c(b), height, offsets, weights, deviation, 1.0f);
        for (int i = 0; i < 16; ++i) {  // the loop clears 16 slots; 15 are used
            if (round == 0) offsets[i].y = 0.0f;
            else offsets[i].x = 0.0f;
        }
        if (round == 1) host.renderer_set_targets_vt98(nullptr);
        host.material_set_texture_slot_00b189f0(host.pass_material_00b4cba0(blur), 0,
            host.holder_texture_00b4cb10(rdptr(owner, round == 0 ? 0x18 : 0x1C)));
        host.pass_set_color0_surface_00b4cb70(blur, host.holder_primary_surface_00b4cb20(rdptr(owner, round == 0 ? 0x1C : 0x20)));
        host.pass_draw_fullscreen_00b4dcc0(blur, 0);
    }
}

// ---------------------------------------------------------------------------
// mshd tree lifetime.

void mshd_delete_owned_vector(RawArray& cells, std::uint32_t resize_address, MshdLifetimeHost& host) {
    auto** slots = reinterpret_cast<OwnedVirtual**>(cells.data);
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(cells.count); ++i) owned_slot_delete(&slots[i]);
    host.resize_pointer_array(resize_address, cells, 0);
    ::operator delete(cells.data);
}

void mshd_lod_phase_destruct_00b56a30(std::uint8_t* phase, MshdLifetimeHost& host) {
    wr32(phase, 0, 0x00D621F0u);
    mshd_delete_owned_vector(array_at_offset(phase, 0x0C), 0x00B56420u, host);
}

void mshd_render_mode_destruct_00b56c90(std::uint8_t* mode, MshdLifetimeHost& host) {
    wr32(mode, 0, 0x00D621FCu);
    mshd_delete_owned_vector(array_at_offset(mode, 0x10), 0x00B56380u, host);
    pooled_string_release(*reinterpret_cast<PooledString*>(mode + 4), host);
}

void mshd_shader_destruct_00b572c0(std::uint8_t* shader, MshdLifetimeHost& host) {
    wr32(shader, 0, 0x00D62200u);
    mshd_delete_owned_vector(array_at_offset(shader, 0x10), 0x00B56470u, host);
    pooled_string_release(*reinterpret_cast<PooledString*>(shader + 8), host);
    host.ref_counted_destroy_base_00bd30f0(shader);
}

void mshd_version_destruct_00b584c0(std::uint8_t* version, MshdLifetimeHost& host) {
    wr32(version, 0, 0x00D62264u);
    mshd_delete_owned_vector(array_at_offset(version, 0x68), 0x00B563D0u, host);
    host.destroy_caps_00b2f690(version + 0x0C);
    pooled_string_release(*reinterpret_cast<PooledString*>(version + 4), host);
}

void shader_constant_list_slot_delete_00b5b620(std::uint8_t** slot, MshdLifetimeHost& host) {
    std::uint8_t* list = *slot;
    if (!list) return;
    RawArray& rows = *reinterpret_cast<RawArray*>(list);
    host.resize_constant_rows_00b580c0(rows, 0);
    ::operator delete(rows.data);
    ::operator delete(list);
    *slot = nullptr;
}

void mshd_layer_destruct_00b5b650(std::uint8_t* layer, MshdLifetimeHost& host) {
    wr32(layer, 0, 0x00D621F8u);
    host.destroy_state_list_00b56de0(layer + 0x0C);
    RawArray& samplers = array_at_offset(layer, 0x24);
    auto** cells = reinterpret_cast<OwnedVirtual**>(samplers.data);
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(samplers.count); ++i) owned_slot_delete(&cells[i]);
    shader_constant_list_slot_delete_00b5b620(reinterpret_cast<std::uint8_t**>(layer + 0x64), host);
    shader_constant_list_slot_delete_00b5b620(reinterpret_cast<std::uint8_t**>(layer + 0x68), host);
    for (std::size_t offset : {0x70u, 0x48u, 0x40u, 0x38u, 0x30u})
        pooled_string_release(*reinterpret_cast<PooledString*>(layer + offset), host);
    host.resize_pointer_array(0x00B34760u, samplers, 0);
    ::operator delete(samplers.data);
    pooled_string_release(*reinterpret_cast<PooledString*>(layer + 4), host);
}

void mshd_layer_construct_00b56b90(std::uint8_t* layer) noexcept {
    wr32(layer, 0, 0x00D621F8u);
    for (std::size_t off = 0x04; off <= 0x10; off += 4) wr32(layer, off, 0);
    layer[0x18] = layer[0x19] = layer[0x1A] = 0;
    wr32(layer, 0x1C, 0);
    wr32(layer, 0x20, 0);
    wr32(layer, 0x14, 1);
    for (std::size_t off = 0x24; off <= 0x4C; off += 4) wr32(layer, off, 0);
    layer[0x58] = 0;
    for (std::size_t off = 0x5C; off <= 0x68; off += 4) wr32(layer, off, 0);
    wr32(layer, 0x6C, 1);
    wr32(layer, 0x70, 0);
    wr32(layer, 0x74, 0);
}

void shader_constant_list_append_00b58160(RawArray& rows, std::int32_t& next_register_0c,
    const ConstantRowRaw& row_without_register, const std::function<void(RawArray&, const ConstantRowRaw&)>& push_back_00b57fd0) {
    ConstantRowRaw row = row_without_register;
    row.register_index = next_register_0c;
    push_back_00b57fd0(rows, row);
    next_register_0c += row.dimension_8 * row.array_count;
}

namespace {
constexpr std::size_t kConstantRowSize = 0x1C;
PooledString& row_name(std::uint8_t* row) noexcept { return *reinterpret_cast<PooledString*>(row + 4); }
} // namespace

void shader_constant_rows_reserve_00b576f0(RawArray& rows, std::int32_t n, const RowCopyFn& copy_row_00b57680,
    StoragePoolHost& pool) {
    if (n < 1) n = 1;
    if (rows.capacity >= n) return;
    auto* fresh = static_cast<std::uint8_t*>(::operator new(static_cast<std::size_t>(n) * kConstantRowSize));
    for (std::int32_t i = 0; i < rows.count; ++i)
        copy_row_00b57680(fresh + i * kConstantRowSize, rows.data + i * kConstantRowSize);
    for (std::int32_t i = 0; i < rows.count; ++i) pooled_string_release(row_name(rows.data + i * kConstantRowSize), pool);
    ::operator delete(rows.data);
    rows.data = fresh;
    rows.capacity = n;
}

void shader_constant_rows_resize_00b580c0(RawArray& rows, std::int32_t n, const RowCopyFn& copy_row_00b57680,
    StoragePoolHost& pool) {
    if (rows.capacity < n) shader_constant_rows_reserve_00b576f0(rows, n, copy_row_00b57680, pool);
    for (std::int32_t i = rows.count; i < n; ++i) row_name(rows.data + i * kConstantRowSize) = PooledString{};
    while (rows.count > n) {
        --rows.count;
        pooled_string_release(row_name(rows.data + rows.count * kConstantRowSize), pool);
    }
    rows.count = n;
}

std::int32_t shader_file_create(const void* file, void** out, bool pixel, ShaderFileHost& host) {
    void* device = host.device_00b1fef0();
    void* stream = host.open_file_stream(file);
    const std::uint32_t size = host.stream_size_vt30(stream);
    void* buffer = ::operator new(size + 4, std::nothrow);
    if (!buffer) {
        host.release(stream);
        return static_cast<std::int32_t>(0x80004005u);
    }
    std::memset(buffer, 0, size + 4);
    host.stream_read_vt24(stream, buffer, size, 0);
    const std::int32_t hr = host.device_create_shader(device, pixel ? 0x1A8u : 0x16Cu, buffer, out);
    host.release(stream);
    ::operator delete(buffer);
    return hr;
}

std::int32_t assemble_pixel_shader_00b611c0(const char* text, void** out, ShaderFileHost& host) {
    void* device = host.device_00b1fef0();
    void* code = nullptr;
    void* errors = nullptr;
    host.d3dx_assemble_shader(text, static_cast<std::uint32_t>(std::strlen(text)), &code, &errors);
    const std::int32_t hr = host.device_create_shader(device, 0x1A8u, host.buffer_pointer_vt0c(code), out);
    if (code) host.com_release(code);
    if (errors) host.com_release(errors);
    return hr;
}

// ---------------------------------------------------------------------------
// Small constructors, accessors and wrappers.

void logical_texture_set_name_00b341d0(std::uint8_t* texture, const PooledString& name, StringHost& strings) {
    auto& target = *reinterpret_cast<PooledString*>(texture + 8);
    if (&target != &name) strings.assign_0041dd40(target, name);
}

void pooled_string_pointer_delete_00b3a6e0(PooledString** slot, StoragePoolHost& pool) {
    PooledString* s = *slot;
    if (!s) return;
    pooled_string_release(*s, pool);
    ::operator delete(s);
    *slot = nullptr;
}

void pointer_delete_with(void** slot, const std::function<void(void*)>& destroy) {
    void* p = *slot;
    if (!p) return;
    destroy(p);
    ::operator delete(p);
    *slot = nullptr;
}

std::uint8_t* render_singleton_construct_00b3c4c0(std::uint8_t* object, void*& published_0108fed8, SingletonLifetimeHost& host) {
    singleton_client_construct_00b61b10(object, published_0108fed8, host);
    wr32(object, 0, 0x00D62B50u);  // 00B61C70
    wr32(object, 0, 0x00D61844u);
    return object;
}

void render_singleton_destruct_00b3c4e0(std::uint8_t* object, void*& published_0108fed8, SingletonLifetimeHost& host) {
    wr32(object, 0, 0x00D61844u);
    singleton_client_destruct_00b61bb0(object, published_0108fed8, host);  // via the 00B61C90 thunk
}

void cockpit_set_camera_view_matrix_00b3c570(std::uint8_t* helper, const std::function<void(void*)>& set_view_00b71490) {
    if (void* camera = rdptr(helper, 0x0C)) set_view_00b71490(camera);
}

void float4_construct_00b3d0d0(std::uint32_t v[4], std::uint32_t x, std::uint32_t y, std::uint32_t z, std::uint32_t w) noexcept {
    v[0] = x;
    v[1] = y;
    v[2] = z;
    v[3] = w;
}

void material_state_list_construct(std::uint8_t* object, std::uint32_t vtable) noexcept {
    ref_counted_construct(object, vtable);
    wr32(object, 8, 0);
    wr32(object, 0x0C, 0);
    wr32(object, 0x10, 0);
}

void instance_generator_destruct(std::uint8_t* generator, std::uint32_t vtable,
    const std::function<void(std::uint8_t*)>& teardown_00b55be0) {
    wr32(generator, 0, vtable);
    teardown_00b55be0(generator);
}

std::uint8_t* layout_record_quad_construct_00b48430(std::uint8_t* quad, const std::function<void(std::uint8_t*)>& construct_00b47640) {
    for (int i = 0; i < 4; ++i) construct_00b47640(quad + i * 0x0C);
    wr32(quad, 0x30, 0);
    return quad;
}

std::uint32_t* layout_record_copy_construct_00b489e0(std::uint32_t* record, const std::uint32_t* source) {
    record[0] = 0;
    ref_record_assign_00b483a0(record, source);
    return record;
}

std::uint8_t* hardware_layout_deleting_destructor_00b48c80(std::uint8_t* layout, std::uint32_t flags,
    const std::function<void(std::uint8_t*)>& destroy_base_00b48960, const std::function<void(std::uint8_t*)>& return_slot_00b479c0) {
    destroy_base_00b48960(layout);
    if (flags & 1) return_slot_00b479c0(layout);
    return layout;
}

std::uint8_t* logical_vertex_stream_construct_00b4a9b0(std::uint8_t* memory, std::uint32_t s20, RefCounted* s24,
    RefCounted* physical, std::uint32_t byte_offset, const std::function<void(std::uint8_t*, RefCounted*)>& base_00b61e20,
    const std::function<void(std::uint8_t*)>& register_00b28a40) {
    base_00b61e20(memory, s24);
    wr32(memory, 0, 0x00D61D6Cu);
    for (std::size_t off : {0x58u, 0x5Cu, 0x68u, 0x6Cu, 0x70u}) wr32(memory, off, 0);
    wr32(memory, 0x64, s20);
    wr32(memory, 0x60, 1);
    if (s24) {
        wrptr(memory, 0x68, s24);
        ++s24->refcount;
    }
    ref_slot_assign(reinterpret_cast<RefCounted**>(memory + 0x58), &physical);
    wr32(memory, 0x5C, byte_offset);
    register_00b28a40(memory);
    return memory;
}

void post_effect_draw_fullscreen_pass_00b4dfb0(void* pass, PostEffectHost& host) { host.pass_draw_fullscreen_00b4dcc0(pass, 0); }

void post_effect_pass_extent_00b4ed30(const std::uint8_t* pass, std::uint32_t out[2]) noexcept {
    out[0] = rd32(pass, 0x24);
    out[1] = rd32(pass, 0x28);
}

void luminance_lock_readback_00b51000(std::uint8_t* owner, ReadbackHost& host) {
    void* readback = rdptr(owner, 0x248);
    host.renderer_get_render_target_data_00b1ff70(host.holder_primary_surface_00b4cb20(rdptr(owner, 0x238)), readback);
    std::uint32_t rect[2] = {};
    host.readback_lock_vt2c(readback, rect, 0, 0x10);
    host.readback_unlock_vt30(readback);
    host.diagnostic_sink_004c14c0();
}

void shader_constant_row_construct_00b56e30(std::uint8_t* row, const PooledString& name, std::int32_t d8, std::int32_t dc,
    std::int32_t count, std::uint32_t definition, std::int32_t register_index, StringHost& strings) {
    wr32(row, 0, definition);
    auto& target = *reinterpret_cast<PooledString*>(row + 4);
    target = PooledString{};
    if (&target != &name) strings.assign_0041dd40(target, name);
    wr32(row, 0x0C, static_cast<std::uint32_t>(d8));
    wr32(row, 0x10, static_cast<std::uint32_t>(dc));
    wr32(row, 0x14, static_cast<std::uint32_t>(count));
    wr32(row, 0x18, static_cast<std::uint32_t>(register_index));
}

void shader_constant_row_copy_00b57680(std::uint8_t* row, const std::uint8_t* source, StringHost& strings) {
    wr32(row, 0, rd32(source, 0));
    auto& target = *reinterpret_cast<PooledString*>(row + 4);
    const auto& from = *reinterpret_cast<const PooledString*>(source + 4);
    target = PooledString{};
    if (&target != &from) strings.assign_0041dd40(target, from);
    std::memcpy(row + 0x0C, source + 0x0C, 0x10);
}

void named_pair_construct_00b573a0(std::uint32_t* pair, const PooledString& name, std::uint32_t a, std::uint32_t b, StringHost& strings) {
    auto& target = *reinterpret_cast<PooledString*>(pair);
    target = PooledString{};
    if (&target != &name) strings.assign_0041dd40(target, name);
    pair[2] = a;
    pair[3] = b;
}

void mshd_shader_construct_00b571e0(std::uint8_t* shader, const PooledString& name, StringHost& strings) {
    ref_counted_construct(shader, 0x00D62200u);
    for (std::size_t off = 8; off <= 0x18; off += 4) wr32(shader, off, 0);
    auto& target = *reinterpret_cast<PooledString*>(shader + 8);
    if (&target != &name) strings.assign_0041dd40(target, name);
}

std::uint8_t* shader_constant_list_construct_00b58400(std::uint8_t* list, const ArrayResizeFn& resize_00b580c0) {
    RawArray& rows = array_at_offset(list, 0);
    rows = RawArray{};
    wr32(list, 0x0C, 0);
    resize_00b580c0(rows, 0);
    return list;
}

void shader_constant_rows_push_back_00b57fd0(RawArray& rows, const std::uint8_t* row, const RowCopyFn& copy_row_00b57680,
    const ArrayResizeFn& reserve_00b576f0) {
    if (rows.count == rows.capacity) reserve_00b576f0(rows, apply_capacity_rule(CapacityRule::kDoubleMinOne, rows.capacity));
    copy_row_00b57680(rows.data + static_cast<std::size_t>(rows.count) * kConstantRowSize, row);
    ++rows.count;
}
// ---------------------------------------------------------------------------
// 00B529A0 old film. x87 expressions are evaluated in double and rounded to float at
// every FSTP float of the listing (f()); uniforms are kept as the double ST0 value the
// image multiplies before rounding.

namespace {
constexpr float kSentinel = -2.0f;  // 00CE7D7C
float f(double v) { return static_cast<float>(v); }
float rdf(const std::uint8_t* base, std::size_t offset) noexcept {
    float v;
    std::memcpy(&v, base + offset, 4);
    return v;
}
void wrf(std::uint8_t* base, std::size_t offset, float v) noexcept { std::memcpy(base + offset, &v, 4); }
// FILD of a dword the image treats as unsigned (FADD 2^32 when the sign bit is set).
double uint_to_double(std::uint32_t v) noexcept { return static_cast<double>(v); }
// FISTP with RC = 11b (truncation), low dword.
std::int32_t trunc_low(double v) noexcept { return static_cast<std::int32_t>(static_cast<std::int64_t>(v)); }

struct Record24 { float x0, y0, x1, y1, u0, v0, u1, v1; std::uint32_t colour; };
static_assert(sizeof(Record24) == 0x24);
constexpr Record24 kSentinelRecord = {kSentinel, kSentinel, kSentinel, kSentinel, 0, 0, 0, 0, 0};

void push_record(RawArray& list, const Record24& r, OldFilmHost& host) {
    if (list.count == list.capacity) host.reserve_records_00b51fc0(list, apply_capacity_rule(CapacityRule::kDoubleMinOne, list.capacity));
    std::memcpy(list.data + static_cast<std::size_t>(list.count) * sizeof(Record24), &r, sizeof r);
    ++list.count;
}
Record24* record_at(RawArray& list, std::uint32_t i) noexcept {
    return reinterpret_cast<Record24*>(list.data + static_cast<std::size_t>(i) * sizeof(Record24));
}
// Rebuild one child when its cached parameter changes (dust +20h/+30h, flicker +A0h/+B0h).
void rebuild_child(std::uint8_t* state, std::size_t cache, std::size_t child, std::size_t texture,
    float value, double scale, OldFilmHost& host) {
    wrf(state, cache, value);
    if (void* old = rdptr(state, child)) {
        host.release(old);
        wrptr(state, child, nullptr);
    }
    if (rdf(state, cache) > 0.0f) {
        const std::int32_t count = trunc_low(static_cast<double>(rdf(state, cache)) * scale);
        wrptr(state, child, host.create_child_00b4e840(static_cast<std::uint32_t>(count + count)));
        host.child_set_texture_slot0(rdptr(state, child), rdptr(state, texture));
    }
    wrf(state, 0xC4, 0.0f);
}
// A toggled timer (+34h/+38h, +B4h/+B8h, +BCh/+C0h): when it has run out the toggle flips
// and the timer is re-armed with U(0.5, 1.5) * (toggle ? on : off).
void toggle_timer(std::uint8_t* state, std::size_t toggle, std::size_t timer, float on, float off, OldFilmHost& host) {
    if (!(0.0f >= rdf(state, timer))) return;
    state[toggle] = state[toggle] == 0 ? 1 : 0;
    const float scale = state[toggle] ? on : off;
    wrf(state, timer, f(host.uniform_00bd2f10(0.5f, 1.5f) * scale));
}
// The random atlas column of the lines: NextU32 % (columns - 1) + 1, then
// u0 = (col - 0.25) / columns, u1 = (col + 0.75) / columns.
void line_column(Record24& r, std::uint32_t columns, OldFilmHost& host) {
    const std::uint32_t col = host.next_u32_00bd2fc0() % (columns - 1) + 1;
    const float colf = f(uint_to_double(col));
    r.u0 = f((colf - 0.25) / uint_to_double(columns));
    r.u1 = f((colf + 0.75) / uint_to_double(columns));
}
} // namespace

void old_film_render_00b529a0(std::uint8_t* state, void* targets, const float* p, bool extended,
    float a, float b, OldFilmHost& host) {
    void* surface = host.targets_color_surface_00b1f6d0(targets, 0);
    const float k_a = f(2.0 / a);                                            // 00D7A308
    const double height = uint_to_double(host.surface_height_vt20(surface));
    const float aspect2 = f(2.0 * (height / uint_to_double(host.surface_width_vt1c(surface))));
    const float bb = f(0.5 - (0.5 - b) / a);                                 // 00D7A280

    // Child rebuilds.
    if (rdf(state, 0x20) != p[3]) rebuild_child(state, 0x20, 0x30, 0x18, p[3], 100.0, host);   // 00D7A220
    const std::int32_t n1 = trunc_low(static_cast<double>(p[8]) * 10.0);                      // 00CE3DC0
    const std::int32_t n2 = trunc_low(static_cast<double>(p[13]) * 1000.0);                   // 00CE47A0
    RawArray& lines = array_at_offset(state, 0x7C);
    RawArray& line_states = array_at_offset(state, 0x8C);
    if (static_cast<std::int32_t>(rd32(state, 0x78)) != n1 + n2) {
        wr32(state, 0x78, static_cast<std::uint32_t>(n1 + n2));
        if (void* old = rdptr(state, 0x88)) {
            host.release(old);
            wrptr(state, 0x88, nullptr);
        }
        if (rd32(state, 0x78) != 0) {
            lines.count = 0;
            line_states.count = 0;
            // The pushed record is an uninitialised stack buffer in the image and the state's
            // bytes 1..3 / word +08h are stale stack words; every one of them is overwritten
            // by the update below before anything reads it (the rebuild zeroes +C4h, so the
            // update runs in this same call when the clock does not go backwards).
            const Record24 scratch{};
            for (std::uint32_t i = 0; i < rd32(state, 0x78); ++i) {
                push_record(lines, scratch, host);
                std::uint8_t line_state[12] = {};
                line_state[0] = static_cast<std::uint8_t>(host.next_u32_00bd2fc0() & 1);
                if (line_states.count == line_states.capacity)
                    host.reserve_line_states_00b52040(line_states, apply_capacity_rule(CapacityRule::kDoubleMinOne, line_states.capacity));
                std::memcpy(line_states.data + static_cast<std::size_t>(line_states.count) * 12, line_state, 12);
                ++line_states.count;
            }
            for (std::int32_t j = 0; j < n1; ++j) push_record(lines, scratch, host);
            wrptr(state, 0x88, host.create_child_00b4e840(static_cast<std::uint32_t>(lines.count) * 2));
            host.child_set_texture_slot0(rdptr(state, 0x88), rdptr(state, 0x70));
        }
        wrf(state, 0xC4, 0.0f);
    }
    if (rdf(state, 0xA0) != p[18]) rebuild_child(state, 0xA0, 0xB0, 0x98, p[18], 10.0, host);

    // Timers.
    const float now = host.clock_now_01090ab0();
    const float dt = rdf(state, 0xC8) > 0.0f ? f(static_cast<double>(now) - rdf(state, 0xC8)) : 0.0f;
    wrf(state, 0xC8, now);
    wrf(state, 0x38, f(static_cast<double>(rdf(state, 0x38)) - dt));
    wrf(state, 0x6C, f(static_cast<double>(rdf(state, 0x6C)) - dt));
    for (std::uint32_t i = 0; i < rd32(state, 0x78); ++i) {
        std::uint8_t* ls = line_states.data + i * 12;
        wrf(ls, 4, f(static_cast<double>(rdf(ls, 4)) - dt));
    }
    wrf(state, 0xB8, f(static_cast<double>(rdf(state, 0xB8)) - dt));
    wrf(state, 0xC0, f(static_cast<double>(rdf(state, 0xC0)) - dt));
    wrf(state, 0xC4, f(static_cast<double>(rdf(state, 0xC4)) - dt));

    if (0.0f >= rdf(state, 0xC4)) {
        const float inv_rate = f(1.0 / p[0]);
        wrf(state, 0xC4, f(static_cast<double>(rdf(state, 0xC4)) + inv_rate));
        if (!(0.0f < rdf(state, 0xC4))) wrf(state, 0xC4, 0.0f);

        // Dust (+30h child, +24h records).
        if (void* dust = rdptr(state, 0x30)) {
            toggle_timer(state, 0x34, 0x38, p[2], p[1], host);
            const bool on = state[0x34] != 0;
            float density = f(host.uniform_00bd2f10(on ? 0.7f : 0.2f, on ? 1.3f : 0.3f));
            if (!extended) density = f(density * 0.20000000298023224);      // double 00CE3D10
            density = f(host.uniform_00bd2f10(0.2f, 1.0f) * density);
            RawArray& records = array_at_offset(state, 0x24);
            records.count = 0;
            const float w4 = f(static_cast<double>(p[4]) * aspect2);
            const float h4 = f(static_cast<double>(p[4]) * k_a);
            const std::uint32_t half = host.child_quad_count_00b4cc00(dust) >> 1;
            const std::uint32_t n = static_cast<std::uint32_t>(trunc_low(static_cast<double>(static_cast<std::int32_t>(half)) * density));
            if (n != 0) {
                const float x_lo = f(-1.0 - w4);                             // double 00D7A250
                const float y_hi = f(h4 + 1.0);                              // double 00D7A210
                for (std::uint32_t k = 0; k < n; ++k) {
                    Record24 r{};
                    const float x = f(host.uniform_00bd2f10(x_lo, 1.0f));
                    const float y = f(host.uniform_00bd2f10(-1.0f, y_hi));
                    const std::uint32_t columns = rd32(state, 0x1C);
                    const float colf = f(uint_to_double(host.next_u32_00bd2fc0() % columns));
                    r.x0 = x;
                    r.y0 = y;
                    r.x1 = f(host.uniform_00bd2f10(0.3f, 1.7f) * w4 + x);
                    r.y1 = f(static_cast<double>(y) - host.uniform_00bd2f10(0.3f, 1.7f) * h4);
                    const double cols = static_cast<double>(f(uint_to_double(columns)));
                    r.v0 = 0.0f;
                    r.v1 = 1.0f;
                    r.u0 = f(colf / cols);
                    r.u1 = f(static_cast<double>(f(colf + 1.0)) / cols);
                    r.colour = (host.next_u32_00bd2fc0() & 1) ? 0x70FFFFFFu : 0xA0000000u;
                    push_record(records, r, host);
                }
            }
            for (std::uint32_t k = n; k < half; ++k) push_record(records, kSentinelRecord, host);
        }

        // Hair (+44h inline record, +68h child).
        if (p[5] > 0.0f && 0.0f >= rdf(state, 0x6C)) {
            wrf(state, 0x6C, f(host.uniform_00bd2f10(0.3f, 1.7f) * p[5]));
            const float size = f(host.uniform_00bd2f10(p[6], p[7]));
            const float w = f(host.uniform_00bd2f10(0.5f, 1.5f) * size * aspect2);
            const float hgt = f(host.uniform_00bd2f10(0.5f, 1.5f) * size * k_a);
            const float x0 = f(host.uniform_00bd2f10(f(-1.0 - w), 1.0f));
            const float y_hi = f((1.0 - bb) + hgt);
            const float y0 = f(host.uniform_00bd2f10(f(static_cast<double>(bb) - 1.0), y_hi));
            const std::uint32_t columns = rd32(state, 0x40);
            const float colf = f(uint_to_double(host.next_u32_00bd2fc0() % columns));
            const double cols = static_cast<double>(f(uint_to_double(columns)));
            const Record24 hair = {x0, y0, f(static_cast<double>(x0) + w), f(static_cast<double>(y0) - hgt),
                f(colf / cols), 0.0f, f(static_cast<double>(f(colf + 1.0)) / cols), 1.0f, 0xFF000000u};
            std::memcpy(state + 0x44, &hair, sizeof hair);
        } else {
            wr32(state, 0x64, 0);
        }

        // Lines (+7Ch records, +8Ch states {active, flag, timer, amplitude}, +88h child).
        const std::uint32_t line_count = rd32(state, 0x78);
        const std::uint32_t columns = rd32(state, 0x74);
        for (std::uint32_t i = 0; i < line_count; ++i) {
            Record24& r = *record_at(lines, i);
            std::uint8_t* ls = line_states.data + i * 12;
            const bool short_line = static_cast<std::int32_t>(i) < n2;      // SETL against [ESP+60h] = n2
            if (0.0f >= rdf(ls, 4)) {
                const bool was_active = ls[0] != 0;
                ls[1] = 0;
                ls[0] = was_active ? 0 : 1;
                bool activate = ls[0] != 0;
                if (!extended && activate && host.next_u32_00bd2fc0() % 3 != 0) {
                    ls[0] = 0;
                    activate = false;
                }
                if (activate) {
                    const float scale = short_line ? p[14] : p[9];
                    const float wl = f(host.uniform_00bd2f10(0.7f, 1.3f) * aspect2 * scale);
                    const float x = f(host.uniform_00bd2f10(f(-1.0 - wl), 1.0f));
                    r.x0 = x;
                    r.y0 = 1.0f;
                    r.y1 = -1.0f;
                    r.x1 = f(static_cast<double>(x) + wl);
                    float amplitude_scale;
                    if (short_line) {
                        const float length = f(host.uniform_00bd2f10(0.1f, 0.3f));
                        const float v0 = f(host.uniform_00bd2f10(0.0f, f(1.0 - length)));
                        r.v0 = v0;
                        r.v1 = f(static_cast<double>(v0) + length);
                        const std::uint32_t modulus = extended ? 9u : 7u;
                        r.colour = ((host.next_u32_00bd2fc0() % modulus + 1) << 24) | 0x00FFFFFFu;
                        amplitude_scale = p[15];
                    } else {
                        r.v0 = 0.0f;
                        r.v1 = 1.0f;
                        if ((host.next_u32_00bd2fc0() & 3) == 0) {
                            r.colour = (((host.next_u32_00bd2fc0() & 0x3F) + 0x40) << 24) | 0x00FFFFFFu;
                        } else {
                            const bool flag = (host.next_u32_00bd2fc0() & 3) == 0;
                            ls[1] = flag ? 1 : 0;
                            r.colour = flag ? 0x4000FF40u : 0xFF000000u;
                        }
                        amplitude_scale = p[10];
                    }
                    wrf(ls, 8, f(host.uniform_00bd2f10(-1.0f, 1.0f) * amplitude_scale));
                } else {
                    r = kSentinelRecord;
                }
                wrf(ls, 4, f(host.uniform_00bd2f10(p[11], p[12])));
            }
            if (ls[0] != 0) {
                const float phase = f(host.uniform_00bd2f10(10.0f, 20.0f) * rdf(ls, 4));
                const float wave = f(std::sin(static_cast<double>(phase)));   // FSIN
                const float wobble = f((static_cast<double>(wave) + 1.0) * (static_cast<double>(rdf(ls, 8)) * inv_rate));
                const std::uint32_t col = host.next_u32_00bd2fc0() % (columns - 1) + 1;
                const float colf = f(uint_to_double(col));
                r.x0 = f(static_cast<double>(wobble) + r.x0);
                r.x1 = f(static_cast<double>(wobble) + r.x1);
                r.u0 = f((colf - 0.25) / uint_to_double(columns));
                r.u1 = f((colf + 0.75) / uint_to_double(columns));
            }
        }
        // Twins of the long lines (states n2..n2+n1-1), at records line_count + j.
        for (std::int32_t j = 0; j < n1; ++j) {
            const std::uint32_t source = static_cast<std::uint32_t>(j - n1) + line_count;
            Record24& twin = *record_at(lines, line_count + static_cast<std::uint32_t>(j));
            if (line_states.data[source * 12 + 1] == 0) {
                twin = kSentinelRecord;
                continue;
            }
            twin = *record_at(lines, source);
            const float x1 = twin.x1;
            const float x0 = twin.x0;
            const float half_width = f((static_cast<double>(x1) - x0) * 0.5);
            twin.x0 = f(static_cast<double>(x0) - half_width);
            twin.x1 = f(static_cast<double>(x1) - half_width);
            line_column(twin, columns, host);
            twin.colour = 0xFF000000u;
        }

        // Flicker (+B0h child, +A4h records), extended mode only.
        if (extended && rdptr(state, 0xB0)) {
            toggle_timer(state, 0xB4, 0xB8, p[17], p[16], host);
            if (state[0xB4] != 0) {
                RawArray& records = array_at_offset(state, 0xA4);
                records.count = 0;
                const std::uint32_t half = host.child_quad_count_00b4cc00(rdptr(state, 0xB0)) >> 1;
                std::uint32_t n = static_cast<std::uint32_t>(trunc_low(host.uniform_00bd2f10(0.2f, 1.0f) * static_cast<std::int32_t>(half)));
                if (n == 0) n = 1;
                const double one_minus_bb = 1.0 - bb;
                const float y_lo = f(static_cast<double>(bb) - 1.0);
                for (std::uint32_t k = 0; k < n; ++k) {
                    const float size = f(host.uniform_00bd2f10(p[19], p[20]));
                    const float w = f(host.uniform_00bd2f10(0.8f, 1.2f) * size * aspect2);
                    const float hgt = f(host.uniform_00bd2f10(0.8f, 1.2f) * size * k_a);
                    const float x0 = f(host.uniform_00bd2f10(f(-1.0 - w), 1.0f));
                    const float y0 = f(host.uniform_00bd2f10(y_lo, f(hgt + one_minus_bb)));
                    const std::uint32_t fcolumns = rd32(state, 0x9C);
                    const float colf = f(uint_to_double(host.next_u32_00bd2fc0() % fcolumns));
                    const double cols = static_cast<double>(f(uint_to_double(fcolumns)));
                    Record24 r = {x0, y0, f(static_cast<double>(x0) + w), f(static_cast<double>(y0) - hgt),
                        f(colf / cols), 0.0f, f(static_cast<double>(f(colf + 1.0)) / cols), 1.0f, 0};
                    const std::uint32_t tint = (host.next_u32_00bd2fc0() & 3) ? 0u : 0x00473E1Au;
                    r.colour = ((host.next_u32_00bd2fc0() % 0x60 + 0x50) << 24) | tint;
                    push_record(records, r, host);
                }
                for (std::uint32_t k = n; k < half; ++k) push_record(records, kSentinelRecord, host);
            }
        }

        // Frame-wide values read by the shader: +08h, +0Ch, +10h jitter, +14h shake.
        wrf(state, 0x08, f(host.uniform_00bd2f10(0.0f, 1.0f)));
        wrf(state, 0x0C, f(host.uniform_00bd2f10(0.0f, 1.0f)));
        const float weight = extended ? 1.0f : 0.2f;
        wrf(state, 0x10, f(host.uniform_00bd2f10(f(1.0 - static_cast<double>(p[22]) * weight), 1.0f)));
        wrf(state, 0x14, 0.0f);
        if (extended && p[23] > 0.0f) {
            toggle_timer(state, 0xBC, 0xC0, p[25], p[24], host);
            if (state[0xBC] != 0) wrf(state, 0x14, f(host.uniform_00bd2f10(-0.5f, 0.5f) * p[23] * k_a));
        }
    }

    // Draws.
    if (void* dust = rdptr(state, 0x30)) {
        host.child_populate_00b4d4c0(dust, array_at_offset(state, 0x24).data);
        host.child_set_frame_target_00b4e3d0(dust, targets);
        host.child_submit_00b4def0(dust);
    }
    if (rd32(state, 0x64) != 0) {
        void* hair = rdptr(state, 0x68);
        host.child_populate_00b4d4c0(hair, state + 0x44);
        host.child_set_frame_target_00b4e3d0(hair, targets);
        host.child_submit_00b4def0(hair);
    }
    if (void* child = rdptr(state, 0x88)) {
        host.child_populate_00b4d4c0(child, lines.data);
        host.child_set_frame_target_00b4e3d0(child, targets);
        host.child_submit_00b4def0(child);
    }
    if (extended && rdptr(state, 0xB0) && state[0xB4] != 0) {
        void* child = rdptr(state, 0xB0);
        host.child_populate_00b4d4c0(child, array_at_offset(state, 0xA4).data);
        host.child_set_frame_target_00b4e3d0(child, targets);
        host.child_submit_00b4def0(child);
    }
}
} // namespace bsp::cc13_render1
