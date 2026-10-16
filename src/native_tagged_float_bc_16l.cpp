#include "bsp/native_tagged_float_bc_16l.hpp"

// Literal FLD retains inherited stack/status/control, SNaN/denormal handling
// and the first-instruction fault prefix. A language-level float load or a
// bits-to-float helper is not substituted for this physical x87 operation.
extern "C" __declspec(naked) float __fastcall bsp_tagged_float_bc_16l_00bc3010(const void*) {
    __asm {
        fld dword ptr [ecx + 0x2c]
        ret
    }
}

extern "C" __declspec(naked) float __fastcall bsp_tagged_float_bc_16l_00bc7aa0(const void*) {
    __asm {
        fld dword ptr [ecx + 0x34]
        ret
    }
}

extern "C" __declspec(naked) float __fastcall bsp_tagged_float_bc_16l_00bc7ab0(const void*) {
    __asm {
        fld dword ptr [ecx + 0x1c]
        ret
    }
}
