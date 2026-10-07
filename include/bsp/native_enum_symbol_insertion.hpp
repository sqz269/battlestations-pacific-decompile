#pragma once
#include "bsp/native_string.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
inline constexpr std::size_t native_scene_enum_owner_allocation_bytes = 0x19c;

// Complete ordinary 008F4DD0 body through a new interface, not a CEnum class ABI.
// Borrow an actual aligned19Ch allocation: profiles0=D16508/4=D162C0, count8,
// 64 bucket heads0C..108, words10C=9999/110=0, owning8h string114/118,
// and inline authored-name area11C..19B. Initialize ONLY its first name byte.
// Type bytes must be the verified00D162D4 binding (one 'E' byte); no type/enum
// namespace or virtual lifecycle is manufactured from these physical stores.
void* initialize_native_scene_enum_owner_008f4dd0(void* actual_raw_19c_owner,
    NativeStringRawPoolContext& actual_strings,
    const void* actual_type_bytes_00d162d4);

// Complete ordinary 008F2850 body. Receiver is owner+4, not the outer owner.
// Lookup a genuine8h query; hit replaces ONLY opaque mappedword+8; miss takes
// a real14h slot from actual00E17578-compatible pool, owns its copied key,
// then publishes next/bucket/count in native order. Return the current count.
// Higher008F2E40 duplicate rejection/retention is separate and unchanged.
std::uint32_t insert_native_enum_symbol_word_008f2850(void* actual_map_receiver,
    const NativeString& actual_key, std::uint32_t opaque_word,
    void* actual_initialized_symbol_pool_00e17578,
    NativeStringRawPoolContext& actual_strings,
    const char* empty_00e186ed, const char* empty_00e17654);

// Successful nonoverflowing, stable/disjoint/coherent ordinary storage only;
// closed NUL-free ASCII queries in C locale, null data only for empty keys.
// No added null guard/default/rollback/reset; destroy owning headers explicitly
// before returning/freeing storage. The slot's allocator-owned+10 is preserved.
// Original globals, FH3/EH/fault/CRT failure/reentry, CEnum destructor/virtual
// namespace/declaration identity, traffic, original ABI and game remain external.
} // namespace bsp
