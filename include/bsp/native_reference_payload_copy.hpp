#pragma once

#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Physical reference payload copy requires MSVC Win32.
#endif

namespace bsp {

inline constexpr std::size_t native_reference_payload_bytes = 8;

// Whole 008F0340..008F0377: 55 bytes / 22 instructions. Actual ECX is an
// 8-byte destination payload; the sole stack DWORD addresses a readable
// 8-byte source payload. Incoming EDX is unused. Source +4 text and +0 raw
// scalar bits are snapshotted before freeing a nonnull destination +4.
// Destination +4 must be null or a sole-owned actual current duplicate.
// Payloads, borrowed NUL-terminated text, old allocation and live call frames
// must be disjoint; in particular, self/shared old-text alias would be UAF.
// Text length+1 must fit without address wrap; current CRT requires DF clear.
// Writes destination +0 scalar and +4 actual new owned copy (or null).
// Full EAX is that copy, NOT the destination. RET4; normal nonvolatiles survive.
// Free each nonnull returned/stored copy once via singleton_lifetime_free
// before disposing/reusing caller-owned payload storage. Never read old freed
// text. Allocation may reuse the old address; pointer inequality is not promised.
// No phase/vtable or other record fields are touched. The old copy may already
// be freed/null when duplication fails; no failure/EH guarantee is admitted.
// Null EDX preserves incoming bits only when old +4 was null (no free).
// Null XOR AF is undefined; nonnull flags derive from nested ADD32(T-48,16),
// where T is the actual source-argument ESP immediately before target CALL.
// No original private free, native class/destructor or whole caller is admitted.
char* __fastcall copy_native_reference_payload_008f0340(
    void* actual_destination_ecx, void* unused_edx,
    const void* actual_source_stack);

}  // namespace bsp
