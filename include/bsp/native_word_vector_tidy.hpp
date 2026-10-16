#pragma once

#include <cstdint>

namespace bsp {

// Whole native 004D29D0 (46 bytes/13 instructions): incoming ECX may carry an
// owner but is never read, EDX is unused, and the actual header is the sole
// stacked word (entry ESP+4). The native return removes that word (RET 4).
// These explicit ignored Source register slots preserve that argument shape;
// they do not establish a recovered original C++ declaration or owner type.
void __fastcall tidy_native_word_vector_range_element_004d29d0(
    void* unused_owner, std::uint32_t unused_edx, void* stack_header);

// Whole native 004CDCE0 (42 bytes/13 instructions): ECX is the actual header,
// EDX is unused, there are no stacked arguments, and the native return is RET.
void __fastcall tidy_native_word_vector_storage_004cdce0(void* actual_header);

// Both directly reuse the accepted whole ordinary 004CB130 service. Capture
// DWORD[header+4] once; if nonzero free that captured pointer, then on either
// returning path clear DWORD+4, +8, +0Ch in order. No direct +0 access, element
// traversal, owner access, diagnostic, validation or additional null guard.
// The header must remain valid/writable through all stores and nonzero backing
// must belong to the admitted current CRT domain. These raw interfaces do not
// establish an element type or an independently safe containing object.
//
// Native EAX is zero on the captured-null path, otherwise incidental void-free
// residue. Both Source interfaces are void; those ambient bits/flags are not a
// semantic result or input. Current C++/CRT behavior and new Source interfaces
// do not prove original heap/CRT, FH3, SEH/fault, private frame/argument aliases,
// mangling, binary ABI, or game/runtime equivalence. Caller EH scopes remain
// caller-owned. Correct native STL_Vector_Tidy library names are retained;
// the new descriptive names are hypotheses.

} // namespace bsp
