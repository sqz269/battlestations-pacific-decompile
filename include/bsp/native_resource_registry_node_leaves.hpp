#pragma once
#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;
struct SingletonLifetimeCallbacks;

// Immutable views of caller-owned, initialized, genuinely live DWORD cells.
// The five bindings represent native stacked left/parent/right/pair/color.
// They may refer into reused DWORD backing; no aggregate is cast over it.
// Metadata itself is disjoint from every native payload/argument/local cell.
struct NativeResourceRegistryNodeArguments {
    volatile std::uint32_t& left;
    volatile std::uint32_t& parent;
    volatile std::uint32_t& right;
    volatile std::uint32_t& pair;
    volatile std::uint32_t& color;
};
struct NativeResourceRegistryNodeAllocationFrame {
    const NativeResourceRegistryNodeArguments incoming;
    const NativeResourceRegistryNodeArguments constructor;
    volatile std::uint32_t& allocation_cleanup; // native EBP-14
    volatile std::uint32_t& placement_cleanup;  // native EBP-18
};
struct NativeResourceRegistryNodeAcquired {
    bool started{};
    bool allocation_completed{};
    bool constructor_completed{};
    bool placement_cleanup_completed{};
    bool catch_free_completed{};
    bool source_failed{};
    int native_eh_state{-1};
    void* captured_allocation{}; // may be dead after the catch; never dereference
    void* catch_freed_address{}; // CURRENT cleanup cell; can differ from captured
};

// B19530[41]: ECX left8B iterator, stack right8B iterator, RET4. Both raw views
// are {owner,node}; existing typed pointer cells OR live DWORD backing are legal.
// Initial owner validation calls the supplied genuine returning BF6713 service.
// Then read CURRENT left.node and right.node. Return left node's high24 bits
// with ONLY AL replaced by equality; predicate callers must inspect LOW byte.
// A null callback is outside the valid binding domain, not a source exception.
std::uint32_t compare_native_resource_registry_iterators_00b19530(
    const void* actual_left, const void* actual_right,
    const SingletonLifetimeCallbacks&);

// B1AB40[137]: ECX mutable actual8B iterator, RET / returning BF6713 tail.
// Preserve current-node reload after initial validation, each ascent store,
// current iterator rereads, and immediate return after either invalid tail.
void decrement_native_resource_registry_iterator_00b1ab40(
    void* actual_iterator, const SingletonLifetimeCallbacks&);

// B1ACA0[112]: ECX actual1Ch node; five stack words; RET14/EAX original node.
// Node: links0/4/8, raw name length/data C/10, borrowed factory14, color/nil
// bytes18/19, untouched padding1A/1B. Pair is a current actual12B name/value.
// Capture left/right/pair/parent before stores, compare self-header before
// zeros, call genuine raw41DD40 preserve1, then reread current name fields.
// Native BF7680 admits overlap; source uses memmove with the established
// zero-count host boundary. Read current pair.value then late color LOW byte.
// No factory credit, automatic NativeString lifetime, local cleanup or rollback.
void* construct_native_resource_registry_node_00b1aca0(void* actual_node,
    const NativeResourceRegistryNodeArguments&, NativeStringRawPoolContext&);

// B1AD10[114] plus explicit source C++ projection of its compiler boundaries.
// Allocate real1Ch first, write cleanup cells/state, then copy CURRENT incoming
// color/pair/right/parent/left into the separate constructor argument cells.
// Return captured allocation even if callback-visible scratch has changed.
// Source constructor failure consumes state1's current two reads/RET-only
// 401130 placement cleanup, then catchB1AD82 frees CURRENT allocation_cleanup
// and rethrows. It does NOT destroy or release a partial name. Diagnostics
// retain addresses/frontiers, not a claim that every acquired block was freed.
//
// Require fresh Acquired and address-stable caller-owned frames/backing through
// completion/failure disposition. Incoming, constructor and cleanup cells have
// distinct native frame locations; their metadata/acquired must be disjoint.
// Payload/current argument aliases are admitted, but cross-provider private
// stack-address coincidence, saved registers, native FH3/SEH and binary ABI
// are not reproduced. No tree insertion/count change or global admission.
void* allocate_native_resource_registry_node_00b1ad10(
    const NativeResourceRegistryNodeAllocationFrame&, NativeStringRawPoolContext&,
    NativeResourceRegistryNodeAcquired&);
} // namespace bsp
