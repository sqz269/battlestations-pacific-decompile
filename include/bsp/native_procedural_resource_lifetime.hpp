#pragma once
#include "bsp/native_render_context.hpp"

namespace bsp {
struct NativeProceduralResourceLifetimeContext {
    NativeRenderActualOwners& owners;
    const volatile std::uint32_t* actual_profile_00d64478;
    const volatile std::uint32_t* actual_profile_00d644b4;
};

// Host diagnostic, never embedded in the native34h allocation. Failure can
// leave a decremented, unresolved child while array cleanup has already freed
// the containing pointer slots. Caller owns those remaining obligations.
struct NativeProceduralResourceLifetimeOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    Phase phase{Phase::fresh};
    std::uint32_t function{}, native_site{};
    void* owner{};
    void* child{};
    void* array_allocation{};
    bool child_release_started{}, child_release_returned{};
    bool array_free_returned{}, base_destroyed{}, owner_free_returned{};
    NativeProceduralResourceLifetimeOperation() = default;
    ~NativeProceduralResourceLifetimeOperation();
    NativeProceduralResourceLifetimeOperation(const NativeProceduralResourceLifetimeOperation&) = delete;
    NativeProceduralResourceLifetimeOperation& operator=(const NativeProceduralResourceLifetimeOperation&) = delete;
    // Frees nothing; caller first resolves retained storage/canonical owners.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// Complete737390..7373DF: ECX actual12-byte data/count/capacity header,
// stack signed requested count, RET4. Existing735FF0 reserve; raw32 pointer
// arithmetic, zero placement with null-slot skip, decrement-before-shrink,
// final count publication. Pointer slots have no element destructor.
void resize_native_procedural_pointer_array_00737390(void*, std::int32_t requested);
// Complete B19750..B1975A: stampD5C104 then tail BD30F0 stampCEB130.
void destroy_native_procedural_resource_base_00b19750(void*) noexcept;
// Complete C304A0..C3054B normal body and source-exception cleanup. ECX owner,
// RET. StampD79B54; current last child +04 decrement / zero-only current
// virtual0 dispatch; reread count, decrement; resize0/free CURRENT +10 data;
// B19750. State1 unwind clears/frees vector then base; state0 only base.
// No payload/string cleanup at +08/+0C and no field nulling after free.
void destroy_native_procedural_resource_00c304a0(void*, NativeRenderActualOwners&,
    NativeProceduralResourceLifetimeOperation&);
// Complete BBC6D0..BBC6ED and BBC7F0..BBC80D. ECX owner, stack flags,
// RET4; EAX captured owner, even after free. Free iff successful dtor and bit0.
void* delete_native_caustics_resource_00bbc6d0(void*, std::uint32_t flags,
    NativeRenderActualOwners&, NativeProceduralResourceLifetimeOperation&);
void* delete_native_shore_wave_resource_00bbc7f0(void*, std::uint32_t flags,
    NativeRenderActualOwners&, NativeProceduralResourceLifetimeOperation&);

class NativeProceduralResourceReference;
struct NativeProceduralResourceCompanionDisposal {
    void* context;
    // Called after concrete terminal destruction/free. Remove caller's SAME
    // canonical binding and optionally dispose companion. No access follows.
    void (*retire)(void*, NativeProceduralResourceReference&) noexcept;
};

// Stable companion for an actual BBC6F0/BBC810-created allocation. Borrows
// actual+04 without initialization or retain; no parallel identity map.
// Caller admits it to the SAME NativeRenderActualOwners domain used by the
// sampler/cache/render references and retains all profile/context dependencies.
// Its terminal validates current virtual0=BD30E0, uses that concrete invoker,
// rereads current profile and requires recovered slot04 BBC6D0/BBC7F0.
// Current supported profile can differ from the profile seen at construction.
class NativeProceduralResourceReference final : public RenderCommandReference {
public:
    NativeProceduralResourceReference(void* actual_resource,
        NativeProceduralResourceLifetimeContext&, NativeProceduralResourceCompanionDisposal);
    ~NativeProceduralResourceReference() override;
    NativeProceduralResourceReference(const NativeProceduralResourceReference&) = delete;
    NativeProceduralResourceReference& operator=(const NativeProceduralResourceReference&) = delete;
    void release_zero_references() noexcept override;
private:
    enum class Phase { bound, destroying, retired };
    void* actual_resource_;
    NativeProceduralResourceLifetimeContext& context_;
    NativeProceduralResourceCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
};

// Valid aligned actual Win32 storage required; null children are not skipped.
// Missing identity/profile is an error, never successful destruction. Canonical
// terminal dependencies must not throw. Direct bodies expose source exceptions
// with diagnostics; native FH3/SEH/hardware-fault identity is not reproduced.
// Existing actual heap/vector and shared reference providers are reused.
// These are new C++ APIs, not binary replacements or game validation.
} // namespace bsp
