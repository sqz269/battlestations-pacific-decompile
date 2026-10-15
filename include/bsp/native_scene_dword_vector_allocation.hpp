#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw scene DWORD-vector allocation requires MSVC Win32.
#endif

namespace bsp {

// Complete00466CE0 [85B/25ops]. ECX unsigned count, unused incoming EDX,
// no stack input, plain RET/EAX raw allocation. Zero still requests zero bytes.
// Counts through3FFFFFFF request count*4 bytes through the actual current CRT
// allocator; larger counts throw Source std::bad_alloc at the accepted existing
// 00BCFEB0 service boundary. No count narrowing or extra zero/null guard.
void* __fastcall allocate_native_scene_dword_vector_00466ce0(
    std::uint32_t count, std::uint32_t unused_edx);

// Complete0046A400 [105B/23ops]. No consumed input or normal return. Reuses
// complete00BD0590: initialize native1Ch message cap15/length0/firstNUL with
// leading DWORD untouched, assign18 bytes, THEN arm completed-message cleanup,
// construct actual28h411700 payload, publishD69260 and throw the existing owning
// NativeSingletonVectorLengthError transport, not generic std::length_error.
// Compiler action00C61C50 maps to that guard's4072D0 message destruction only;
// it is not exposed as a new Source binary entry or an original private frame.
[[noreturn]] void throw_native_scene_dword_vector_length_0046a400();

// Source-only delegation after full native body/target and unwind comparison;
// neither original calls its canonical provider. Correct native library names
// remain unchanged. Existing current malloc/_callnewh/throw and owning message
// implementations are real providers, not callbacks/stubs. New catch types,
// RTTI, current CRT/heap/errno/handler policy and host C++ EH do not establish
// original static-CRT/throwinfo/FH3/SEH/private-stack/register/binary ABI or game
// identity. No application admission or native differential proof is implied.
} // namespace bsp
