#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native tick subnode base cleanup requires MSVC Win32.
#endif

namespace bsp {

// Complete 00875B30..00875B83[84] through a new naked Source interface. ECX
// supplies the actual node; EDX is unused placement. Two stack words hold the
// actual F/M publication-cell addresses. Source RET8 consumes those references;
// the Native entry takes no stack argument and ends in plain RET.
//
// Read node+8, then stamp opaque DWORD 00CFD99C at node+0 while retaining the
// comparison flags. Only if that first read is zero, read node+C afresh. Either
// nonzero link takes the slow path: capture node+4 parent BEFORE the getter or
// lock; capture getter-result+4 section once; optional Enter/depth++ at +18h;
// raw unlink on captured parent+1Ch and captured node; TEST current section,
// clear node+4 as a full DWORD, then optional depth--/Leave. No local unwind
// release is added. The profile write also occurs on the empty fast path.
//
// Borrow node backing through +0Fh, selected parent/list and neighbor backing
// required by the admitted raw unlink, and the actual Win32 section/depth
// backing. Keep both publication cells at stable addresses for the call; their
// values remain mutable under the admitted getter. Use actual publications,
// not copies or newly invented private cells. Preserve the added Source address
// argument words until their selected loads, plus active saved-register slots
// and return backing under aliases. The leaf's conditional EDI spill must retain
// the captured section; this body deliberately uses actual current EDI afterward.
//
// No Node/class type, callable profile, owner, allocation, validation, RAII,
// production caller or binding for the existing pure-virtual facade is supplied.
// Source getter/CRT/Win32 policy, extra stack arguments, failures, stack aliases,
// Original ABI/placement and runtime behavior remain separately qualified.
// Void supplies no common EAX result; this declaration deliberately lacks noexcept.
void __fastcall cleanup_native_tick_subnode_base_00875b30(
    void* actual_node,
    std::uint32_t unused_edx,
    void* volatile& actual_registry_publication_00f878cc,
    void* volatile& actual_manager_publication_01090aa0);

} // namespace bsp
