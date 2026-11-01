#pragma once
#include <cstdint>

namespace bsp {
// Whole ordinary NORMAL 00735F90, original ECX actual12h data/count/capacity,
// one signed capacity DWORD/RET4. New C++ interface; no semantic DwordVector.
// Clamp to1; signed capacity gate; wrapped request*4 actual allocation; current
// count/table reads on each reached forward copy; current old table free; then
// publish replacement table before capacity. Count and inactive slots untouched.
// Borrow genuine initialized reached raw storage and the established physical
// allocation/free domain. No pointee retain/release, guard, rollback or cleanup
// is introduced. Original ABI/FH3/fault/runtime/game permission is not claimed.
void reserve_native_texture_atlas_item_pointers_00735f90(
    void* actual_header12, std::int32_t requested_capacity);
} // namespace bsp
