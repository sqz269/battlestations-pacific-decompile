#pragma once
#include "bsp/native_resource_registry_node_leaves.hpp"
#include "bsp/native_resource_registry_tree_leaves.hpp"
#include "bsp/native_legacy_sbo_string.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
struct NativeResourceRegistryInsertionContext {
    NativeStringRawPoolContext& strings;
    const SingletonLifetimeCallbacks& validation; // genuine returning BF6713
    const char* const text_00ce47bc; // current actual19-byte map/set length text
};

// Begin these trivial payload lifetimes before use and supply all preimages.
// The first8B has its iterator lifetime throughout both end/result roles.
struct NativeResourceRegistryInsertResult {
    NativeResourceRegistryTreeIterator iterator;
    std::uint8_t inserted;
    std::uint8_t padding[3];
};
static_assert(sizeof(NativeResourceRegistryInsertResult) == 12);
static_assert(offsetof(NativeResourceRegistryInsertResult, inserted) == 8);

// Immutable views over actual live caller argument/local cells. Do not cast
// aggregates over reused WORD backing. Pointer-cell iterators must already be
// live objects. Metadata/acquired are disjoint from payload and native backing.
struct NativeResourceRegistryLinkFrame {
    volatile std::uint32_t& output_argument;
    volatile std::uint32_t& left_argument;
    volatile std::uint32_t& parent_argument;
    volatile std::uint32_t& pair_argument;
    NativeLegacySboStringStorage& length_message; // native S-50, actual1Ch
    const NativeResourceRegistryNodeAllocationFrame& allocation;
};
struct NativeResourceRegistryPairInsertFrame {
    volatile std::uint32_t& output_argument;
    volatile std::uint32_t& pair_argument;
    volatile std::uint32_t& direction; // S-C; only LOW byte written, full word pushed
    NativeResourceRegistryTreeIterator& cursor; // S-8/S-4
    const NativeResourceRegistryLinkFrame& link;
};
struct NativeResourceRegistryHintInsertFrame {
    volatile std::uint32_t& output_argument;
    NativeResourceRegistryTreeIterator& incoming_hint; // original S+8/S+C, mutated
    volatile std::uint32_t& pair_argument;
    NativeResourceRegistryInsertResult& local; // S-C; also end iterator, same object
    const NativeResourceRegistryLinkFrame& link;
    const NativeResourceRegistryPairInsertFrame& fallback;
};
struct NativeResourceRegistryValueFrame {
    volatile std::uint32_t& name_argument;
    NativeResourceRegistryTreeIterator& result; // S-20/S-1C
    volatile std::uint32_t (&temporary_pair)[3]; // S-18/-14/-10, length/data/value
    const NativeResourceRegistryHintInsertFrame& hint;
};
struct NativeResourceRegistryRegistrationFrame {
    volatile std::uint32_t& name_argument;
    volatile std::uint32_t& factory_argument;
    NativeResourceRegistryTreeIterator& result; // S-8/S-4
    const NativeResourceRegistryValueFrame& value;
};

// Fresh, address-stable, disjoint diagnostics per invocation. Nested records
// retain the actual allocation/free prefix. Reusing a started entry is rejected
// before that entry's native work; caller guarantees the entire reached tree is
// fresh. Metadata does not add a retain or substitute cleanup after failure.
struct NativeResourceRegistryLinkAcquired {
    bool started{};
    int length_eh_state{-1};
    bool linked{};
    bool result_published{};
    NativeResourceRegistryNodeAcquired node;
};
struct NativeResourceRegistryPairInsertAcquired {
    bool started{};
    NativeResourceRegistryLinkAcquired link;
};
struct NativeResourceRegistryHintInsertAcquired {
    bool started{};
    NativeResourceRegistryLinkAcquired direct;
    NativeResourceRegistryPairInsertAcquired fallback;
};
struct NativeResourceRegistryValueAcquired {
    bool started{};
    int temporary_eh_state{-1};
    std::uint32_t captured_temporary_data{};
    bool normal_return_started{};
    bool normal_return_completed{};
    bool unwind_cleanup_started{};
    bool unwind_cleanup_completed{};
    NativeResourceRegistryHintInsertAcquired hint;
};
struct NativeResourceRegistryRegistrationAcquired {
    bool started{};
    NativeResourceRegistryValueAcquired value;
};

