#pragma once
#include "bsp/native_scene_registry_storage.hpp"
#include <cstdint>

namespace bsp {
using NativeSceneRegistryInvalidParameter = void (__cdecl*)();
struct NativeSceneRegistryVectorMutationBindings {
    NativeSceneRegistryStorageBindings& storage;
    // Required genuine returning BF6713 service (source CRT transport is
    // explicit). A returning handler may repair current argument/storage cells.
    NativeSceneRegistryInvalidParameter const volatile& actual_00bf6713;
};
struct NativeSceneRegistryVectorEraseArguments {
    volatile std::uint32_t result_00;
    volatile std::uint32_t first_owner_04;
    volatile std::uint32_t first_position_08;
    volatile std::uint32_t last_owner_0c;
    volatile std::uint32_t last_position_10;
};
struct NativeSceneRegistryInsertFrame {
    // Actual initialized, address-stable source cells. Map to native EBP-1C,
    // -18, -14, +8, +C, +10, +14 respectively. The gaps are intentionally
    // absent: this is NOT a native private-stack/saved-register/EH-frame ABI.
    volatile std::uint32_t pair_00[2];
    volatile std::uint32_t copy_unused_08;
    volatile std::uint32_t iterator_owner_0c;
    volatile std::uint32_t position_10;
    volatile std::uint32_t count_14;
    volatile std::uint32_t pair_or_work_18;
};
static_assert(sizeof(NativeSceneRegistryVectorEraseArguments) == 20);
static_assert(sizeof(NativeSceneRegistryInsertFrame) == 28);

// Complete B82BF0[106]: native ECX vector, five stack words, EAX actual
// result pair, RET14h. Native validation is only nonnull/equal iterator owners;
// reached pointers must describe accessible finite 8-byte ranges. Preserves
// forward componentwise overlap and owner reload only after a nonempty move.
void* erase_native_scene_registry_pairs_00b82bf0(void* actual_vector,
    NativeSceneRegistryVectorEraseArguments&, NativeSceneRegistryVectorMutationBindings&);
// Complete B82FD0[577] normal paths and explicit source C++ catch projection.
// Native ECX vector, owner/position/count/pair stack words, RET10h, no result.
// Pair is read even when count==0. Growth reads current count/position after
// allocation, and current buffer argument after free. In-capacity branches
// overwrite pair/count argument cells exactly. Padding/allocator bytes stay
// untouched. Required genuine allocator/free/length-error domains; no rollback
// beyond native catch B83137 freeing CURRENT pair-or-work before rethrow.
// Caller owns frame preimages and valid 8-byte iterator/allocation domain.
// No native FH3/SEH or arbitrary private helper-stack alias claim is made.
void insert_native_scene_registry_pairs_00b82fd0(void* actual_vector,
    NativeSceneRegistryInsertFrame&, NativeSceneRegistryStorageBindings&);

// Complete original helper bodies. New explicit EDX slot retains native
// thiscall B82C90's three stacked words and RET0C. EAX is advanced destination.
void* __fastcall copy_native_scene_registry_pairs_00b82c90(
    std::uint32_t incoming_ecx, void* unused_edx, const void* first,
    const void* last, void* destination) noexcept;
// B82A90[62], cdecl three words, EAX destination-(last-first), RET.
// Backward pair traversal still reads/stores word0 BEFORE reading word1.
void* __cdecl move_native_scene_registry_pairs_backward_00b82a90(
    const void* first, const void* last, void* destination_end) noexcept;
// B824F0[36], cdecl three words, RET. Reread both source words each iteration
// with the first store before the second read. Finite valid ranges required.
void __cdecl assign_native_scene_registry_pairs_00b824f0(
    void* first, void* last, const void* pair) noexcept;
} // namespace bsp
