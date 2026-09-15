#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native shadow pointer vector storage requires MSVC Win32.
#endif

namespace bsp {

// Complete AE1440[95] / AE19B0[80]. Native ECX is the actual three-DWORD
// header: data+0, signed count+4, signed capacity+8. One signed public stack
// word is consumed by RET4. Unused EDX preserves that original stack slot.
// These functions borrow actual header/backing storage; elements are opaque
// DWORDs. The caller supplies valid storage for every reached native access.
void __fastcall reserve_native_shadow_pointer_vector_00ae1440(
    void* actual_header, void* unused_edx, std::int32_t requested);
void __fastcall resize_native_shadow_pointer_vector_00ae19b0(
    void* actual_header, void* unused_edx, std::int32_t requested);

// Reserve uses the existing real source-CRT allocation/free domain, with
// wrapped DWORD byte counts. Source exceptions escape; no rollback or native
// EH frame is added. Original CRT hooks, OOM/fault/unwind behavior and incidental
// provider register values are not established by these new C++ interfaces.

} // namespace bsp
