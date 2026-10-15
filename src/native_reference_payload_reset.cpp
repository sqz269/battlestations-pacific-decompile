#include "bsp/native_reference_payload_reset.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {

// Whole Original34B/12 instructions SHA256:
// cebff76019d054f62e5664408c7f5094a557b977e8db9b049c0347fe3e260437
// Rebind only CALL operand [12,16) to the accepted current canonical free.
// The other 30 bytes retain the actual conditional clear and scalar reset.
__declspec(naked) void __fastcall reset_native_reference_payload_008f0380(
    void*) noexcept {
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 4]
        test eax, eax
        jz L_clear_scalar
        push eax
        call singleton_lifetime_free
        add esp, 4
        mov dword ptr [esi + 4], 0
    L_clear_scalar:
        mov dword ptr [esi], 0
        pop esi
        ret
    }
}

} // namespace bsp
