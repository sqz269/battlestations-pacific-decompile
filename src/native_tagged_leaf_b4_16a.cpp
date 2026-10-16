#include "bsp/native_tagged_leaf_b4_16a.hpp"

#include <cstring>

// Literal instruction forms retain zero-result/flags before the first store,
// then the actual three stores and any fault prefix. No object type inferred.
#define BSP_B4_ZERO_THREE(address) \
extern "C" __declspec(naked) std::uint32_t __fastcall bsp_tagged_b4_##address(void*) { \
    __asm { xor eax, eax } \
    __asm { mov dword ptr [ecx], eax } \
    __asm { mov dword ptr [ecx + 4], eax } \
    __asm { mov dword ptr [ecx + 8], eax } \
    __asm { ret } \
}
BSP_B4_ZERO_THREE(00b401a0)
BSP_B4_ZERO_THREE(00b401b0)
BSP_B4_ZERO_THREE(00b47670)
#undef BSP_B4_ZERO_THREE

extern "C" void* __fastcall bsp_tagged_b4_00b403f0(void* destination, const void* source) {
    std::uint32_t word;
    std::memcpy(&word, static_cast<const unsigned char*>(source) + 4, sizeof word);
    std::memcpy(static_cast<unsigned char*>(destination) + 4, &word, sizeof word);
    return destination;
}

// Real two-input Source bindings replace only the Original stack destination
// fetch and RET4. Every x87 data instruction retains its Native memory order.
extern "C" __declspec(naked) void* __fastcall bsp_tagged_b4_00b40750(const void*, void*) {
    __asm {
        mov eax, edx
        fld dword ptr [ecx + 0x390]
        fstp dword ptr [eax]
        fld dword ptr [ecx + 0x394]
        fstp dword ptr [eax + 4]
        fld dword ptr [ecx + 0x398]
        fstp dword ptr [eax + 8]
        fld dword ptr [ecx + 0x39c]
        fstp dword ptr [eax + 12]
        ret
    }
}

extern "C" __declspec(naked) void* __fastcall bsp_tagged_b4_00b40780(const void*, void*) {
    __asm {
        mov eax, edx
        fld dword ptr [ecx + 0x38]
        fstp dword ptr [eax]
        fld dword ptr [ecx + 0x3c]
        fstp dword ptr [eax + 4]
        fld dword ptr [ecx + 0x40]
        fstp dword ptr [eax + 8]
        ret
    }
}

// ADD rather than DEC preserves the Original CF/AF and single RMW instruction.
#define BSP_B4_DECREMENT_WORD(address) \
extern "C" __declspec(naked) void __fastcall bsp_tagged_b4_##address(void*) { \
    __asm { add dword ptr [ecx + 4], -1 } \
    __asm { ret } \
}
BSP_B4_DECREMENT_WORD(00b40f10)
BSP_B4_DECREMENT_WORD(00b47ec0)
BSP_B4_DECREMENT_WORD(00b48040)
#undef BSP_B4_DECREMENT_WORD

extern "C" void __stdcall bsp_tagged_b4_00b44bf0(std::uint32_t) {}

#define BSP_B4_READ_WORD(address, offset) \
extern "C" std::uint32_t __fastcall bsp_tagged_b4_##address(const void* backing) { \
    std::uint32_t word; \
    std::memcpy(&word, static_cast<const unsigned char*>(backing) + offset, sizeof word); \
    return word; \
}
BSP_B4_READ_WORD(00b47660, 4)
BSP_B4_READ_WORD(00b47830, 4)
BSP_B4_READ_WORD(00b47840, 0)
BSP_B4_READ_WORD(00b47870, 4)
BSP_B4_READ_WORD(00b47880, 0)
BSP_B4_READ_WORD(00b478f0, 0x30)
#undef BSP_B4_READ_WORD
