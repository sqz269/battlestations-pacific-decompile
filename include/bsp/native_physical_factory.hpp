#pragma once

#include <cstdint>

namespace bsp {

// Borrow the application's actual publications in the existing raw singleton
// manager domain. Both bindings remain stable; their volatile values may change.
// Factory storage is the native8h primary D68CFC / secondary+4 D68CF8 owner.
struct NativePhysicalFactoryContext {
    void* volatile& actual_manager_publication_01090aa0;
    void* volatile& actual_factory_publication_0109dbe8;
};

// Complete BED990[206]. Native no-input getter, EAX factory, RET. Fast return
// preserves the first publication read. Slow path captures the first manager's
// real section+10, rechecks/publishes under that lock, captures factory+4 BEFORE
// the second manager getter, and registers that captured secondary pointer.
// Registration failure retains publication/allocation and releases the guard.
void* get_native_physical_factory_00bed990(NativePhysicalFactoryContext&);

// Complete BED950[58]. Native ECX primary factory, stacked flags, EAX original
// possibly freed address, RET4. Clears publication, writes secondary CE3818 and
// primary CFE9F4; flags bit0 frees the entire8h allocation. No unregister call.
void* delete_native_physical_factory_00bed950(void* actual_primary,
    std::uint32_t flags, NativePhysicalFactoryContext&) noexcept;
// Complete raw BED910[8]: SUB ECX,4; JMP BED950, inherited RET4. The registered
// pointer is factory+4, so canonical shutdown must invoke this adjustment.
void* delete_native_physical_factory_secondary_00bed910(void* actual_secondary,
    std::uint32_t flags, NativePhysicalFactoryContext&) noexcept;

// New C++ service interfaces, not drop-in binary ABI/FH3 replacements. Source
// CRT and C++ exception identity are explicit boundaries. No manager, shutdown
// dispatcher, private publication, provider pool or factory startup is created.
// Descriptive names are hypotheses. Caller composes the same lifetime domain.
} // namespace bsp
