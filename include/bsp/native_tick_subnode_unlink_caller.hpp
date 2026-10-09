#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native tick subnode unlink composition requires MSVC Win32.
#endif

namespace bsp {

// Ordinary 00875960[69] through a new explicit naked fastcall Source API.
// ECX is the actual receiver, EDX an unused placement word. Three stack words
// hold addresses of the actual borrowed node-input/F-publication/M-publication
// cells. RET 0Ch pops those three references; Original instead uses RET 4.
// Void does not invent a common semantic EAX result: Leave may replace it.
//
// Keep all three actual cells alive at stable addresses for the whole call;
// do not pass snapshots or private publication cells. The current node word
// is read only after the actual getter and optional Enter/depth increment.
// The getter borrows both actual publication cells under its own contract.
// Its result+4 section is captured once for Enter/depth/Leave. Depth is the
// raw DWORD at section+18h, incremented after Enter and decremented after the
// post-unlink node+4 clear, before Leave. No owned unwind cleanup is added.
//
// Borrow writable receiver backing through +27h (embedded raw list at +1Ch),
// node backing through +0Fh, and selected neighbor/section backing required
// by the admitted raw unlink leaf and current Win32 imports. Preserve the
// leaf's actual current-EDI behavior: the following +4 clear does not reload
// the input cell or use EAX. Normal saved-register restoration and return
// require valid selected memory and intact spill/control-stack backing.
//
// No validation, clamp, RAII, catch, helper, static owner, node/list producer,
// profile, virtual-slot binding or forced application retention is supplied.
// These extra Source cells, getter arguments and frame are not Original stack
// identity. Current Source getter/import policy, Original ABI/placement/callers,
// exceptions, faults, concurrency, common register residuals and gameplay
// remain separately qualified; no Original drop-in compatibility is claimed.
void __fastcall unlink_native_tick_subnode_00875960(
    void* actual_receiver,
    std::uint32_t unused_edx,
    void* volatile& actual_node_input_word,
    void* volatile& actual_registry_publication_00f878cc,
    void* volatile& actual_manager_publication_01090aa0);

} // namespace bsp