// B1ADA0[492]: ECX actual tree, stack output/left/parent/pair, RET10/EAX output.
// Preserve current count/head, captured parent/pair, CURRENT left LOW byte after
// allocation, and final root-black store BEFORE current output pointer fetch.
// Real1Ch node allocation then count/link/rebalance; publish node before owner.
void* link_native_resource_registry_node_00b1ada0(void* actual_tree,
    const NativeResourceRegistryLinkFrame&, NativeResourceRegistryInsertionContext&,
    NativeResourceRegistryLinkAcquired&);

// B1AF90[276]: ECX tree, stack output12B/pair, RET8/EAX output. Captured pair,
// current key contents, direction BYTE/full-DWORD distinction, real predecessor,
// current output pointer; publish node, inserted BYTE, owner, preserving padding.
void* insert_native_resource_registry_pair_00b1af90(void* actual_tree,
    const NativeResourceRegistryPairInsertFrame&, NativeResourceRegistryInsertionContext&,
    NativeResourceRegistryPairInsertAcquired&);

// B1B0B0[508]: ECX tree, stack output/hint-owner/hint-node/pair, RET10/EAX output.
// Decrement/increment overwrite the SAME incoming_hint cells. The local12B
// result reuses its own first8B as the end iterator. Direct link returns captured
// outer output; fallback stores owner BEFORE loading returned node for its store.
void* insert_native_resource_registry_hint_00b1b0b0(void* actual_tree,
    const NativeResourceRegistryHintInsertFrame&, NativeResourceRegistryInsertionContext&,
    NativeResourceRegistryHintInsertAcquired&);

// B1B2B0[238]: ECX tree, stack captured name, RET4/EAX actual node+14.
// Temporary cleanup differs: normal uses data captured just after resize plus
// CURRENT length+1, with state disarmed before getter; EH reads CURRENT header.
// Preserve provider failure prefixes: no node rollback after successful insertion,
// no extra name cleanup before temp state0 or after its normal disarm. Cleanup
// throwing during source unwinding terminates; no native FH3/SEH claim.
void* find_or_insert_native_resource_registry_value_00b1b2b0(void* actual_tree,
    const NativeResourceRegistryValueFrame&, NativeResourceRegistryInsertionContext&,
    NativeResourceRegistryValueAcquired&);

// B1B3A0[84]: ECX actual10h registry, stack name/factory, RET8. Existing key
// keeps its value. Miss inserts with captured name then reads CURRENT factory
// argument, publishing borrowed bits only; no retain or canonical admission.
void register_native_resource_factory_00b1b3a0(void* actual_registry,
    const NativeResourceRegistryRegistrationFrame&, NativeResourceRegistryInsertionContext&,
    NativeResourceRegistryRegistrationAcquired&);

// B19DD0[29], exact instruction-normalized alias of existing raw41DD20. ECX
// current8B header, RET. Capture data then current length+1 before actual getter;
// return storage without clearing header. Raw-context getter may throw.
void destroy_native_resource_registry_temporary_00b19dd0(void* actual_header,
    NativeStringRawPoolContext&);

// These APIs borrow a valid actual1Ch-node tree, same raw string pool and real
// CRT/returning-validation domain. Frames and all typed payload lifetimes persist
// through failure disposition. Separate provider-frame private stack coincidence,
// native saved registers/FH3/throw ABI, arbitrary faults and concurrent mutation
// are not reproduced. No generic map, logical owner, global startup or game claim.
} // namespace bsp
