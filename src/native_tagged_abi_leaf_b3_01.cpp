#include "bsp/native_tagged_abi_leaf_b3_01.hpp"

// Complete measured x87 load/return, no frame, extra memory access or FP reset.
extern "C" __declspec(naked) float __fastcall bsp_tagged_abi_b3_00b3ffa0(const void*) {
    __asm {
        fld dword ptr [ecx + 0x18]
        ret
    }
}

// Literal readable instructions retain the actual stack cells, scratch writes,
// SSE load/store order and RET cleanup, including aliases into the scratch or
// return/envelope cells. No incoming ECX/EDX value is consumed by this body.
extern "C" __declspec(naked) void* __stdcall bsp_tagged_abi_b3_00b3d5e0(
    void*, std::uint32_t, std::uint32_t, const void*) {
    __asm {
        sub esp, 0x10
        mov eax, dword ptr [esp + 0x20]
        movss xmm0, dword ptr [eax]
        movss dword ptr [esp], xmm0
        movss xmm0, dword ptr [eax + 0x04]
        mov ecx, dword ptr [esp]
        movss dword ptr [esp + 0x04], xmm0
        movss xmm0, dword ptr [eax + 0x08]
        mov edx, dword ptr [esp + 0x04]
        movss dword ptr [esp + 0x08], xmm0
        movss xmm0, dword ptr [eax + 0x0c]
        mov eax, dword ptr [esp + 0x14]
        mov dword ptr [eax], ecx
        mov ecx, dword ptr [esp + 0x08]
        mov dword ptr [eax + 0x04], edx
        movss dword ptr [esp + 0x0c], xmm0
        mov edx, dword ptr [esp + 0x0c]
        mov dword ptr [eax + 0x08], ecx
        mov dword ptr [eax + 0x0c], edx
        add esp, 0x10
        ret 0x10
    }
}
