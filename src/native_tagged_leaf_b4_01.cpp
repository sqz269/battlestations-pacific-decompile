#include "bsp/native_tagged_leaf_b4_01.hpp"

#include <cstring>

// OriginalABI at every entry: ECX is the only consumed input; EAX is the
// complete 32-bit result; a plain RET leaves argument cleanup to the caller.
// The LEA operations have modulo-2^32 semantics for every input bit pattern.
// Reads require real readable storage through the specified four-byte span;
// memcpy expresses the raw load without assigning a fictional object type.

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40000(std::uint32_t ecx_value) noexcept {
    return ecx_value + 0x1b4u;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40010(std::uint32_t ecx_value) noexcept {
    return ecx_value + 0x184u;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40020(std::uint32_t ecx_value) noexcept {
    return ecx_value + 0x194u;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40030(std::uint32_t ecx_value) noexcept {
    return ecx_value + 0x1ecu;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40070(std::uint32_t ecx_value) noexcept {
    return ecx_value + 8u;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40080(const void* storage) noexcept {
    std::uint32_t value;
    std::memcpy(&value, storage, sizeof value);
    return value;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b400a0(const void* storage) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const unsigned char*>(storage) + 4, sizeof value);
    return value;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b400d0(std::uint32_t ecx_value) noexcept {
    return ecx_value;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b400e0(const void* storage) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const unsigned char*>(storage) + 4, sizeof value);
    return value;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40100(const void* storage) noexcept {
    std::uint32_t value;
    std::memcpy(&value, storage, sizeof value);
    return value;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40130(const void* storage) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const unsigned char*>(storage) + 0x10, sizeof value);
    return value;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40140(std::uint32_t ecx_value) noexcept {
    return ecx_value;
}
