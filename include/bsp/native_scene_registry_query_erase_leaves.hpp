#pragma once
#include "bsp/native_scene_registry_storage.hpp"
#include <cstdint>

namespace bsp {
struct NativeSceneRegistryEqualRangeArguments {
    volatile std::uint32_t output; // actual16B pair of iterator pairs
    volatile std::uint32_t key;    // borrowed actual pointer-key DWORD address
};
struct NativeSceneRegistryEqualRangeScratch {
    std::uint32_t opaque_00;
    std::uint32_t opaque_04;
    volatile std::uint32_t first_08;
};
struct NativeSceneRegistryEraseArguments {
    volatile std::uint32_t output; // actual8B iterator; read AFTER actual free
    volatile std::uint32_t owner;  // actual embedded list address, registry+4
    volatile std::uint32_t node;   // current cell is reused by the native body
};
struct NativeSceneRegistryCountArguments {
    volatile std::uint32_t first_owner;
    volatile std::uint32_t first_node;
    volatile std::uint32_t last_owner;
    volatile std::uint32_t last_node;
    volatile std::uint32_t count; // address of caller's existing DWORD counter
    std::uint32_t unused_tag;
};
static_assert(sizeof(NativeSceneRegistryEqualRangeArguments) == 8);
static_assert(sizeof(NativeSceneRegistryEqualRangeScratch) == 12);
static_assert(sizeof(NativeSceneRegistryEraseArguments) == 12);
static_assert(sizeof(NativeSceneRegistryCountArguments) == 24);

// Actual28h registry: embedded list+4 {opaque0,head4,count8}, boundary vector
// +10 {opaque0,begin4,end8,capacityC}, mask20/active24. List entries are actual
// 0Ch {next0,previous4,borrowed pointer-key8}; iterators are actual8B {owner,node}.
// No SceneNodeRegistry, SceneResource, companion keys or ownership operations.
//
// Valid domain: accessible Win32 words, valid same-owner iterators/non-sentinel
// dereferences, in-bounds boundary vector and terminating ranges. There is no
// asynchronous mutation. Reached actual free may mutate live registry/argument
// cells; they must remain valid at every subsequent native read. Native CRT
// invalid-parameter paths are excluded; source rejects a detected violation
// with invalid_argument, not a native CRT continuation or compatible exception.
// Arguments/scratch are caller-owned initialized preimages. They are borrowed,
// not snapshots. Only first_08 is written in equal-range scratch, only on the
// second-scan path; unused words are preserved. No native private stack/FH3 ABI
// or arbitrary saved-register aliases are implied by these source structures.

// Complete361B normal body. Original ECX registry; stack output/key; RET8.
// Signed quotient/remainder hash, unsigned key comparisons, CURRENT key pointer
// on each comparison, current output after traversal. Success stores output
// offsets C,0,8,4; absence stores4,0,8,C. Returns captured actual output identity.
void* equal_range_native_scene_registry_00b82650(void* actual_registry,
    NativeSceneRegistryEqualRangeArguments&, NativeSceneRegistryEqualRangeScratch&);

// Complete308B normal body. Original ECX registry; stack output/owner/node;
// RET12. Repair current backward equal boundaries before unlink/free. Reload
// node argument after each boundary owner write. Capture iterator owner before
// rewriting node argument; capture next before unlink; after actual free update
// CURRENT count, then read CURRENT output and store next before captured owner.
// Free is the existing genuine BF65AC domain; no fallback or key destruction.
// A throwing source binding leaves preceding writes intact; no rollback/EH
// cleanup is introduced. Caller owns recovery after such an external failure.
void* erase_native_scene_registry_entry_00b827c0(void* actual_registry,
    NativeSceneRegistryEraseArguments&, NativeSceneRegistryStorageBindings&);

// Complete70B normal body. Original cdecl stacked first/last iterator pairs,
// counter pointer and unused tag; RET. Captures first node,last node,first owner,
// counter in native order. Increment CURRENT counter before reading next; the
// existing counter is not initialized. last_owner is reread on every turn.
void count_native_scene_registry_range_00b820b0(NativeSceneRegistryCountArguments&);
} // namespace bsp
