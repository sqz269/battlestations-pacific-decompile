#pragma once
#include "bsp/native_d3d9_shader_construction.hpp"
#include "bsp/native_ref_counted.hpp"

namespace bsp {
// Borrow the SAME application publication slots/domain used for construction.
// Table pointers view current original numeric code words, not host vtables.
struct NativeD3d9ShaderLifetimeContext {
    NativeResourceSupportStorage* volatile& actual_support_0108fedc;
    SingletonLifetimeDomain& actual_lifetime;
    void* const volatile& actual_renderer_00f8d394;
    NativeRenderActualOwners& owners;
    const volatile std::uint32_t* actual_pixel_profile_00d62a60;
    const volatile std::uint32_t* actual_vertex_profile_00d62a70;
};

// Persistent diagnostic metadata outside the actual10h owner. Failure leaves
// the partial native work and base-cleanup effect; it never retries Release.
// Keep frame, domain and accessed owners alive and exclude independent
// retirement/replacement while running/failed. A successful scalar deletion
// may free owner; its recorded address is then identity-only metadata.
struct NativeD3d9ShaderLifetimeOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    Phase phase{Phase::fresh};
    std::uint32_t function{}, destructor_function{}, native_site{}, flags{};
    NativeD3d9ShaderStorage* owner{};
    void* captured_renderer{};
    void* captured_com{};
    bool support_entered{}, support_returned{}, unregister_returned{};
    bool com_release_entered{}, com_release_returned{}, base_destroyed{};
    bool free_entered{}, free_returned{};
    NativeD3d9ShaderLifetimeOperation() = default;
    ~NativeD3d9ShaderLifetimeOperation();
    NativeD3d9ShaderLifetimeOperation(const NativeD3d9ShaderLifetimeOperation&) = delete;
    NativeD3d9ShaderLifetimeOperation& operator=(const NativeD3d9ShaderLifetimeOperation&) = delete;
    // Only AFTER caller resolves failed state; metadata retirement, no replay.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// ECX actual renderer, stacked raw wrapper, AL found, RET4. Forward address of
// that pointer cell to actual registry first-match swap removal; no owner release.
bool unregister_native_vertex_shader_00b268a0(void*, void*) noexcept;
bool unregister_native_pixel_shader_00b268c0(void*, void*) noexcept;
// ECX actual10h owner, RET. Pixel: support then fresh renderer unregister;
// vertex: renderer unregister then support. Read current08, call current COM
// virtual8 Release and only then clear08. Preserve04/0C and restore CEB130.
// State0 cleanup calls the established BD30F0 base on C++ dependency exceptions;
// this reproduces that cleanup effect, not the original private FH3 ABI.
void destroy_native_pixel_shader_00b5f410(NativeD3d9ShaderStorage&,
    NativeD3d9ShaderLifetimeContext&, NativeD3d9ShaderLifetimeOperation&);
void destroy_native_vertex_shader_00b5f490(NativeD3d9ShaderStorage&,
    NativeD3d9ShaderLifetimeContext&, NativeD3d9ShaderLifetimeOperation&);
// ECX owner, stacked flags, EAX original pointer even after free, RET4.
// BF65AC shared actual allocator free iff flags&1 AFTER complete destruction.
NativeD3d9ShaderStorage* delete_native_pixel_shader_00b5f6e0(NativeD3d9ShaderStorage*,
    std::uint32_t, NativeD3d9ShaderLifetimeContext&, NativeD3d9ShaderLifetimeOperation&);
NativeD3d9ShaderStorage* delete_native_vertex_shader_00b5f700(NativeD3d9ShaderStorage*,
    std::uint32_t, NativeD3d9ShaderLifetimeContext&, NativeD3d9ShaderLifetimeOperation&);

class NativeD3d9ShaderReference;
struct NativeD3d9ShaderCompanionDisposal {
    void* context;
    // After actual destruction/free; unbind/dispose the canonical companion.
    // No raw-owner or companion access occurs after this callback.
    void (*retire)(void*, NativeD3d9ShaderReference&) noexcept;
};
// One canonical companion in context.owners, borrowing SAME raw+04. No count
// initialization or extra retain. Current virtual0 must be BD30E0; its current
// virtual4 can select either established shader scalar destructor. Other tables
// or code words fail admission/dispatch, with no no-op terminal fallback.
class NativeD3d9ShaderReference final : public RenderCommandReference,
    private NativeRefCountedDeleteCalls {
public:
    NativeD3d9ShaderReference(NativeD3d9ShaderStorage&, NativeD3d9ShaderLifetimeContext&,
        NativeD3d9ShaderCompanionDisposal);
    ~NativeD3d9ShaderReference() override;
    NativeD3d9ShaderReference(const NativeD3d9ShaderReference&) = delete;
    NativeD3d9ShaderReference& operator=(const NativeD3d9ShaderReference&) = delete;
    void release_zero_references() noexcept override;
private:
    enum class Phase { bound, destroying, retired };
    NativeD3d9ShaderStorage& storage_;
    NativeD3d9ShaderLifetimeContext& context_;
    NativeD3d9ShaderCompanionDisposal disposal_;
    NativeD3d9ShaderLifetimeOperation terminal_;
    Phase phase_{Phase::bound};
    const volatile std::uint32_t* table(std::uint32_t) const noexcept;
    void require_virtual0() const noexcept;
    void delete_vslot04(void*, std::uint32_t, std::uint32_t) override;
};
} // namespace bsp
