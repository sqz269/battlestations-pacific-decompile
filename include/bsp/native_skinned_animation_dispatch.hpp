#pragma once
#include "bsp/native_resource_root_dispatch.hpp"
#include "bsp/native_adopted_substream.hpp"

namespace bsp {
struct NativeResourceStreamReadContext;
struct NativeStringRawPoolContext;

// Bind the registered D630D8/+8 target B92F20 to its actual item reader. The
// parser receiver is unused by that native body. Existing renderer hooks,
// resource append/classification and every other parser stay in the chain.
class NativeSkinnedAnimationResourceCalls final : public NativeResourceDispatchCalls {
public:
    NativeSkinnedAnimationResourceCalls(NativeResourceDispatchCalls&,
        NativeResourceStreamReadContext&) noexcept;
    void renderer_hook(std::uintptr_t,void*) override;
    void* parse_item(std::uintptr_t,void*,void*) override;
    void append_item(std::uintptr_t,void*,void*) override;
private:
    NativeResourceDispatchCalls& other_;
    NativeResourceStreamReadContext& reads_;
};

// Compose the zero-terminal AFTER the caller's actual +4 decrement. This
// adapter neither retains nor decrements. Borrow the same raw string cells
// as the reader and the current D63700 profile words through +4. No new
// owner/registry/pool is created. Unrecognized domains are forwarded; a
// changed reached D63700 terminal is an explicit source boundary.
class NativeSkinnedAnimationReferences final : public NativeAdoptedSubstreamDispatch {
public:
    NativeSkinnedAnimationReferences(NativeAdoptedSubstreamDispatch&,
        NativeStringRawPoolContext&,const volatile std::uint32_t* profile_00d63700) noexcept;
    std::uint8_t source_is_open(std::uintptr_t,void*) override;
    std::uint32_t source_seek(std::uintptr_t,void*,std::uint32_t,std::uint32_t,std::uint32_t) override;
    void source_read(std::uintptr_t,void*,void*,std::uint32_t,std::uint32_t*) override;
    void source_write(std::uintptr_t,void*,const void*,std::uint32_t,std::uint32_t*) override;
    void source_zero_reference(std::uintptr_t,void*,std::uintptr_t) override;
private:
    NativeAdoptedSubstreamDispatch& other_;
    NativeStringRawPoolContext& strings_;
    const volatile std::uint32_t* profile_;
};
// Source composition only. Item failures retain the underlying bodies' partial
// mutations/ownership; no rollback is added. Application wiring, native ABI/EH
// transport, arbitrary profile mutation and gameplay need separate validation.
} // namespace bsp
