#pragma once

#include <cstdint>

namespace bsp {
struct AvoidZoneDynHullMemory;
struct DynBodyCreationContext;
struct NativeGamePhysicsLifetimeCalls;
struct NativeGamePhysicsLifetimeProgress;
}

namespace bsp::cc16_dyn_pointer_retire {

// Mandatory actual allocator custody. The two frees are separate native entry
// contracts; this implementation does not collapse them or install defaults.
struct DynPointerRetireAllocators {
    void* (__cdecl* operator_new_00bf55be)(std::uint32_t bytes);
    void (__cdecl* storage_free_00bf6989)(void* storage);
    void (__cdecl* object_free_00bf65ac)(void* object);
};

// Borrow actual destruction services/memory and the original body-construction
// binding. No native object class is asserted. An explicitly supplied null body
// binding is valid only when the existing lifetime provider's real attachment
// policy permits it; it is not a default used to manufacture closure.
struct DynPointerRetireContext {
    const DynPointerRetireAllocators& allocators;
    const AvoidZoneDynHullMemory& destruction_memory;
    NativeGamePhysicsLifetimeCalls& lifetime_calls;
    NativeGamePhysicsLifetimeProgress& lifetime_progress;
    const DynBodyCreationContext* body_creation;

    DynPointerRetireContext(
        const DynPointerRetireAllocators& actual_allocators,
        const AvoidZoneDynHullMemory& actual_destruction_memory,
        NativeGamePhysicsLifetimeCalls& actual_lifetime_calls,
        NativeGamePhysicsLifetimeProgress& actual_lifetime_progress,
        const DynBodyCreationContext* actual_body_creation) noexcept
        : allocators(actual_allocators),
          destruction_memory(actual_destruction_memory),
          lifetime_calls(actual_lifetime_calls),
          lifetime_progress(actual_lifetime_progress),
          body_creation(actual_body_creation) {}
};

// Complete native 00C55DF0..00C55E9F: ESI raw {data,count,capacity}, EDX search
// pointer, no stack arguments, RET. This is a new ordinary cdecl C++ interface,
// not an original ABI adapter. Normal removal requires a nonempty list and a
// present pointer; native missing/empty behavior is intentionally unguarded.
// The captured removed value is passed unchanged to the REAL existing C4DC60
// implementation, then to the distinct BF65AC provider if destruction returns.
// Genuine removed-object layout/construction/context binding is still required.
// No class cast, allocation/failure guard, rollback or exception transport.
void __cdecl remove_and_destroy_00c55df0(
    void* list, void* search_pointer, const DynPointerRetireContext& context);

}  // namespace bsp::cc16_dyn_pointer_retire
