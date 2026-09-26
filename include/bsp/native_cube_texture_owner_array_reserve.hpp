#pragma once

#include "bsp/native_render_pointer_arrays.hpp"
#include <cstdint>

namespace bsp {

// Complete 00735FF0..0073604E: native ECX actual12-byte table/count/capacity
// header, stack signed request, RET4; no semantic result. New Win32 C++ API.
// Clamp request to1; retain wrapped byte arithmetic, current field reloads,
// actual allocation/free and table-then-capacity publication. Count is untouched.
// All reached storage must be backed; no new checks, ownership or rollback.
void reserve_native_cube_texture_owner_array_00735ff0(
    void* actual_header, std::int32_t requested_capacity);

// Typed path for an already-live canonical header. Each reached source slot
// must hold a live void*; aligned replacement slots begin their scalar pointer
// lifetime only at the native reached copy store. Inactive capacity is untouched.
// Preserve volatile current reads, DWORD wrapping, free and publication order.
// This overload does not admit raw renderer/image headers or initialize them.
void reserve_native_cube_texture_owner_array_00735ff0(
    NativeRenderPointerArrayStorage& actual_header, std::int32_t requested_capacity);

} // namespace bsp
