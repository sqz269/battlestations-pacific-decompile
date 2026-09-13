#pragma once

#include "bsp/native_logical_buffer_device_restore.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {

// Required real renderer recreation. B29670 is not reconstructed by this
// packet: a provider must execute its actual resource/device lifecycle on the
// supplied current renderer. A no-op/success stand-in is outside this contract.
class NativeLogicalVertexDeviceRecreation {
public:
    virtual ~NativeLogicalVertexDeviceRecreation() = default;
    virtual void call_00b29670(void* actual_renderer) = 0;
};

struct NativeLogicalVertexOwnerContext {
    const void* volatile& actual_renderer_00f8d394;
    NativeRendererSynchronizationGlobals& actual_synchronization_0108d6dc;
    NativePhysicalBufferOwnerContext& actual_physical;
    const NativeLogicalBufferDeviceRestoreProfiles& actual_physical_profiles;
    NativeRenderActualOwners& actual_owners;
    void* actual_logical_vertex_pool_0108fe18;
    volatile std::uint32_t& actual_next_id_0108fee0;
    const volatile std::uint32_t* actual_type_sizes_00d61cc0;
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8;
    const volatile std::uint32_t* actual_logical_profile_00d61d6c;
    NativeLogicalVertexDeviceRecreation& actual_device_recreation;
    // Original ctor's uninitialized pool local, entry ESP-1Ch. Required input
    // only when flags' low nibble exceeds3; never a synthesized default pool.
    std::uint32_t native_pool_stack_bits;
};

// Complete raw declaration leaves, ECX declaration, stack usage/occurrence,
// RET8. Has returns AL only; Find returns flat index or FFFFFFFF. Each raw
// record is14h bytes. No bounds checks, semantic conversion, or host vectors.
bool native_vertex_declaration_has_semantic_00b47c90(const void*, std::uint32_t usage,
    std::uint32_t occurrence) noexcept;
std::uint32_t native_vertex_declaration_find_semantic_00b47ce0(const void*,
    std::uint32_t usage, std::uint32_t occurrence) noexcept;
std::uint32_t native_vertex_declaration_semantic_offset_00b47c40(const void*,
    std::uint32_t usage, std::uint32_t occurrence) noexcept;
std::uint32_t native_vertex_declaration_semantic_type_00b47c20(const void*,
    std::uint32_t usage, std::uint32_t occurrence) noexcept;
std::uint32_t native_vertex_declaration_semantic_size_00b47c60(const void*,
    std::uint32_t usage, std::uint32_t occurrence,
    const volatile std::uint32_t* actual_type_sizes_00d61cc0) noexcept;

// Full74h actual logical storage; trailing slab index+74 in78h slot preserved.
// Base ctor B61E20: ECX owner, stack declaration, EAX owner, RET4. Base dtor
// B62010: ECX owner, RET. +4C is a retained actual identity of unresolved runtime
// type: decrement SAME+04 and resolve canonical CURRENT terminal only at zero.
// This supports the full sequence; it does not establish the nonnull producer.
void* construct_native_logical_vertex_base_00b61e20(void*, const void* declaration,
    NativeLogicalVertexOwnerContext&) noexcept;
void destroy_native_logical_vertex_base_00b62010(void*, NativeRenderActualOwners&);

// Full ctor B4BC00: ECX owner, stack count/declaration/flags, EAX owner, RET0Ch.
// Dynamic uses actual shared physical+1974. Private uses real CreateVertexBuffer,
// current device retry through required B29670, private physical owner and real
// Attach. COM temporary Release is unconditional, including original null fault.
void* construct_native_logical_vertex_stream_00b4bc00(void*, std::uint32_t count,
    void* declaration, std::uint32_t flags, NativeLogicalVertexOwnerContext&);
// Complete ECX owner/RET and ECX owner/stack flags/EAX same owner/RET4.
void destroy_native_logical_vertex_stream_00b4b5d0(void*, NativeLogicalVertexOwnerContext&);
void* delete_native_logical_vertex_stream_00b4bf10(void*, std::uint32_t flags,
    NativeLogicalVertexOwnerContext&);

// Full pool allocation/initialization:32 slots of78h in F44h slab; real initialized
// CRITICAL_SECTION+0C and actual slab array+28/+2C/+30, first available+34.
// Native B4AE80 ECX pool/RET/EAX slot; B4B370 discards ECX/tail-jumps B4AE80;
// B48EB0 ECX slab, stack index, EAX slab, RET4. No allocation-failure repair.
void* initialize_native_logical_vertex_slab_00b48eb0(void*, std::uint32_t index) noexcept;
void* allocate_native_logical_vertex_slot_00b4ae80(void* actual_pool);
void* allocate_native_logical_vertex_stream_00b4b370(void* actual_pool_0108fe18);

// Full actual renderer pointer-array reserve; ECX header, stack capacity, RET4.
void reserve_native_renderer_pointer_array_00b22d10(void*, std::uint32_t capacity);
// Complete actual physical raw-pointer registration; ECX physical/stack stream/
// RET4. Optional native guard; no AddRef. Inner tracked lock belongs to caller.
void register_native_physical_vertex_stream_00b4b1e0(void*, void* actual_stream,
    NativeLogicalVertexOwnerContext&);
// Complete seven-byte B1FE50, ECX renderer, AL raw byte+19AC, RET.
std::uint8_t native_renderer_secondary_stream_registration_00b1fe50(const void*) noexcept;
// Complete B287C0: ECX renderer, stack count/flags/declaration, EAX same registered
// owner, RET0Ch. Appends even null to actual+1AAC and iff current+58 returns AL1
// to actual+19B0. No semantic stream, cache, implicit retain or shadow registry.
void* create_native_registered_vertex_stream_00b287c0(void* actual_renderer,
    std::uint32_t count, std::uint32_t flags, void* declaration,
    NativeLogicalVertexOwnerContext&, void** acquired_before_registration = nullptr);
// Optional source-interface publication receives the completed creator before
// renderer-array registration. A later exception preserves it; constructor
// failure still follows native allocation unwind and does not publish. This
// neither retains the object nor registers a canonical host companion.

class NativeLogicalVertexReference;
struct NativeLogicalVertexCompanionDisposal {
    void* context;
    void (*retire)(void*, NativeLogicalVertexReference&) noexcept;
};
// One canonical companion per raw owner, borrowing its SAME actual+04. Binding
// does not initialize or retain it. CURRENT D61D6C/BD30E0/B4BF10 terminal only.
class NativeLogicalVertexReference final : public RenderCommandReference {
public:
    NativeLogicalVertexReference(void* actual_stream, NativeLogicalVertexOwnerContext&,
        NativeLogicalVertexCompanionDisposal);
    ~NativeLogicalVertexReference() override;
    NativeLogicalVertexReference(const NativeLogicalVertexReference&) = delete;
    NativeLogicalVertexReference& operator=(const NativeLogicalVertexReference&) = delete;
    void* storage() const noexcept { return actual_stream_; }
    void release_zero_references() noexcept override;
private:
    enum class Phase { bound, destroying, retired };
    void* actual_stream_;
    NativeLogicalVertexOwnerContext& context_;
    NativeLogicalVertexCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
    void require_current_profile() const noexcept;
};
// New source interfaces, not original binary ABI. Valid raw storage, immutable
// observed profile cells and real services are required. Runtime identity at
// base+4C and whole renderer recreation remain separately unproven boundaries.
} // namespace bsp
