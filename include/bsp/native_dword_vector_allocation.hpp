#pragma once

#include "bsp/native_singleton_vector_allocation.hpp"

#include <cstdint>

namespace bsp {

// Complete 00553260..005532B4 at the accepted current-CRT boundary. Original
// ECX unsigned count, incoming EDX and stack unconsumed, EAX raw storage, RET0.
// All 25 native instructions match 00BCFEB0 after local branch relocation:
// pass the SAME count, return raw void*, and do not reinterpret pointer slots.
// Count0 still requests zero bytes; counts through 3FFFFFFF request count*4
// bytes. Larger counts throw the accepted Source std::bad_alloc. Allocation
// shares the accepted malloc/_callnewh/free domain, not the game's static CRT
// heap, global new-handler state or D6923C/E03CC0 native bad_alloc identity.
void* __fastcall allocate_native_dword_vector_00553260(
    std::uint32_t count, void* unused_edx);

// Complete 00556C20..00556C88 through the fully matched 00BD0590 service.
// No consumed input or normal return. The actual 1Ch SBO temporary receives
// capacity15/length0/NUL and the counted18-byte message, then its cleanup is
// armed before 00411700 constructs the owning28h legacy exception payload.
// The accepted NativeSingletonVectorLengthError retains D69260 as address
// data; its Current C++ catch type/RTTI/EH replace native D83F98/BF6885/FH3.
// The delegated helper already performs completed-message cleanup exactly
// once. Do not surround this call with the separate action below.
[[noreturn]] void __cdecl throw_native_dword_vector_length_error_00556c20();

// Complete 00C6D790..00C6D797: original LEA ECX,[EBP-50h]; JMP004072D0.
// Explicit reference to the existing actual 1Ch completed-message storage
// replaces the original private frame lookup; this is not a frame overlay or
// a new owning temporary. Call actual legacy destruction once. Original
// EBP/FH3 action ABI and arbitrary spill aliases are outside this Source API.
void cleanup_native_dword_vector_length_message_00c6d790(
    NativeLegacySboStringStorage& completed_message) noexcept;

// Existing STL library names are preserved in the native ledger. These are
// new Source interfaces, not installed binary/exception-ABI replacements.
} // namespace bsp
