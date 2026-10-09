#include "bsp/native_invalid_parameter_noinfo_call.hpp"

// Use the actual selected Windows SDK declaration; do not redeclare the CRT API
// with the five physical words as fabricated semantic arguments.
#include <corecrt.h>

namespace bsp {

static_assert(sizeof(void*) == 4);

// Complete nine-op schedule. The Source import's encoding, relocation and size
// require compiled-object review; the Native call is a five-byte E8 rel32.
__declspec(naked) void __cdecl invoke_native_invalid_parameter_00bf6713() {
    __asm {
        xor eax, eax                         // 00BF6713
        push eax                             // 00BF6715
        push eax                             // 00BF6716
        push eax                             // 00BF6717
        push eax                             // 00BF6718
        push eax                             // 00BF6719
        call _invalid_parameter_noinfo       // 00BF671A: qualified Source policy
        add esp, 14h                         // 00BF671F
        ret                                  // 00BF6722
    }
}

} // namespace bsp
