#include "bsp/native_tagged_leaf_bcbd_16k.hpp"

#include <cstring>

static_assert(sizeof(std::uintptr_t) == sizeof(std::uint32_t));

// 00BC04B0: full EAX zero, with the measured XOR flags.
extern "C" std::uint32_t __cdecl bsp_tagged_bcbd_16k_00bc04b0() noexcept {
    return 0;
}

// Integer address arithmetic keeps DWORD address wrap without an invented
// receiver class, pointer extent, field alignment or backing-null guard.
extern "C" std::uint32_t __fastcall bsp_tagged_bcbd_16k_00bc3030(const void* storage) noexcept {
    std::uint32_t value;
    const auto address = reinterpret_cast<std::uintptr_t>(storage) + 0x14u;
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof value);
    return value;
}

extern "C" std::uint32_t __fastcall bsp_tagged_bcbd_16k_00bc3080(const void* storage) noexcept {
    std::uint32_t value;
    const auto address = reinterpret_cast<std::uintptr_t>(storage) + 0x68u;
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof value);
    return value;
}

extern "C" std::uint32_t __fastcall bsp_tagged_bcbd_16k_00bc3100(const void* storage) noexcept {
    std::uint32_t value;
    const auto address = reinterpret_cast<std::uintptr_t>(storage) + 0x20u;
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof value);
    return value;
}

extern "C" std::uint32_t __fastcall bsp_tagged_bcbd_16k_00bc7a60(const void* storage) noexcept {
    std::uint32_t value;
    const auto address = reinterpret_cast<std::uintptr_t>(storage) + 8u;
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof value);
    return value;
}

// 00BD1830: the byte return permits MOV AL,1 without broadening to EAX=1.
// Complete emitted-body verification retains incoming upper EAX and flags.
extern "C" std::uint8_t __cdecl bsp_tagged_bcbd_16k_00bd1830() noexcept {
    return 1;
}
