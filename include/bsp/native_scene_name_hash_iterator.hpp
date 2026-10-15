#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw scene name-hash iteration requires MSVC Win32.
#endif

namespace bsp {

// Complete00466BB0 [65B/22ops] and00466C00 [92B/33ops]. ECX iterator12,
// EDX unused, no stacked inputs, plain RET. Begin has no common semantic EAX
// result; Next returns fully zero-extended EAX0/1. Descriptive names are hypotheses.
// Actual iterator words: table0,node4,index8; table test4,heads8+index*4;
// node linkC. No nominal container representation or count invariant is imposed.
void __fastcall begin_native_scene_name_hash_00466bb0(void* actual_iterator12);
std::uint32_t __fastcall next_native_scene_name_hash_00466c00(void* actual_iterator12);

// Begin captures table once, leaves index unchanged when table+4 is zero,
// and may read head64 before returning. Next preserves its distinct scan bound,
// captured scan table, CURRENT post-scan index/table loads and final node reload.
// Reached raw storage may alias wherever all reached accesses stay valid.
// Implicit native/Source stack aliases, asynchronous mutation, private frame,
// register/flags/fault/mangling/binary ABI and game identity are not established.

} // namespace bsp
