#include "bsp/native_tagged_leaf_b9bb_16i.hpp"
#include <cstring>

namespace bsp {
static_assert(sizeof(std::uintptr_t) == 4);
extern "C" {

void __cdecl bsp_tagged_b9bb_00b94b20() {}
__declspec(naked) void __stdcall bsp_tagged_b9bb_00b96e50(std::uint32_t) {
    __asm { ret 4 }
}
__declspec(naked) void __stdcall bsp_tagged_b9bb_00b9a1c0(std::uint32_t) {
    __asm { ret 4 }
}
__declspec(naked) void __stdcall bsp_tagged_b9bb_00bb8ff0(
    std::uint32_t, std::uint32_t, std::uint32_t) {
    __asm { ret 0x0c }
}
void __cdecl bsp_tagged_b9bb_00bbb630() {}
__declspec(naked) std::uint8_t __stdcall bsp_tagged_b9bb_00bbb620(
    std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t) {
    __asm {
        xor al, al
        ret 0x10
    }
}

std::uint32_t __fastcall bsp_tagged_b9bb_00b9a3d0(const void* backing) {
    std::uint32_t value;
    const auto address = reinterpret_cast<std::uintptr_t>(backing) + 0xbcu;
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof(value));
    return value;
}
std::uint32_t __fastcall bsp_tagged_b9bb_00b9a4b0(const void* backing) {
    std::uint32_t value;
    const auto address = reinterpret_cast<std::uintptr_t>(backing) + 0x14u;
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof(value));
    return value;
}
std::uint8_t __fastcall bsp_tagged_b9bb_00b9a500(const void* backing) {
    std::uint8_t value;
    const auto address = reinterpret_cast<std::uintptr_t>(backing) + 0x10u;
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof(value));
    return value;
}
std::uint32_t __fastcall bsp_tagged_b9bb_00ba7170(const void* backing) {
    std::uint32_t value;
    const auto address = reinterpret_cast<std::uintptr_t>(backing) + 0x08u;
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof(value));
    return value;
}
std::uint32_t __fastcall bsp_tagged_b9bb_00ba7180(const void* backing) {
    std::uint32_t value;
    const auto address = reinterpret_cast<std::uintptr_t>(backing) + 0x30u;
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof(value));
    return value;
}
std::uint32_t __fastcall bsp_tagged_b9bb_00ba7190(const void* backing) {
    std::uint32_t value;
    const auto address = reinterpret_cast<std::uintptr_t>(backing) + 0x44u;
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof(value));
    return value;
}
std::uint32_t __fastcall bsp_tagged_b9bb_00bbce40(const void* backing) {
    std::uint32_t value;
    const auto address = reinterpret_cast<std::uintptr_t>(backing) + 0x94u;
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof(value));
    return value;
}
std::uint32_t __fastcall bsp_tagged_b9bb_00bbce50(const void* backing) {
    std::uint32_t value;
    const auto address = reinterpret_cast<std::uintptr_t>(backing) + 0x08u;
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof(value));
    return value;
}
std::uint8_t __fastcall bsp_tagged_b9bb_00bbce70(const void* backing) {
    std::uint8_t value;
    const auto address = reinterpret_cast<std::uintptr_t>(backing) + 0x99u;
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof(value));
    return value;
}
}
} // namespace bsp
