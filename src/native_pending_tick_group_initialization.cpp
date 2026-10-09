#include "bsp/native_pending_tick_group_initialization.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4, "Pending group link words require Win32 pointers.");

// Exactly twenty ordered DWORD writes. Extra LEAs relocate the Native immediate
// head/tail words to the actual borrowed base without changing the XOR flags.
// There is no Source owner, production caller, executable C++ wrapper or local EH.
__declspec(naked) void __fastcall initialize_native_pending_tick_group_sentinels_00cd27c0(
    void*) {
    __asm {
        xor eax, eax                    // Native 00CD27C0

        mov dword ptr [ecx+04h], eax     // Native 00CD27C2: group 0 head+4
        lea edx, [ecx+34h]              // Source actual group 0 tail
        mov dword ptr [ecx+08h], edx     // Native 00CD27C7: group 0 head+8
        mov dword ptr [ecx+3ch], eax     // Native 00CD27D1: group 0 tail+8
        lea edx, [ecx]                  // Source actual group 0 head
        mov dword ptr [ecx+38h], edx     // Native 00CD27D6: group 0 tail+4

        mov dword ptr [ecx+6ch], eax     // Native 00CD27E0: group 1 head+4
        lea edx, [ecx+9ch]              // Source actual group 1 tail
        mov dword ptr [ecx+70h], edx     // Native 00CD27E5: group 1 head+8
        mov dword ptr [ecx+0a4h], eax    // Native 00CD27EF: group 1 tail+8
        lea edx, [ecx+68h]              // Source actual group 1 head
        mov dword ptr [ecx+0a0h], edx    // Native 00CD27F4: group 1 tail+4

        mov dword ptr [ecx+0d4h], eax    // Native 00CD27FE: group 2 head+4
        lea edx, [ecx+104h]             // Source actual group 2 tail
        mov dword ptr [ecx+0d8h], edx    // Native 00CD2803: group 2 head+8
        mov dword ptr [ecx+10ch], eax    // Native 00CD280D: group 2 tail+8
        lea edx, [ecx+0d0h]             // Source actual group 2 head
        mov dword ptr [ecx+108h], edx    // Native 00CD2812: group 2 tail+4

        mov dword ptr [ecx+13ch], eax    // Native 00CD281C: group 3 head+4
        lea edx, [ecx+16ch]             // Source actual group 3 tail
        mov dword ptr [ecx+140h], edx    // Native 00CD2821: group 3 head+8
        mov dword ptr [ecx+174h], eax    // Native 00CD282B: group 3 tail+8
        lea edx, [ecx+138h]             // Source actual group 3 head
        mov dword ptr [ecx+170h], edx    // Native 00CD2830: group 3 tail+4

        mov dword ptr [ecx+1a4h], eax    // Native 00CD283A: group 4 head+4
        lea edx, [ecx+1d4h]             // Source actual group 4 tail
        mov dword ptr [ecx+1a8h], edx    // Native 00CD283F: group 4 head+8
        mov dword ptr [ecx+1dch], eax    // Native 00CD2849: group 4 tail+8
        lea edx, [ecx+1a0h]             // Source actual group 4 head
        mov dword ptr [ecx+1d8h], edx    // Native 00CD284E: group 4 tail+4

        ret                             // Native 00CD2858
    }
}

} // namespace bsp
