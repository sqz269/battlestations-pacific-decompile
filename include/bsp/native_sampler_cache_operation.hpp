#pragma once
#include "bsp/native_sampler_loader_context.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <optional>

namespace bsp {
struct NativeVfsDateRouteContext;

struct NativeSamplerCacheContext {
    ActualNativeStringPoolStorage& strings;
    const SingletonLifetimeCallbacks& validation;
    void* volatile& actual_manager_01090aa0;
    void* volatile& actual_factory_registry_00f8d41c;
    const NativeResourceRegistryLookupContext& factories;
    NativeVfsDateRouteContext& dates;
    // Borrow actual original profile storage; re-read its reached slot. This
    // is the CE7D24 procedural route, not D5F088 texture loading.
    const void* actual_secondary_profile_00ce7d24;
};

// Persistent host frame over actual native headers/record. Native fields stay
// uninitialized until their original construction point. This is not FH3 and
// does not automatically release a created resource or an orphaned alias.
struct NativeSamplerCacheOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    void* actual_cache{};
    const void* input_name{};
    const void* options{};
    NativeSamplerCacheContext* context{};
    std::uint32_t requested[2], resolved[2], hidden_or_resource[2];
    std::uint32_t date_output[5];
    NativeRenderResourceRecord record;
    void* result{};
    void* created_resource{};
    NativeRenderResourceAliasNode* pending_alias{};
    bool requested_live{}, resolved_live{}, hidden_live{};
    bool record_name_live{}, record_live{}, append_entered{}, append_returned{};
    std::optional<NativeSamplerLoaderOperation> factory_child;
    NativeSamplerCacheOperation() noexcept {} // Preserve unspecified raw fields.
    ~NativeSamplerCacheOperation();
    NativeSamplerCacheOperation(const NativeSamplerCacheOperation&) = delete;
    NativeSamplerCacheOperation& operator=(const NativeSamplerCacheOperation&) = delete;
    // Caller resolves actual string/record/child/orphan obligations first.
    // This only retires diagnostics; no implicit cleanup, retry or rollback.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// PARTIAL ENTRY: complete normal continuation B1A51D..B1AA29 only. Caller MUST
// first execute actual BECCD0 on the raw platform captured from current0109CF04
// at B1A50E, using the actual message/XLive/cursor domain, before invoking this
// function. Pass the cache receiver captured before that pump and the original
// argument pointers. Do not pre-copy/read the name to admit this continuation.
// This function begins its first name read after entry. It neither calls a
// projected pump nor substitutes a callback or a successful pump marker.
//
// Original enclosing entry: ECX secondary cache; stack name/options/low-byte
// retain-new/low-byte load-if-missing; EAX resource, RET10. The source interface
// has its own ABI. B1A4F0 and B1B4D0 remain unimplemented full entries until the
// actual platform provider and wrapper composition are supplied.
void* continue_native_sampler_cache_after_pump_00b1a51d(void* captured_cache,
    const void* actual_name, const void* options, std::uint8_t retain_new,
    std::uint8_t load_if_missing, NativeSamplerCacheContext&,
    NativeSamplerCacheOperation&);

// Keep all actual owners, raw publications and contexts alive and exclude
// retirement while running/failed. Existing child helpers retain their own
// documented host EH/orphan limits. No resource terminal, private FH3, original
// binary ABI, whole-entry loading or game behavior is established here.
} // namespace bsp
