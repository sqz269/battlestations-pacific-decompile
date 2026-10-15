#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This literal reference-payload entry requires MSVC Win32.
#endif

namespace bsp {

// Whole [008F0380,008F03A2): ECX actual writable eight-byte reference
// payload, opaque full DWORD scalar at +0 and owned current-heap pointer +4.
// Incoming EDX is unused; no stack arguments; plain RET; ESI saved/restored.
// Old +4 is null or a sole-owned actual current duplicate from accepted
// physical duplicate/0310/0340 provenance. Retire it before clearing +4;
// clear scalar +0 on both paths. The null path never writes pointer +4.
// Payload, allocation and active frame must be live/disjoint as applicable.
// Successful ordinary current-free domain only: no alias, reentry, concurrent
// mutation, invalid memory, fault or overflow. This frees no payload root.
// The 03F0 array header uses the reverse layout and is not interchangeable.
// No caller/class/vtable, Original private CRT/EH/full ABI or game claim.
// The descriptive name is a hypothesis, not a recovered symbol.
void __fastcall reset_native_reference_payload_008f0380(
    void* actual_payload_ecx) noexcept;

} // namespace bsp
