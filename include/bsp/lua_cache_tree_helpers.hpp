#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Lua cache string comparisons require MSVC Win32.
#endif

namespace bsp {

// These are range/vector comparisons, despite this packet's historical filename.
// Actual string headers are eight bytes: length+0 and char* data+4. Vector
// owners retain their existing word+0; begin/end/capacity are at +4/+8/+0Ch.
// No object construction, ownership transfer, allocation, or string copy occurs.
// All addresses used by a taken native path must have valid original backing.

// Complete 00B6C480..00B6C4DF, cdecl/plain RET, EAX=output. Six physical DWORD
// arguments are retained; only output/first/last/second_first are consumed.
// Walk eight-byte headers until first==last or unequal recorded lengths, or
// nonzero equal lengths with current CRT _stricmp !=0. Empty equal lengths do
// not read data pointers. Write second mismatch to output+4 BEFORE first to
// output+0, so output may alias the traversed storage. Return the same output.
// Established library name: STL_NativeStringRange_Mismatch_00b6c480.
void* __cdecl mismatch_native_string_range_00b6c480(void* output,
    const void* first, const void* last, const void* second_first,
    std::uint32_t unused_iterator_word, std::uint32_t unused_dispatch_word);

// Complete 00B6C7B0..00B6C815, cdecl/plain RET, AL Boolean only. Six physical
// DWORD args; the final two are unused. Capture SAR32((last-first),3), report
// null second_owner, then validate the wrapping second_first+count*8 endpoint
// against CURRENT owner end then begin. A returning report CONTINUES using
// captured pointers, without retry or fallback. Call the exact mismatch above
// and compare its first result with captured last. No first-range validation.
// The source uses a private two-DWORD result in place of overwriting argument
// slots 3/4. Native caller-frame aliasing and upper EAX are not modelled.
bool __cdecl equal_native_string_range_00b6c7b0(const void* first,
    const void* last, const void* second_owner, const void* second_first,
    std::uint32_t unused_end_word, std::uint32_t unused_dispatch_word);

// Complete 00B6C990..00B6CA25. Native/source ECX=left, EDX=right, EAX=0/1,
// plain RET, no stacked input. Compare signed SAR3 element counts (null begin
// means zero); a mismatch returns BEFORE validation. Preserve captured right
// begin, captured left end, and later captured left begin across returning
// invalid reports, including both separate left-range checks. Delegate to the
// range helper with all six stack words; dead dispatch padding is zeroed in
// source rather than reading indeterminate native local high bytes.
std::uint32_t __fastcall equal_native_string_vectors_00b6c990(
    const void* left, const void* right);

// CRT boundary: _stricmp uses the current Source CRT/locale. Invalid iterator
// reports use existing invoke_native_invalid_parameter_00bf6713, whose Source
// handler policy is explicitly qualified. No noexcept/noreturn, new cleanup,
// exception conversion, original CRT/EH/stack identity, production cache host
// binding, native differential result, or game validation is supplied here.

} // namespace bsp
