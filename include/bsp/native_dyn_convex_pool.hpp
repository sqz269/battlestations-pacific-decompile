#pragma once
#include "bsp/allocator_list.hpp"
#include "bsp/dyn_body_creation.hpp"
#include <cstdlib>

namespace bsp {
// Actual 0109ECF0 storage and common E188B4 registry, not a second allocator.
// The native D7A1EC table identifies one trim slot, 00407E30. Its source
// dispatch binding is registered before native list publication/allocation.
class NativeDynConvexPool final {
public:
    NativeDynConvexPool(AllocatorListDomain&,DynConvexShapePoolStorage&,
        const AvoidZoneDynHullMemory&);
    NativeDynConvexPool(const NativeDynConvexPool&)=delete;
    NativeDynConvexPool& operator=(const NativeDynConvexPool&)=delete;
    ~NativeDynConvexPool()=default; // Explicit native callback owns destruction.
    // Existing 407C70 successful constructor semantics on the canonical list.
    DynConvexShapePoolStorage& initialize_00407c70();
    // Complete 160B, ECX pool/RET. Free wholly empty pages, swap the last page,
    // update all 128 moved slots' page-index words, recompute first-free page.
    // No lock entry; callers must exclude concurrent pool mutation.
    void trim_empty_pages_00407e30();
    // Complete 178B global destructor/RET. Frees pages and table, unwinds a
    // positive signed lock depth, deletes the CS, then unlinks the base element.
    // Native table/count/capacity/free-page/link fields deliberately stay stale.
    void destroy_00407d70();
private:
    AllocatorListDomain& list_;
    DynConvexShapePoolStorage& storage_;
    AvoidZoneDynHullMemory memory_;
    AllocatorListElement& element() noexcept;
    static void invoke_trim(void*);
};
// Complete 17B CC89C0 initializer and 5B CD9240 tail callback. Bind one actual
// process owner before startup. Preserve real CRT registration result without
// rollback. Normal-return interfaces; native FH3/SEH/OOM behavior is separate.
void bind_static_native_dyn_convex_pool_0109ecf0(NativeDynConvexPool&);
int initialize_static_native_dyn_convex_pool_00cc89c0(int (*registration)(void (*)())=&std::atexit);
void destroy_static_native_dyn_convex_pool_00cd9240();
} // namespace bsp
