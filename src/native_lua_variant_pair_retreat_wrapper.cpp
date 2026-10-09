#include "bsp/native_lua_variant_pair_retreat_wrapper.hpp"

#include "bsp/native_lua_variant_pair_retreat.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4);

// Keep the complete physical schedule. Do not replace this with an ordinary
// HLL return of a captured parameter: EAX reads the actual post-child ESI.
__declspec(naked) void* __fastcall retreat_native_lua_variant_pair_wrapper_006ede90(void*) {
    __asm {
        push esi                              // 006EDE90: current save backing
        mov esi, ecx                          // 006EDE91: capture incoming pair
        call retreat_native_lua_variant_pair_006edd80 // 006EDE93: actual admitted Source137
        mov eax, esi                          // 006EDE98: POST-child ESI
        pop esi                               // 006EDE9A: late current saved word
        ret                                   // 006EDE9B: current return backing
    }
}

} // namespace bsp
