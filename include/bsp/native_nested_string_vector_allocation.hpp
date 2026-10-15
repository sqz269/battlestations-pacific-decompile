#pragma once

#include "bsp/native_string_vector_allocation.hpp"

#include <cstdint>

namespace bsp {
// Complete00553300..00553350 at the accepted actual current-CRT boundary.
// Native ECX unsigned count, EDX not consumed, no stack inputs; EAX allocation,
// plain RET. Source fastcall reserves ignored EDX. Positive unsigned
// FFFFFFFF/count>=16 admits count<=0FFFFFFF; maximum requestFFFFFFF0 bytes.
// Zero STILL requests0 bytes through the actual allocation service. No extra
// host-size limit, callback, vector construction or allocation model is added.
//
// Uses singleton_lifetime_allocate (current malloc/_callnewh/retry/bad_alloc),
// object byte-domain, native_bytes=host_bytes=count*16, matching singleton free.
// Overflow has the same Source std::bad_alloc boundary as accepted0042BF40;
// original BF6340/D6923C/E03CC0 payload/RTTI/FH3 and static-CRT heap/new-handler
// identities remain qualified. This is raw backing for16B nested headers.
void* __fastcall allocate_native_nested_string_vector_00553300(
    std::uint32_t count, void* unused_edx);

// Complete00556C90..00556CF8 via exact matched accepted00431F70 owning service.
// No inputs/normal return. Actual no-init SBO cap15,len0,NUL; counted18-byte
// vector<T> too long; actual00411700 legacy payload/D69260 word; completed
// temporary cleanup4072D0, armed only after counted assignment. Throws the
// accepted NativeSingletonVectorLengthError owning the actual28h payload,
// without generic std::length_error or additional SBO initialization/cleanup.
// Source catch/RTTI/C++EH replace originalD83F98/BF6885/FH3 identity; address
// vtable word is data rather than a callable host vtable. No original binary
// ABI/native exception execution or gameplay claim.
[[noreturn]] void __cdecl throw_native_nested_string_vector_length_error_00556c90();
} // namespace bsp
