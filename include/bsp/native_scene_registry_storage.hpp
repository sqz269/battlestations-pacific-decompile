#pragma once
#include "bsp/native_scene_registry_storage_leaves.hpp"
#include <cstdint>

namespace bsp {
using NativeSceneRegistryFree = void (__cdecl*)(void*);
using NativeSceneRegistryLengthFailure = void (__cdecl*)();
struct NativeSceneRegistryStorageBindings {
    // Required genuine allocator/free domain and current B82DD0 length-throw
    // entry. No fallback, host SceneNodeRegistry, or default/no-op is supplied.
    NativeSceneRegistryAllocator const volatile& actual_00bf681b;
    NativeSceneRegistryFree const volatile& actual_00bf65ac;
    // Must not return. Native message is "vector<T> too long" including NUL,
    // native length_error D69260 / ThrowInfo D83F98; source transport may differ.
    NativeSceneRegistryLengthFailure const volatile& actual_00b82dd0;
};
struct NativeSceneRegistryVectorArguments {
    volatile std::uint32_t count_00;
    const void* volatile pair_source_04;
};
struct NativeSceneRegistryConstructScratch {
    // Actual two-DWORD iterator payload and current nested argument cells.
    // Stable caller-owned initialized preimages. Source layout, not native
    // saved-register/return-address/EH-frame ABI. May be observed by bindings.
    std::uint32_t pair_00[2];
    NativeSceneRegistryVectorArguments vector_arguments_08;
};
static_assert(sizeof(NativeSceneRegistryVectorArguments) == 8);
static_assert(sizeof(NativeSceneRegistryConstructScratch) == 16);

// Complete B81B90 normal/overflow decision. Original ECX unsigned count, EAX
// allocation, RET. Zero still allocates zero bytes; >1FFFFFFF throws source
// std::bad_alloc (explicit transport boundary, not native FH3/payload ABI).
void* allocate_native_scene_registry_pairs_00b81b90(std::uint32_t count,
    NativeSceneRegistryStorageBindings&);
// Original ECX vector, stack count/pair, RET8. Clear only+4/+8/+C. Capture
// count before clears; after allocation zero LOW BYTE of current count cell;
// write capacity before reading current pair pointer and writing begin/end.
// Fill via actual B82570; count0 skips allocator and argument-cell overwrite.
// Source catch cleanup mirrors B832B6->B82C60->rethrow, not native SEH/FH3.
void construct_native_scene_registry_vector_00b83220(void* actual_vector,
    NativeSceneRegistryVectorArguments&, NativeSceneRegistryStorageBindings&);
// Original ECX registry28h, two stack allocator-byte pointers, RET8; second
// pointer unused. Writes only byte0 before sentinel allocation, list head8 and
// countC, then nine actual {registry+4,captured head} pairs at vector10h;
// mask20/active24=1 only after success. Opaque padding/list0/vector0 untouched.
// On vector failure, destroy CURRENT embedded list before propagating.
void* construct_native_scene_registry_00b83600(void* actual_registry,
    const void* allocator_byte, const void* unused_second_allocator,
    NativeSceneRegistryConstructScratch&, NativeSceneRegistryStorageBindings&);
// Current begin+4 free if nonnull, then zero4/8/C in order. B82C60 full42B.
void clear_native_scene_registry_vector_00b82c60(void* actual_vector,
    NativeSceneRegistryStorageBindings&);
// Embedded list0Ch: opaque0,head4,count8; node0Ch next0/previous4/borrowed key8.
// Reset next, reload head for previous, compare current head, then count0;
// capture next before each free and compare CURRENT head after it. Finally
// free current sentinel and clear head. No key destruction or owner release.
void destroy_native_scene_registry_list_00b829d0(void* actual_list,
    NativeSceneRegistryStorageBindings&);
void destroy_native_scene_registry_list_00b82b40(void* actual_list,
    NativeSceneRegistryStorageBindings&); // exact five-byte tail alias
// Free CURRENT vector begin14, clear14/18/1C, then tail to list at+4.
void destroy_native_scene_registry_00b82e90(void* actual_registry,
    NativeSceneRegistryStorageBindings&);
// New source interfaces. Native FH3/SEH, arbitrary private-stack aliases,
// whole scene-resource ownership and gameplay remain outside this packet.
} // namespace bsp
