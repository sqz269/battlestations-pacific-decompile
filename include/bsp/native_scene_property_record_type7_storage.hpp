#pragma once

#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene property record type-7 storage requires MSVC Win32.
#endif

namespace bsp {

// Whole [008EF270,008EF2AF): 63 bytes, 19 instructions. Descriptive names
// are hypotheses. This partially initializes raw storage; it does not create
// a reconstructed vector class or establish an owning native object lifetime.
inline constexpr std::size_t native_scene_property_record_type7_storage_bytes = 0x38;

// Supply actual fresh/unowned writable 56-byte destination storage and an
// actual stable readable 12-byte input span. They must be disjoint from each
// other and from the active target call frame. The caller retains both storage
// lifetimes; the borrowed input pointer is neither retained nor freed. Do not
// overwrite a live owning property object.
// ECX=destination; incoming EDX unused; [entry ESP+4]=input pointer; RET4.
// EAX=destination, ECX=0, EDX=second input DWORD (+4); EBX/ESI/EDI/EBP and
// DF are preserved. Final XOR flags: CF=OF=SF=0, ZF=PF=1; AF is undefined.
// Writes 45 bytes: [00,08), [0C,28), [2C,2D), [30,38).
// Preserves 11 bytes: [08,0C), [28,2C), [2D,30).
// Writes literal opaque phase identity 00CE89D4 and DWORD tag 7 before the
// three interleaved input loads/output stores. Raw input DWORDs +0/+4/+8 go
// to destination +0C/+10/+14. No numeric, SSE, x87 or MXCSR operation occurs.
// No overlapping-memory/snapshot-copy or invalid-input fault contract is
// supplied. Never dispatch the opaque phase word as a Source vtable here.
// Native clone/allocator/EH, parsing, class dispatch and ownership/destruction
// are outside this raw storage contract.
void* __fastcall construct_native_scene_property_record_type7_storage_008ef270(
    void* actual_record_ecx, void* unused_edx,
    const void* actual_payload_words) noexcept;

} // namespace bsp
