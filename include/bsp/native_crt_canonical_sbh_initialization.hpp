#pragma once

#include <cstdint>

namespace bsp {

// Complete 00C11CF5..00C11D3C: one raw DWORD cdecl threshold, EAX 0/1,
// frameless original entry and plain RET; caller removes the argument.
// Caller owns readable/writable canonical storage and the valid current CRT
// heap handle at 0109E1BC. Admission of page 0109E000 alone creates no heap.
// Calls real HeapAlloc(current heap, 0, 140h), publishes even a null result to
// 0109ED68, and on success reads/clears 0109E310 then 0109ED64, publishes the
// saved allocation to 0109ED70, the delayed-read threshold to 0109ED6C, and
// 10h to 0109ED74. No context argument, allocation clearing or failure repair.
// Original instruction/flag/import schedule is retained except the genuine IAT
// relocation. Relocatable code supplies no original placement/fault PCs, heap
// bootstrap/lifetime, native caller reachability, startup or gameplay closure.
std::uint32_t __cdecl initialize_native_crt_canonical_sbh_00c11cf5(
    std::uint32_t actual_threshold);

} // namespace bsp
