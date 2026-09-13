#pragma once

#include "bsp/native_texture_loading_cache.hpp"
#include <array>

namespace bsp {
struct NativeStringRawPoolContext;
struct NativeVfsNameResolutionContext;

// Configure before invoking the native body. The raw pool cells must be the
// SAME actual01090AA8/01090AA4/01090AA0 domain used by cache.strings. The existing
// concrete BDF4C0 binder checks the cache/VFS string and manager domains.
// Profiles and literal storage are borrowed current words/bytes, never copies.
struct NativeRenderServiceTextureConstructionContext {
    NativeRenderServiceTextureConstructionContext(NativeTextureCacheContext&,
        NativeStringRawPoolContext&, NativeVfsNameResolutionContext&,
        const volatile std::uint32_t* renderer_profile_00d5f0a8,
        const volatile std::uint32_t* texture_profile_00d61948,
        const void* kosz_01_00d620a0, const void* szor_01_00d62094,
        const void* csikok_00d62088, const void* splotch_00d6207c);
    NativeTextureCacheContext& cache;
    NativeStringRawPoolContext& strings;
    const volatile std::uint32_t* renderer_profile;
    const volatile std::uint32_t* texture_profile;
    std::array<const void*,4> literal_storage;
};

// Caller-retained single invocation, not another texture owner/refcount. The
// actual local24h region retains four raw8h string headers, the saved owner,
// and the first header-length slot subsequently used for saved-height spills.
// It starts uninitialized; only native reached stores initialize its bytes.
// Failed child cache/VFS acquisitions MUST remain alive until externally
// resolved under their existing contracts; their destructors reject unresolved
// failures. Do not copy, reset or retry this parent/its children after entry.
struct NativeRenderServiceTextureConstructionAcquired {
    enum class Phase { fresh, running, complete, failed };
    NativeRenderServiceTextureConstructionAcquired() = default;
    NativeRenderServiceTextureConstructionAcquired(const NativeRenderServiceTextureConstructionAcquired&) = delete;
    NativeRenderServiceTextureConstructionAcquired& operator=(const NativeRenderServiceTextureConstructionAcquired&) = delete;
    Phase phase{Phase::fresh};
    void* owner{};
    int unwind_state{-1};
    std::uint32_t native_site{};
    alignas(4) unsigned char native_locals_10_33[0x24];
    std::array<NativeTextureCacheAcquired,4> loads;
};

// Complete B52550..B5283F,752 bytes. Original ECX actualCCh allocation, zero
// stack arguments, EAX same owner, RET. No allocation or owner-size substitution.
// Compose fresh current renderer/profile loads, native temporary cleanup,
// publication-before-cleanup, saved dimensions, unsigned DIV/SHR and FH3 states.
// Supported current profiles/targets: D5F0A8/+64=B319B0, D61948 dimensions
// selecting B3CE70/B3CE80. Unsupported dispatch throws an explicit boundary;
// no successful fallback, null texture guard or default ratio is provided.
// Accessed storage, literal/context pointer values and private frames remain
// valid at native accesses. Original caller ABI, unrestricted fault/native EH
// identity, concurrency, cold-load coverage and gameplay require separate proof.
void* construct_native_render_service_textures_00b52550(void* actual_owner,
    NativeRenderServiceTextureConstructionContext&,
    NativeRenderServiceTextureConstructionAcquired&);
} // namespace bsp
