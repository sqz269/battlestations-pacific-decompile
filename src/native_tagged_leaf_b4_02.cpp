#include "bsp/native_tagged_leaf_b4_02.hpp"

#include <cstring>

// See docs/GOAL15O_TAGGED_LEAF_B4_02.md for whole-body and OriginalABI evidence.
// Backed readable storage is required through the indicated four-byte span.
// memcpy models raw unaligned storage without inventing a typed object/field.
// Strict Win32 compiled sections preserve every native instruction and operand,
// including full EAX width, unchanged flags and plain RET with no stack cleanup.

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b401c0(const void* storage) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const unsigned char*>(storage) + 8, sizeof value);
    return value;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40300(const void* storage) noexcept {
    std::uint32_t value;
    std::memcpy(&value, storage, sizeof value);
    return value;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40310(const void* storage) noexcept {
    std::uint32_t value;
    std::memcpy(&value, storage, sizeof value);
    return value;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40490(std::uint32_t ecx_value) noexcept {
    return ecx_value;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40510(const void* storage) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const unsigned char*>(storage) + 8, sizeof value);
    return value;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40550(const void* storage) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const unsigned char*>(storage) + 0x50, sizeof value);
    return value;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40560(const void* storage) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const unsigned char*>(storage) + 0x1c, sizeof value);
    return value;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40570(const void* storage) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const unsigned char*>(storage) + 0x10, sizeof value);
    return value;
}
