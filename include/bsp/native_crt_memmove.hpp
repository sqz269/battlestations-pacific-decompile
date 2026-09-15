#pragma once

#include <cstdint>

namespace bsp {
// Complete BF87E0 copy behavior through a new MSVC Win32 cdecl interface.
// The fourth argument must borrow the actual stable canonical 0109EEA4 word;
// it creates no feature state. Read only after the native forward/size>=256
// gates. An eligible nonzero word and matching pointer low bits tail-jump to
// the actual native_crt_vector_copy provider. The caller removes 16 bytes.
// Requires DF=0 and valid readable/writable nonwrapping extents; overlap is
// supported by the original unsigned direction decision. Extents must avoid
// live argument/return slots and active source/provider frames. Actual SSE2
// CPU/OS support must hold whenever the selected provider path requires it.
// Zero count does not read buffer data; equal pointers retain native copying.
// Returns destination; preserves EBP/EBX/ESI/EDI. EAX/ECX/EDX and arithmetic
// flags are volatile. Bounded branch dispatch replaces native table reads,
// so arithmetic flags and code/table/fault-PC layout are not native identity.
// No validation, catch, owner, overflow repair or fault recovery is supplied.
// Native access order and partial writes remain; a fault between backward
// STD and CLD can expose DF=1. This four-argument ABI is not the native entry.
void* __cdecl move_native_crt_bytes_00bf87e0(
    void* destination, const void* source, std::uint32_t byte_count,
    const volatile std::uint32_t& actual_feature_word_0109eea4);
} // namespace bsp
