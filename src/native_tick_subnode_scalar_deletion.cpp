#include "bsp/native_tick_subnode_scalar_deletion.hpp"

#include "bsp/native_tick_subnode_base_cleanup.hpp"

#include <cstdlib>

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);

// Preserve the late flag-byte load, the physically present Native ADD ESP,4,
// and current-ESI semantics. Only publication forwarding/RET consumption adapt
// the Source interface; the actual CRT import form is compiled-review-owned.
__declspec(naked) void* __fastcall scalar_delete_native_tick_subnode_0071c4d0(
    void*, std::uint32_t, std::uint32_t, void* volatile&, void* volatile&) {
    __asm {
        push esi                                  // 0071C4D0
        mov esi, ecx                              // 0071C4D1
        push dword ptr [esp + 10h]                // Source: actual M cell address
        push dword ptr [esp + 10h]                // Source: actual F cell address
        call cleanup_native_tick_subnode_base_00875b30 // 0071C4D3: Source RET8
        test byte ptr [esp + 8], 1                // 0071C4D8: current flags, late
        jz short scalar_complete                  // 0071C4DD
        push esi                                  // 0071C4DF: actual current ESI
        call free                                 // 0071C4E0: current Source CRT
        add esp, 4                                // 0071C4E5: omitted saved start
    scalar_complete:
        mov eax, esi                              // 0071C4E8: opaque after free
        pop esi                                   // 0071C4EA: current saved word
        ret 0ch                                   // 0071C4EB: Source 3 stack words
    }
}

} // namespace bsp
