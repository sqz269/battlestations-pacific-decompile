#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT double classification requires MSVC Win32.
#endif

namespace bsp {

// Complete native __sptype C12F3E[91] and __fpclass BFA52F[148]. New cdecl
// raw-word interfaces avoid C++ floating argument conversion. low_word and
// high_word occupy the actual callee's eight binary64 argument bytes. Both
// native bodies read a full unaligned DWORD at EBP+0Eh: the last two input
// bytes AND the next two caller-storage bytes. readable_tail_word supplies
// that readable extent for public source calls. Its bits do not influence
// classification after the native masks; do not narrow the original read.
// EAX is the native integer result. Caller removes all three public words.
std::int32_t __cdecl native_crt_sptype_00c12f3e(
    std::uint32_t low_word, std::uint32_t high_word,
    std::uint32_t readable_tail_word);
std::int32_t __cdecl native_crt_fpclass_00bfa52f(
    std::uint32_t low_word, std::uint32_t high_word,
    std::uint32_t readable_tail_word);

// __sptype uses only integer operations and does not touch x87 state.
// __fpclass preserves the native FLD/FSTP64 roundtrip before its direct call
// to full __sptype. That internal call prepares ONLY the original eight-byte
// temporary; __sptype's extra two-byte read reaches readable saved-frame
// bytes there, not an added argument. Exceptional inputs can change during
// the x87 roundtrip or raise a hardware exception before classification.
// Its zero test remains FLDZ/FCOMP/FNSTSW/TEST AH,44h/JP, not a host compare.
// The actual x87 control/status environment is neither replaced nor restored.
// Returning __fpclass paths balance their temporary x87 stack entry; callers
// must provide room for that entry and preserve valid input/frame storage.

// Complete native __frnd C28548[17]. ASSEMBLY CALLERS ONLY: two raw words
// occupy the actual eight-byte binary64 argument slot, caller removes them.
// The body performs FLD/FRNDINT/FSTP64/FLD64 with the current x87 environment.
// It returns one value on ST0, which the assembly caller must consume/pop;
// this void declaration does not convey that result to the C++ compiler.
// Provide one free x87 stack slot. The two ECX scratch pushes/pops are native:
// the final ECX contains the high DWORD of the rounded binary64 scratch.
// Rounding/status and hardware exception/partial stack effects are untouched.
void __cdecl native_crt_frnd_st0_00c28548(
    std::uint32_t low_word, std::uint32_t high_word);

// All three entries retain complete original instruction sequences; only the
// __fpclass direct call is bound to the full source __sptype. No host fpclass,
// rounding service, literal/global environment or argument adapter is used.
// New source interfaces; no original caller ABI, native SEH or runtime/game
// validation follows merely from compiled instruction/relocation identity.

} // namespace bsp
