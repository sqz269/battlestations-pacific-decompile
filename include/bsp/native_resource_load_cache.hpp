#pragma once
#include <cstdint>
#include <memory>

namespace bsp {
struct NativeStringRawPoolContext;
struct NativeVfsNameResolutionContext;
class NativeVfsNameResolutionAcquired;
class NativeVfsRuntimeBindings;
struct NativeResourceRootDispatchContext;

// Original numeric targets are identities, never callable process addresses.
class NativeResourceLoadCacheCalls {
public:
    virtual ~NativeResourceLoadCacheCalls() = default;
    virtual std::uint32_t allocation_metric(std::uintptr_t target, void* owner) = 0;
    virtual void* create_resource(std::uintptr_t target, void* factory, const void* name) = 0;
    virtual void* open_stream(std::uintptr_t target, void* manager, const void* name,
        std::uint32_t flags) = 0;
};

struct NativeResourceLoadCacheContext {
    void* volatile& actual_vfs_0109ceec;
    void* volatile& actual_allocation_stats_0109cefc;
    NativeVfsNameResolutionContext& names;
    NativeResourceRootDispatchContext& dispatch;
    NativeResourceLoadCacheCalls& calls;
};

enum class NativeResourceLoadCachePhase { fresh, running, complete, failed };
// One immovable invocation, including the nested VFS resolution frame. The
// source follows the five recovered cleanup states for ordinary C++ exceptions.
// A failed nested resolution must remain retained, as required by that existing
// provider boundary; destroying it terminates. Other failures may be inspected
// and destroyed after cleanup. No replay, resource rollback or cache rollback.
class NativeResourceLoadCacheAcquired final {
public:
    NativeResourceLoadCacheAcquired();
    ~NativeResourceLoadCacheAcquired();
    NativeResourceLoadCacheAcquired(const NativeResourceLoadCacheAcquired&) = delete;
    NativeResourceLoadCacheAcquired& operator=(const NativeResourceLoadCacheAcquired&) = delete;
    NativeResourceLoadCachePhase phase() const noexcept;
    std::uint32_t failure_site() const noexcept;
    std::int32_t native_state_at_failure() const noexcept;
    const void* actual_reader() const noexcept;
    const void* resolved_name() const noexcept;
    const NativeVfsNameResolutionAcquired& resolution_invocation() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend void* load_and_cache_native_resource_00b80720(void*, const void*, void*,
        NativeResourceLoadCacheContext&, NativeResourceLoadCacheAcquired&);
};

// Complete811B: ECX raw28h manager; stack actual name/factory; EAX resource;
// RET8. Publish factory+20 BEFORE cache lookup. Hit publishes resource+24,
// atomically retains it and returns CURRENT+24 without clearing the factory.
// Miss creates resource, resolves a copied name (ignoring AL), opens flags2,
// reads/dispatches it, inserts ORIGINAL name, clears factory and stores wrapping
// starting-minus-ending metric in CURRENT resource+40. Return captured resource
// after root/reader/name cleanup. No null, cache-insert-result or success gate.
void* load_and_cache_native_resource_00b80720(void* manager, const void* name,
    void* factory, NativeResourceLoadCacheContext&, NativeResourceLoadCacheAcquired&);

// Complete29B each: ECX actual0Ch cache pair, RET. Return current name storage
// through the current raw pool. Keep stale header and borrowed resource+8.
void destroy_native_resource_cache_pair_00b7e8f0(void*, NativeStringRawPoolContext&);
void destroy_native_resource_cache_insert_pair_00b7e910(void*, NativeStringRawPoolContext&);
// Complete30B: owner ignored; RET; EAX40000000. The binary zeroes a34h local
// block and subtracts its zero DWORD+1C. No OS memory query is present.
std::uint32_t read_native_resource_allocation_budget_00be2700() noexcept;

// Known BE2700 metric and B88340/71B870 factories compose actual source bodies.
// Open delegates the captured target to the existing VFS runtime binding.
// Other metrics/factories remain explicit calls; no substitute result exists.
class NativeDefaultResourceLoadCacheCalls final : public NativeResourceLoadCacheCalls {
public:
    NativeDefaultResourceLoadCacheCalls(NativeResourceLoadCacheCalls&,
        NativeStringRawPoolContext&, NativeVfsRuntimeBindings&);
    std::uint32_t allocation_metric(std::uintptr_t, void*) override;
    void* create_resource(std::uintptr_t, void*, const void*) override;
    void* open_stream(std::uintptr_t, void*, const void*, std::uint32_t) override;
private:
    NativeResourceLoadCacheCalls& other_;
    NativeStringRawPoolContext& strings_;
    NativeVfsRuntimeBindings& vfs_;
};
// New C++ service ABI. Native FH3/SEH identity, private stack aliases, hardware
// faults, manager/parser bootstrap, concrete parsers and gameplay are unproved.
} // namespace bsp
