#pragma once

namespace bsp {
// Full AE1C60..AE1C76[23]. Original ECX is the actual three-DWORD array
// header (data+0, signed count+4, signed capacity+8), no public words, RET.
// Source EDX is explicitly unused. Full AE19B0(0), then a fresh data read,
// source-CRT free, discard that argument and return. Preserves ESI.
void __fastcall destroy_native_shadow_pointer_vector_00ae1c60(
    void* actual_header, void* unused_edx);

// Header/current backing must satisfy the existing EP raw resize and shared
// source CRT domain. No pointer/capacity clear, element release, header free,
// null skip or failure cleanup is added. Native unwind callers establish only
// a +44 array subobject, not complete parent/EH or actual producer lifetimes.
// Provider call relocations, source allocator failure and exceptions across
// naked frames do not establish unrestricted native ABI/fault/FH3/game parity.
} // namespace bsp
