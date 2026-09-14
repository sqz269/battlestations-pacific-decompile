#pragma once
#include "bsp/native_adopted_substream.hpp"
#include <cstdint>

namespace bsp {
struct NativeResourceManagerContext;
class NativeMaterialParameterPool;

struct NativeResourceContainerLifetimeContext {
    NativeResourceManagerContext& manager;
    // The companion borrowing actual static owner0109022C, also used by the
    // hierarchy parser. Its initialized storage/list must outlive every record.
    NativeMaterialParameterPool& hierarchy_pool_0109022c;
    NativeAdoptedSubstreamDispatch& item_references;
};

// B88430[636]: native ECX actual44h-or-larger resource, RET. StampD63228;
// decrement each CURRENT primary item once, dispatch current slot0 at zero;
// actual4C1400 getter then B801C0 name-key removal; destroy each nonnull
// hierarchy payload and return its captured88h pool slot; free hierarchy then
// primary arrays; return captured current name data; stampCEB130. No item-slot
// clear, pointer/capacity reset, metric change, or pointee retry on failure.
// EH3 owns hierarchy array, primary array, name, base, in that unwind order.
void destroy_native_resource_container_00b88430(void*, NativeResourceContainerLifetimeContext&);

// 718810[96]: stampCFD8CC, free/zero checked triplets68/58/48 in that order,
// preserve proxy64/54/44, then tail-call base destruction. No item references
// are released through these borrowed classification arrays.
void destroy_native_game_resource_container_00718810(void*, NativeResourceContainerLifetimeContext&);

// Native ECX object, stacked flags, EAX captured object, RET4. Call the entire
// destructor before testing flags bit0; optionally free and return its address.
void* delete_native_resource_container_00b88760(void*, std::uint32_t flags, NativeResourceContainerLifetimeContext&);
void* delete_native_game_resource_container_00718c20(void*, std::uint32_t flags, NativeResourceContainerLifetimeContext&);

// Compose actual reference decrement -> BD30E0 -> current slot4 for D63228,
// CFD8CC and the existing fallback itemD631C0. Forward other profiles and stream
// operations. The context's item dispatch returns through this same adapter;
// put camera/GroupParams and other complete item bindings in the supplied chain.
class NativeResourceContainerReferences final : public NativeAdoptedSubstreamDispatch {
public:
    NativeResourceContainerReferences(NativeAdoptedSubstreamDispatch&, NativeResourceManagerContext&,
        NativeMaterialParameterPool& actual_hierarchy_pool);
    NativeResourceContainerReferences(const NativeResourceContainerReferences&) = delete;
    NativeResourceContainerReferences& operator=(const NativeResourceContainerReferences&) = delete;
    NativeResourceContainerLifetimeContext& context() noexcept { return context_; }
    std::uint8_t source_is_open(std::uintptr_t, void*) override;
    std::uint32_t source_seek(std::uintptr_t, void*, std::uint32_t, std::uint32_t, std::uint32_t) override;
    void source_read(std::uintptr_t, void*, void*, std::uint32_t, std::uint32_t*) override;
    void source_write(std::uintptr_t, void*, const void*, std::uint32_t, std::uint32_t*) override;
    void source_zero_reference(std::uintptr_t, void*, std::uintptr_t) override;
private:
    NativeAdoptedSubstreamDispatch& other_;
    NativeResourceContainerLifetimeContext context_;
};
// Actual names, counts, pointer storage, pool and publication identities are
// borrowed. Numeric native tables must be readable but are never called as
// code. These new source interfaces do not supply native FH3/SEH/fault handling,
// original private stack aliases, unknown item terminals or game validation.
} // namespace bsp
