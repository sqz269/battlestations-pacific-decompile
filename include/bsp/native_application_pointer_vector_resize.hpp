#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Application pointer-vector resize requires MSVC Win32 assembly.
#endif

#include "bsp/native_shader_preload.hpp"

namespace bsp {

// Complete ordinary 00735F30..00735F7F schedule, 80 bytes/31 instructions.
// This public C++ interface borrows the EXISTING actual 0Ch vector header and
// allocation binding; it is distinct from native ECX/header, stack signed
// request and RET4. There is no established semantic native EAX result.
// Compare signed capacity first and call the concrete 735EC0 reserve service
// only when required. Growth reloads data each iteration, tests the calculated
// low32 slot address, and writes a zero DWORD there only when that address is
// nonzero. Its index wraps; growth makes no direct count-field publication.
// Shrink decrements the actual count field each iteration and compares its
// current signed value again. Always finish by storing the requested count.
// Preserve signed negative inputs, wrapping arithmetic, aliases and cycles;
// do not replace request0 with clear, free or per-element release. The actual
// reached storage and allocation/free services must satisfy their existing
// contracts. Invalid accesses may fault with preceding stores/effects retained.
// No validation, noexcept promise, catch, rollback or new cleanup is added.
// Original CRT/FH3/fault/service-register identity, callable Application tables,
// common owner construction and production wiring remain separate boundaries.
void resize_native_application_pointer_vector_00735f30(
    NativeApplicationPointerVectorStorage& actual_rows,
    std::int32_t requested,
    const NativeApplicationPointerVectorAllocation& allocation);

} // namespace bsp
