#pragma once

#include <cstdint>

namespace bsp {

// Complete 00735FF0..0073604E: native ECX actual12-byte table/count/capacity
// header, stack signed request, RET4; no semantic result. New Win32 C++ API.
// Clamp request to1; retain wrapped byte arithmetic, current field reloads,
// actual allocation/free and table-then-capacity publication. Count is untouched.
// All reached storage must be backed; no new checks, ownership or rollback.
void reserve_native_cube_texture_owner_array_00735ff0(
    void* actual_header, std::int32_t requested_capacity);

} // namespace bsp
