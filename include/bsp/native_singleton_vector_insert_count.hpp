#pragma once

#include <cstdint>

namespace bsp {

// Complete 00BD0700..00BD08B3 (436 bytes). ECX is the actual raw vector
// owner; EDX and iterator_owner are unconsumed. Four stack arguments,
// RET10h. There is no specified result register. The owner layout is
// untouched DWORD+0, begin+4, end+8, capacity-end+0C; each slot is 4 bytes.
//
// Captures *value_slot into its own argument word before even count==0.
// Preserves the full unsigned count, growth arithmetic and both in-place
// insertion paths. Uses fixed completed raw helpers and the actual shared
// malloc/free and SDK memmove_s services; no typed manager or callback.
//
// No local exception cleanup exists in the original or rebuilt entry.
// Provider exceptions propagate; replacement storage is not compensated if
// a later provider throws. Source allocation/length exceptions and source
// CRT invalid-handler/errno ownership are explicit provider boundaries,
// not original static-CRT/FH3/RTTI or fault-site equivalence.
void __fastcall insert_count_native_singleton_slots_00bd0700(
    void* owner, void* unused_edx, const void* iterator_owner,
    void* position, std::uint32_t count, const void* value_slot);

} // namespace bsp
