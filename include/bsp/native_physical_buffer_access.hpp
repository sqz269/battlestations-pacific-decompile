#pragma once

#include <cstdint>

namespace bsp {
// Complete native ECX-input, plain-RET operations on actual 2Ch physical
// buffer storage. Private and pooled profiles share these field meanings.
// No owner construction, destruction, allocator selection or metadata reset.

// B23270/B23180: capture COM+28, perform AddRef/Release on that same object
// using its current table for each call, then reload COM+28 for the final
// Release. Clear +28 only after that final call returns. Exceptions propagate.
void __fastcall release_native_physical_vertex_buffer_for_reset_00b23270(
    void* actual_owner);
void __fastcall release_native_physical_index_buffer_for_reset_00b23180(
    void* actual_owner);

// B4B800/B4B9B0: return the raw borrowed owner's capacity DWORD at +18 in EAX.
std::uint32_t __fastcall native_physical_index_buffer_capacity_00b4b800(
    const void* actual_owner) noexcept;
std::uint32_t __fastcall native_physical_vertex_buffer_capacity_00b4b9b0(
    const void* actual_owner) noexcept;

// B4B820/B4B9D0: null COM+28 leaves depth unchanged. Otherwise invoke that
// COM object's current stdcall Unlock slot+30 and decrement current depth+20
// after return, including failed HRESULTs. A throw prevents the decrement.
void __fastcall unlock_native_physical_index_buffer_00b4b820(void* actual_owner);
void __fastcall unlock_native_physical_vertex_buffer_00b4b9d0(void* actual_owner);
} // namespace bsp
