#pragma once

#include "bsp/native_nested_string_vector_allocation.hpp"

#include <cstdint>

namespace bsp {

// Whole 00696890, 81B/25 instructions. Native ECX unsigned count, unused EDX,
// no stacked inputs, plain RET and raw allocation EAX. Complete native bytes
// match accepted 00553300 except relocated CALL operands with identical targets.
// Pass the SAME count to that actual 16-byte allocation service. Zero still
// requests zero bytes; counts through 0FFFFFFF admit count*16 (max FFFFFFF0).
// No four-byte allocator/count bridge, typed container, callback or host limit.
void* __fastcall allocate_native_dword_vector_header_storage_00696890(
    std::uint32_t count, void* unused_edx);

// Whole 006978D0, 105B/23 instructions, no consumed input or normal return.
// Exact full-helper match to accepted 00556C90, including counted18-character
// message, actual legacy logic-error payload and completed-message cleanup.
// Delegate the FULL owning service; do not add another temporary or cleanup.
[[noreturn]] void __cdecl throw_native_dword_vector_header_length_error_006978d0();

// Whole C7EBA0, 8B/2 instructions, selected only by state0 in the proved44B
// native EH map. Native LEA ECX,[EBP-50h]; JMP actual004072D0. This explicit
// reference selects an EXISTING completed1Ch message; it creates no ownership
// and exposes no EBP/frame helper. Invoke actual destruction once. The full
// throwing delegate already owns this cleanup; do not wrap it with this call.
void cleanup_native_dword_vector_header_length_message_00c7eba0(
    NativeLegacySboStringStorage& completed_message) noexcept;

// These new Source interfaces use the accepted current allocation/free and
// owning NativeSingletonVectorLengthError boundaries. Native D6923C/E03CC0
// bad_alloc identity, D69260/D83F98 legacy RTTI/throw transport, static CRT heap
// and new-handler identity, FH3/SEH/fault/private-frame aliases, original binary
// ABI and gameplay behavior remain unproved. Library names/preimages survive.

} // namespace bsp
