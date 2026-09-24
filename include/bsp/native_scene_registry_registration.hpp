#pragma once
#include "bsp/native_scene_registry_mutation_adapters.hpp"
#include "bsp/native_scene_registry_query_erase_leaves.hpp"
#include <cstdint>

namespace bsp {
struct NativeSceneRegistryRegistrationContext {
    NativeSceneRegistryVectorMutationBindings& mutation;
    const char* actual_message_00ce38f8;
};
struct NativeSceneRegistryRegistrationInsertFrame {
    // Native 30h local allocation: [ESP+10..3C] after four register pushes.
    // Words3/6/8/10 are opaque preimages. No saved registers/return slots/EH ABI.
    volatile std::uint32_t local_00[12];
    volatile std::uint32_t output_argument;
    volatile std::uint32_t key_argument;
    NativeSceneRegistryResizeFrame resize;
    NativeSceneRegistryListGrowthScratch list_growth;
};
struct NativeSceneRegistryRegistrationAcquired {
    bool started{}, complete{};
    std::uint32_t active_call_site{};
    void* allocated_node{};
    bool allocation_completed{}, size_growth_completed{}, node_linked{};
    // Fresh per invocation; retain until failure disposition or successful
    // handoff is complete. Disjoint diagnostics, no ownership/count stores.
    // If size growth throws,
    // native leaves this completed allocation and earlier bucket mutations.
    // Caller owns explicit failure disposition; no destructor/rollback added.
};
struct NativeSceneRegistryClearFrame {
    volatile std::uint32_t pair[2];
    NativeSceneRegistryAssignFrame assign;
};
struct NativeSceneRegistryEraseRangeFrame {
    volatile std::uint32_t saved_registry;
    volatile std::uint32_t output_argument;
    volatile std::uint32_t first_owner_argument;
    volatile std::uint32_t first_node_argument;
    volatile std::uint32_t last_owner_argument;
    volatile std::uint32_t last_node_argument;
    NativeSceneRegistryEraseArguments entry;
    NativeSceneRegistryClearFrame clear;
};
struct NativeSceneRegistryEraseKeyFrame {
    volatile std::uint32_t range[4];
    volatile std::uint32_t key_or_count_argument;
    NativeSceneRegistryEqualRangeArguments query;
    NativeSceneRegistryEqualRangeScratch query_scratch;
    NativeSceneRegistryCountArguments count;
    NativeSceneRegistryEraseRangeFrame erase;
};

// All four ECX domains are the ACTUAL 28h registry. For an actual 3Ch scene
// resource this is resource+14h, never the resource base or a host logical
// SceneNodeRegistry. Nodes are actual0Ch {next,previous,borrowed key}; iterator
// pairs are actual8B {registry+4,node}. No owner credit or object release occurs.
// Frames are initialized caller-owned stable storage, disjoint from metadata;
// they expose only stated local/argument words and nested source frames, not
// native private-stack gaps or arbitrary nested helper-stack aliases.
// Own returning BF6713 guards use the required genuine current service. Nested
// B82650/B827C0/B820B0 retain their VALID-ITERATOR DOMAIN: invalid native CRT
// continuation there is excluded and source invalid_argument is not equivalent.
// All reached words/ranges must remain valid after reentrant callbacks. No
// asynchronous mutation or universal corrupt-state/exception compatibility.

// Complete B83700[1247] normal body, native ECX registry, output/key pointer
// stack words, EAX actual output, RET8. Split BEFORE duplicate check; current
// key pointer/value reread; allocator BEFORE checked size growth; no local EH.
// Output stores owner,node,ONE BYTE inserted; output+9 padding untouched.
void* insert_native_scene_registry_00b83700(void* actual_registry,
    NativeSceneRegistryRegistrationInsertFrame&,
    NativeSceneRegistryRegistrationContext&, NativeSceneRegistryRegistrationAcquired&);
// B83BE0[98], ECX registry, RET. Reset sentinel links/count before node frees;
// compare current head after each free; assign nine boundaries, then mask=1,
// active=1. Preserve sentinel, vector capacity and borrowed key ownership.
void clear_native_scene_registry_00b83be0(void* actual_registry,
    NativeSceneRegistryClearFrame&, NativeSceneRegistryRegistrationContext&);
// B83D90[178], ECX registry, output/first pair/last pair, EAX output, RET14h.
// Whole-list range routes through clear; loop captures next before entry erase
// and lets entry output overwrite the SAME first-owner/node argument cells.
void* erase_native_scene_registry_range_00b83d90(void* actual_registry,
    NativeSceneRegistryEraseRangeFrame&, NativeSceneRegistryRegistrationContext&);
// B83E50[106], ECX registry, key-source pointer, EAX current erased counter,
// RET4. Reuses actual range output, zeroes original key argument as counter
// only after query, captures three iterator words and rereads first owner
// after count, then erases; final counter is read AFTER free/clear callbacks.
std::uint32_t erase_native_scene_registry_key_00b83e50(void* actual_registry,
    NativeSceneRegistryEraseKeyFrame&, NativeSceneRegistryRegistrationContext&);
} // namespace bsp
