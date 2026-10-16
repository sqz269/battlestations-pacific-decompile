#include "bsp/native_tagged_x87_leaf_b4_01.hpp"

// Literal measured instructions, not an encoded byte array. Ordinary strict
// float-return C++ forms inserted stack-mediated stores/reloads (see report).
// There is no compiler prologue, epilogue, arithmetic, provider or fake input.
extern "C" __declspec(naked) float __fastcall bsp_tagged_x87_b4_00b40040(const void*) {
    __asm {
        // 00B40040: D9 81 F8 01 00 00
        fld dword ptr [ecx + 0x1f8]
        // 00B40046: C3
        ret
    }
}
