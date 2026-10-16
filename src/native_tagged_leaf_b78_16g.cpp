#include "bsp/native_tagged_leaf_b78_16g.hpp"

#include <cstring>

static_assert(sizeof(std::uintptr_t) == sizeof(std::uint32_t));

extern "C" std::uint32_t __fastcall bsp_tagged_b78_16g_00b726a0(std::uint32_t address_bits) noexcept {
    return address_bits + 0x98u;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b78_16g_00b72b30(const void* storage) noexcept {
    std::uint32_t value;
    const auto address = reinterpret_cast<std::uintptr_t>(storage) + 0x50u;
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof value);
    return value;
}

extern "C" void __fastcall bsp_tagged_b78_16g_00b87080(void* destination, const void* source) noexcept {
    if (destination != nullptr) {
        std::uint32_t word;
        std::memcpy(&word, source, sizeof word);
        std::memcpy(destination, &word, sizeof word);
    }
}

// Preserve EAX=word0 and EDX=word1, including at a second-store fault. Read
// word1 only after store0, so partial source/destination overlap is admitted.
extern "C" __declspec(naked) void __fastcall bsp_tagged_b78_16g_00b87110(void*, const void*) noexcept {
    __asm {
        test ecx, ecx
        jz b87110_done
        mov eax, dword ptr [edx]
        mov dword ptr [ecx], eax
        mov edx, dword ptr [edx + 4]
        mov dword ptr [ecx + 4], edx
    b87110_done:
        ret
    }
}

// The source argument slot is read only after the destination null test.
extern "C" __declspec(naked) void* __stdcall bsp_tagged_b78_16g_00b875e0(void*, const void*) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4]
        test eax, eax
        jz b875e0_done
        mov ecx, dword ptr [esp + 8]
        mov edx, dword ptr [ecx]
        mov dword ptr [eax], edx
    b875e0_done:
        ret 8
    }
}

// Keep EDX=word0, ECX=word1, EAX=destination and the literal fault/load prefix.
extern "C" __declspec(naked) void* __stdcall bsp_tagged_b78_16g_00b87610(void*, const void*) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4]
        test eax, eax
        jz b87610_done
        mov ecx, dword ptr [esp + 8]
        mov edx, dword ptr [ecx]
        mov dword ptr [eax], edx
        mov ecx, dword ptr [ecx + 4]
        mov dword ptr [eax + 4], ecx
    b87610_done:
        ret 8
    }
}

// Keep EAX=entry ECX, ECX=0 and XOR flags BEFORE the first possibly faulting
// store, with the three four-byte stores in their actual order.
extern "C" __declspec(naked) void* __fastcall bsp_tagged_b78_16g_00b88b50(void*) noexcept {
    __asm {
        mov eax, ecx
        xor ecx, ecx
        mov dword ptr [eax + 4], ecx
        mov dword ptr [eax + 8], ecx
        mov dword ptr [eax + 0ch], ecx
        ret
    }
}
