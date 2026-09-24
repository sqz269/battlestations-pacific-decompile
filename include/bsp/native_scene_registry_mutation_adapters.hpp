#pragma once
#include "bsp/native_scene_registry_vector_mutation.hpp"
#include "bsp/native_legacy_sbo_string.hpp"
#include <cstdint>

namespace bsp {
struct NativeSceneRegistryResizeFrame {
    volatile std::uint32_t requested_count;
    volatile std::uint32_t pair_argument[2];
    NativeSceneRegistryInsertFrame insert;
    NativeSceneRegistryVectorEraseArguments erase;
};
struct NativeSceneRegistryAssignFrame {
    volatile std::uint32_t count_argument;
    volatile std::uint32_t pair_source_argument;
    volatile std::uint32_t captured_pair[2];
    volatile std::uint32_t erase_result[2];
    NativeSceneRegistryInsertFrame insert;
    NativeSceneRegistryVectorEraseArguments erase;
};
struct NativeSceneRegistryListGrowthScratch {
    NativeLegacySboStringStorage message;
};
static_assert(sizeof(NativeSceneRegistryResizeFrame) == 60);
static_assert(sizeof(NativeSceneRegistryAssignFrame) == 72);
static_assert(sizeof(NativeSceneRegistryListGrowthScratch) == 28);

// Frames are caller-owned initialized preimages, address-stable and disjoint
// from actual vector/backing storage. The incoming assign pair may alias vector
// data or caller cells. Explicit nested argument cells preserve callback reads;
// nested scratch is not initialized by these wrappers. Native private stack,
// saved registers and FH3 frames are not their source layouts. Reached genuine
// returning BF6713 service and allocator/free/length-error bindings must be
// supplied; no fallback or successful no-op handler is installed. Continuing
// callbacks must leave every later reached address/range valid.

// Complete B83490[170], native ECX vector, count and inline pair, RET0Ch.
// Captured count/begin/size/end survive callbacks. Growth passes CURRENT inline
// pair to B82FD0. Shrink overwrites pair_argument[1] with captured begin BEFORE
// validation, then B82BF0 uses the SAME pair_argument as its result buffer.
void resize_native_scene_registry_boundaries_00b83490(void* actual_vector,
    NativeSceneRegistryResizeFrame&, NativeSceneRegistryVectorMutationBindings&);

// Complete B83560[108], native ECX vector, count/pair-pointer, RET8. Capture
// incoming pair word1 BEFORE word0; write local pair after the initial range
// comparison but before any handler. Erase whole vector into separate scratch;
// after erase/validation reread CURRENT count argument before B82FD0 insertion.
void assign_native_scene_registry_boundaries_00b83560(void* actual_vector,
    NativeSceneRegistryAssignFrame&, NativeSceneRegistryVectorMutationBindings&);

// Complete B82D30[147] normal/overflow choice, native ECX embedded list,
// captured increment, RET4. Unsigned 3FFFFFFF-count comparison; successful
// store uses CAPTURED count+increment (DWORD wrap), never a post-call reread.
// On overflow use borrowed actual CE38F8 sixteen-byte "list<T> too long" text,
// existing raw408720 and411700/D69260 owning source exception transport. Arm
// message cleanup only AFTER assignment. Normal path leaves scratch untouched.
// Source throws NativeHardwareLayoutTreeLengthError; its host RTTI/copy/throw
// transport explicitly differs from native BF6885/D83F98/FH3/private28h storage.
// The caller retains scratch/preimages; no guessed zeros or CRT port is added.
void grow_native_scene_registry_list_size_00b82d30(void* actual_list,
    std::uint32_t captured_increment, NativeSceneRegistryListGrowthScratch&,
    const char* actual_message_00ce38f8);
} // namespace bsp
