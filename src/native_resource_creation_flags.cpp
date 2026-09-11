#include "bsp/native_resource_creation_flags.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource creation flags require MSVC Win32.
#endif

namespace bsp {
namespace {

// memcpy supports actual unaligned and partially overlapping output cells;
// each call copies exactly one independent DWORD, with no output read.
void store_word(void* destination, std::uint32_t value) noexcept {
    std::memcpy(destination, &value, sizeof(value));
}

} // namespace

void translate_native_resource_creation_flags_00b20a80(
    void* usage_output, void* pool_output,
    std::uint32_t flags, std::uint32_t kind) noexcept {
    const auto pool = flags & 0x0fu;
    if (pool <= 3) {
        store_word(pool_output, pool);
        if (pool == 0 && (kind == 6 || kind == 7)) flags |= 0x10000u;
    }
    std::uint32_t usage = (flags & 0x10u) != 0 ? 1u : 0u;
    switch (flags & 0x0f00u) {
    case 0x0100: usage |= 0x0002; break;
    case 0x0200: usage |= 0x4000; break;
    case 0x0300: usage |= 0x0040; break;
    case 0x0400: usage |= 0x0100; break;
    case 0x0500: usage |= 0x0080; break;
    }
    if ((flags & 0x0f000u) == 0x01000u) usage |= 0x200u;
    if ((kind == 6 || kind == 7) && (flags & 0x0f0000u) == 0x010000u) usage |= 8u;
    if ((kind == 3 || kind == 5) && (flags & 0xff000000u) == 0x01000000u) usage |= 0x400u;
    store_word(usage_output, usage);
}

} // namespace bsp
