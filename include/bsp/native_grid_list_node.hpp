#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native grid list node requires MSVC Win32.
#endif

namespace bsp {

// Complete 0041C0B0..0041C0E2 raw 0Ch list-node producer. Original/Source
// stdcall: three DWORD inputs, EAX allocation result, RET 0Ch. Incoming ECX
// is not an input. The node's next/previous/value words are +0h/+4h/+8h;
// no payload class, hull-node record or outer-owner lifetime is declared.
//
// The naked entry allocates first, then conditionally loads each CURRENT input
// stack cell immediately before its native store/use. Stores are next, previous,
// then *value_source. Individual computed destination guards and DWORD address
// wrap are retained. No argument snapshot or blanket null-allocation return.
// Supply valid writable current argument cells and reached spans, including a
// readable DWORD at the late value_source address. Raw allocation is through the
// genuine object12/12 CRT service; no new failure policy or fake callback.
// Current CRT heap/new-handler/exception identity, external provider frames,
// original binary binding and gameplay equivalence remain qualified.
// Evidence: docs/GOAL15D_GRID_LIST_NODE.md.
void* __stdcall buy_native_grid_list_node_0041c0b0(
    std::uint32_t next, std::uint32_t previous, const void* value_source);

} // namespace bsp
