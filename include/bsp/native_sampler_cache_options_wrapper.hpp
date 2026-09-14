#pragma once

#include "bsp/native_sampler_cache_entry.hpp"

#include <cstdint>
#include <optional>

namespace bsp {

// Host-owned diagnostic frame. The two DWORDs in requested are the actual
// temporary name header, but their placement is a source ABI, not native
// EBP-14h. Initial-copy failure has no native cleanup action; the caller must
// resolve any retained partial header before retiring this frame. A failed
// cache child is never acknowledged or discarded by this wrapper.
struct NativeSamplerOptionsLoadOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    NativeSamplerLoaderSingletonStorage* receiver{};
    const void* captured_name{};
    const void* late_options{};
    std::uint32_t requested[2];
    char* captured_requested_data{};
    void* result{};
    bool temporary_obligation{};
    bool cleanup_armed{};
    std::optional<NativeSamplerCacheEntryOperation> cache_child;
    NativeSamplerOptionsLoadOperation() noexcept {}
    ~NativeSamplerOptionsLoadOperation();
    NativeSamplerOptionsLoadOperation(const NativeSamplerOptionsLoadOperation&) = delete;
    NativeSamplerOptionsLoadOperation& operator=(const NativeSamplerOptionsLoadOperation&) = delete;
    // Caller first resolves any unarmed partial header and failed child.
    // This method frees nothing and cannot replay the operation.
    void acknowledge_diagnostic_cleanup() noexcept;
};

struct NativeSamplerOptionsWrapperContext {
    NativeSamplerCacheEntryContext& cache;
    NativeSamplerOptionsLoadOperation& operation;
};

// Complete normal B1B400..B1B4C0 source schedule. Native ECX is the actual
// 1Ch complete loader, native stack contains name/options and RET8 consumes
// those two words. This source __fastcall adds EDX as a stable context; its
// naked adapter forwards addresses of the ORIGINAL public stack slots. The
// private body captures name at entry but reads options only after lowercase.
// It calls the concrete B1A4F0 cache at loader+4 with literal flags 1/1.
// Source C++ cleanup covers the recovered state-0 current-header action, not
// native FH3/SEH faults. Valid actual pool, cache, platform, profile and owner
// domains are required; no fallback or resource release is supplied here.
void* __fastcall load_native_sampler_with_options_00b1b400(
    NativeSamplerLoaderSingletonStorage* actual_loader,
    NativeSamplerOptionsWrapperContext& context,
    const void* actual_name, const void* actual_options);

} // namespace bsp
