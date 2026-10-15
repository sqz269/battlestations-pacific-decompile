#pragma once

namespace bsp {
// Complete ordinary008D48E0 on actual0Ch {data,count,capacity} vector headers
// and actual8h {stored_length,text} element headers. Counts are native signed
// DWORDs: unequal counts fail, equal nonpositive counts succeed without data
// access. A zero stored length equals only another zero stored length; both
// text pointers are ignored then. Otherwise compare NUL-terminated text using
// current CRT _stricmp, without comparing the two nonzero stored lengths.
// Bases and the other count are reloaded each iteration. No allocation,
// snapshot, header validation or replacement callback is introduced.
// Requires admitted readable native header words and CRT string/locale
// contracts for taken comparisons. Source bool/cdecl2ptr differs from native
// thiscallECX/stackRET4/AL, and makes no upperEAX, fault, CRTversion or game claim.
bool equal_native_string_vector_casefold_008d48e0(
    const void* vector_header, const void* other_header);
} // namespace bsp
