#include "bsp/native_subtree_invalidation.hpp"

namespace bsp {

// Complete 38-byte/13-instruction native schedule; the CALL targets this
// Source entry. Both MOV stores preserve the flags from the captured-child
// TEST. The sibling is read only after the entire recursive child returns.
__declspec(naked) void __fastcall invalidate_native_subtree_pose_0042ed50(void*) {
    __asm {
        push esi
        mov esi, dword ptr [ecx + 48h]
        test esi, esi
        mov byte ptr [ecx + 0c8h], 0
        mov byte ptr [ecx + 10ch], 0
        jz short subtree_done
    subtree_child:
        mov ecx, esi
        call invalidate_native_subtree_pose_0042ed50
        mov esi, dword ptr [esi + 44h]
        test esi, esi
        jnz short subtree_child
    subtree_done:
        pop esi
        ret
    }
}

} // namespace bsp
