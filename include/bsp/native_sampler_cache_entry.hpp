#pragma once
#include "bsp/native_sampler_cache_operation.hpp"
#include "bsp/native_platform_load_messages.hpp"

namespace bsp {
struct NativeSamplerCacheEntryContext {
    NativeSamplerCacheContext& cache;
    void* volatile& actual_platform_0109cf04;
    NativePlatformLoadMessagesContext& messages;
    const MSG& native_message_preimage;
    const void* actual_default_text_00ce3c5c;
};
struct NativeSamplerCacheEntryOperation final {
    enum class Phase { fresh,running,complete,failed,diagnostic_retired };
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    void* captured_cache{};
    void* captured_platform{};
    const void* input_name{};
    const void* options{};
    void* result{};
    NativeSamplerCacheEntryContext* context{};
    std::optional<NativePlatformLoadMessagesOperation> pump;
    std::optional<NativeSamplerCacheOperation> continuation;
    NativeSamplerCacheEntryOperation()=default;
    ~NativeSamplerCacheEntryOperation();
    NativeSamplerCacheEntryOperation(const NativeSamplerCacheEntryOperation&)=delete;
    NativeSamplerCacheEntryOperation& operator=(const NativeSamplerCacheEntryOperation&)=delete;
    void acknowledge_diagnostic_cleanup() noexcept;
};
struct NativeSamplerDefaultLoadOperation final {
    enum class Phase { fresh,running,complete,failed,diagnostic_retired };
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    NativeSamplerLoaderSingletonStorage* receiver{};
    const void* input_name{};
    NativeSamplerCacheEntryContext* context{};
    std::uint32_t requested[2],options[2];
    char* captured_requested_data{};
    void* result{};
    bool requested_live{},options_live{};
    std::optional<NativeSamplerCacheEntryOperation> cache_child;
    NativeSamplerDefaultLoadOperation() noexcept {}
    ~NativeSamplerDefaultLoadOperation();
    NativeSamplerDefaultLoadOperation(const NativeSamplerDefaultLoadOperation&)=delete;
    NativeSamplerDefaultLoadOperation& operator=(const NativeSamplerDefaultLoadOperation&)=delete;
    void acknowledge_diagnostic_cleanup() noexcept;
};
// Full normal entry composition. ECX actual secondary cache, stacked name,
// options, low-byte retain-new/load-missing; EAX result, RET10. Capture raw
// platform before real BECCD0; only then enter unchanged B1A51D continuation.
void* load_native_sampler_cache_00b1a4f0(void* actual_cache,const void* actual_name,
    const void* options,std::uint8_t retain_new,std::uint8_t load_if_missing,
    NativeSamplerCacheEntryContext&,NativeSamplerCacheEntryOperation&);
// ECX actual1Ch singleton; stack actual name; EAX resource, RET4. Copy/lowercase
// actual pooled name, construct actual pooled seven-byte Default, call actual
// receiver+4 cache with flags1/1; release CURRENT option buffer then CAPTURED
// requested buffer with CURRENT lengths. Keep child/acquisitions on host error.
void* load_native_sampler_with_default_options_00b1b4d0(
    NativeSamplerLoaderSingletonStorage*,const void* actual_name,
    NativeSamplerCacheEntryContext&,NativeSamplerDefaultLoadOperation&);
// Explicit source ABIs, not native FH3. Failure frames own diagnostic/acquired
// state until the caller resolves every obligation. Never silently clean up a
// retained continuation, acknowledge a child, replay, or provide a fallback.
} // namespace bsp
