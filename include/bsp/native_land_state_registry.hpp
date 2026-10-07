#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {

// Actual Win32 eight-byte vector element. Ordered volatile word access retains
// the native copy/fill observations; names and states are borrowed identities.
struct NativeBotStatePair {
    const char* volatile name_00;
    void* volatile state_04;
};

// Actual vector subobject at registry+4. No default buffer/proxy/allocator.
struct NativeBotStateVectorStorage {
    std::uint32_t proxy_00;
    NativeBotStatePair* volatile begin_04;
    NativeBotStatePair* volatile end_08;
    NativeBotStatePair* volatile capacity_end_0c;
};

struct NativeBotStateRegistryStorage {
    // 411E20 writes numeric CE37DC. This SOURCE word is UNCALLABLE: complete
    // class profile/scalar destructor 411810 and original binding are absent.
    volatile std::uint32_t profile_word_00;
    NativeBotStateVectorStorage vector_04;
};

struct NativeBotStateIterator {
    NativeBotStateVectorStorage* volatile owner_00;
    NativeBotStatePair* volatile position_04;
};

// Required immutable mappings to the eight actual original char-pointer
// identities. No copied/allocated names, lookup callback or default labels.
struct NativeLandStateNames {
    const char* moveto_00d1fe1c;
    const char* follow_00d1fe0c;
    const char* line_00d1fe00;
    const char* standby_00d1fdf0;
    const char* begin_00d1fde4;
    const char* final_00d1fdd8;
    const char* park_00d1fdcc;
    const char* abort_00d1fdc0;
};

static_assert(sizeof(void*) == 4, "native registry SOURCE requires Win32");
static_assert(sizeof(NativeBotStatePair) == 8);
static_assert(sizeof(NativeBotStateVectorStorage) == 0x10);
static_assert(sizeof(NativeBotStateRegistryStorage) == 0x14);
static_assert(sizeof(NativeBotStateIterator) == 8);
static_assert(offsetof(NativeBotStateRegistryStorage, vector_04) == 4);
static_assert(offsetof(NativeBotStateVectorStorage, begin_04) == 4);
static_assert(offsetof(NativeBotStateVectorStorage, end_08) == 8);
static_assert(offsetof(NativeBotStateVectorStorage, capacity_end_0c) == 0xc);

// NEW SOURCE APIs, not original register/stack bridges. Complete ordinary
// integer bodies at the named addresses. Require coherent stable actual
// storage/ranges and native-valid signed address distances. Element counts,
// capacities, requested reserve capacity AND total size+insert count must be
// <=1FFFFFFF and satisfy the native length/address arithmetic requirements.
// Require terminating ranges and valid nonoverlapping
// allocation/copy destinations (intentional in-place shift overlap admitted),
// and ordinary allocator returns. Input pair may alias an existing element:
// 411960 captures it before copying/growth. All backing allocations must use
// the existing singleton_lifetime_allocate/free CRT domain. Actual addresses
// and fields must remain valid; no structural reentry/concurrent mutation.
// Native CRT diagnostics, overflow/throw paths, private EH/cleanup, arbitrary
// invalid/null placement, original binary ABI and gameplay remain unbound.
NativeBotStatePair* allocate_native_bot_state_pairs_00410400(std::uint32_t count);
NativeBotStatePair* copy_native_bot_state_pairs_00410c00(
    const NativeBotStatePair* first, const NativeBotStatePair* last,
    NativeBotStatePair* destination);
void assign_native_bot_state_pairs_00410fa0(NativeBotStatePair* first,
    NativeBotStatePair* last, const NativeBotStatePair& pair);
void fill_native_bot_state_pairs_00410fe0(NativeBotStatePair* destination,
    std::uint32_t count, const NativeBotStatePair& pair);
NativeBotStatePair* shift_native_bot_state_pairs_backward_004112e0(
    NativeBotStatePair* first, NativeBotStatePair* last,
    NativeBotStatePair* destination_end);
NativeBotStatePair* fill_native_bot_state_pairs_004114d0(
    NativeBotStateVectorStorage& metadata, NativeBotStatePair* destination,
    std::uint32_t count, const NativeBotStatePair& pair);
NativeBotStatePair* copy_native_bot_state_pairs_00411670(
    NativeBotStateVectorStorage& metadata, const NativeBotStatePair* first,
    const NativeBotStatePair* last, NativeBotStatePair* destination);
void insert_native_bot_state_pairs_00411960(NativeBotStateVectorStorage&,
    NativeBotStatePair* position, std::uint32_t count,
    const NativeBotStatePair& pair);
void reserve_native_bot_state_pairs_00411bb0(
    NativeBotStateVectorStorage&, std::uint32_t requested_capacity);
NativeBotStateIterator* insert_native_bot_state_pair_00411ca0(NativeBotStateVectorStorage&,
    NativeBotStateIterator& output, const NativeBotStateIterator& position,
    const NativeBotStatePair& pair);
void append_native_bot_state_pair_00411d90(
    NativeBotStateVectorStorage&, const NativeBotStatePair& pair);
NativeBotStateRegistryStorage* construct_native_bot_state_registry_00411e20(
    NativeBotStateRegistryStorage&);
NativeBotStateRegistryStorage* add_native_bot_state_00411e70(
    NativeBotStateRegistryStorage&, const char* actual_name, void* actual_state);

// Complete 9AF9A0 consumer: SAME actual approach (task+3F8), registry+B8
// (task+4B0), eight embedded state identities. Requires at least274h bytes of
// live actual approach storage, an admitted initialized registry and immutable
// actual name mappings. Does not allocate/init states, enter them or touch the
// canonical task404 cell. Whole9B2E50 construction/profile/lifetimes unbound.
void register_native_land_states_009af9a0(void* actual_approach,
    const NativeLandStateNames& actual_names);

} // namespace bsp
