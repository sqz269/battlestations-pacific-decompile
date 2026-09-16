#pragma once
#include "bsp/allocator_list.hpp"
#include "bsp/native_render_context.hpp"
#include <array>
#include <cstdlib>

namespace bsp {

// Actual 60h object; the pool's separate slab-index DWORD is at +60.
struct NativeMeshSectionStorage {
    volatile std::uint32_t native_vtable_00;
    std::atomic<std::int32_t> references_04;
    std::uint32_t primitive_08;
    std::uint32_t range_words_0c[4];
    std::uint32_t instance_count_1c;
    void* material_20;
    std::uint32_t bounds_bits_24[4];
    std::uint32_t word_34;
    void* next_section_38;
    void* vertex_streams_3c[4];
    std::int32_t vertex_stream_count_4c;
    void* vertex_layout_50;
    std::uint32_t uninterpreted_54;
    std::uint8_t indexed_58;
    std::array<std::byte, 3> untouched_59;
    void* instance_generator_binding_5c;
};

// Exact canonical 010901D4 pool storage, sharing the existing allocator list.
struct NativeMeshSectionPoolStorage {
    AllocatorListElement allocator_00;
    alignas(4) std::byte critical_section_0c[24];
    std::int32_t recursion_24;
    std::byte** slabs_28;
    std::uint32_t slab_count_2c;
    std::uint32_t table_capacity_30;
    std::uint32_t first_free_slab_34;
};
class NativeMeshSectionPool {
public:
    static constexpr std::uint32_t native_vtable = 0x00d6319c;
    static constexpr std::uint32_t native_virtual0 = 0x00b85c90;
    static constexpr std::size_t object_bytes = 0x60, slot_bytes = 0x64;
    static constexpr std::size_t slot_slab_index_offset = 0x60, slab_bytes = 0x1984;
    static constexpr std::uint32_t slots_per_slab = 64;
    NativeMeshSectionPool(AllocatorListDomain&, NativeMeshSectionPoolStorage&);
    ~NativeMeshSectionPool() = default; // Explicit native lifecycle below.
    NativeMeshSectionPool(const NativeMeshSectionPool&) = delete;
    NativeMeshSectionPool& operator=(const NativeMeshSectionPool&) = delete;
    void initialize_00b85bb0();
    void* allocate_slot_00b85d30();
    void return_slot_00b859b0(void*) noexcept;
    void trim_empty_slabs_00b85c90();
    void destroy_00b858f0(); // Frees slabs without invoking payload destructors.
    NativeMeshSectionPoolStorage& storage() noexcept { return storage_; }
    static std::uint32_t live_slab_index(const void*) noexcept;
private:
    AllocatorListDomain& allocator_list_;
    NativeMeshSectionPoolStorage& storage_;
    static void invoke_trim(void*);
    void destroy_critical_section_00402f70() noexcept;
    void free_table_unwind_00b85720() noexcept;
};
// Original allocator entry replaces ECX (requested60h) with canonical010901D4.
// Caller supplies that SAME concrete pool; no process-global substitute pool.
void* allocate_native_mesh_section_slot_00b85ee0(NativeMeshSectionPool&);
void return_native_mesh_section_slot_00b85b20(NativeMeshSectionPool&, void*);

// 00CD8250/00CE0EC0 select the actual 010901D4 pool. Its storage, companion
// and shared allocator domain must outlive the real CRT exit callback.
using NativeMeshSectionPoolAtexit = int (*)(void (*)());
void bind_static_native_mesh_section_pool_010901d4(NativeMeshSectionPool& pool);
int initialize_static_native_mesh_section_pool_00cd8250(
    NativeMeshSectionPoolAtexit register_atexit = &std::atexit);
void destroy_static_native_mesh_section_pool_00ce0ec0();

struct NativeMeshSectionEnvironment {
    NativeMeshSectionPool& pool_010901d4;
    NativeRenderActualOwners& retained_owners;
    const volatile std::uint32_t* vtable_00d63194; // Actual current two-slot table.
};
// ECX storage, EAX same, RET. Constant is read before any stores. Preserves
// unused +3C..48 and +59..5B bytes and pool+60; no implicit initializer.
NativeMeshSectionStorage* construct_native_mesh_section_00b857f0(
    void*, const volatile std::uint32_t& bounds_w_00ce4970) noexcept;
// No semantic native input, EAX pointer, RET. Null allocation skips ctor.
NativeMeshSectionStorage* create_native_mesh_section_00533fa0(
    NativeMeshSectionPool&, const volatile std::uint32_t& bounds_w_00ce4970);

// ECX fresh destination, stack source, EAX destination, RET4. Copies the
// actual section while retaining its material, next section, active streams,
// layout and generator binding in the SAME owner domain. Five float words at
// +24..34 pass through x87; +54 copies without retain; unused streams, +59..5B
// and pool+60 remain untouched. Source cells/count are read in native order.
// The caller allocates the raw slot and registers the one canonical companion.
// On constructor unwind only the base profile is restored, as in CC24B0;
// retained resources are not rolled back. Fresh distinct storage, valid owner
// identities and masked x87 exceptions form the supported construction domain.
NativeMeshSectionStorage* copy_construct_native_mesh_section_00b85ef0(
    void*, const NativeMeshSectionStorage&, NativeRenderActualOwners&,
    const volatile std::uint32_t& bounds_w_00ce4970);

// ECX section, pointer on stack, RET4; stream setter has index+pointer, RET8.
// Equal identity skips. Publish then retain incoming actual+04, release old.
// Stream replacement requires an already initialized selected slot; it neither
// grows the count nor initializes an unused constructor-poisoned slot.
void set_native_mesh_section_material_00b864c0(
    NativeMeshSectionStorage&, NativeRenderActualOwners&, void*);
void set_native_mesh_section_vertex_stream_00b86500(
    NativeMeshSectionStorage&, NativeRenderActualOwners&, std::int32_t index, void*);
void set_native_mesh_section_vertex_layout_00b86650(
    NativeMeshSectionStorage&, NativeRenderActualOwners&, void*);
void set_native_mesh_section_instance_generator_binding_00b417e0(
    NativeMeshSectionStorage&, NativeRenderActualOwners&, void*);
// Retain nonnull actual stream, append, then increment count. No capacity check.
void append_native_mesh_section_vertex_stream_00b85b80(NativeMeshSectionStorage&, void*);
void set_native_mesh_section_indexed_00b85600(NativeMeshSectionStorage&, std::uint8_t) noexcept;
// Reload count after each callback, release then clear visited slots, count=0.
void clear_native_mesh_section_vertex_streams_00b86550(
    NativeMeshSectionStorage&, NativeRenderActualOwners&);
// Releases material, streams, layout in order; DOES NOT reset stream count.
void clear_native_mesh_section_resources_00b86390(
    NativeMeshSectionStorage&, NativeRenderActualOwners&);

// Actual stack key passed to renderer virtual+40: four descriptor identities
// followed by signed count. Only active cells are initialized. No extra retain.
struct NativeMeshSectionLayoutKey { void* descriptors[4]; std::int32_t count; };
class NativeMeshSectionLayoutServices {
public:
    virtual ~NativeMeshSectionLayoutServices() = default;
    virtual void* current_stream_descriptor_virtual24(void* actual_stream) = 0;
    // Must dispatch the CURRENT actual renderer virtual+40, returning its owned
    // native layout reference (e.g. existing B2F710 factory/tree/pool domain).
    virtual void* current_renderer_layout_virtual40(const NativeMeshSectionLayoutKey&) = 0;
};
// ECX section, stack actual mesh, RET4. Existing layout first clears streams;
// count0 selects actual mesh+64 stream0, else keeps selections. No COM defaults.
void rebuild_native_mesh_section_vertex_layout_00b865a0(
    NativeMeshSectionStorage&, NativeRenderActualOwners&, void* actual_mesh,
    NativeMeshSectionLayoutServices&);

// ECX section, RET. Publish D63194, release+38,+5C, clear resources; base CEB130
// on success or C++ unwind. No callback retry on unwind and no release of+54.
void destroy_native_mesh_section_00b86420(
    NativeMeshSectionStorage&, NativeRenderActualOwners&);
// ECX section, stack flags, EAX original address, RET4. Return to canonical
// pool iff flags&1 after successful destructor. Does not directly heap-free.
NativeMeshSectionStorage* delete_native_mesh_section_00b86690(
    NativeMeshSectionStorage*, NativeMeshSectionEnvironment&, std::uint32_t flags);

class NativeMeshSectionReference;
struct NativeMeshSectionCompanionDisposal {
    void* context;
    void (*retire)(void*, NativeMeshSectionReference&) noexcept;
};
// One canonical companion borrowing the SAME actual+04; construction does not
// retain. Register in NativeRenderActualOwners before mesh+54 publication.
// Current profile must be D63194 with slots BD30E0/B86690 at terminal dispatch.
class NativeMeshSectionReference final : public RenderCommandReference {
public:
    NativeMeshSectionReference(NativeMeshSectionStorage&, NativeMeshSectionEnvironment&,
        NativeMeshSectionCompanionDisposal);
    ~NativeMeshSectionReference() override;
    NativeMeshSectionReference(const NativeMeshSectionReference&) = delete;
    NativeMeshSectionReference& operator=(const NativeMeshSectionReference&) = delete;
    NativeMeshSectionStorage& storage() noexcept { return storage_; }
    void release_zero_references() noexcept override;
private:
    enum class Phase { bound, destroying, retired };
    NativeMeshSectionStorage& storage_;
    NativeMeshSectionEnvironment& environment_;
    NativeMeshSectionCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
    void require_current_profile() const noexcept;
};

// Valid native domain: actual aligned live owners; 0<=count<=4; in-range stream
// index; nonnull appended/selected stream. Real pool/new-handler/list lifetimes,
// current material/layout/stream owners and callbacks are caller dependencies.
// Native material/effect construction, generator attachment B85610/B451D0,
// section assignment, renderer submission and game ABI external.
} // namespace bsp
