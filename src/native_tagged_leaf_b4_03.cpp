#include "bsp/native_tagged_leaf_b4_03.hpp"

#include <cstring>

// Reads require real backing storage through their complete byte span.
// No fields or class receivers are inferred. Identity preserves all uint32 bits.

// Semantic Source: returns the backed address and clears its dword at +0x10.
// Native copies ECX into EAX before its store; this compiler does so afterward.
extern "C" void* __fastcall bsp_tagged_b4_00b40120(void* storage) noexcept {
    const std::uint32_t zero = 0;
    std::memcpy(static_cast<unsigned char*>(storage) + 0x10, &zero, sizeof zero);
    return storage;
}

// Semantic Source: independent raw dword writes at 0, 4, 8, then EAX=0.
// Requires twelve writable bytes; no typed object/alignment narrowing is added.
extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40180(void* storage) noexcept {
    const std::uint32_t zero = 0;
    auto* bytes = static_cast<unsigned char*>(storage);
    std::memcpy(bytes, &zero, sizeof zero);
    std::memcpy(bytes + 4, &zero, sizeof zero);
    std::memcpy(bytes + 8, &zero, sizeof zero);
    return zero;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40190(void* storage) noexcept {
    const std::uint32_t zero = 0;
    auto* bytes = static_cast<unsigned char*>(storage);
    std::memcpy(bytes, &zero, sizeof zero);
    std::memcpy(bytes + 4, &zero, sizeof zero);
    std::memcpy(bytes + 8, &zero, sizeof zero);
    return zero;
}

// Semantic Source: unsigned wrap is defined for every pair of stored bit patterns.
// Emitted ADD changes flags and EDX/ECX effects differ; no Native ABI claim.
extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40e80(const void* storage) noexcept {
    std::uint32_t second;
    std::uint32_t first;
    std::memcpy(&second, static_cast<const unsigned char*>(storage) + 4, sizeof second);
    std::memcpy(&first, storage, sizeof first);
    return first + second * 4u - 4u;
}

extern "C" std::uint32_t __cdecl bsp_tagged_b4_00b40150() noexcept {
    return 14u;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40160(const void* storage) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const unsigned char*>(storage) + 4, sizeof value);
    return value;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40170(const void* storage) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const unsigned char*>(storage) + 4, sizeof value);
    return value;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40580(const void* storage) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const unsigned char*>(storage) + 0x50, sizeof value);
    return value != 0u;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40740(const void* storage) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const unsigned char*>(storage) + 0x188, sizeof value);
    return value;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40850(std::uint32_t ecx_value) noexcept {
    return ecx_value;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b40970(const void* storage) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const unsigned char*>(storage) + 0xc, sizeof value);
    return value;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b41030(const void* storage) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const unsigned char*>(storage) + 0x20, sizeof value);
    return value;
}
