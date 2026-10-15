#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene record map lookup requires MSVC Win32.
#endif

namespace bsp {

// Complete 00468140..00468192 (83 bytes). Native ECX actual map; one stacked
// actual8h key-header pointer; RET4; EAX node. Source retains the physical
// inputs using fastcall plus explicitly unused EDX. Read map+4 head once,
// head+4 root, and node+19 sentinel before any key/header access. Compare
// node+C against key through the accepted concrete00443D00 predicate (the
// native inlines that exact length/C-string rule and calls CRT BF7FBF).
// Take current right+8 if less; otherwise remember node and take current
// left+0. Return captured head when no candidate exists. No map/key checks.
void* __fastcall lower_bound_native_scene_record_map_00468140(
    void* actual_map, std::uint32_t unused_edx, const void* actual_key_header);

// Complete 00468CD0..00468D35 (102 bytes). Native ECX map; stacked hidden
// output/key; RET8; EAX output. Source retains physical slots and unused EDX.
// Run exact lower-bound BEFORE null-owner CRT report; a returning report
// continues. Compare found node with CURRENT map+4 head; if unequal compare
// key against node+C. Fallback rereads CURRENT map+4 after the comparison.
// Capture owner/node before ordered output+0 owner, output+4 node writes,
// retaining output/map/node/key alias effects. No iterator object is created.
void* __fastcall find_native_scene_record_map_00468cd0(void* actual_map,
    std::uint32_t unused_edx, void* actual_iterator_output,
    const void* actual_key_header);

// Ordinary Source over existing actual storage, concrete comparator and
// current-SDK returning CRT provider; no STL replacement, host dispatch,
// allocation, validation expansion, noexcept, rollback or local EH. Source
// pointer arguments do not expose native private argument-slot mutation or
// private iterator temporaries. Current CRT locale/handler policy, valid taken
// backing and provider ABI domains remain inherited. Physical slots alone
// establish no native binary/SEH/fault-address or game equivalence.

} // namespace bsp
