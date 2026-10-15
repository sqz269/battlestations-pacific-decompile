#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual outer vector resize requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete006A6350. Native ECX actual16B outer header, EDX unconsumed;
// stack requested_count then an OWNED by-value16B inner header, RET14h.
// No semantic result. Source transfers its five scalar stack words into a
// canonical private header without another copy constructor/allocation.
// The caller transfers the begin/end/capacity ownership; opaque0 is only bits.
// Every admitted exit consumes that owned fill, including equal/shrink/throws.
void __fastcall resize_native_outer_vector_006a6350(void* header,
    std::uint32_t unused_edx, std::uint32_t requested_count,
    std::uint32_t owned_fill_opaque, std::uint32_t owned_fill_begin,
    std::uint32_t owned_fill_end, std::uint32_t owned_fill_capacity,
    NativeStringRawPoolContext& strings);

// Complete00C7F4B0: LEA ECX,[EBP+8]; JMP004324A0. Source receives the
// owned actual inner header explicitly; no original private-EH-frame adapter.
void __fastcall unwind_native_outer_resize_parameter_00c7f4b0(
    void* owned_fill, NativeStringRawPoolContext& strings);

// Added context/private storage and current C++/CRT services do not establish
// original register/stack/FH3/SEH/fault/cleanup-failure/gameplay equivalence.
// No extra validation, deep-copy argument, callback, or container is supplied.
} // namespace bsp
